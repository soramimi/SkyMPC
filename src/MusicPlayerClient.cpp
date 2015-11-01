#include "MusicPlayerClient.h"


MusicPlayerClient::MusicPlayerClient()
{
}

MusicPlayerClient::~MusicPlayerClient()
{
	close();
}

bool MusicPlayerClient::recv(QTcpSocket *sock, std::vector<QString> *lines)
{
	int timeout = 5000;
	while (sock->waitForReadyRead(timeout)) {
		while (sock->canReadLine()) {
			QByteArray ba = sock->readLine();
			QString s;
			int n = ba.size();
			if (n > 0) {
				char const *p = ba.data();
				if (n > 0 && p[n - 1] == '\n') {
					n--;
				}
				if (n > 0) {
					s = QString::fromUtf8(p, n);
				}
			}
			if (s == "OK") {
				return true;
			}
			lines->push_back(s);
			if (s.startsWith("ACK")) {
				ushort const *p = s.utf16();
				p = ucschr(p, '}');
				if (p) {
					do {
						p++;
					} while (*p && iswspace(*p));
					s = QString::fromUtf16((ushort const *)p);
				}
				return false;
			}
		}
		timeout = 100;
	}
	return false;
}

bool MusicPlayerClient::exec(QString const &command, std::vector<QString> *lines)
{
	lines->clear();
	exception = QString();

	if (sock.waitForReadyRead(0)) {
		sock.readAll();
	}

	QByteArray ba = (command + '\n').toUtf8();
	sock.write(ba.data(), ba.size());

	return recv(&sock, lines);
}

MusicPlayerClient::OpenResult MusicPlayerClient::open(QTcpSocket *sock, Host const &host)
{
	OpenResult result;
	try {
		if (!host.isValid()) {
			throw QString("Specified host is not valid.");
		}
		sock->connectToHost(host.address(), host.port(DEFAULT_MPD_PORT));
		if (!sock->waitForConnected(10000)) throw sock->errorString();
		if (!sock->waitForReadyRead(10000))  throw sock->errorString();
		if (sock->canReadLine()) {
			QByteArray ba = sock->readLine();
			if (!ba.isEmpty()) {
				result.log = QString::fromUtf8(ba.data(), ba.size());
			}
		}
		if (result.log.isEmpty()) throw QString("The server does not respond.");
		if (!result.log.startsWith("OK MPD ")) throw QString("Host is not MPD server.");
		result.success = true;
		QString pw = host.password();
		if (!pw.isEmpty()) {
			QString str = "password " + pw + '\n';
			sock->write(str.toUtf8());
			std::vector<QString> lines;
			if (recv(sock, &lines)) {
				result.success = true;
				result.incorrect_password = false;
			} else {
                for (QString const &line : lines) {
					result.log += line + '\n';
				}
				result.incorrect_password = true;
			}
		}
	} catch (...) {
		sock->close();
		throw;
	}
	return result;
}

bool MusicPlayerClient::open(Host const &host)
{
	try {
		OpenResult r = open(&sock, host);
		if (r.success) {
			if (r.incorrect_password) {
				throw QString("Authentication failure.");
			}
			if (!ping()) throw QString("The server does not respond.");
			return true;
		}
		return false;
	} catch (QString const &e) {
		exception = e;
	}
	return false;
}

void MusicPlayerClient::close()
{
	if (isOpen()) {
		sock.write("close");
		sock.waitForBytesWritten(100);
		sock.close();
	}
}

bool MusicPlayerClient::isOpen() const
{
	return sock.isOpen();
}

bool MusicPlayerClient::ping()
{
	for (int i = 0; i < 3; i++) {
		std::vector<QString> lines;
		if (exec("ping", &lines)) {
			return true;
		}
	}
	return false;
}

void MusicPlayerClient::parse_result(std::vector<QString> const &lines, std::vector<Item> *out)
{
	Item info;
	auto it = lines.begin();
	while (1) {
		QString key;
		QString value;
		bool end = false;
		if (it == lines.end()) {
			end = true;
		} else {
			QString line = *it;
			ushort const *str = line.utf16();
			ushort const *ptr = ucschr(str, ':');
			if (ptr) {
				key = QString::fromUtf16(str, ptr - str);
				value = QString::fromUtf16(ptr + 1).trimmed();
			}
			it++;
		}
		bool low = iswlower(key.utf16()[0]);
		if (low || end) {
			if (!info.kind.isEmpty() || !info.map.empty()) {
				out->push_back(info);
			}
			if (end) {
				break;
			}
			info = Item();
		}
		if (!key.isEmpty()) {
			if (low) {
				info.kind = key;
				info.text = value;
			} else {
				info.map.map[key] = value;
			}
		}
	}
}

void MusicPlayerClient::parse_result(std::vector<QString> const &lines, std::vector<KeyValue> *out)
{
	out->clear();
	auto it = lines.begin();
	while (1) {
		QString key;
		QString value;
		if (it == lines.end()) {
			return;
		}
		QString line = *it;
		ushort const *str = line.utf16();
		ushort const *ptr = ucschr(str, ':');
		if (ptr) {
			key = QString::fromUtf16(str, ptr - str);
			value = QString::fromUtf16(ptr + 1).trimmed();
		}
		it++;
		if (!key.isEmpty()) {
			out->push_back(KeyValue(key, value));
		}
	}
}

void MusicPlayerClient::parse_result(std::vector<QString> const &lines, StringMap *out)
{
	out->clear();
	std::vector<KeyValue> vec;
	parse_result(lines, &vec);
    for (KeyValue const &item : vec) {
		out->map[item.key] = item.value;
	}
}

bool MusicPlayerClient::do_status(StringMap *out)
{
	out->map.clear();
	std::vector<QString> lines;
	if (exec("status", &lines)) {
		parse_result(lines, out);
		return true;
	}
	return false;
}

template <typename T> bool MusicPlayerClient::info_(QString const &command, QString const &path, T *out)
{
	std::vector<QString> lines;
	QString cmd = command;
	if (!path.isEmpty()) {
		cmd = command + " \"" + path + '\"';
	}
	out->clear();
	if (exec(cmd, &lines)) {
		parse_result(lines, out);
		return true;
	}
	return false;
}

bool MusicPlayerClient::do_lsinfo(QString const &path, std::vector<Item> *out)
{
	return info_("lsinfo", path, out);
}

bool MusicPlayerClient::do_listall(QString const &path, std::vector<Item> *out)
{
	return info_("listall", path, out);
}

bool MusicPlayerClient::do_listallinfo(QString const &path, std::vector<KeyValue> *out)
{
	return info_("listallinfo", path, out);
}

bool MusicPlayerClient::do_listallinfo(QString const &path, std::vector<Item> *out)
{
	return info_("listallinfo", path, out);
}

bool MusicPlayerClient::do_clear()
{
	std::vector<QString> lines;
	return exec("clear", &lines);
}

bool MusicPlayerClient::do_playlistinfo(QString const &path, std::vector<Item> *out)
{
	return info_("playlistinfo", path, out);
}

bool MusicPlayerClient::do_add(QString const &path)
{
	std::vector<QString> lines;
	return exec(QString("add \"") + path + "\"", &lines);
}

bool MusicPlayerClient::do_deleteid(int id)
{
	std::vector<QString> lines;
	return exec(QString("deleteid ") + QString::number(id), &lines);
}

bool MusicPlayerClient::do_move(int from, int to)
{
	std::vector<QString> lines;
	return exec(QString("move ") + QString::number(from) + ' ' + QString::number(to), &lines);
}

bool MusicPlayerClient::do_swap(int a, int b)
{
	std::vector<QString> lines;
	return exec(QString("swap ") + QString::number(a) + ' ' + QString::number(b), &lines);
}

int MusicPlayerClient::do_addid(QString const &path, int to)
{
	std::vector<QString> lines;
	QString cmd = QString("addid \"") + path + '\"';
	if (to >= 0) {
		cmd += ' ';
		cmd += QString::number(to);
	}
	if (exec(cmd, &lines)) {
		StringMap map;
		parse_result(lines, &map);
		QString s = map.get("Id");
		if (!s.isEmpty()) {
			bool ok = false;
			int id = s.toInt(&ok);
			if (ok) {
				return id;
			}
		}
	}
	return -1;
}

bool MusicPlayerClient::do_currentsong(StringMap *out)
{
	std::vector<QString> lines;
	if (exec("currentsong", &lines)) {
		parse_result(lines, out);
		return true;
	}
	return false;
}

bool MusicPlayerClient::do_play(int i)
{
	std::vector<QString> lines;
	QString cmd = "play";
	if (i >= 0) {
		cmd += ' ';
		cmd += QString::number(i);
	}
	return exec(cmd, &lines);
}

bool MusicPlayerClient::do_pause(bool f)
{
	std::vector<QString> lines;
	return exec(f ? "pause 1" : "pause 0", &lines);
}

bool MusicPlayerClient::do_stop()
{
	std::vector<QString> lines;
	return exec("stop", &lines);
}

bool MusicPlayerClient::do_next()
{
	std::vector<QString> lines;
	return exec("next", &lines);
}

bool MusicPlayerClient::do_previous()
{
	std::vector<QString> lines;
	return exec("previous", &lines);
}

bool MusicPlayerClient::do_repeat(bool f)
{
	std::vector<QString> lines;
	return exec(f ? "repeat 1" : "repeat 0", &lines);
}

bool MusicPlayerClient::do_single(bool f)
{
	std::vector<QString> lines;
	return exec(f ? "single 1" : "single 0", &lines);
}

bool MusicPlayerClient::do_consume(bool f)
{
	std::vector<QString> lines;
	return exec(f ? "consume 1" : "consume 0", &lines);
}

bool MusicPlayerClient::do_random(bool f)
{
	std::vector<QString> lines;
	return exec(f ? "random 1" : "random 0", &lines);
}

bool MusicPlayerClient::do_setvol(int n)
{
	std::vector<QString> lines;
	return exec("setvol " + QString::number(n), &lines);
}

bool MusicPlayerClient::do_seek(int song, int pos)
{
	std::vector<QString> lines;
	return exec("seek " + QString::number(song) + ' ' + QString::number(pos), &lines);
}

bool MusicPlayerClient::do_save(QString const &name)
{
	std::vector<QString> lines;
	return exec(QString("save \"") + name + "\"", &lines);
}

bool MusicPlayerClient::do_load(QString const &name)
{
	std::vector<QString> lines;
	return exec(QString("load \"") + name + "\"", &lines);
}

bool MusicPlayerClient::do_listplaylistinfo(QString const &name, std::vector<Item> *out)
{
	return info_("listplaylistinfo", name, out);
}

bool MusicPlayerClient::do_rename(QString const &curname, QString const &newname)
{
	std::vector<QString> lines;
	return exec(QString("rename \"") + curname + "\" \"" + newname + "\"", &lines);
}

bool MusicPlayerClient::do_rm(QString const &name)
{
	std::vector<QString> lines;
	return exec(QString("rm \"") + name + "\"", &lines);
}

bool MusicPlayerClient::do_update()
{
	std::vector<QString> lines;
	return exec("update", &lines);
}


