#include "BasicMainWindow.h"
#include "AskRemoveOverlappedFileDialog.h"
#include "ConnectionDialog.h"
#include "EditLocationDialog.h"
#include "main.h"
#include "MainWindowPrivate.h"
#include "MusicPlayerClient.h"
#include "MySettings.h"
#include "Server.h"
#include "ServersComboBox.h"
#include "SleepTimerDialog.h"
#include "TinyMainWindow.h"
#include "Toast.h"
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QMessageBox>
#include <QToolButton>
#include <set>

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
		return QString("font: %1pt \"%2\"").arg(pt).arg(name);
	};

#ifdef Q_OS_WIN
	QString default_font = font("Meiryo", 10);
	QString progress_font = font("Arial", 10);
	QString clock_font = font("Arial", 15);
#endif

#ifdef Q_OS_MAC
	QString default_font = font("Lucida Grande", 14);
	QString progress_font = font("Lucida Grande", 14);
	QString clock_font = font("Lucida Grande", 20);
#endif

#ifdef Q_OS_HAIKU
	QString default_font = font("Noto Sans", 10);
	QString progress_font = font("Noto Sans", 10);
	QString clock_font = font("Noto Sans", 14);
#endif

#ifdef Q_OS_LINUX
	QString clock_font = font("Sans Serif", 15);
	QString s;
	s += "#label_title, #label_artist, #label_disc {font-weight: bold;}";
	s += "#label_clock {%1; font-weight: bold;}";
	return s.arg(clock_font);
#else
	QString s;
	s += "* {%1;}";
	s += "#label_title, #label_artist, #label_disc {font-weight: bold;}";
	s += "#label_progress {%2;}";
	s += "#label_clock {%3; font-weight: bold;}";
	return s.arg(default_font).arg(progress_font).arg(clock_font);
#endif
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

void BasicMainWindow::execSleepTimerDialog()
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
		QString state = info.status.get("state");
		if (state == "play") {
			status = PlayingStatus::Play;
		} else if (state == "pause") {
			status = PlayingStatus::Pause;
		}

		if (status == PlayingStatus::Stop) {
			pv->total_seconds = 0;
			pv->status.now.index = -1;
			displayCurrentSongLabels(QString(), QString(), QString());
			displayStopStatus();
		} else {
			pv->status.now.index = info.status.get("song").toInt();
			pv->volume = info.status.get("volume").toInt();

			setRepeatEnabled(info.status.get("repeat").toInt() != 0);
			setSingleEnabled(info.status.get("single").toInt() != 0);
			setConsumeEnabled(info.status.get("consume").toInt() != 0);
			setRandomEnabled(info.status.get("random").toInt() != 0);

			QString prop_id = info.property.get("Id");
			QString prop_artist = info.property.get("Artist");
			QString prop_album = info.property.get("Album");
			QString prop_track = info.property.get("Track");
			QString prop_title = info.property.get("Title");
			QString prop_file = info.property.get("file");

			if (info.status.get("songid") == prop_id) {
				pv->status.now.title = prop_title;
				pv->status.now.artist = prop_artist;
				pv->status.now.track = prop_track.toInt();
				pv->status.now.disc.clear();

				if (!prop_album.isEmpty()) {
					if (pv->status.now.track > 0) {
						pv->status.now.disc = QString("Tr.") + QString::number(pv->status.now.track) + ", ";
					}
					pv->status.now.disc += prop_album;
				}
				if (pv->status.now.title.isEmpty()) {
					QString file = prop_file;
					int i = file.lastIndexOf('/');
					if (i >= 0) file = file.mid(i + 1);
					pv->status.now.title = file;
				}

				pv->total_seconds = 0;
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

	if (status != pv->status.now.status) {
		pv->status.now.status = status;
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
	pv->status.ago.index = -1;
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
		savePlaylist("_backup_before_clear_", false);
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
	if (savePlaylist("_quick_save_1_", true)) {
		showNotify(tr("Quick Save 1 was completed"));
	}
}

void BasicMainWindow::doQuickSave2()
{
	if (savePlaylist("_quick_save_2_", true)) {
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
	if (pv->status.ago != pv->status.now) { // 再生中の曲が変わった？
		pv->status.ago = pv->status.now;
		updatePlaylist();
	}
}

void BasicMainWindow::updatePlayIcon(PlayingStatus status, QToolButton *button, QAction *action)
{
	if (status == PlayingStatus::Play) {
		QString text = tr("Pause");
		QIcon icon(":/image/pause.svgz");
		button->setText(text);
		button->setToolTip(text);
		button->setIcon(icon);
		action->setText(tr("&Pause"));
		action->setIcon(icon);
	} else {
		QString text = tr("Play");
		QIcon icon(":/image/play.svgz");
		button->setText(text);
		button->setToolTip(text);
		button->setIcon(icon);
		action->setText(tr("&Play"));
		action->setIcon(icon);
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

	{
		QTime t = QTime::currentTime();
		char tmp[100];
		sprintf(tmp, "%02u:%02u", t.hour(), t.minute());
		QString text = tmp;
		updateClock(text);
	}

	displayExtraInformation(text2, text3);

	checkDisconnected();
}



void BasicMainWindow::updatePlaylist(QListWidget *listwidget, QList<MusicPlayerClient::Item> *items)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	listwidget->setUpdatesEnabled(false);
	int row = listwidget->currentRow();

	listwidget->clear();

	for (MusicPlayerClient::Item const &mpcitem : *items) {
		if (mpcitem.kind == "file") {
			QString path = mpcitem.text;
			QString text;
			QString range;
			int pos;
			if (path.indexOf("://") > 0) {
				text = path;
			} else {
				if (0) { // debug
					qDebug() << "---" << path;
					std::map<QString, QString> const &map = mpcitem.map.map;
					for (auto const &pair : map) {
						qDebug() << pair.first << "=" << pair.second;
					}
				}
				QString title = mpcitem.map.get("Title");
				QString artist = mpcitem.map.get("Artist");
				QString album = mpcitem.map.get("Album");
				pos = mpcitem.map.get("Pos").toInt();
				range = mpcitem.map.get("Range");
				text = title;
				if (text.isEmpty()) {
					int i = path.lastIndexOf('/');
					if (i < 0) {
						text = path;
					} else {
						text = path.mid(i + 1);
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
			listitem->setData(ITEM_PosRole, pos);
			listitem->setData(ITEM_PathRole, path);
			listitem->setData(ITEM_SongIdRole, id);
			if (!range.isEmpty()) listitem->setData(ITEM_RangeRole, range);
			listitem->setIcon(QIcon(":/image/notplaying.png"));
			listwidget->addItem(listitem);
		}
	}

	listwidget->setCurrentRow(row);
	listwidget->setUpdatesEnabled(true);
	QApplication::restoreOverrideCursor();
}

void BasicMainWindow::makeServersComboBox(QComboBox *cbox, const QString &firstitem, const Host &current_host)
{
	cbox->setUpdatesEnabled(false);
	cbox->clear();
	int sel = -1;
	if (!firstitem.isEmpty()) {
		cbox->addItem(firstitem);
	}
	std::vector<ServerItem> servers;
	loadPresetServers(&servers);
	for (int i = 0; i < (int)servers.size(); i++) {
		int row = cbox->count();
		QString text = servers[i].name;
		cbox->addItem(text);
		cbox->setItemData(row, text);
		if (current_host == servers[i].host) {
			sel = i;
		}
	}
	if (sel < 0) {
		QString text = ServersComboBox::makeServerText(current_host);
		sel = cbox->count();
		cbox->addItem(text);
	}
	cbox->addItem(ServersComboBox::trConnect());
	cbox->setCurrentIndex(sel);
	cbox->setUpdatesEnabled(true);
}

void BasicMainWindow::onServersComboBoxIndexChanged(QComboBox *cbox, int index)
{
	if (cbox->updatesEnabled()) {
		QString name = cbox->itemData(index).toString();
		if (name.isEmpty() && cbox->itemText(index) == ServersComboBox::trConnect()) {
			execConnectionDialog();
		} else {
			Host host;
			std::vector<ServerItem> servers;
			loadPresetServers(&servers);
			for (ServerItem const &server : servers) {
				if (name == server.name) {
					host = server.host;
				}
			}
			if (!host.isValid()) {
				host = Host(name);
			}
			connectToMPD(host);
		}
	}
}

QString BasicMainWindow::currentSongTitle() const
{
	if (pv->status.now.status == PlayingStatus::Play || pv->status.now.status == PlayingStatus::Pause) {
		return pv->status.now.title;
	}
	return QString();
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

	invalidateCurrentSongIndicator();

	startStatusThread();
}

bool BasicMainWindow::isPlaying() const
{
	return pv->status.now.status == PlayingStatus::Play;
}

void BasicMainWindow::startStatusThread()
{
	pv->status_thread.setHost(pv->host);
	pv->status_thread.start();
}

int BasicMainWindow::currentPlaylistCount()
{
	QList<MusicPlayerClient::Item> items;
	mpc()->do_playlist(&items);
	return items.size();
}

QString BasicMainWindow::textForExport(const MusicPlayerClient::Item &item)
{
	QString text;
	QString title = item.map.get("Title");
	QString range = item.map.get("Range");
	if (range.isEmpty()) {
		if (title.isEmpty()) {
			text = item.text;
		} else {
			text = title;
		}
	} else {
		text = title + "/.../" + range;
	}
	return text;
}

void BasicMainWindow::addPlaylsitToPlaylist(QString const &path, int to)
{
	int before = mpc()->current_playlist_file_count();
	mpc()->do_load(path);
	int after = mpc()->current_playlist_file_count();
	if (to >= 0 && to != before && before < after) {
		int n = after - before;
		for (int i = 0; i < n; i++) {
			mpc()->do_move(after - 1, to);
		}
	}
}

void BasicMainWindow::addToPlaylist(const QString &path, int to, bool update)
{
	if (path.isEmpty()) return;

	using mpcitem_t = MusicPlayerClient::Item;
	QList<mpcitem_t> fileitems;
	QList<MusicPlayerClient::Item> playlist;
	if (path.indexOf("://") > 0) {
		if (to < 0) {
			mpc()->do_add(path);
		} else {
			mpc()->do_addid(path, to);
			to++;
		}
	} else if (mpc()->do_listall(path, &fileitems)) {
		for (mpcitem_t const &mpcitem : fileitems) {
			if (mpcitem.kind == "file") {
				if (to < 0) {
					mpc()->do_add(mpcitem.text);
				} else {
					mpc()->do_addid(mpcitem.text, to);
					to++;
				}
			}
		}
	} else if (mpc()->do_listplaylistinfo(path, &playlist)) {
		addPlaylsitToPlaylist(path, to);
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
}

void BasicMainWindow::play(bool toggle)
{
	if (toggle) {
		if (pv->status.now.status == PlayingStatus::Play) {
			pause();
		} else {
			play();
		}
	} else {
		if (pv->status.now.status != PlayingStatus::Play) {
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

bool BasicMainWindow::validateForSavePlaylist()
{
	bool has_range = false;
	QList<MusicPlayerClient::Item> vec;
	mpc()->do_playlistinfo(QString(), &vec);
	for (MusicPlayerClient::Item const &item : vec) {
		QString range = item.map.get("Range");
		// Range属性を持つアイテムは外部プレイリストを参照しているので、たぶん保存が失敗する。
		if (!range.isEmpty()) {
			has_range = true;
		}
	}

	bool ok = true;
	if (has_range) {
		QString msg;
		msg = tr("The playlist contains the unsupported playlist item.");
		msg += '\n';
		msg += tr("This operation probably does not work as expected.");
		msg += '\n';
		msg += tr("Do you want to continue ?");
		ok = QMessageBox::warning(this, qApp->applicationName(), msg, QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok;
	}

	return ok;
}

bool BasicMainWindow::savePlaylist(const QString &name, bool warning_if_needed)
{
	bool ok = true;
	if (warning_if_needed) {
		ok = validateForSavePlaylist();
	}

	if (ok) {
		mpc()->do_rm(name);
		if (mpc()->do_save(name)) {
			return true;
		} else {
			showError(tr("Failed to save playlist.") + '(' + mpc()->message() + ')');
			return false;
		}
	}
	return false;
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

bool BasicMainWindow::isTinyMode(QObject *hint)
{
	BasicMainWindow *mw = findMainWindow(hint);
	if (dynamic_cast<TinyMainWindow *>(mw)) {
		return true;
	}
	return false;
}

void BasicMainWindow::unify()
{
	using mpcitem_t = MusicPlayerClient::Item;
	QString text;
	QList<mpcitem_t> vec;
	std::vector<int> dup;
	mpc()->do_playlistinfo(QString(), &vec);
	{
		std::for_each(vec.begin(), vec.end(), [&](mpcitem_t &item){
			item.text = textForExport(item);
		});
		std::set<QString> set;
		for (int i = 0; i < (int)vec.size(); i++) {
			QString item = textForExport(vec[i]);
			if (set.find(item) == set.end()) {
				set.insert(item);
			} else {
				dup.push_back(i);
				text += item + '\n';
			}
		}
	}
	if (dup.empty()) {
		showNotify(tr("Overlapped item was not found."));
	} else {
		AskRemoveOverlappedFileDialog dlg(this, text);
		if (dlg.exec() == QDialog::Accepted) {
			std::sort(dup.begin(), dup.end());
			int i = (int)dup.size();
			while (i > 0) {
				i--;
				int row = dup[i];
				int id = vec[row].map.get("Id").toInt();
				mpc()->do_deleteid(id);
			}
			updatePlaylist();
		}
	}
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



