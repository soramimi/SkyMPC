#ifndef MAINWINDOWPRIVATE_H
#define MAINWINDOWPRIVATE_H

#include "MainWindow.h"
#include "VerticalVolumePopup.h"
#include "VolumeIndicatorPopup.h"
#include "NotifyOverlayWindow.h"
#include <QTimer>
#include <QWaitCondition>
#include <QMenu>
#include <QEvent>
#include <vector>

class QLabel;

enum {
	EVENT_FocusChanged = QEvent::User,
	EVENT_QueryInfo,
};

struct SongItem {
	int index;
	QString path;
	SongItem()
		: index(-1)
	{
	}
	SongItem(int index, QString const &path)
		: index(index)
		, path(path)
	{
	}
};

namespace PlayingStatus {
enum e {
	Stop,
	Play,
	Pause,
};
}

struct MainWindow::Private {
    QLabel *status_label;
	bool connected;
	MusicPlayerClient mpc;
	Host host;
	std::vector<SongItem> drop_before;
//	QTimer interval_timer;
	struct Playing {
		PlayingStatus::e playing;
		int current_song;
		int current_song_indicator;
		QString windowtitle;
		QString song_information;
		double total;
		double elapsed;
		Playing()
			: current_song(0)
			, total(0)
			, elapsed(0)
		{
		}
	} status;
	double total_seconds;
	bool repeat_enabled;
	bool single_enabled;
	bool consume_enabled;
	bool random_enabled;
	int volume;
	VerticalVolumePopup volume_popup;

	int update_information_count;
	int slider_down_count;
    int notify_visible_count;

	QMenu menu;

	QIcon server_icon;
	QIcon folder_icon;

	std::map<QString, QAction *> command_action_map;
	std::map<int, QString> key_command_map;
};

#endif // MAINWINDOWPRIVATE_H
