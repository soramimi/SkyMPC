#include "BasicMainWindow.h"
#include "AskRemoveDuplicatedFileDialog.h"
#include "Common.h"
#include "ConnectionDialog.h"
#include "EditLocationDialog.h"
#include "main.h"
#include "MainWindowPrivate.h"
#include "MusicPlayerClient.h"
#include "MySettings.h"
#include "SleepTimerDialog.h"
#include "Toast.h"
#include <QApplication>
#include <QListWidget>

BasicMainWindow::BasicMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	pv = new Private();
	connect(&pv->volume_popup, SIGNAL(valueChanged()), this, SLOT(onVolumeChanged()));
	connect(&pv->status_thread, SIGNAL(onUpdate()), this, SLOT(onUpdateStatus()));
}

BasicMainWindow::~BasicMainWindow()
{
	stopStatusThread();
	mpc()->close();
	delete pv;
}

MusicPlayerClient *BasicMainWindow::mpc()
{
	return &pv->mpc;
}

QString BasicMainWindow::makeStyleSheetText()
{
	auto font = [](QString const &name, int pt){
		return QString("font: %1pt \"%2\";").arg(pt).arg(name);
	};

#ifdef Q_OS_WIN
	QString default_font = font("Meiryo", 10);
	QString progress_font = font("Arial", 10);
#endif

#ifdef Q_OS_MAC
	QString default_font = font("Lucida Grande", 14);
	QString progress_font = font("Lucida Grande", 14);
#endif

#ifdef Q_OS_LINUX
	QString default_font = font("VL PGothic", 10);
	QString progress_font = font("VL PGothic", 10);
#endif

	QString s;
	s += "* {%1}";
	s += "#label_title, #label_artist, #label_disc {font-weight: bold;}";
	s += "#label_progress {%2}";
	return s.arg(default_font).arg(progress_font);
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

	if (mpc()->isOpen()) {
		PlayingInfo info;
		pv->status_thread.data(&info);
		QString current_file = info.property.get("file");
		QString state = info.status.get("state");
		if (state == "play") {
			status = PlayingStatus::Play;
		} else if (state == "pause") {
			status = PlayingStatus::Pause;
		}

		if (status == PlayingStatus::Stop) {
			pv->total_seconds = 0;
			displayPlayStatus(QString(), QString(), QString());
			displayStopStatus();
		} else {
			pv->status.current_song = info.status.get("song").toInt();

			pv->volume = info.status.get("volume").toInt();

			setRepeatEnabled(info.status.get("repeat").toInt() != 0);
			setSingleEnabled(info.status.get("single").toInt() != 0);
			setConsumeEnabled(info.status.get("consume").toInt() != 0);
			setRandomEnabled(info.status.get("random").toInt() != 0);

			if (info.status.get("songid") == info.property.get("Id")) {
				pv->status.current_title = info.property.get("Title");
				pv->status.current_artist = info.property.get("Artist");
				pv->status.current_track = info.property.get("Track").toInt();
				pv->status.current_disc.clear();

				QString album = info.property.get("Album");
				if (!album.isEmpty()) {
					if (pv->status.current_track > 0) {
						pv->status.current_disc = QString("Tr.") + QString::number(pv->status.current_track) + ", ";
					}
					pv->status.current_disc += album;
				}
				if (pv->status.current_title.isEmpty()) {
					std::wstring file = info.property.get("file").toStdWString();
					wchar_t const *p = wcsrchr(file.c_str(), L'/');
					if (p) {
						pv->status.current_title = QString::fromUtf16((ushort const *)p + 1);
					}
				}

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

	if (status != pv->status.playing) {
		pv->status.playing = status;
		updatePlayIcon();
		invalidateCurrentSongIndicator();
	}
}

void BasicMainWindow::displayStopStatus()
{
	seekProgressSlider(0, 0);
	displayProgress(0);
}

void BasicMainWindow::displayProgress(double elapsed)
{
	char tmp[100];
	int e = (int)elapsed;
	if (pv->total_seconds > 0) {
		int t = (int)pv->total_seconds;
		sprintf(tmp, "%u:%02u / %u:%02u", e / 60, e % 60, t / 60, t % 60);
	} else {
		sprintf(tmp, "%u:%02u", e / 60, e % 60);
	}
	displayProgress(QString(tmp));
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
		mpc()->do_update();
	}

	updateTreeTopLevel();
	updatePlaylist();
	updatePlayingStatus();
	updateCurrentSongInfo();
}

void BasicMainWindow::checkDisconnected()
{
	if (!mpc()->isOpen()) {
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
	mpc()->do_playlistinfo(QString(), &vec);
	for (mpcitem_t const &item : vec) {
		if (item.kind == "file") {
			count++;
		}
	}
	if (count > 0) {
		savePlaylist("_backup_before_clear_");
	}
	mpc()->do_clear();
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

void BasicMainWindow::doUpdateStatus()
{
	updatePlayingStatus();
	if (pv->status.current_song != pv->status.current_song_indicator) {
		updatePlaylist();
		updateCurrentSongInfo();
	}
}

void BasicMainWindow::timerEvent(QTimerEvent *)
{
	QString text2;
	QString text3;
	if (pv->connected) {
		QTime time;
		time.start();
		if (mpc()->ping(1)) {
			int ms = time.elapsed();
			pv->ping_failed_count = 0;
			text3 = "ping:";
			text3 += QString::number(ms);
			text3 += "ms";
		} else {
			pv->ping_failed_count++;
			if (pv->ping_failed_count >= 10) {
				mpc()->close();
				checkDisconnected();
			}
			text3 = tr("Waiting for connection");
			text3 += " (";
			text3 += QString::number(pv->ping_failed_count);
			text3 += ')';
		}

		if (isPlaying() && pv->sleep_time.isValid()) {
			QDateTime now = QDateTime::currentDateTime();
			qint64 secs = now.secsTo(pv->sleep_time);
			if (secs > 0) {
				int s = secs % 60;
				int m = (secs / 60) % 60;
				int h = (secs / 3600);
				char tmp[100];
				sprintf(tmp, "%u:%02u:%02u", h, m, s);
				text2 = tr("Pause in %1 later").arg(tmp);
			} else {
				pv->sleep_time = QDateTime();
				pause();
			}
		}
	}

	displayExtraInformation(text2, text3);

	checkDisconnected();
}

void BasicMainWindow::updatePlaylist(MusicPlayerClient *mpc, QListWidget *listwidget, QList<MusicPlayerClient::Item> *items)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	listwidget->setUpdatesEnabled(false);
	int row = listwidget->currentRow();

	listwidget->clear();

	for (MusicPlayerClient::Item const &mpcitem : *items) {
		if (mpcitem.kind == "file") {
			QString path = mpcitem.text;
			QString text;
			QString artist;
			if (path.indexOf("://") > 0) {
				text = path;
			} else {
				QString album;
				QString time;
				{
					QList<MusicPlayerClient::Item> v;
					mpc->do_listallinfo(path, &v);
					if (v.size() == 1) {
						text = v.front().map.get("Title");
						artist = v.front().map.get("Artist");
						album = v.front().map.get("Album");
						time = MusicPlayerClient::timeText(v.front());
					}
				}
				QString suffix;
				if (!artist.isEmpty() && !album.isEmpty()) {
					suffix = artist + '/' + album;
				} else if (!artist.isEmpty()) {
					suffix = artist;
				} else if (!album.isEmpty()) {
					suffix = album;
				}
#if DISPLAY_TIME
				if (!time.isEmpty()) {
					text += " (" + time + ")";
				}
#endif
				if (!suffix.isEmpty()) {
					text += " -- " + suffix;
				}
			}
			QString id = mpcitem.map.get("Id");
			QListWidgetItem *listitem = new QListWidgetItem();
			listitem->setText(text);
			listitem->setData(ITEM_PathRole, path);
			listitem->setData(ITEM_SongIdRole, id);
			listitem->setIcon(QIcon(":/image/notplaying.png"));
			listwidget->addItem(listitem);
		}
	}

	listwidget->setCurrentRow(row);
	listwidget->setUpdatesEnabled(true);
	QApplication::restoreOverrideCursor();
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

void BasicMainWindow::connectToMPD(const Host &host)
{
	stopSleepTimer();

	qApp->setOverrideCursor(Qt::WaitCursor);

	pv->ping_failed_count = 0;
	mpc()->close();
	stopStatusThread();

	pv->host = host;
	if (mpc()->open(pv->host)) {
		pv->connected = true;
		setPageConnected();
		updatePlayingStatus();
		updateTreeTopLevel();
		updatePlaylist();

		// check volume support
		pv->volume = -1;
		int vol = -1;
		for (int i = 0; i < 3; i++) {
			int v = mpc()->get_volume();
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
			mpc()->do_setvol(v);
			if (v == mpc()->get_volume()) {
				pv->volume = vol;
			}
			mpc()->do_setvol(vol);
			mpc()->do_setvol(vol);
			mpc()->do_setvol(vol);
		}
		setVolumeEnabled(pv->volume >= 0);
	} else {
		clearTreeAndList();
		setPageDisconnected();
	}

	qApp->restoreOverrideCursor();

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
			mpc()->do_add(path);
		} else {
			mpc()->do_addid(path, to);
			to++;
		}
	} else if (mpc()->do_listall(path, &mpcitems)) {
		for (mpcitem_t const &mpcitem : mpcitems) {
			if (mpcitem.kind == "file") {
				if (to < 0) {
					mpc()->do_add(mpcitem.text);
				} else {
					mpc()->do_addid(mpcitem.text, to);
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
	mpc()->do_play();
}

void BasicMainWindow::pause()
{
	mpc()->do_pause(true);
}

void BasicMainWindow::stop()
{
	mpc()->do_stop();
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
	mpc()->close();
	stopSleepTimer();
}

void BasicMainWindow::loadPlaylist(const QString &name, bool replace)
{
	if (replace) {
		mpc()->do_stop();
		mpc()->do_clear();
	}
	if (mpc()->do_load(name)) {
		updatePlaylist();
	} else {
		showError(tr("Failed to load playlist.") + '(' + mpc()->message() + ')');
	}
}

bool BasicMainWindow::savePlaylist(const QString &name)
{
	mpc()->do_rm(name);
	if (mpc()->do_save(name)) {
		return true;
	} else {
		showError(tr("Failed to save playlist.") + '(' + mpc()->message() + ')');
		return false;
	}
}

void BasicMainWindow::execAddLocationDialog()
{
	QString http = "http://";
	EditLocationDialog dlg(this);
	dlg.setLocation(http);
	if (dlg.exec() == QDialog::Accepted) {
		QStringList locs;
		QString text = dlg.location().trimmed();
		if (text.startsWith(http)) {
			locs = text.split(' ', QString::SkipEmptyParts);
		} else {
			locs = text.split('\n', QString::SkipEmptyParts);
		}
		for (QString const &loc : locs) {
			if (!loc.isEmpty()) {
				mpc()->do_add(loc);
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

void BasicMainWindow::onVolumeChanged()
{
	int v = pv->volume_popup.value();
	mpc()->do_setvol(v);
}

void BasicMainWindow::onUpdateStatus()
{
	doUpdateStatus();
}


