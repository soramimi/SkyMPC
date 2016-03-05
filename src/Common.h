#ifndef COMMON_H
#define COMMON_H

#include <QEvent>
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

#endif // COMMON_H
