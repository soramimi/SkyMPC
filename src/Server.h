#ifndef SERVER_H
#define SERVER_H

#include "MusicPlayerClient.h"

struct ServerItem {
	QString name;
	Host host;
	QString description;
	bool operator < (ServerItem const &r) const
	{
		return name < r.name;
	}
};

bool loadPresetServers(std::vector<ServerItem> *out);
bool savePresetServers(std::vector<ServerItem> const *servers);

#endif // SERVER_H
