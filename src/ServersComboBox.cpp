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
	if (host.isValid()) {
		if (name.indexOf(':') >= 0) {
			name = '[' + name + ']';
		}
		name += ':' + QString::number(host.port());
	}
	return name;
}

void ServersComboBox::resetContents(const Host &current_host, bool caption, bool connection)
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
	if (sel < 0 && current_host.isValid()) {
		QString text = makeServerText(current_host);
		if (!text.isEmpty()) {
			sel = count();
			addItem(text);
		}
	}
	if (connection) {
		addItem(trConnect());
	}
	setCurrentIndex(sel);
	setUpdatesEnabled(true);

}
