#include "MemoryReader.h"
#include "PlaylistFile.h"
#include "webclient.h"

#include <QBuffer>
#include <QXmlStreamReader>

bool PlaylistFile::parse_pls(char const *begin, char const *end, std::vector<PlaylistFile::Item> *out)
{
	out->clear();
	MemoryReader in;
	in.setData(begin, end - begin);
	if (in.open(QBuffer::ReadOnly)) {
		std::map<int, Item> songs;
		bool valid = false;
		QString File = "File";
		QString Title = "Title";
		QString Length = "Length";
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			int eq = line.indexOf('=');
			if (line.compare("[playlist]", Qt::CaseInsensitive) == 0) {
				valid = true;
			} else if (valid) {
				if (line.startsWith(File, Qt::CaseInsensitive)) {
					int i = File.size();
					if (eq > i) {
						int n = line.mid(i, eq - i).trimmed().toInt();
						songs[n].file = line.mid(eq + 1).trimmed();
					}
				} else if (line.startsWith(Title, Qt::CaseInsensitive)) {
					int i = Title.size();
					if (eq > i) {
						int n = line.mid(i, eq - i).trimmed().toInt();
						songs[n].title = line.mid(eq + 1).trimmed();
					}
				} else if (line.startsWith(Length, Qt::CaseInsensitive)) {
					int i = Length.size();
					if (eq > i) {
						int n = line.mid(i, eq - i).trimmed().toInt();
						songs[n].length = line.mid(eq + 1).trimmed().toInt();
					}
				}
			} else if (!line.isEmpty()) {
				return false;
			}
		}
		for (auto pair : songs) {
			Item const &item = pair.second;
			if (item.file.startsWith("http")) {
				out->push_back(item);
			}
		}
		return true;
	}
	return false;
}

bool PlaylistFile::parse_m3u(char const *begin, char const *end, std::vector<PlaylistFile::Item> *out)
{
	out->clear();
	MemoryReader in;
	in.setData(begin, end - begin);
	if (in.open(QBuffer::ReadOnly)) {
		bool ext = false;
		QString EXTINF = "#EXTINF";
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.startsWith('#')) {
				if (line.compare("#EXTM3U") == 0) {
					ext = true;
				} else {
					// nop (comment)
				}
			} else if (ext && line.startsWith(EXTINF)) {
				Item song;
				int i = EXTINF.size();
				if (i < line.size()) {
					int colon = line.indexOf(':', i);
					if (colon > 0) {
						int comma = line.indexOf(',', i);
						QString length;
						if (comma > colon) {
							length = line.mid(colon + 1, (comma > colon) ? (comma - colon - 1) : -1);
							song.title = line.mid(comma + 1);
						} else {
							length = line.mid(colon + 1);
						}
						song.length = length.trimmed().toInt();
						if (song.length < 1) {
							song.length = -1;
						}
					}
				}
				line = in.readLine();
				song.file = line.trimmed();
				out->push_back(song);
			} else if (line.startsWith("http://")) {
				Item song;
				song.file = line;
				out->push_back(song);
			} else {
				return false;
			}
		}
		return true;
	}
	return false;
}

bool PlaylistFile::parse_xspf(char const *begin, char const *end, std::vector<PlaylistFile::Item> *out)
{
	out->clear();
	MemoryReader in;
	in.setData(begin, end - begin);
	if (in.open(QBuffer::ReadOnly)) {
		enum State {
			STATE_UNKNOWN,
			STATE_PLAYLIST,
			STATE_PLAYLIST_TITLE,
			STATE_PLAYLIST_TRACKLIST,
			STATE_PLAYLIST_TRACKLIST_TRACK,
			STATE_PLAYLIST_TRACKLIST_TRACK_LOCATION,
			STATE_PLAYLIST_TRACKLIST_TRACK_TITLE,
		};
		QXmlStreamReader reader(&in);
		std::vector<State> state_stack;
		Item song;
		QString title;
		QString text;
		while (!reader.atEnd()) {
			State state = STATE_UNKNOWN;
			State laststate = STATE_UNKNOWN;
			if (!state_stack.empty()) {
				laststate = state_stack.back();
			}
			switch (reader.readNext()) {
			case QXmlStreamReader::StartElement:
				if (state_stack.empty()) {
					if (reader.name().compare("playlist", Qt::CaseInsensitive) == 0) {
						QStringRef data;
//						QXmlStreamAttributes atts = reader.attributes();
						data = reader.namespaceUri();
						if (!data.startsWith("http://xspf.org/ns/")) {
							return false;
						}
						state = STATE_PLAYLIST;
					}
				} else if (laststate == STATE_PLAYLIST) {
					if (reader.name().compare("title", Qt::CaseInsensitive) == 0) {
						state = STATE_PLAYLIST_TITLE;
					} else if (reader.name().compare("tracklist", Qt::CaseInsensitive) == 0) {
						state = STATE_PLAYLIST_TRACKLIST;
					}
				} else if (laststate == STATE_PLAYLIST_TITLE) {
				} else if (laststate == STATE_PLAYLIST_TRACKLIST) {
					if (reader.name().compare("track", Qt::CaseInsensitive) == 0) {
						song = Item();
						state = STATE_PLAYLIST_TRACKLIST_TRACK;
					}
				} else if (laststate == STATE_PLAYLIST_TRACKLIST_TRACK) {
					if (reader.name().compare("location", Qt::CaseInsensitive) == 0) {
						state = STATE_PLAYLIST_TRACKLIST_TRACK_LOCATION;
					} else if (reader.name().compare("title", Qt::CaseInsensitive) == 0) {
						state = STATE_PLAYLIST_TRACKLIST_TRACK_TITLE;
					}
				}
				text = QString();
				state_stack.push_back(state);
				break;
			case QXmlStreamReader::Characters:
				text += reader.text();
				break;
			case QXmlStreamReader::EndElement:
				if (laststate == STATE_PLAYLIST_TITLE) {
					title = text;
				} else if (laststate == STATE_PLAYLIST_TRACKLIST_TRACK_LOCATION) {
					song.file = text;
				} else if (laststate == STATE_PLAYLIST_TRACKLIST_TRACK_TITLE) {
					song.title = text;
				} else if (laststate == STATE_PLAYLIST_TRACKLIST_TRACK) {
					out->push_back(song);
					song = Item();
				}
				text = QString();
				state_stack.pop_back();
				break;
			}
		}
		return true;
	}
	return false;
}

class MyWebClientHandler : public WebClientHandler {
public:
	void checkHeader(WebClient *wc)
	{
		std::string ct = wc->content_type();
		if (ct == "audio/mpeg") {
			abort();
		}
	}
	void checkContent(const char *, size_t len)
	{
		if (len > 100000) {
			abort();
		}
	}
};

bool PlaylistFile::parse(const QString &loc, std::vector<Item> *out)
{
	out->clear();
	bool parsed = false;
	WebContext wc;
	WebClient web(&wc);
	MyWebClientHandler handler;
	int s = web.get(WebClient::URL(loc.toStdString().c_str()), &handler);
	if (s == 200 && !web.response().content.empty()) {
		char const *begin = &web.response().content[0];
		char const *end = begin + web.response().content.size();
		parsed = parsed || PlaylistFile::parse_pls(begin, end, out);
		parsed = parsed || PlaylistFile::parse_m3u(begin, end, out);
		parsed = parsed || PlaylistFile::parse_xspf(begin, end, out);
	}
	return parsed;
}
