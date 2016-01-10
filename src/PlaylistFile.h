#ifndef PLAYLISTFILE_H
#define PLAYLISTFILE_H

#include <QString>
#include <vector>

class QString;
class QByteArray;

class PlaylistFile {
public:
	struct Item {
		QString file;
		QString title;
		int length = -1;
	};

	static bool parse_pls(char const *begin, char const *end, std::vector<Item> *out);
	static bool parse_m3u(char const *begin, char const *end, std::vector<Item> *out);
	static bool parse_xspf(char const *begin, char const *end, std::vector<Item> *out);
};

#endif // PLAYLISTFILE_H
