#ifndef COMMON_H
#define COMMON_H

#include "MusicPlayerClient.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QString>


struct RequestItem {
	QString path;
	QModelIndex index;
	RequestItem()
	{
	}
	RequestItem(QString const &path, QModelIndex const &index)
		: path(path)
		, index(index)
	{
	}
};

struct ResultItem {
	RequestItem req;
	QList<MusicPlayerClient::Item> vec;
};

class Command {
	friend class TinyMainWindow;
private:
	struct {
		QString command;
	} data;
	static int compare(QString const &l, QString const &r)
	{
		return l.compare(r, Qt::CaseInsensitive);
	}
	int compare(Command const &r) const
	{
		return compare(data.command, r.data.command);
	}
public:
	Command()
	{
	}

	Command(QString const &command)
	{
		data.command = command;
	}

	QString const &command() const
	{
		return data.command;
	}

	bool operator < (Command const &r) const
	{
		return compare(r) < 0;
	}
	bool operator == (Command const &r) const
	{
		return compare(r) == 0;
	}
	bool operator != (Command const &r) const
	{
		return compare(r) != 0;
	}
};

class BasicMainWindow : public QMainWindow {
public:
	BasicMainWindow(QWidget *parent)
		: QMainWindow(parent)
	{
	}
	virtual void eatMouse() = 0;
	virtual bool isAutoReconnectAtStartup() = 0;
	virtual void preexec() = 0;
};

extern BasicMainWindow *the_mainwindow;

#endif // COMMON_H
