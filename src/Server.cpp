#include "Server.h"
#include "main.h"
#include "pathcat.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QTime>
#include <stdint.h>
#include "misc.h"

static const char SERVERS_XML[] = "servers.xml";

enum State {
	STATE_ROOT,
	STATE_UNKNOWN,
	STATE_SERVERS,
};

QString encodePassword(QString const &str)
{
	if (!str.isEmpty()) {
		char tmp[4];
		uint32_t t = QTime::currentTime().msecsSinceStartOfDay();
		tmp[0] = t >> 24;
		tmp[1] = t >> 16;
		tmp[2] = t >> 8;
		tmp[3] = t;
		QByteArray ba = str.toUtf8();
		ba.insert(0, tmp, 4);
		pseudo_crypto_encode(ba.data(), ba.size());
		ba = ba.toHex();
		return QString::fromUtf8(ba);
	}
	return QString();
}

QString decodePassword(QString const &str)
{
	if (!str.isEmpty()) {
		QByteArray ba = QByteArray::fromHex(str.toUtf8());
		pseudo_crypto_decode(ba.data(), ba.size());
		if (ba.size() > 4) {
			ba.remove(0, 4);
			return QString::fromUtf8(ba.data(), ba.size());
		}
	}
	return QString();
}

bool loadPresetServers(std::vector<ServerItem> *out)
{
	out->clear();
	bool ok = false;
	QString path = pathcat(application_data_dir, SERVERS_XML);
	QFile file(path);
	if (file.open(QFile::ReadOnly)) {
		std::vector<State> state_stack;
		QXmlStreamReader reader(&file);
		reader.setNamespaceProcessing(false);
		while (!reader.atEnd()) {
			State state = STATE_UNKNOWN;
			State laststate = STATE_UNKNOWN;
			if (!state_stack.empty()) {
				laststate = state_stack.back();
			}
			switch (reader.readNext()) {
			case QXmlStreamReader::StartElement:
				if (state_stack.empty()) {
					if (reader.name() == "servers") {
						state = STATE_SERVERS;
					}
				} else if (laststate == STATE_SERVERS) {
					QStringRef data;
					QXmlStreamAttributes atts = reader.attributes();
					if (reader.name() == "item") {
						QString name = atts.value("name").toString();
						QString address = atts.value("address").toString();
						int port = atts.value("port").toString().toInt();
						QString password = atts.value("password").toString();
						QString desc = atts.value("description").toString();
						ServerItem item;
						item.name = name;
						item.host = Host(address, port);
						item.host.setPassword(decodePassword(password));
						item.description = desc;
						out->push_back(item);
					}
				}
				state_stack.push_back(state);
				break;
			case QXmlStreamReader::Characters:
				break;
			case QXmlStreamReader::EndElement:
				state_stack.pop_back();
				break;
			}
		}
		ok = true;
	}
	return ok;
}

bool savePresetServers(std::vector<ServerItem> const *servers)
{
	QString path = pathcat(application_data_dir, SERVERS_XML);
	QFile file(path);
	if (!file.open(QFile::WriteOnly)) {
		return false;
	}
	{
		QXmlStreamWriter writer(&file);
		writer.setAutoFormatting(true);
		writer.writeStartDocument();
		writer.writeStartElement("servers");
        for (ServerItem const &server : *servers) {
			writer.writeStartElement("item");
			writer.writeAttribute("name", server.name);
			writer.writeAttribute("address", server.host.address());
			writer.writeAttribute("port", QString::number(server.host.port()));
			writer.writeAttribute("password", encodePassword(server.host.password()));
			writer.writeAttribute("description", server.description);
			writer.writeEndElement();
		}
		writer.writeEndElement();
	}
	file.close();
	return true;
}
