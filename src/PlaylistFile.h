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
private:
	std::vector<PlaylistFile::Item> locations_;
	static bool parse_pls(char const *begin, char const *end, std::vector<Item> *out);
	static bool parse_m3u(char const *begin, char const *end, std::vector<Item> *out);
	static bool parse_xspf(char const *begin, char const *end, std::vector<Item> *out);
	static bool parse(const QString &loc, std::vector<Item> *out);
public:

	std::vector<PlaylistFile::Item> const *locations() const
	{
		return &locations_;
	}

	bool parse(QString const &loc)
	{
		return parse(loc, &locations_);
	}
};

#endif // PLAYLISTFILE_H
