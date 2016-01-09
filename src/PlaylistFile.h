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
		int length;
		Item()
		{
			length = -1;
		}
	};

	static bool parse_pls(const QByteArray &ba, std::vector<Item> *out);
	static bool parse_m3u(const QByteArray &ba, std::vector<Item> *out);
	static bool parse_xspf(const QByteArray &ba, std::vector<Item> *out);
};

#endif // PLAYLISTFILE_H
