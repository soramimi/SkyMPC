#include "MusicPlayerClient.h"
#include "ServersComboBox.h"
#include "Server.h"

ServersComboBox::ServersComboBox(QWidget *parent)
	: QComboBox(parent)
{

}

QString ServersComboBox::makeServerText(const Host &host)
{
	QString name;
	name = host.address();
	if (name.indexOf(':') >= 0) {
		name = '[' + name + ']';
	}
	name += ':' + QString::number(host.port());
	return name;
}

void ServersComboBox::resetContents(const Host &current_host, bool caption)
{
	setUpdatesEnabled(false);
	clear();
	int sel = -1;
	if (caption) {
		addItem(tr("MPD Servers"));
	}
	std::vector<ServerItem> servers;
	loadPresetServers(&servers);
	for (int i = 0; i < (int)servers.size(); i++) {
		int row = count();
		QString text = servers[i].name;
		addItem(text);
		setItemData(row, text);
		if (current_host == servers[i].host) {
			sel = i;
		}
	}
	if (sel < 0) {
		QString text = makeServerText(current_host);
		sel = count();
		addItem(text);
	}
	addItem(trConnect());
	setCurrentIndex(sel);
	setUpdatesEnabled(true);

}
