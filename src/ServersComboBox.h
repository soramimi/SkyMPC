#ifndef SERVERSCOMBOBOX_H
#define SERVERSCOMBOBOX_H

#include <QComboBox>
class Host;

class ServersComboBox : public QComboBox
{
	Q_OBJECT
public:
	explicit ServersComboBox(QWidget *parent = 0);

	void resetContents(const Host &current_host, bool caption = false);
signals:

public slots:
public:
	static QString makeServerText(const Host &host);
	static QString trConnect()
	{
		return tr("Connect...");
	}
};

#endif // SERVERSCOMBOBOX_H
