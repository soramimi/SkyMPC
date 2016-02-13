#include "TinyMainWindow.h"
#include "ui_TinyMainWindow.h"
#include "AboutDialog.h"
#include "AskRemoveDuplicatedFileDialog.h"
#include "ConnectionDialog.h"
#include "EditLocationDialog.h"
#include "EditPlaylistDialog.h"
#include "main.h"
#include "TinyMainWindowPrivate.h"
#include "misc.h"
#include "MySettings.h"
#include "platform.h"
#include "SelectLocationDialog.h"
#include "SongPropertyDialog.h"
#include "Toast.h"
#include "VolumeIndicatorPopup.h"
#include "webclient.h"
#include <list>
#include <QBuffer>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTime>
#include <QTimer>
#include <QXmlStreamReader>
#include <set>
#include <string>
#include "KeyboardCustomizeDialog.h"
#include "SleepTimerDialog.h"

#define DISPLAY_TIME 0

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

class QueryInfoEvent : public QEvent {
public:
	RequestItem request_item;
	QueryInfoEvent(RequestItem const &req)
		: QEvent((QEvent::Type)EVENT_QueryInfo)
		, request_item(req)
	{
	}
};



//

class Font {
private:
	QString const name;
	int pt = 10;
public:
	Font(QString const &name, int pt)
		: name(name)
		, pt(pt)
	{
	}
	QString text() const
	{
		return "font: " + QString::number(pt) + "pt \"" + name + "\";";
	}
};


QString TinyMainWindow::makeStyleSheetText()
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


TinyMainWindow::TinyMainWindow(QWidget *parent) :
	BasicMainWindow(parent),
	ui(new Ui::TinyMainWindow)
{
	pv = new Private();
	ui->setupUi(this);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_QuitOnClose);

	setWindowState(windowState() | Qt::WindowFullScreen);

	pv->release_mouse_event = false;

	pv->status_label1 = new QLabel();
	ui->statusBar->addWidget(pv->status_label1, 1);
	pv->status_label2 = new QLabel();
	ui->statusBar->addWidget(pv->status_label2, 0);
//	pv->status_label3 = new QLabel();
//	ui->statusBar->addWidget(pv->status_label3, 0);

#if 0 //def Q_OS_WIN
	priv->folder_icon = QIcon(":/image/winfolder.png");
#else
	pv->folder_icon = QIcon(":/image/macfolder.png");
#endif

#if 0
	{
#ifdef Q_OS_WIN
		QFile file(":/TinyMainWindow_win.ss");
#endif
#ifdef Q_OS_MAC
		QFile file(":/TinyMainWindow_osx.ss");
#endif
#ifdef Q_OS_LINUX
		QFile file(":/TinyMainWindow_lin.ss");
#endif
		file.open(QFile::ReadOnly);
		QByteArray ba = file.readAll();
		file.close();
		QString ss = QString::fromUtf8(ba.data(), ba.size());
		setStyleSheet(ss);
	}
#else
	{
		QString ss = makeStyleSheetText();
		setStyleSheet(ss);
	}
#endif

#ifdef Q_OS_MAC
#else
	ui->action_help_about->setText(tr("&About SkyMPC"));
#endif

	pv->menu.addAction(ui->action_help_about);
	pv->menu.addAction(ui->action_debug);

	connect(&pv->volume_popup, SIGNAL(valueChanged()), this, SLOT(onVolumeChanged()));
	connect(&pv->status_thread, SIGNAL(onUpdate()), this, SLOT(onUpdateStatus()));

	setRepeatEnabled(false);
	setRandomEnabled(false);

	if (!start_with_shift_key) {
		Qt::WindowStates state = windowState();
		MySettings settings;

//		settings.beginGroup("TinyMainWindow");
//		bool maximized = settings.value("Maximized").toBool();
//		restoreGeometry(settings.value("Geometry").toByteArray());
//		settings.endGroup();
//		if (maximized) {
//			state |= Qt::WindowMaximized;
//			setWindowState(state);
//		}

		settings.beginGroup("Connection");
		QString addr = settings.value("Address").toString();
		int port = settings.value("Port").toInt();
		QString password = settings.value("Password").toString();
		settings.endGroup();
		pv->host = Host(addr, port);
		pv->host.setPassword(password);
	}

	qApp->installEventFilter(this);

#if 0
	pv->command_action_map["random"] = ui->action_random;
	pv->command_action_map["repeat"] = ui->action_repeat;
	pv->command_action_map["play"] = ui->action_play_always;
	pv->command_action_map["stop"] = ui->action_stop;
	pv->command_action_map["prev"] = ui->action_previous;
	pv->command_action_map["next"] = ui->action_next;
	pv->command_action_map["single"] = ui->action_single;
	pv->command_action_map["exit"] = ui->action_file_close;
	pv->command_action_map["vu"] = ui->action_volume_up;
	pv->command_action_map["vd"] = ui->action_volume_down;
	pv->command_action_map["qs1"] = ui->action_playlist_quick_save_1;
	pv->command_action_map["qs2"] = ui->action_playlist_quick_save_2;
	pv->command_action_map["ql1"] = ui->action_playlist_quick_load_1;
	pv->command_action_map["ql2"] = ui->action_playlist_quick_load_2;
	pv->command_action_map["clear"] = ui->action_playlist_clear;

	//	priv->key_command_map[Qt::Key_P] = "play";
	//	priv->key_command_map[Qt::Key_S] = "stop";
#endif
}

TinyMainWindow::~TinyMainWindow()
{
	pv->mpc.close();
	stopStatusThread();
	delete ui;
	delete pv;
}

void TinyMainWindow::updateStatusBar()
{
//	int count = ui->listWidget_playlist->count();
//	QString text1 = tr("%1 songs in playlist").arg(count);
//	pv->status_label1->setText(text1);
}

void TinyMainWindow::releaseMouseIfGrabbed()
{
	if (pv->release_mouse_event) {
		releaseMouse();
		pv->release_mouse_event = false;
	}
}

QIcon TinyMainWindow::folderIcon()
{
	return pv->folder_icon;
}

bool TinyMainWindow::execCommand(Command const &c)
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

void TinyMainWindow::closeEvent(QCloseEvent *event)
{
	if (pv->sleep_time.isValid()) {
		QString text;
		text += tr("Now sleep timer is working.") + '\n';
		text += tr("If this program is closed, the sleep timer will be canceled.") + '\n';
		text += tr("Are you sure you want to close ?");
		if (QMessageBox::warning(this, qApp->applicationName(), text, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
			event->ignore();
			return;
		}
	}

//	setWindowOpacity(0);
//	Qt::WindowStates state = windowState();
//	bool maximized = (state & Qt::WindowMaximized) != 0;
//	if (maximized) {
//		state &= ~Qt::WindowMaximized;
//		setWindowState(state);
//	}
	{
		MySettings settings;

//		settings.beginGroup("TinyMainWindow");
//		settings.setValue("Maximized", maximized);
//		settings.setValue("Geometry", saveGeometry());
//		settings.endGroup();

		settings.beginGroup("Connection");
		settings.setValue("Address", pv->host.address());
		settings.setValue("Port", pv->host.port());
		settings.setValue("Password", pv->host.password());
		settings.endGroup();
	}
	QMainWindow::closeEvent(event);
}

bool TinyMainWindow::isAutoReconnectAtStartup()
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

void TinyMainWindow::preexec()
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

	startTimer(1000);
	connectToMPD(pv->host);
}

QString makeServerText(Host const &host)
{
	QString name;
	name = host.address();
	if (name.indexOf(':') >= 0) {
		name = '[' + name + ']';
	}
	name += ':' + QString::number(host.port());
	return name;
}

QString TinyMainWindow::serverName() const
{
	return pv->host.address();
}

void TinyMainWindow::showNotify(const QString &text)
{
	Toast::show(this, text, Toast::LENGTH_MOMENT);
}

void TinyMainWindow::showError(const QString &text)
{
	Toast::show(this, text, Toast::LENGTH_LONG);
}

bool TinyMainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FocusIn) {
		QApplication::postEvent(this, new QEvent((QEvent::Type)EVENT_FocusChanged));
	}
	return QMainWindow::eventFilter(obj, event);
}

bool TinyMainWindow::event(QEvent *event)
{
	if (event->type() == EVENT_QueryInfo) {
		QueryInfoEvent *e = (QueryInfoEvent *)event;
		ResultItem item;
		item.req = e->request_item;
		if (!item.req.path.isEmpty()) {
			if (pv->mpc.do_lsinfo(item.req.path, &item.vec)) {
//				updateTree(&item);
			}
		}
		event->accept();
		return true;
	}
	return QMainWindow::event(event);
}

bool TinyMainWindow::isPlaying() const
{
	return pv->status.playing == PlayingStatus::Play;
}


void TinyMainWindow::onUpdateStatus()
{
	updatePlayingStatus();
}



void TinyMainWindow::timerEvent(QTimerEvent *)
{
	QString text2;
	QString text3;
	if (pv->connected) {
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

	if (text2.isEmpty()) {
		pv->status_label2->setVisible(false);
	} else {
		pv->status_label2->setText(text2);
		pv->status_label2->setVisible(true);
	}

//	pv->status_label3->setText(text3);

	checkDisconnected();
}

void TinyMainWindow::startStatusThread()
{
	pv->status_thread.setHost(pv->host);
	pv->status_thread.start();
}

void TinyMainWindow::stopStatusThread()
{
	pv->status_thread.requestInterruption();
	pv->status_thread.wait(1000);
}

// MPDサーバへ接続
void TinyMainWindow::connectToMPD(Host const &host)
{
	stopSleepTimer();

	pv->ping_failed_count = 0;
	pv->mpc.close();
	stopStatusThread();

	pv->host = host;
	if (pv->mpc.open(pv->host)) {
		pv->connected = true;
//		ui->stackedWidget->setCurrentWidget(ui->page_connected);

		updatePlayingStatus();

//		updateTreeTopLevel();

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
	}

	startStatusThread();
}

void TinyMainWindow::update(bool mpdupdate)
{
	if (mpdupdate) {
		pv->mpc.do_update();
	}

	updatePlaylist();
	updatePlayingStatus();
}

void TinyMainWindow::setRepeatEnabled(bool f)
{
	if (pv->repeat_enabled != f) {
		pv->repeat_enabled = f;
		ui->action_repeat->setChecked(f);
	}
}

void TinyMainWindow::setSingleEnabled(bool f)
{
	if (pv->single_enabled != f) {
		pv->single_enabled = f;
		ui->action_single->setChecked(f);
	}
}

void TinyMainWindow::setConsumeEnabled(bool f)
{
	if (pv->consume_enabled != f) {
		pv->consume_enabled = f;
		ui->action_consume->setChecked(f);
	}
}

void TinyMainWindow::setRandomEnabled(bool f)
{
	if (pv->random_enabled != f) {
		pv->random_enabled = f;
		ui->action_random->setChecked(f);
	}
}

void TinyMainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void TinyMainWindow::checkDisconnected()
{
	if (!pv->mpc.isOpen()) {
		if (pv->connected) {
			pv->connected = false;
			pv->ping_failed_count = 0;
//			clear();
		}
	}
}

void TinyMainWindow::displayProgress(double elapsed)
{
	char tmp[100];
	int e = (int)elapsed;
	int t = (int)pv->total_seconds;
	sprintf(tmp, "%u:%02u / %u:%02u", e / 60, e % 60, t / 60, t % 60);
	pv->status_label1->setText(tmp);
}

void TinyMainWindow::updatePlayingStatus()
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
			ui->label_title->clear();
			ui->label_artist->clear();
			displayProgress(0);
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
						ui->label_title->setText(title);
						ui->label_artist->setText(artist);
					}
				}
				windowtitle = title + " - " + artist;

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
		if (status == PlayingStatus::Play) {
			QString text = tr("Pause");
			ui->toolButton_play->setText(text);
			ui->toolButton_play->setToolTip(text);
			ui->toolButton_play->setIcon(QIcon(":/image/pause.svgz"));
			ui->action_play->setText(tr("&Pause"));
			ui->action_play->setIcon(QIcon(":/image/pause.svgz"));
		} else {
			QString text = tr("Play");
			ui->toolButton_play->setText(text);
			ui->toolButton_play->setToolTip(text);
			ui->toolButton_play->setIcon(QIcon(":/image/play.svgz"));
			ui->action_play->setText(tr("&Play"));
			ui->action_play->setIcon(QIcon(":/image/play.svgz"));
		}
		invalidateCurrentSongIndicator();
	}
}



void TinyMainWindow::invalidateCurrentSongIndicator()
{
	pv->status.current_song_indicator = -1;
}

static void sort(QList<MusicPlayerClient::Item> *vec)
{
	std::sort(vec->begin(), vec->end(), [](MusicPlayerClient::Item const &left, MusicPlayerClient::Item const &right){
		int i;
		i = QString::compare(left.kind, right.kind, Qt::CaseInsensitive);
		if (i == 0) {
			i = QString::compare(left.text, right.text, Qt::CaseInsensitive);
			if (i == 0) {
				QString l_title = left.map.get("Title");
				QString r_title = right.map.get("Title");
				i = QString::compare(l_title, r_title, Qt::CaseInsensitive);
			}
		}
		return i < 0;
	});
}



QString TinyMainWindow::timeText(MusicPlayerClient::Item const &item)
{
	unsigned int sec = item.map.get("Time").toUInt();
	if (sec > 0) {
		unsigned int m = sec / 60;
		unsigned int s = sec % 60;
		char tmp[100];
		sprintf(tmp, "%u:%02u", m, s);
		return tmp;
	}
	return QString();
}

bool TinyMainWindow::updatePlaylist()
{
	QList<MusicPlayerClient::Item> vec;

	if (!pv->mpc.do_playlistinfo(QString(), &vec)) {
		return false;
	}

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for (MusicPlayerClient::Item const &mpcitem : vec) {
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
					pv->mpc.do_listallinfo(path, &v);
					if (v.size() == 1) {
						text = v.front().map.get("Title");
						artist = v.front().map.get("Artist");
						album = v.front().map.get("Album");
						time = timeText(v.front());
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
			//QString pos = mpcitem.map.get("Pos");
		}
	}

	QApplication::restoreOverrideCursor();

//	updateCurrentSongIndicator();

	{ // update status bar label 1
//		int count = ui->listWidget_playlist->count();
//		QString text1 = tr("%1 songs in playlist").arg(count);
//		pv->status_label1->setText(text1);

	}

	return true;
}

void TinyMainWindow::execSongProperty(QString const &path, bool addplaylist)
{
	if (path.isEmpty()) {
		return;
	}
	std::vector<MusicPlayerClient::KeyValue> vec;
	pv->mpc.do_listallinfo(path, &vec);
	if (vec.size() > 0) {
		if (vec[0].key == "file") {
			SongPropertyDialog dlg(this, &vec, addplaylist);
			if (dlg.exec() == QDialog::Accepted) {
				if (addplaylist && dlg.addToPlaylistClicked()) {
					QString path = vec[0].value;
					pv->mpc.do_add(path);
					updatePlaylist();
				}
			}
		}
	}
}

void TinyMainWindow::clearPlaylist()
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

void TinyMainWindow::addToPlaylist(QString const &path, int to, bool update)
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

void TinyMainWindow::mouseReleaseEvent(QMouseEvent *e)
{
	releaseMouseIfGrabbed();
	QMainWindow::mouseReleaseEvent(e);
}

void TinyMainWindow::startSleepTimer(int mins)
{
	if (mins > 0) {
		QDateTime t = QDateTime::currentDateTime();
		pv->sleep_time = t.addSecs((qint64)mins * 60);
	} else {
		pv->sleep_time = QDateTime();
	}
}

void TinyMainWindow::stopSleepTimer()
{
	startSleepTimer(0);
}

void TinyMainWindow::execSleepTimerDialog()
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

void TinyMainWindow::on_toolButton_sleep_timer_clicked()
{
	execSleepTimerDialog();
}

void TinyMainWindow::onVolumeChanged()
{
	int v = pv->volume_popup.value();
	pv->mpc.do_setvol(v);
}

void TinyMainWindow::onSliderPressed()
{
	if (pv->status.playing == PlayingStatus::Play) {
		pv->mpc.do_pause(true);
	}
}

void TinyMainWindow::on_action_file_close_triggered()
{
	close();
}

void TinyMainWindow::on_action_help_about_triggered()
{
	AboutDialog dlg(this);
	dlg.exec();
}

void TinyMainWindow::play()
{
	pv->mpc.do_play();
}

void TinyMainWindow::pause()
{
	pv->mpc.do_pause(true);
}

void TinyMainWindow::stop()
{
	pv->mpc.do_stop();
	invalidateCurrentSongIndicator();
}

void TinyMainWindow::play(bool toggle)
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

void TinyMainWindow::eatMouse()
{
	grabMouse();
	pv->release_mouse_event = true;
}

void TinyMainWindow::on_action_play_triggered()
{
	play(true);
}



void TinyMainWindow::on_action_stop_triggered()
{
	stop();
}

void TinyMainWindow::on_action_previous_triggered()
{
	pv->mpc.do_previous();
}

void TinyMainWindow::on_action_next_triggered()
{
	pv->mpc.do_next();
}

void TinyMainWindow::on_action_repeat_triggered()
{
	pv->mpc.do_repeat(!pv->repeat_enabled);
}

void TinyMainWindow::on_action_random_triggered()
{
	pv->mpc.do_random(!pv->random_enabled);
}

void TinyMainWindow::on_action_single_triggered()
{
	pv->mpc.do_single(!pv->single_enabled);
}

void TinyMainWindow::on_action_consume_triggered()
{
	pv->mpc.do_consume(!pv->consume_enabled);
}

void TinyMainWindow::on_action_network_connect_triggered()
{
	ConnectionDialog dlg(this, pv->host);
	if (dlg.exec() == QDialog::Accepted) {
		Host host = dlg.host();
		connectToMPD(host);
	}
}

void TinyMainWindow::disconnectNetwork()
{
	pv->mpc.close();
	stopSleepTimer();
}

void TinyMainWindow::on_action_network_disconnect_triggered()
{
	disconnectNetwork();
}

void TinyMainWindow::on_action_network_reconnect_triggered()
{
	connectToMPD(pv->host);
	update(false);

	showNotify(tr("Reconnected"));
}

void TinyMainWindow::on_toolButton_play_clicked()
{
	ui->action_play->trigger();
}

void TinyMainWindow::on_toolButton_stop_clicked()
{
	ui->action_stop->trigger();
}

void TinyMainWindow::on_toolButton_prev_clicked()
{
	ui->action_previous->trigger();
}

void TinyMainWindow::on_toolButton_next_clicked()
{
	ui->action_next->trigger();
}

void TinyMainWindow::on_toolButton_single_clicked()
{
	ui->action_single->trigger();

}

void TinyMainWindow::on_toolButton_consume_clicked()
{
	ui->action_consume->trigger();
}

void TinyMainWindow::loadPlaylist(QString const &name, bool replace)
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

bool TinyMainWindow::savePlaylist(QString const &name)
{
	deletePlaylist(name);
	if (pv->mpc.do_save(name)) {
		return true;
	} else {
		showError(tr("Failed to save playlist.") + '(' + pv->mpc.message() + ')');
		return false;
	}
}

bool TinyMainWindow::deletePlaylist(QString const &name)
{
	if (pv->mpc.do_rm(name)) {
		return true;
	} else {
		showError(tr("Failed to delete playlist.") + '(' + pv->mpc.message() + ')');
		return false;
	}
}

void TinyMainWindow::on_action_playlist_edit_triggered()
{
	EditPlaylistDialog dlg(this, &pv->mpc);
	if (dlg.exec() != QDialog::Accepted) return;
	QString name = dlg.name();
	if (name.isEmpty()) return;
	if (!MusicPlayerClient::isValidPlaylistName(name)) {
		QMessageBox::warning(this, qApp->applicationName(), tr("The name is invalid."));
		return;
	}
	loadPlaylist(name, dlg.forReplace());
}

void TinyMainWindow::on_pushButton_manage_connections_clicked()
{
	ui->action_network_connect->trigger();
}

void TinyMainWindow::execAddLocationDialog()
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

void TinyMainWindow::on_action_playlist_add_location_triggered()
{
	execAddLocationDialog();
}

void TinyMainWindow::on_action_playlist_update_triggered()
{
	update(true);
}

void TinyMainWindow::set_volume_(int v)
{
	pv->mpc.do_setvol(v);

	QString text = QString::number(v) + '%';
	showNotify(text);
}

void TinyMainWindow::on_action_playlist_quick_save_1_triggered()
{
	if (savePlaylist("_quick_save_1_")) {
		showNotify(tr("Quick Save 1 was completed"));
	}
}

void TinyMainWindow::on_action_playlist_quick_save_2_triggered()
{
	if (savePlaylist("_quick_save_2_")) {
		showNotify(tr("Quick Save 2 was completed"));
	}
}

void TinyMainWindow::on_action_playlist_quick_load_1_triggered()
{
	loadPlaylist("_quick_save_1_", true);
}

void TinyMainWindow::on_action_playlist_quick_load_2_triggered()
{
	loadPlaylist("_quick_save_2_", true);
}

void TinyMainWindow::on_action_playlist_clear_triggered()
{
	clearPlaylist();
}

void TinyMainWindow::on_action_sleep_timer_triggered()
{
	execSleepTimerDialog();
}

void TinyMainWindow::on_action_debug_triggered()
{
}

void TinyMainWindow::on_action_edit_keyboard_customize_triggered()
{
#if 0
	KeyboardCustomizeDialog dlg(this);
	dlg.exec();
#else
	Command c("play");
	execCommand(c);
#endif
}
















void TinyMainWindow::on_toolButton_close_clicked()
{
	close();
}
