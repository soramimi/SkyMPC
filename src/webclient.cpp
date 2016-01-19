
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#if USE_OPENSSL
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif
typedef SOCKET socket_t;
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#define closesocket(S) close(S)
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

#include "webclient.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/types.h>

#if USE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

#pragma warning(disable:4996)

#define USER_AGENT "Generic Web Client"

struct WebContext::Private {
#if USE_OPENSSL
	SSL_CTX *ctx;
#endif
};

WebClient::URL::URL(char const *str)
{
	char const *left;
	char const *right;
	left = str;
	right = strstr(left, "://");
	if (right) {
		scheme_.assign(str, right - str);
		left = right + 3;
	}
	right = strchr(left, '/');
	if (right) {
		char const *p = strchr(left, ':');
		if (p && left < p && p < right) {
			int n = 0;
			char const *q = p + 1;
			while (q < right) {
				if (isdigit(*q & 0xff)) {
					n = n * 10 + (*q - '0');
				} else {
					n = -1;
					break;
				}
				q++;
			}
			host_.assign(left, p - left);
			if (n > 0 && n < 65536) {
				port_ = n;
			}
		} else {
			host_.assign(left, right - left);
		}
		path_ = right;
	}
}

bool WebClient::URL::isssl() const
{
	if (scheme() == "https") return true;
	if (scheme() == "http") return false;
	if (port() == 443) return true;
	return false;
}


void WebClientHandler::abort(const std::string &message)
{
	throw WebClient::Error(message);
}

static void cleanup()
{
#if USE_OPENSSL
	ERR_free_strings();
#endif
#ifdef WIN32
	WSACleanup();
#endif
}

void WebClient::initialize()
{
#ifdef WIN32
	WSADATA wsaData;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsaData);
	atexit(cleanup);
#endif
#if USE_OPENSSL
	OpenSSL_add_all_algorithms();
#endif
}

WebClient::Error const &WebClient::error() const
{
	return data.error;
}

void WebClient::clear_error()
{
	data.error = Error();
}

#if USE_OPENSSL
static std::string get_ssl_error()
{
	char tmp[1000];
	unsigned long e = ERR_get_error();
	ERR_error_string_n(e, tmp, sizeof(tmp));
	return tmp;
}
#endif

int WebClient::get_port(URL const *uri, char const *scheme, char const *protocol)
{
	int port = uri->port();
	if (port < 1 || port > 65535) {
		struct servent *s;
		s = getservbyname(uri->scheme().c_str(), protocol);
		if (s) {
			port = ntohs(s->s_port);
		} else {
			s = getservbyname(scheme, protocol);
			if (s) {
				port = ntohs(s->s_port);
			}
		}
		if (port < 1 || port > 65535) {
			port = 80;
		}
	}
	return port;
}

static inline std::string to_s(size_t n)
{
	char tmp[100];
	sprintf(tmp, "%u", n);
	return tmp;
}

void WebClient::set_default_headers(URL const &uri, Post const *post)
{
	add_header("Host: " + uri.host());
	add_header("User-Agent: " USER_AGENT);
	add_header("Accept: */*");
	add_header("Connection: close");
	if (post) {
		add_header("Content-Length: " + to_s(post->data.size()));
		add_header("Content-Type: application/x-www-form-urlencoded");
	}
}

std::string WebClient::make_http_request(URL const &uri, Post const *post)
{
	std::string str;

	str = post ? "POST " : "GET ";
	str += uri.path();
	str += " HTTP/1.0";
	str += "\r\n";

	for (std::vector<std::string>::const_iterator it = data.request_headers.begin(); it != data.request_headers.end(); it++) {
		str += *it;
		str += "\r\n";
	}

	str += "\r\n";
	return str;
}

void WebClient::parse_http_header(char const *begin, char const *end, std::vector<std::string> *header)
{
	if (begin < end) {
		char const *left = begin;
		char const *right = left;
		while (1) {
			if (right >= end) {
				break;
			}
			if (*right == '\r' || *right == '\n') {
				if (left < right) {
					header->push_back(std::string(left, right));
				}
				if (right + 1 < end && *right == '\r' && right[1] == '\n') {
					right++;
				}
				right++;
				if (*right == '\r' || *right == '\n') {
					if (right + 1 < end && *right == '\r' && right[1] == '\n') {
						right++;
					}
					right++;
					left = right;
					break;
				}
				left = right;
			} else {
				right++;
			}
		}
	}
}

void WebClient::parse_http_header(const char *begin, const char *end, WebClient::Response *out)
{
	*out = Response();
	parse_http_header(begin, end, &out->header);
	parse_header(&out->header, out);
}

static void send_(socket_t s, char const *ptr, int len)
{
	while (len > 0) {
		int n = send(s, ptr, len, 0);
		if (n < 1 || n > len) {
			throw WebClient::Error("send request failed.");
		}
		ptr += n;
		len -= n;
	}
}

void WebClient::on_end_header(std::vector<char> const *vec, WebClientHandler *handler)
{
	if (vec->empty()) return;
	char const *begin = &vec->at(0);
	char const *end = begin + vec->size();
	parse_http_header(begin, end, &data.response);
	if (handler) {
		handler->checkHeader(this);
	}
}

void WebClient::append(char const *ptr, size_t len, std::vector<char> *out, WebClientHandler *handler)
{
	size_t offset = out->size();
	out->insert(out->end(), ptr, ptr + len);

	if (data.crlf_state < 0) {
		// nop
	} else {
		for (size_t i = 0; i < len; i++) {
			int c = ptr[i] & 0xff;
			if (c == '\r') {
				data.crlf_state |= 1;
			} else if (c == '\n') {
				data.crlf_state |= 1;
				data.crlf_state++;
			} else {
				data.crlf_state = 0;
			}
			if (data.crlf_state == 4) {
				data.content_offset = offset + i + 1;
				on_end_header(out, handler);
				data.crlf_state = -1;
				break;
			}
		}
	}
	if (handler && data.content_offset > 0) {
		offset = out->size();
		if (offset > data.content_offset) {
			size_t len = offset - data.content_offset;
			char const *ptr = &out->at(data.content_offset);
			handler->checkContent(ptr, len);
		}
	}
}

#if USE_OPENSSL
#else
typedef void SSL;
#endif

class AutoClose {
private:
	socket_t sock;
	SSL *ssl;
public:
	AutoClose(socket_t sock, SSL *ssl = 0)
		: sock(sock)
		, ssl(ssl)
	{
	}
	~AutoClose()
	{
#if USE_OPENSSL
		SSL_shutdown(ssl);
		closesocket(sock);
		SSL_free(ssl);
#else
		closesocket(sock);
#endif
	}
};

bool WebClient::http_get(URL const &uri, Post const *post, std::vector<char> *out, WebClientHandler *handler)
{
	clear_error();
	out->clear();

	socket_t s;
	struct hostent *servhost;
	struct sockaddr_in server;

	servhost = gethostbyname(uri.host().c_str());
	if (!servhost) {
		throw Error("gethostbyname failed.");
	}

	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;

	memcpy((char *)&server.sin_addr, servhost->h_addr, servhost->h_length);

	server.sin_port = htons(get_port(&uri, "http", "tcp"));

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		throw Error("socket failed.");
	}

	if (connect(s, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR) {
		throw Error("connect failed.");
	}

	AutoClose autoclose(s);

	set_default_headers(uri, post);

	std::string request = make_http_request(uri, post);

	send_(s, request.c_str(), (int)request.size());
	if (post && !post->data.empty()) {
		send_(s, (char const *)&post->data[0], (int)post->data.size());
	}

	data.crlf_state = 0;
	data.content_offset = 0;

	while (1) {
		char buf[4096];
		int n = recv(s, buf, sizeof(buf), 0);
		if (n < 1) break;
		append(buf, n, out, handler);
	}

	return true;
}

#if USE_OPENSSL
static void ssend_(SSL *ssl, char const *ptr, int len)
{
	while (len > 0) {
		int n = SSL_write(ssl, ptr, len);
		if (n < 1 || n > len) {
			throw WebClient::Error(get_ssl_error());
		}
		ptr += n;
		len -= n;
	}
}

void get_strings(X509_NAME *x509name, std::vector<std::string> *out)
{
	out->clear();
	if (x509name) {
		int n = X509_NAME_entry_count(x509name);
		for (int i = 0; i < n; i++) {
			X509_NAME_ENTRY *entry = X509_NAME_get_entry(x509name, i);
			ASN1_STRING *asn1str = X509_NAME_ENTRY_get_data(entry);
			int asn1len = ASN1_STRING_length(asn1str);
			unsigned char *p = ASN1_STRING_data(asn1str);
			std::string str((char const *)p, asn1len);
			out->push_back(str);
		}
	}
}
#endif

#if USE_OPENSSL
void output_debug_strings(std::vector<std::string> const *vec)
{
	for (std::vector<std::string>::const_iterator it = vec->begin(); it != vec->end(); it++) {
		std::string s = *it;
		s += '\n';
		OutputDebugStringA(s.c_str());
	}
}

bool WebClient::https_get(URI const &uri, Post const *post, std::vector<char> *out, WebClientHandler *handler)
{
#define sslctx() (data.webcx->pv->ctx)

	if (!data.webcx || !data.webcx->pv->ctx) {
		OutputDebugString("SSL context is null.\n");
		return false;
	}

	clear_error();
	out->clear();

	int ret;
	socket_t s;
	struct hostent *servhost;
	struct sockaddr_in server;

	SSL *ssl;

	servhost = gethostbyname(uri.host().c_str());
	if (!servhost) {
		throw Error("gethostbyname failed.");
	}

	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;

	memcpy((char *)&server.sin_addr, servhost->h_addr, servhost->h_length);

	server.sin_port = htons(get_port(&uri, "https", "tcp"));

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		throw Error("socket failed.");
	}

	if (connect(s, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR) {
		throw Error("connect failed.");
	}

	ssl = SSL_new(sslctx());
	if (!ssl) {
		throw Error(get_ssl_error());
	}

	SSL_set_options(ssl, SSL_OP_NO_SSLv2);
	SSL_set_options(ssl, SSL_OP_NO_SSLv3);

	ret = SSL_set_fd(ssl, s);
	if (ret == 0) {
		throw Error(get_ssl_error());
	}

	RAND_poll();
	while (RAND_status() == 0) {
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}

	ret = SSL_connect(ssl);
	if (ret != 1) {
		throw Error(get_ssl_error());
	}

	AutoClose autoclose(s, ssl);

	std::string cipher = SSL_get_cipher(ssl);
	cipher += '\n';
	OutputDebugString(cipher.c_str());

	std::string version = SSL_get_cipher_version(ssl);
	version += '\n';
	OutputDebugString(version.c_str());

	X509 *x509 = SSL_get_peer_certificate(ssl);
	if (x509) {
		std::string fingerprint;
		for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
			if (i > 0) {
				fingerprint += ':';
			}
			char tmp[10];
			sprintf(tmp, "%02X", x509->sha1_hash[i]);
			fingerprint += tmp;
		}
		fingerprint += '\n';
		OutputDebugString(fingerprint.c_str());


		long l = SSL_get_verify_result(ssl);
		if (l == X509_V_OK) {
			// ok
		} else {
			// wrong
			std::string err = X509_verify_cert_error_string(l);
			err += '\n';
			OutputDebugString(err.c_str());
		}

		std::vector<std::string> vec;

		X509_NAME *subject = X509_get_subject_name(x509);
		get_strings(subject, &vec);
		OutputDebugString("--- subject ---\n");
		output_debug_strings(&vec);

		X509_NAME *issuer = X509_get_issuer_name(x509);
		get_strings(issuer, &vec);
		OutputDebugString("--- issuer ---\n");
		output_debug_strings(&vec);

		ASN1_TIME *not_before = X509_get_notBefore(x509);
		ASN1_TIME *not_after  = X509_get_notAfter(x509);

		X509_free(x509);
	} else {
		// wrong
	}

	set_default_headers(uri, post);

	std::string request = make_http_request(uri, post);

	ssend_(ssl, request.c_str(), (int)request.size());
	if (post && !post->data.empty()) {
		ssend_(ssl, (char const *)&post->data[0], (int)post->data.size());
	}

	data.crlf_state = 0;
	data.content_offset = 0;

	while (1) {
		char buf[4096];
		int n = SSL_read(ssl, buf, sizeof(buf));
		if (n < 1) break;
		append(buf, n, out, handler);
	}

	return true;
#undef sslctx
}
#endif

void WebClient::get(URL const &uri, Post const *post, Response *out, WebClientHandler *handler)
{
	*out = Response();
	try {
		std::vector<char> res;
		if (uri.isssl()) {
#if USE_OPENSSL
			https_get(uri, post, &res, handler);
#endif
		} else {
			http_get(uri, post, &res, handler);
		}
		if (!res.empty()) {
			char const *begin = &res[0];
			char const *end = begin + res.size();
			char const *ptr = begin + data.content_offset;
			if (ptr < end) {
				out->content.assign(ptr, end);
			}
		}
		return;
	} catch (Error const &e) {
		if (handler) {
			data.error = e;
		}
	}
	*out = Response();
}

void WebClient::parse_header(std::vector<std::string> const *header, WebClient::Response *res)
{
	if (header->size() > 0) {
		std::string const &line = header->at(0);
		char const *begin = line.c_str();
		char const *end = begin + line.size();
		if (line.size() > 5 && strncmp(line.c_str(), "HTTP/", 5) == 0) {
			int state = 0;
			res->version.hi = res->version.lo = res->code = 0;
			char const *ptr = begin + 5;
			while (1) {
				int c = 0;
				if (ptr < end) {
					c = *ptr & 0xff;
				}
				switch (state) {
				case 0:
					if (isdigit(c)) {
						res->version.hi = res->version.hi * 10 + (c - '0');
					} else if (c == '.') {
						state = 1;
					} else {
						state = -1;
					}
					break;
				case 1:
					if (isdigit(c)) {
						res->version.lo = res->version.lo * 10 + (c - '0');
					} else if (isspace(c)) {
						state = 2;
					} else {
						state = -1;
					}
					break;
				case 2:
					if (isspace(c)) {
						if (res->code != 0) {
							state = -1;
						}
					} else if (isdigit(c)) {
						res->code = res->code * 10 + (c - '0');
					} else {
						state = -1;
					}
					break;
				default:
					state = -1;
					break;
				}
				if (c == 0 || state < 0) {
					break;
				}
				ptr++;
			}
		}
	}
}

std::string WebClient::header_value(std::vector<std::string> const *header, std::string const &name)
{
	for (size_t i = 1; i < header->size(); i++) {
		std::string const &line = header->at(i);
		char const *begin = line.c_str();
		char const *end = begin + line.size();
		char const *colon = strchr(begin, ':');
		if (colon) {
#ifndef WIN32
#define strnicmp(A, B, C) strncasecmp(A, B, C)
#endif
			if (strnicmp(begin, name.c_str(), name.size()) == 0) {
				char const *ptr = colon + 1;
				while (ptr < end && isspace(*ptr & 0xff)) ptr++;
				return std::string(ptr, end);
			}
		}
	}
	return std::string();
}

std::string WebClient::header_value(std::string const &name) const
{
	return header_value(&data.response.header, name);
}

std::string WebClient::content_type() const
{
	std::string s = header_value("Content-Type");
	char const *begin = s.c_str();
	char const *end = begin + s.size();
	char const *ptr = begin;
	while (ptr < end) {
		int c = *ptr & 0xff;
		if (c == ';' || c < 0x21) break;
		ptr++;
	}
	if (ptr < end) return std::string(begin, ptr);
	return s;
}

size_t WebClient::content_length() const
{
	return data.response.content.size();
}

char const *WebClient::content_data() const
{
	if (data.response.content.empty()) return "";
	return &data.response.content[0];
}

int WebClient::get(URL const &uri, WebClientHandler *handler)
{
	get(uri, 0, &data.response, handler);
	return data.response.code;
}

int WebClient::post(URL const &uri, Post const *post, WebClientHandler *handler)
{
	get(uri, post, &data.response, handler);
	return data.response.code;
}

void WebClient::add_header(std::string const &text)
{
	data.request_headers.push_back(text);
}

WebClient::Response const *WebClient::response() const
{
	return &data.response;
}

//

WebContext::WebContext()
{
	pv = new Private();
#if USE_OPENSSL
	SSL_load_error_strings();
	SSL_library_init();
	pv->ctx = SSL_CTX_new(SSLv23_client_method());
#endif
}

WebContext::~WebContext()
{
#if USE_OPENSSL
	SSL_CTX_free(pv->ctx);
#endif
	delete pv;
}

#if USE_OPENSSL
bool WebContext::load_crt(char const *path)
{
	// path = "C:\\develop\\httpsget\\ca-bundle.crt";
	int r = SSL_CTX_load_verify_locations(pv->ctx, path, 0);
	return r == 1;
}
#endif

