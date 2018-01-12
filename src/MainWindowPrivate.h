#ifndef MAINWINDOWPRIVATE_H
#define MAINWINDOWPRIVATE_H

#include "MainWindow.h"
#include "VerticalVolumePopup.h"
#include "VolumeIndicatorPopup.h"
#include "StatusThread.h"
#include "StatusLabel.h"
#include <QTimer>
#include <QWaitCondition>
#include <QMenu>
#include <QEvent>
#include <vector>
#include <QThread>
#include <QTime>

class QLabel;

struct SongItem {
	int index = -1;
	QString path;
	SongItem()
	{
	}
	SongItem(int index, QString const &path)
		: index(index)
		, path(path)
	{
	}
};

struct BasicMainWindow::Private {
    StatusLabel *status_label1;
    StatusLabel *status_label2;
    StatusLabel *status_label3;
	bool connected = false;
	MusicPlayerClient mpc;
	StatusThread status_thread;
	Host host;
	std::vector<SongItem> drop_before;
	struct Playing {
		struct Status {
			PlayingStatus status = PlayingStatus::Unknown;
			int index = -1;
			QString title;
			QString artist;
			QString disc;
			int track = 0;
			bool operator == (Status const &r) const
			{
				return
						status == r.status &&
						index == r.index &&
						title == r.title &&
						artist == r.artist &&
						disc == r.disc &&
						track == r.track
						;
			}
			bool operator != (Status const &r) const
			{
				return !operator == (r);
			}
		} now, ago;
	} status;
	double total_seconds = 0;
	bool repeat_enabled = false;
	bool single_enabled = false;
	bool consume_enabled = false;
	bool random_enabled = false;
	int volume = -1;
	VerticalVolumePopup volume_popup;

	QMenu menu;

	QIcon folder_icon;
	QIcon song_icon;
	QIcon playlist_icon;

//	std::map<QString, QAction *> command_action_map;
//	std::map<int, QString> key_command_map;

	bool release_mouse_event = false;

	QString connect_text;

	QDateTime sleep_time;

	int ping_failed_count = 0;
};

#endif // MAINWINDOWPRIVATE_H
