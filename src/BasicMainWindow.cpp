#include <QApplication>
#include "BasicMainWindow.h"
#include "AskRemoveDuplicatedFileDialog.h"
#include "Common.h"
#include "ConnectionDialog.h"
#include "EditLocationDialog.h"
#include "MainWindowPrivate.h"
#include "MusicPlayerClient.h"
#include "MySettings.h"
#include "SleepTimerDialog.h"
#include "Toast.h"

BasicMainWindow::BasicMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
}

QString BasicMainWindow::makeStyleSheetText()
{
#ifdef Q_OS_WIN
	Font default_font("Meiryo", 10);
	Font progress_font("Arial", 10);
#endif

#ifdef Q_OS_MAC
	Font default_font("Lucida Grande", 14);
	Font progress_font("Lucida Grande", 14);
#endif

#ifdef Q_OS_LINUX
	Font default_font("VL PGothic", 10);
	Font progress_font("VL PGothic", 10);
#endif

	QString s;
	s += "* {";
	s += default_font.text();
	s += "}";
	s += "#label_title, #label_artist, #label_disc {";
	s += "font-weight: bold;";
	s += "}";
	s += "#label_progress {";
	s += progress_font.text();
	s += "}";
	return s;
}

void BasicMainWindow::releaseMouseIfGrabbed()
{
	if (pv->release_mouse_event) {
		releaseMouse();
		pv->release_mouse_event = false;
	}
}

void BasicMainWindow::stopStatusThread()
{
	pv->status_thread.requestInterruption();
	pv->status_thread.wait(1000);
}

void BasicMainWindow::BasicMainWindow::execSleepTimerDialog()
{
	MySettings settings;
	settings.beginGroup("Playback");
	int mins = settings.value("SleepTimer").toInt();
	if (mins == 0) mins = 60;
	settings.endGroup();

	SleepTimerDialog dlg(this);
	dlg.setMinutes(mins);
	int r = dlg.exec();
	if (r != QDialog::Rejected) {
		mins = 0;
		if (r == SleepTimerDialog::Start) {
			mins = dlg.minutes();
			if (mins > 0) {
				settings.beginGroup("Playback");
				settings.setValue("SleepTimer", mins);
				settings.endGroup();
			} else {
				mins = 0;
			}
		}
		if (mins > 0) {
			startSleepTimer(mins);
		} else {
			stopSleepTimer();
		}
	}
}

void BasicMainWindow::eatMouse()
{
	grabMouse();
	pv->release_mouse_event = true;
}

void BasicMainWindow::updatePlayingStatus()
{
	PlayingStatus status = PlayingStatus::Stop;

	QString windowtitle = qApp->applicationName();

	if (pv->mpc.isOpen()) {
		PlayingInfo info;
		pv->status_thread.data(&info);

		QString state = info.status.get("state");
		if (state == "play") {
			status = PlayingStatus::Play;
		} else if (state == "pause") {
			status = PlayingStatus::Pause;
		} else {
			pv->status.song_information.clear();
		}

		if (status == PlayingStatus::Stop) {
			pv->total_seconds = 0;
			displayStopStatus();
		} else {
			pv->status.current_song = info.status.get("song").toInt();

			pv->volume = info.status.get("volume").toInt();

			setRepeatEnabled(info.status.get("repeat").toInt() != 0);
			setSingleEnabled(info.status.get("single").toInt() != 0);
			setConsumeEnabled(info.status.get("consume").toInt() != 0);
			setRandomEnabled(info.status.get("random").toInt() != 0);

			if (info.status.get("songid") == info.property.get("Id")) {
				QString title = info.property.get("Title");
				QString artist = info.property.get("Artist");
				int track = info.property.get("Track").toInt();
				QString album = info.property.get("Album");
				QString disc;
				if (!album.isEmpty()) {
					if (track > 0) {
						disc = QString("Tr.") + QString::number(track) + ", ";
					}
					disc += album;
				}
				if (title.isEmpty()) {
					std::wstring file = info.property.get("file").toStdWString();
					wchar_t const *p = wcsrchr(file.c_str(), L'/');
					if (p) {
						title = QString::fromUtf16((ushort const *)p + 1);
					}
				}
				{
					QString text = title + '\t' + artist + '\t' + disc;
					if (text != pv->status.song_information) {
						pv->status.song_information = text;
						displayPlayStatus(title, artist, disc);
					}
				}
				windowtitle = title + " - " + windowtitle;

				double elapsed = 0;
				{
					std::string s;
					s = info.status.get("time").toStdString();
					int t, e;
					if (sscanf(s.c_str(), "%d:%d", &e, &t) == 2) {
						elapsed = e;
						pv->total_seconds = t;
					}
				}
				{
					bool ok = false;
					double e = info.status.get("elapsed").toDouble(&ok);
					if (ok) {
						elapsed = e;
					}
				}
				seekProgressSlider(elapsed, pv->total_seconds);
				displayProgress(elapsed);
			}
		}
	}

	if (windowtitle != pv->status.windowtitle) {
		pv->status.windowtitle = windowtitle;
		setWindowTitle(windowtitle);
	}


	if (status != pv->status.playing) {
		pv->status.playing = status;
		updatePlayIcon();
		invalidateCurrentSongIndicator();
	}
}

void BasicMainWindow::setRepeatEnabled(bool f)
{
	pv->repeat_enabled = f;
}

void BasicMainWindow::setSingleEnabled(bool f)
{
	pv->single_enabled = f;
}

void BasicMainWindow::setConsumeEnabled(bool f)
{
	pv->consume_enabled = f;
}

void BasicMainWindow::setRandomEnabled(bool f)
{
	pv->random_enabled = f;
}

void BasicMainWindow::invalidateCurrentSongIndicator()
{
	pv->status.current_song_indicator = -1;
}

QString BasicMainWindow::makeServerText(const Host &host)
{
	QString name;
	name = host.address();
	if (name.indexOf(':') >= 0) {
		name = '[' + name + ']';
	}
	name += ':' + QString::number(host.port());
	return name;
}

QString BasicMainWindow::serverName() const
{
	return pv->host.address();
}

void BasicMainWindow::showNotify(const QString &text)
{
	Toast::show(this, text, Toast::LENGTH_MOMENT);
}

void BasicMainWindow::showError(const QString &text)
{
	Toast::show(this, text, Toast::LENGTH_LONG);
}

void BasicMainWindow::update(bool mpdupdate)
{
	if (mpdupdate) {
		pv->mpc.do_update();
	}

	updateTreeTopLevel();
	updatePlaylist();
	updatePlayingStatus();
	updateCurrentSongIndicator();
}

void BasicMainWindow::checkDisconnected()
{
	if (!pv->mpc.isOpen()) {
		if (pv->connected) {
			pv->connected = false;
			pv->ping_failed_count = 0;
			setPageDisconnected();
			clearTreeAndList();
		}
	}
}

void BasicMainWindow::clearPlaylist()
{
	int count = 0;
	using mpcitem_t = MusicPlayerClient::Item;
	QList<mpcitem_t> vec;
	pv->mpc.do_playlistinfo(QString(), &vec);
	for (mpcitem_t const &item : vec) {
		if (item.kind == "file") {
			count++;
		}
	}
	if (count > 0) {
		savePlaylist("_backup_before_clear_");
	}
	pv->mpc.do_clear();
	updatePlaylist();
}

void BasicMainWindow::startSleepTimer(int mins)
{
	if (mins > 0) {
		QDateTime t = QDateTime::currentDateTime();
		pv->sleep_time = t.addSecs((qint64)mins * 60);
	} else {
		pv->sleep_time = QDateTime();
	}
}

void BasicMainWindow::doQuickSave1()
{
	if (savePlaylist("_quick_save_1_")) {
		showNotify(tr("Quick Save 1 was completed"));
	}
}

void BasicMainWindow::doQuickSave2()
{
	if (savePlaylist("_quick_save_2_")) {
		showNotify(tr("Quick Save 2 was completed"));
	}
}

void BasicMainWindow::doQuickLoad1()
{
	loadPlaylist("_quick_save_1_", true);
}

void BasicMainWindow::doQuickLoad2()
{
	loadPlaylist("_quick_save_2_", true);
}

bool BasicMainWindow::isAutoReconnectAtStartup()
{
	bool f = true;
	MySettings settings;
	settings.beginGroup("Connection");
	if (settings.contains(KEY_AutoReconnect)) {
		f = settings.value(KEY_AutoReconnect).toBool();
	}
	settings.endGroup();
	return f;
}

void BasicMainWindow::preexec()
{
	bool conndlg = false;

	if (qApp->keyboardModifiers() & Qt::ShiftModifier) {
		conndlg = true;
	} else if (pv->host.isValid()) {
		if (isAutoReconnectAtStartup()) {
			conndlg = false;
		} else {
			conndlg = true;
		}
	} else {
		conndlg = true;
	}

	if (conndlg) {
		ConnectionDialog dlg(this, pv->host);
		if (dlg.exec() == QDialog::Accepted) {
			pv->host = dlg.host();
		}
	}

	updateServersComboBox();

	startTimer(1000);
	connectToMPD(pv->host);
}

bool BasicMainWindow::execCommand(const Command &c)
{
	auto it = pv->command_action_map.find(c.command());
	if (it != pv->command_action_map.end()) {
		QAction *a = it->second;
		Q_ASSERT(a);
		a->trigger();
		return true;
	}
	return false;
}

void BasicMainWindow::connectToMPD(const Host &host)
{
	stopSleepTimer();

	pv->ping_failed_count = 0;
	pv->mpc.close();
	stopStatusThread();

	pv->host = host;
	if (pv->mpc.open(pv->host)) {
		pv->connected = true;

		updatePlayingStatus();

		updateTreeTopLevel();

		updatePlaylist();

		// check volume support
		pv->volume = -1;
		int vol = -1;
		for (int i = 0; i < 3; i++) {
			int v = pv->mpc.get_volume();
			if (i == 0) {
				vol = v;
			} else {
				if (vol != v) {
					vol = -1;
					break;
				}
			}
		}
		if (vol >= 0) {
			int v = vol < 2 ? 2 : vol - 1;
			pv->mpc.do_setvol(v);
			if (v == pv->mpc.get_volume()) {
				pv->volume = vol;
			}
			pv->mpc.do_setvol(vol);
			pv->mpc.do_setvol(vol);
			pv->mpc.do_setvol(vol);
		}
		setVolumeEnabled(pv->volume >= 0);
	} else {
		clearTreeAndList();
		setPageDisconnected();
	}

	startStatusThread();
}

bool BasicMainWindow::isPlaying() const
{
	return pv->status.playing == PlayingStatus::Play;
}

void BasicMainWindow::startStatusThread()
{
	pv->status_thread.setHost(pv->host);
	pv->status_thread.start();
}

void BasicMainWindow::addToPlaylist(const QString &path, int to, bool update)
{
	if (path.isEmpty()) return;

	using mpcitem_t = MusicPlayerClient::Item;
	QList<mpcitem_t> mpcitems;
	if (path.indexOf("://") > 0) {
		if (to < 0) {
			pv->mpc.do_add(path);
		} else {
			pv->mpc.do_addid(path, to);
			to++;
		}
	} else if (pv->mpc.do_listall(path, &mpcitems)) {
		for (mpcitem_t const &mpcitem : mpcitems) {
			if (mpcitem.kind == "file") {
				if (to < 0) {
					pv->mpc.do_add(mpcitem.text);
				} else {
					pv->mpc.do_addid(mpcitem.text, to);
					to++;
				}
			}
		}
	}
	if (update) {
		updatePlaylist();
	}
}

void BasicMainWindow::play()
{
	pv->mpc.do_play();
}

void BasicMainWindow::pause()
{
	pv->mpc.do_pause(true);
}

void BasicMainWindow::stop()
{
	pv->mpc.do_stop();
	invalidateCurrentSongIndicator();
}

void BasicMainWindow::play(bool toggle)
{
	if (toggle) {
		if (pv->status.playing == PlayingStatus::Play) {
			pause();
		} else {
			play();
		}
	} else {
		if (pv->status.playing != PlayingStatus::Play) {
			play();
		}
	}
}

void BasicMainWindow::disconnectNetwork()
{
	pv->mpc.close();
	stopSleepTimer();
}

void BasicMainWindow::loadPlaylist(const QString &name, bool replace)
{
	if (replace) {
		pv->mpc.do_stop();
		pv->mpc.do_clear();
	}
	if (pv->mpc.do_load(name)) {
		updatePlaylist();
	} else {
		showError(tr("Failed to load playlist.") + '(' + pv->mpc.message() + ')');
	}
}

bool BasicMainWindow::savePlaylist(const QString &name)
{
	pv->mpc.do_rm(name);
	if (pv->mpc.do_save(name)) {
		return true;
	} else {
		showError(tr("Failed to save playlist.") + '(' + pv->mpc.message() + ')');
		return false;
	}
}



void BasicMainWindow::execAddLocationDialog()
{
	EditLocationDialog dlg(this);
	dlg.setLocation("http://");
	if (dlg.exec() == QDialog::Accepted) {
		QStringList locs = dlg.location().split(' ', QString::SkipEmptyParts);
		for (QString const &loc : locs) {
			if (!loc.isEmpty()) {
				pv->mpc.do_add(loc);
			}
		}
		updatePlaylist();
	}
}

BasicMainWindow *BasicMainWindow::findMainWindow(QObject *hint)
{
	if (hint) {
		while (hint) {
			BasicMainWindow *mw = dynamic_cast<BasicMainWindow *>(hint);
			if (mw) return mw;
			hint = hint->parent();
		}
	}
	QWidgetList list = QApplication::topLevelWidgets();
	for (QWidget *w : list) {
		BasicMainWindow *mw = dynamic_cast<BasicMainWindow *>(w);
		if (mw) return mw;
	}
	return 0;
}


