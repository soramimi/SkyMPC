
#ifndef __WebClient_h
#define __WebClient_h

#include <vector>
#include <string>

#define USE_OPENSSL 0

class WebContext;
class WebClient;

class URI {
private:
	std::string scheme_;
	std::string host_;
	int port_;
	std::string path_;
public:
	URI(char const *str);
	std::string const &scheme() const { return scheme_; }
	std::string const &host() const { return host_; }
	int port() const { return port_; }
	std::string const &path() const { return path_; }
	bool isssl() const;
};

class WebClientHandler {
protected:
	void abort(std::string const &message = std::string());
public:
	virtual ~WebClientHandler()
	{
	}
	virtual void checkHeader(WebClient * /*wc*/)
	{
	}
	virtual void checkContent(char const * /*ptr*/, size_t /*len*/)
	{
	}
};

class WebClient {
public:
	class Error {
	private:
		std::string msg_;
	public:
		Error()
		{
		}
		Error(std::string const &message)
			: msg_(message)
		{
		}
		virtual ~Error()
		{
		}
		std::string message() const
		{
			return msg_;
		}
	};
	struct Response {
		unsigned int code;
		struct {
			unsigned int hi, lo;
		} version;
		std::vector<std::string> header;
		std::vector<char> content;
	};
	struct Post {
		std::vector<char> data;
	};
private:
	struct {
		std::vector<std::string> request_headers;
		Error error;
		Response response;
		WebContext *webcx;
		int crlf_state;
		size_t content_offset;
	} data;
	void clear_error();
	static int get_port(URI const *uri, char const *scheme, char const *protocol);
	void set_default_headers(URI const &uri, Post const *post);
	std::string make_http_request(URI const &uri, Post const *post);
	void parse_http_header(char const *begin, char const *end, std::vector<std::string> *header);
	void parse_http_header(char const *begin, char const *end, Response *out);
	bool http_get(URI const &uri, Post const *post, std::vector<char> *out, WebClientHandler *handler);
#if USE_OPENSSL
	bool https_get(URI const &uri, Post const *post, std::vector<char> *out, WebClientHandler *handler);
#endif
	void get(URI const &uri, Post const *post, Response *out, WebClientHandler *handler);
	static void parse_header(std::vector<std::string> const *header, WebClient::Response *res);
	static std::string header_value(std::vector<std::string> const *header, const std::string &name);
	void append(const char *ptr, size_t len, std::vector<char> *out, WebClientHandler *handler);
	void on_end_header(const std::vector<char> *vec, WebClientHandler *handler);
public:
	static void initialize();
	WebClient(WebContext *webcx)
	{
		data.webcx = webcx;
	}
	~WebClient()
	{
	}
	Error const &error() const;
	int get(URI const &uri, WebClientHandler *handler = 0);
	int post(URI const &uri, Post const *post, WebClientHandler *handler = 0);
	void add_header(std::string const &text);
	Response const &response() const;
	std::string header_value(std::string const &name) const;
	std::string content_type() const;
	size_t content_length() const;
	const char *content_data() const;
};

class WebContext {
	friend class WebClient;
private:
	struct Private;
	Private *priv;
	WebContext(WebContext const &r);
	void operator = (WebContext const &r);
public:
	WebContext();
	~WebContext();

#if USE_OPENSSL
	bool load_crt(char const *path);
#endif
};


#endif
