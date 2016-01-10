#ifndef MUSICPLAYERCLIENT_H
#define MUSICPLAYERCLIENT_H

#include <QString>
#include <QTcpSocket>
#include <vector>
#include <map>
#include <QSharedPointer>
#include "misc.h"

#define DEFAULT_MPD_PORT 6600

class Host {
private:
	struct Data {
		QString address;
		int port = 0;
		QString password;
	} data;
public:
	Host()
	{
	}
	Host(QString const &hostname, int port = 0)
	{
		set(hostname, port);
	}
	void set(QString const &hostname, int port = 0);
	QString address() const
	{
		return data.address;
	}
	void setAddress(QString const &addr)
	{
		data.address = addr;
	}
	int port(int def = 0) const
	{
		return data.port > 0 && data.port < 65536 ? data.port : def;
	}
	void setPort(int port)
	{
		if (port < 1) {
			port = 1;
		} else if (port > 65535) {
			port = 65535;
		}
		data.port = port;
	}
	void setPassword(QString const &password)
	{
		data.password = password;
	}
	QString password() const
	{
		return data.password;
	}
	bool isValid() const
	{
		return !data.address.isEmpty() && data.port > 0 && data.port < 65536;
	}
	bool operator == (Host const &r) const
	{
		return data.address == r.data.address && data.port == r.data.port;
	}
	bool operator != (Host const &r) const
	{
		return !operator == (r);
	}
};

class MusicPlayerClient : public QObject {
	Q_OBJECT
public:
	struct KeyValue {
		QString key;
		QString value;
		KeyValue()
		{
		}
		KeyValue(QString key, QString value)
			: key(key)
			, value(value)
		{
		}
	};
	class StringMap {
		friend class MusicPlayerClient;
	private:
	public:
		std::map<QString, QString> map;
	public:
		QString get(QString const &name) const
		{
			auto it = map.find(name);
			if (it != map.end()) {
				return it->second;
			}
			return QString();
		}
		bool empty() const
		{
			return map.empty();
		}
		void clear()
		{
			map.clear();
		}
	};
	struct Item {
		QString kind;
		QString text;
		StringMap map;
		bool operator < (Item const &r) const
		{
			return text < r.text;
		}
	};
	struct Playlist {
		QString name;
		std::vector<MusicPlayerClient::Item> songs;
	};
	class Logger {
	public:
		virtual void append(QString const &text) = 0;
	};
private:
	QSharedPointer<QTcpSocket> sock_;
	QTcpSocket &sock()
	{
		if (sock_.isNull()) {
			sock_.reset(new QTcpSocket());
		}
		return *sock_;
	}
	QTcpSocket const *sock_p() const
	{
		return sock_.isNull() ? nullptr : &*sock_;
	}
	QString exception;
private:
	static bool recv(QTcpSocket *sock, QStringList *lines);
	bool exec(QString const &command, QStringList *lines);
	void parse_result(QStringList const &lines, QList<Item> *out);
	void parse_result(QStringList const &lines, std::vector<KeyValue> *out);
	void parse_result(QStringList const &lines, StringMap *out);
	template <typename T> bool info_(QString const &command, QString const &path, T *out);
	bool send_password(QString const &password);
public:
	MusicPlayerClient();
	MusicPlayerClient(MusicPlayerClient const &) = delete;
	void operator = (MusicPlayerClient const &) = delete;
	virtual ~MusicPlayerClient();
	QString message() const
	{
		return exception;
	}
	struct OpenResult {
		bool success = false;
		bool incorrect_password = false;
	};
	static OpenResult open(QTcpSocket *sock, Host const &host, Logger *logger = 0);
	bool open(Host const &host);
	void close();
	bool isOpen() const;
	bool ping(int retry = 3);
	bool do_status(StringMap *out);
	bool do_lsinfo(QString const &path, QList<Item> *out);
	bool do_listall(QString const &path, QList<Item> *out);
	bool do_listallinfo(QString const &path, std::vector<KeyValue> *out);
	bool do_listallinfo(QString const &path, QList<Item> *out);
	bool do_clear();
	bool do_playlistinfo(QString const &path, QList<Item> *out);
	bool do_add(QString const &path);
	bool do_deleteid(int id);
	bool do_move(int from, int to);
	bool do_swap(int a, int b);
	int do_addid(QString const &path, int to);
	bool do_currentsong(StringMap *out);
	bool do_play(int i = -1);
	bool do_pause(bool f);
	bool do_stop();
	bool do_next();
	bool do_previous();
	bool do_repeat(bool f);
	bool do_single(bool f);
	bool do_consume(bool f);
	bool do_random(bool f);
	bool do_setvol(int n);
	bool do_seek(int song, int pos);
	bool do_save(QString const &name);
	bool do_load(QString const &name);
	bool do_listplaylistinfo(QString const &name, QList<Item> *out);
	bool do_rename(QString const &curname, QString const &newname);
	bool do_rm(QString const &name);
	bool do_update();
	int get_volume();
};

#endif // MUSICPLAYERCLIENT_H
