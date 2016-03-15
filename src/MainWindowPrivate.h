#ifndef MAINWINDOWPRIVATE_H
#define MAINWINDOWPRIVATE_H

#include "MainWindow.h"
#include "VerticalVolumePopup.h"
#include "VolumeIndicatorPopup.h"
#include "StatusThread.h"
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
	QLabel *status_label1;
	QLabel *status_label2;
	QLabel *status_label3;
	bool connected = false;
	MusicPlayerClient mpc;
	StatusThread status_thread;
	Host host;
	std::vector<SongItem> drop_before;
	struct Playing {
		struct Status {
			PlayingStatus status = PlayingStatus::Unknown;
			int index = -1;
			bool operator == (Status const &r) const
			{
				return status == r.status && index == r.index;
			}
			bool operator != (Status const &r) const
			{
				return !operator == (r);
			}
		} now, ago;
		double total = 0;
		QString current_title;
		QString current_artist;
		QString current_disc;
		int current_track = 0;
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
