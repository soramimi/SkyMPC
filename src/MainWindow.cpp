#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MainWindowPrivate.h"
#include "AboutDialog.h"
#include "AddLocationDialog.h"
#include "AskRemoveDuplicatedFileDialog.h"
#include "ConnectionDialog.h"
#include "EditPlaylistDialog.h"
#include "main.h"
#include "misc.h"
#include "MySettings.h"
#include "SongPropertyDialog.h"
#include "VolumeIndicatorPopup.h"
#include <list>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <set>
#include <string>

#define DISPLAY_TIME 0

#ifdef WIN32
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

static QTreeWidgetItem *new_RootQTreeWidgetItem(QTreeWidget *parent)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(parent);
	item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
	item->setData(0, ITEM_IsRoot, true);
	item->setSizeHint(0, QSize(20, 20));
	return item;
}

static QTreeWidgetItem *new_QTreeWidgetItem(QTreeWidgetItem *parent)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(parent);
	item->setSizeHint(0, QSize(20, 20));
	return item;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
    impl = new Impl();
	ui->setupUi(this);

//    ui->statusBar->hide();
    impl->status_label = new QLabel();
    ui->statusBar->addWidget(impl->status_label);

	// ↓常にmacフォルダ画像を使うことにした。2015-02-06
#if 0 //def Q_OS_WIN
	priv->folder_icon = QIcon(":/image/winfolder.png");
#else
    impl->folder_icon = QIcon(":/image/macfolder.png");
#endif

//	priv->notify_overlay_window = 0;

	{
#ifdef Q_OS_WIN
        QFile file(":/MainWindow_win.ss");
#endif
#ifdef Q_OS_MAC
        QFile file(":/MainWindow_osx.ss");
#endif
#ifdef Q_OS_LINUX
        QFile file(":/MainWindow_lin.ss");
#endif
        file.open(QFile::ReadOnly);
		QByteArray ba = file.readAll();
		file.close();
		QString ss = QString::fromUtf8(ba.data(), ba.size());
		setStyleSheet(ss);
	}

#ifdef Q_OS_MAC
#else
	ui->action_help_about->setText(tr("&About SkyMPC"));
#endif

#ifdef Q_OS_WIN
    ui->menu_Help->addAction(ui->action_diagnostic);
#endif

#ifdef QT_NO_DEBUG
	ui->toolButton_menu->hide();
#else
#endif
    impl->connected = false;
    impl->total_seconds = 0;

    impl->menu.addAction(ui->action_help_about);
    impl->menu.addAction(ui->action_debug);
    ui->toolButton_menu->setMenu(&impl->menu);
    ui->toolButton_menu->setPopupMode(QToolButton::InstantPopup);

    QList<QAction*> list = ui->menuBar->actions();
    fixActionText(list);

    connect(ui->treeWidget, SIGNAL(onContextMenuEvent(QContextMenuEvent*)), this, SLOT(onTreeViewContextMenuEvent(QContextMenuEvent*)));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(on_treeWidgetItem_doubleClicked(QTreeWidgetItem*,int)));
    connect(ui->listWidget_playlist, SIGNAL(onContextMenu(QContextMenuEvent*)), this, SLOT(onListViewContextMenuEvent(QContextMenuEvent*)));
    connect(ui->listWidget_playlist, SIGNAL(onDropEvent(bool)), this, SLOT(onDropEvent(bool)));
    connect(&impl->volume_popup, SIGNAL(valueChanged()), this, SLOT(onVolumeChanged()));
    connect(ui->horizontalSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));

    impl->status.current_song = -1;
    impl->status.current_song_indicator = -1;

    impl->repeat_enabled = false;
    impl->single_enabled = false;
    impl->consume_enabled = false;
    impl->random_enabled = false;
    setRepeatEnabled(false);
    setRandomEnabled(false);

    if (!start_with_shift_key) {
        Qt::WindowStates state = windowState();
		MySettings settings;

		settings.beginGroup("MainWindow");
		bool maximized = settings.value("Maximized").toBool();
		restoreGeometry(settings.value("Geometry").toByteArray());
		ui->splitter->restoreState(settings.value("SplitterState").toByteArray());
		settings.endGroup();
		if (maximized) {
			state |= Qt::WindowMaximized;
			setWindowState(state);
		}

		settings.beginGroup("Connection");
		QString addr = settings.value("Address").toString();
		int port = settings.value("Port").toInt();
		QString password = settings.value("Password").toString();
		settings.endGroup();
        impl->host = Host(addr, port);
        impl->host.setPassword(password);
	}

	qApp->installEventFilter(this);

    impl->update_information_count = 0;
    impl->slider_down_count = 0;
    impl->notify_visible_count = 0;

    impl->command_action_map["random"] = ui->action_random;
    impl->command_action_map["repeat"] = ui->action_repeat;
    impl->command_action_map["play"] = ui->action_play_always;
    impl->command_action_map["stop"] = ui->action_stop;
    impl->command_action_map["prev"] = ui->action_previous;
    impl->command_action_map["next"] = ui->action_next;
    impl->command_action_map["single"] = ui->action_single;
    impl->command_action_map["exit"] = ui->action_file_close;
    impl->command_action_map["vu"] = ui->action_volume_up;
    impl->command_action_map["vd"] = ui->action_volume_down;
    impl->command_action_map["qs1"] = ui->action_playlist_quick_save_1;
    impl->command_action_map["qs2"] = ui->action_playlist_quick_save_2;
    impl->command_action_map["ql1"] = ui->action_playlist_quick_load_1;
    impl->command_action_map["ql2"] = ui->action_playlist_quick_load_2;
    impl->command_action_map["clear"] = ui->action_playlist_clear;

//	priv->key_command_map[Qt::Key_P] = "play";
//	priv->key_command_map[Qt::Key_S] = "stop";
}

MainWindow::~MainWindow()
{
    impl->mpc.close();
	delete ui;
    delete impl;
}

void MainWindow::setDefaultStatusBarText()
{
	int count = ui->listWidget_playlist->count();
	QString text = tr("{n} songs in playlist");
	text.replace("{n}", QString::number(count));
    impl->status_label->setText(text);
}

QIcon MainWindow::folderIcon()
{
    return impl->folder_icon;
}

bool MainWindow::execCommand(Command const &c)
{
    auto it = impl->command_action_map.find(c.command());
    if (it != impl->command_action_map.end()) {
		QAction *a = it->second;
		Q_ASSERT(a);
		a->trigger();
		return true;
	}
	return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	setWindowOpacity(0);
	Qt::WindowStates state = windowState();
	bool maximized = (state & Qt::WindowMaximized) != 0;
	if (maximized) {
		state &= ~Qt::WindowMaximized;
		setWindowState(state);
	}
	{
		MySettings settings;

		settings.beginGroup("MainWindow");
		settings.setValue("Maximized", maximized);
		settings.setValue("Geometry", saveGeometry());
		settings.setValue("SplitterState", ui->splitter->saveState());
		settings.endGroup();

		settings.beginGroup("Connection");
        settings.setValue("Address", impl->host.address());
        settings.setValue("Port", impl->host.port());
        settings.setValue("Password", impl->host.password());
		settings.endGroup();
	}
	QMainWindow::closeEvent(event);
}

#ifdef Q_OS_MAC

QString MainWindow::fixMenuText(QString const &s)
{
	return removeKeyAcceleratorText(s);
}

void MainWindow::fixActionText(QList<QAction*> &acts)
{
    for (int i = 0; i < acts.size(); i++) {
        QAction *a = acts.at(i);
		QString s = a->text();
		s = fixMenuText(s);
		a->setText(s);
		QMenu *m = a->menu();
		if (m) {
            QList<QAction*> list = m->actions();
            fixActionText(list);
		}
	}
}

#else

QString MainWindow::fixMenuText(QString const &s)
{
	return s;
}

void MainWindow::fixActionText(QList<QAction*> &)
{
}

#endif

bool MainWindow::isAutoReconnectAtStartup()
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

void MainWindow::preExec()
{
	bool conndlg = false;

	if (impl->host.isValid()) {
		if (!isAutoReconnectAtStartup()) {
			conndlg = true;
		}
	} else {
		conndlg = true;
	}

	if (conndlg) {
		ConnectionDialog dlg(this, impl->host);
		if (dlg.exec() == QDialog::Accepted) {
            impl->host = dlg.host();
		}
	}
	updateServersComboBox();

	connectToMPD(impl->host);
	startTimer(10);
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

void MainWindow::updateServersComboBox()
{
	ui->comboBox->setUpdatesEnabled(false);
	ui->comboBox->clear();
	int sel = -1;
	std::vector<ServerItem> servers;
	loadPresetServers(&servers);
	for (int i = 0; i < (int)servers.size(); i++) {
		QString text = servers[i].name;
		ui->comboBox->addItem(text);
        if (impl->host == servers[i].host) {
			sel = i;
		}
	}
	if (sel < 0) {
        QString text = makeServerText(impl->host);
		sel = ui->comboBox->count();
		ui->comboBox->addItem(text);
	}
	ui->comboBox->setCurrentIndex(sel);
	ui->comboBox->setUpdatesEnabled(true);
}

QString MainWindow::serverName() const
{
	return ui->comboBox->currentText();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FocusIn) {
		QApplication::postEvent(this, new QEvent((QEvent::Type)EVENT_FocusChanged));
	}
	return QMainWindow::eventFilter(obj, event);
}

bool MainWindow::event(QEvent *event)
{
	if (event->type() == EVENT_FocusChanged) {
		QWidget *focus = focusWidget();
		{
			ui->action_edit_copy->setEnabled(focus == ui->treeWidget || focus == ui->listWidget_playlist);
			ui->action_edit_cut->setEnabled(focus == ui->listWidget_playlist);
			ui->action_edit_paste_insert->setEnabled(focus == ui->listWidget_playlist);
			ui->action_edit_paste_bottom->setEnabled(focus == ui->listWidget_playlist);
			ui->action_edit_delete->setEnabled(focus == ui->listWidget_playlist);
		}
		event->accept();
		return true;
	} else if (event->type() == EVENT_QueryInfo) {
		QueryInfoEvent *e = (QueryInfoEvent *)event;
		ResultItem item;
		item.req = e->request_item;
		if (!item.req.path.isEmpty()) {
            if (impl->mpc.do_lsinfo(item.req.path, &item.vec)) {
				updateTree(&item);
			}
		}
		event->accept();
		return true;
	}
	return QMainWindow::event(event);
}

bool isRoot(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsRoot).toBool();
}

bool isFolder(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsFolder).toBool();
}

bool isFile(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsFile).toBool();
}

bool MainWindow::isPlaying() const
{
    return impl->status.playing == PlayingStatus::Play;
}

void MainWindow::execPrimaryCommand(QTreeWidgetItem *item)
{
	if (item && isFile(item)) {
		QString path = item->data(0, ITEM_PathRole).toString();
		if (qApp->keyboardModifiers() & Qt::AltModifier) {
			execSongProperty(path, true);
		} else {
			int i = ui->listWidget_playlist->count();
			if (i < 0) {
				i = 0;
			}
			addToPlaylist(path, -1, true);
			if (!isPlaying()) {
                impl->mpc.do_play(i);
				updatePlayingStatus();
				invalidateCurrentSongIndicator();
				updateCurrentSongIndicator();
			}
		}
	}
}

static void toggleExpandCollapse(QTreeWidgetItem *item)
{
	item->setExpanded(!item->isExpanded());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->modifiers() & Qt::ControlModifier) {
		int i = -1;
		switch (event->key()) {
		case Qt::Key_1: i = 0; break;
		case Qt::Key_2: i = 1; break;
		case Qt::Key_3: i = 2; break;
		case Qt::Key_4: i = 3; break;
		case Qt::Key_5: i = 4; break;
		case Qt::Key_6: i = 5; break;
		case Qt::Key_7: i = 6; break;
		case Qt::Key_8: i = 7; break;
		case Qt::Key_9: i = 8; break;
		case Qt::Key_0: i = 9; break;
		}
		if (i >= 0) {
			std::vector<ServerItem> servers;
			loadPresetServers(&servers);
			if (i < ui->comboBox->count()) {
				ui->comboBox->setCurrentIndex(i);
			}
			event->accept();
			return;
		}
	}
	int key = event->key();
	QWidget *focus = focusWidget();
	switch (key) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		if (focus == ui->treeWidget) {
			QTreeWidgetItem *item = ui->treeWidget->currentItem();
			if (isFile(item)) {
				execPrimaryCommand(item);
			} else if (isFolder(item)) {
				toggleExpandCollapse(item);
			}
		} else if (focus == ui->listWidget_playlist) {
			int i = ui->listWidget_playlist->currentRow();
            impl->mpc.do_play(i);
		}
		event->accept();
		return;
	case Qt::Key_Insert:
		if (focus == ui->treeWidget) {
			if (key == Qt::Key_Insert) {
				QList<QTreeWidgetItem *> items = ui->treeWidget->selectedItems();
                for (QTreeWidgetItem *item : items) {
					QString path = item->data(0, ITEM_PathRole).toString();
					if (path.isEmpty()) continue;
					std::vector<MusicPlayerClient::Item> mpcitems;
                    if (impl->mpc.do_listall(path, &mpcitems)) {
                        for (MusicPlayerClient::Item const &mpcitem : mpcitems) {
							if (mpcitem.kind == "file") {
								QString path = mpcitem.text;
                                impl->mpc.do_add(path);
							}
						}
					}
				}
				updatePlaylist();
				event->accept();
				return;
			}
		}
		event->accept();
		return;
	case Qt::Key_Escape:
		ui->treeWidget->setFocus();
		event->accept();
#ifdef Q_OS_MAC
	case Qt::Key_Delete:
		ui->action_edit_delete->trigger();
		event->accept();
		return;
#endif
	}
	{
        auto it = impl->key_command_map.find(key);
        if (it != impl->key_command_map.end()) {
			Command c(it->second);
			if (execCommand(c)) {
				event->accept();
				return;
			}
		}
	}
}

void MainWindow::timerEvent(QTimerEvent *)
{
	if (ui->horizontalSlider->isSliderDown()) {
        impl->slider_down_count = 50;
    } else if (impl->slider_down_count > 0)  {
        impl->slider_down_count--;
	} else {
        impl->slider_down_count = 0;
	}

    if (impl->update_information_count > 1) {
        impl->update_information_count--;
	} else {
        if (impl->slider_down_count == 0) {
			updatePlayingStatus();
            if (impl->status.current_song != impl->status.current_song_indicator) {
				updateCurrentSongIndicator();
			}
		}
        impl->update_information_count = 25;
	}

    if (impl->notify_visible_count > 0) {
        impl->notify_visible_count--;
    } else {
        impl->notify_visible_count--;
		setDefaultStatusBarText();
	}

}

void MainWindow::connectToMPD(Host const &host)
{
	ui->label_connecting_text->setText(tr("Connecting to MPD server: ") + host.address() + " (" + QString::number(host.port(DEFAULT_MPD_PORT)) + ")");
	ui->stackedWidget->setCurrentWidget(ui->page_connecting);

    impl->mpc.close();

    impl->host = host;
    if (impl->mpc.open(impl->host)) {
        impl->connected = true;
		ui->stackedWidget->setCurrentWidget(ui->page_connected);

		updatePlayingStatus();

		updateTreeTopLevel();
		updatePlaylist();

        impl->volume = -1;
		MusicPlayerClient::StringMap status;
        if (impl->mpc.do_status(&status)) {
			int vol = status.get("volume").toInt();
			int v = vol < 2 ? 2 : vol - 1;
            if (impl->mpc.do_setvol(v)) {
                impl->mpc.do_setvol(vol);
                impl->volume = vol;
			}
		}
        if (impl->volume < 0) {
			ui->toolButton_volume->setEnabled(false);
            ui->toolButton_volume->setToolTip(tr("Volume change is not supported"));
		} else {
			ui->toolButton_volume->setEnabled(true);
			ui->toolButton_volume->setToolTip(tr("Volume"));
		}
	} else {
		clear();
		ui->stackedWidget->setCurrentWidget(ui->page_disconnecccted);
	}
}

void MainWindow::clear()
{
	ui->treeWidget->clear();
	ui->listWidget_playlist->clear();
}

void MainWindow::setRepeatEnabled(bool f)
{
    if (impl->repeat_enabled != f) {
        impl->repeat_enabled = f;
		ui->action_repeat->setChecked(f);
		ui->toolButton_repeat->setChecked(f);
	}
}

void MainWindow::setSingleEnabled(bool f)
{
    if (impl->single_enabled != f) {
        impl->single_enabled = f;
		ui->action_single->setChecked(f);
		ui->toolButton_single->setChecked(f);
	}
}

void MainWindow::setConsumeEnabled(bool f)
{
    if (impl->consume_enabled != f) {
        impl->consume_enabled = f;
		ui->action_consume->setChecked(f);
		ui->toolButton_consume->setChecked(f);
	}
}

void MainWindow::setRandomEnabled(bool f)
{
    if (impl->random_enabled != f) {
        impl->random_enabled = f;
		ui->action_random->setChecked(f);
		ui->toolButton_random->setChecked(f);
	}
}

void MainWindow::changeEvent(QEvent *e)
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

void MainWindow::displayProgress(double elapsed)
{
	char tmp[100];
	int e = (int)elapsed;
    int t = (int)impl->total_seconds;
	sprintf(tmp, "%u:%02u / %u:%02u", e / 60, e % 60, t / 60, t % 60);
	ui->label_progress->setText(tmp);
}

void MainWindow::updatePlayingStatus()
{
	PlayingStatus::e playing = PlayingStatus::Stop;

	QString windowtitle = "SkyMPC";

    if (impl->mpc.isOpen()) {
		MusicPlayerClient::StringMap status;
		MusicPlayerClient::StringMap current;
		bool f_status, f_currentsong;
        f_status = impl->mpc.do_status(&status);
        f_currentsong = impl->mpc.do_currentsong(&current);
		if (f_status && f_currentsong) {
			QString state = status.get("state");
			if (state.isEmpty()) { // something wrong
                connectToMPD(impl->host); // reconnect
				return;
			} else if (state == "play") {
				playing = PlayingStatus::Play;
			} else if (state == "pause") {
				playing = PlayingStatus::Pause;
			}

            impl->status.current_song = status.get("song").toInt();

            impl->volume = status.get("volume").toInt();

			setRepeatEnabled(status.get("repeat").toInt() != 0);
			setSingleEnabled(status.get("single").toInt() != 0);
			setConsumeEnabled(status.get("consume").toInt() != 0);
			setRandomEnabled(status.get("random").toInt() != 0);

			if (status.get("songid") == current.get("Id")) {
				QString title = current.get("Title");
				QString artist = current.get("Artist");
				int track = current.get("Track").toInt();
				QString album = current.get("Album");
				QString disc;
				if (!album.isEmpty()) {
					if (track > 0) {
						disc = QString("Tr.") + QString::number(track) + ", ";
					}
					disc += album;
				}
				if (title.isEmpty()) {
					std::wstring file = current.get("file").toStdWString();
                    wchar_t const *p = wcsrchr(file.c_str(), L'/');
					if (p) {
						title = QString::fromUtf16((ushort const *)p + 1);
					}
				}
				{
					QString text = title + '\t' + artist + '\t' + disc;
                    if (text != impl->status.song_information) {
                        impl->status.song_information = text;
						ui->label_title->setText(title);
						ui->label_artist->setText(artist);
						ui->label_disc->setText(disc);
					}
				}
				if (playing != PlayingStatus::Stop) {
					windowtitle = title + " - " + windowtitle;
				}

				double elapsed = 0;
				{
					std::string s;
					s = status.get("time").toStdString();
					int t, e;
					if (sscanf(s.c_str(), "%d:%d", &e, &t) == 2) {
						elapsed = e;
                        impl->total_seconds = t;
					}
				}
				{
					bool ok = false;
					double e = status.get("elapsed").toDouble(&ok);
					if (ok) {
						elapsed = e;
					}
				}
				ui->horizontalSlider->setUpdatesEnabled(false);
                ui->horizontalSlider->setMaximum((int)(impl->total_seconds * 100));
				ui->horizontalSlider->setValue((int)(elapsed * 100));
				ui->horizontalSlider->setUpdatesEnabled(true);

                if (impl->status.elapsed > elapsed) {
					updatePlaylist();
				}
                impl->status.elapsed = elapsed;

				displayProgress(elapsed);
			}
		} else {
            if (!impl->mpc.ping()) {
                QString s = impl->mpc.message();
                impl->mpc.close();
			}
		}
	} else {
        if (impl->connected) {
            impl->connected = false;
			ui->stackedWidget->setCurrentWidget(ui->page_disconnecccted);
			clear();
		}
	}

    if (windowtitle != impl->status.windowtitle) {
        impl->status.windowtitle = windowtitle;
		setWindowTitle(windowtitle);
	}

    if (playing != impl->status.playing) {
        impl->status.playing = playing;
		if (playing == PlayingStatus::Play) {
			QString text = tr("Pause");
			ui->toolButton_play->setText(text);
			ui->toolButton_play->setToolTip(text);
			ui->toolButton_play->setIcon(QIcon(":/image/pause.svgz"));
			ui->action_play->setText(fixMenuText(tr("&Pause")));
			ui->action_play->setIcon(QIcon(":/image/pause.svgz"));
		} else {
			QString text = tr("Play");
			ui->toolButton_play->setText(text);
			ui->toolButton_play->setToolTip(text);
			ui->toolButton_play->setIcon(QIcon(":/image/play.svgz"));
			ui->action_play->setText(fixMenuText(tr("&Play")));
			ui->action_play->setIcon(QIcon(":/image/play.svgz"));
		}
		invalidateCurrentSongIndicator();
	}
}

void MainWindow::updateCurrentSongIndicator()
{
	if (ui->listWidget_playlist->count() > 0) {
		for (int i = 0; i < ui->listWidget_playlist->count(); i++) {
			QListWidgetItem *item = ui->listWidget_playlist->item(i);
			Q_ASSERT(item);
			QString s = ":image/notplaying.png";
            if (i == impl->status.current_song) {
                if (impl->status.playing == PlayingStatus::Play) {
					s = ":image/playing.svgz";
                } else if (impl->status.playing == PlayingStatus::Pause) {
					s = ":image/pause.png";
				}
			}
			item->setIcon(QIcon(s));
		}
        impl->status.current_song_indicator = impl->status.current_song;
	} else {
		invalidateCurrentSongIndicator();
	}
}

void MainWindow::invalidateCurrentSongIndicator()
{
    impl->status.current_song_indicator = -1;
}

static void sort(std::vector<MusicPlayerClient::Item> *vec)
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

void MainWindow::updateTreeTopLevel()
{
	ui->treeWidget->clear();
	std::vector<MusicPlayerClient::Item> vec;
    impl->mpc.do_lsinfo(QString(), &vec);
	sort(&vec);

	ui->treeWidget->setRootIsDecorated(true);
    for (MusicPlayerClient::Item const &item : vec) {
		if (item.kind == "directory") {
			QString path = item.text;
			QTreeWidgetItem *treeitem = new_RootQTreeWidgetItem(ui->treeWidget);
			treeitem->setText(0, path);
			treeitem->setIcon(0, folderIcon());
			treeitem->setData(0, ITEM_IsFolder, true);
			treeitem->setData(0, ITEM_PathRole, path);
			QTreeWidgetItem *g = new_QTreeWidgetItem(treeitem);
			g->setText(0, "Reading...");
			treeitem->addChild(g);
			ui->treeWidget->addTopLevelItem(treeitem);
		}
	}
}

QString timeText(MusicPlayerClient::Item const &item)
{
	int sec = item.map.get("Time").toInt();
	if (sec > 0) {
		int m = sec / 60;
		int s = sec % 60;
		char tmp[100];
		sprintf(tmp, "%u:%02u", m, s);
		return tmp;
	}
	return QString();
}

void MainWindow::updatePlaylist()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	ui->listWidget_playlist->setUpdatesEnabled(false);

	ui->listWidget_playlist->clear();

	std::vector<MusicPlayerClient::Item> vec;
    impl->mpc.do_playlistinfo(QString(), &vec);

    for (MusicPlayerClient::Item const &mpcitem : vec) {
		if (mpcitem.kind == "file") {
			QString path = mpcitem.text;
			QString text;
			QString artist;
			QString album;
			QString time;
			{
				std::vector<MusicPlayerClient::Item> v;
                impl->mpc.do_listallinfo(path, &v);
				if (v.size() == 1) {
					text = v.front().map.get("Title");
					artist = v.front().map.get("Artist");
					album = v.front().map.get("Album");
					time = timeText(v.front());
				}
			}
			QString dir;
			if (text.isEmpty()) {
				ushort const *str = path.utf16();
				ushort const *ptr = ucsrchr(str, '/');
				if (ptr) {
					dir = QString::fromUtf16(str, ptr - str);
					text = QString::fromUtf16(ptr + 1);
				}
			}
			QString suffix;
			if (!artist.isEmpty() && !album.isEmpty()) {
				suffix = artist + '/' + album;
			} else if (!artist.isEmpty()) {
				suffix = artist;
			} else if (!album.isEmpty()) {
				suffix = album;
			} else if (!dir.isEmpty()){
				suffix = dir;
			}
#if DISPLAY_TIME
			if (!time.isEmpty()) {
				text += " (" + time + ")";
			}
#endif
			if (!suffix.isEmpty()) {
				text += " -- " + suffix;
			}
			QString id = mpcitem.map.get("Id");
			QString pos = mpcitem.map.get("Pos");
			QListWidgetItem *listitem = new QListWidgetItem();
			listitem->setText(text);
			listitem->setData(ITEM_PathRole, path);
			listitem->setData(ITEM_SongIdRole, id);
			listitem->setIcon(QIcon(":/image/notplaying.png"));
			ui->listWidget_playlist->addItem(listitem);
		}
	}

	ui->listWidget_playlist->setUpdatesEnabled(true);
	QApplication::restoreOverrideCursor();

	updateCurrentSongIndicator();

	setDefaultStatusBarText();
}

void MainWindow::updateTree(ResultItem *info)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	ui->treeWidget->setUpdatesEnabled(false);

	QTreeWidgetItem *treeitem = ui->treeWidget->itemFromIndex(info->req.index);
	if (treeitem) {
		{
			int i = treeitem->childCount();
			while (i > 0) {
				i--;
				delete treeitem->takeChild(i);
			}
		}
		sort(&info->vec);
        for (MusicPlayerClient::Item const &mpcitem : info->vec) {
			ushort const *str = mpcitem.text.utf16();
			ushort const *ptr = ucsrchr(str, '/');
			if (ptr) {
				if (mpcitem.kind == "directory") {
					QTreeWidgetItem *child = new_QTreeWidgetItem(treeitem);
					child->setText(0, QString::fromUtf16(ptr + 1));
					child->setIcon(0, folderIcon());
					child->setData(0, ITEM_IsFolder, true);
					child->setData(0, ITEM_PathRole, mpcitem.text);
					QTreeWidgetItem *g = new_QTreeWidgetItem(child);
					g->setText(0, "Reading...");
					child->addChild(g);
					treeitem->addChild(child);
				} else if (mpcitem.kind == "file") {
					QString path = mpcitem.text;
					QString text;
					std::vector<MusicPlayerClient::Item> v;
                    if (impl->mpc.do_listallinfo(path, &v)) {
						if (v.size() == 1) {
							text = v.front().map.get("Title");
							if (text.isEmpty()) {
								int i = path.lastIndexOf('/');
								if (i < 0) {
									text = path;
								} else {
									text = QString::fromUtf16(path.utf16() + i + 1);
								}
							}
#if DISPLAY_TIME
							QString time = timeText(v.front());
							if (!time.isEmpty()) {
								text += " (" + time + ")";
							}
#endif
						}
					}
					if (text.isEmpty()) {
						ushort const *str = path.utf16();
						ushort const *ptr = ucsrchr(str, '/');
						if (ptr) {
							text = QString::fromUtf16(ptr + 1);
						}
					}
					QTreeWidgetItem *child = new_QTreeWidgetItem(treeitem);
					child->setText(0, text);
					child->setIcon(0, QIcon(":/image/song.png"));
					child->setData(0, ITEM_IsFile, true);
					child->setData(0, ITEM_PathRole, path);
					treeitem->addChild(child);
				}
			}
		}
		treeitem->setExpanded(true);
	}

	ui->treeWidget->setUpdatesEnabled(true);
	QApplication::restoreOverrideCursor();
}

static bool isPlaceHolder(QTreeWidgetItem *item)
{
	if (item->childCount() == 1) {
		QTreeWidgetItem *child = item->child(0);
		if (child->data(0, ITEM_PathRole).isNull()) {
			return true;
		}
	}
	return false;
}

void MainWindow::refreshTreeItem(QTreeWidgetItem *item)
{
	QString path = item->data(0, ITEM_PathRole).toString();
	QModelIndex index = ui->treeWidget->indexFromItem(item);
	QApplication::postEvent(this, new QueryInfoEvent(RequestItem(path, index)));
}

void MainWindow::on_treeWidget_itemExpanded(QTreeWidgetItem *item)
{
	if (isPlaceHolder(item)) {
		refreshTreeItem(item);
	}
}

void MainWindow::deleteSelectedSongs()
{
	int row = ui->listWidget_playlist->currentRow();

	QList<QListWidgetItem *> list = ui->listWidget_playlist->selectedItems();
	for (int i = 0; i < list.size(); i++) {
		QListWidgetItem *item = list.at(i);
		int id = item->data(ITEM_SongIdRole).toInt();
        impl->mpc.do_deleteid(id);
	}
	updatePlaylist();

	int count = ui->listWidget_playlist->count();
	if (count <= row) {
		row = count - 1;
	}
	if (row >= 0) {
		ui->listWidget_playlist->setCurrentRow(row);
		updateCurrentSongIndicator();
	}
}

void MainWindow::execSongProperty(QString const &path, bool addplaylist)
{
	if (path.isEmpty()) {
		return;
	}
	std::vector<MusicPlayerClient::KeyValue> vec;
    impl->mpc.do_listallinfo(path, &vec);
	if (vec.size() > 0) {
		if (vec[0].key == "file") {
			SongPropertyDialog dlg(this, &vec, addplaylist);
			if (dlg.exec() == QDialog::Accepted) {
				if (addplaylist && dlg.addToPlaylistClicked()) {
					QString path = vec[0].value;
                    impl->mpc.do_add(path);
					updatePlaylist();
				}
			}
		}
	}
}

void MainWindow::clearPlaylist()
{
	int count = 0;
	std::vector<MusicPlayerClient::Item> vec;
    impl->mpc.do_playlistinfo(QString(), &vec);
    for (MusicPlayerClient::Item const &item : vec) {
		if (item.kind == "file") {
			count++;
		}
	}
	if (count > 0) {
		savePlaylist("_backup_before_clear_");
	}
    impl->mpc.do_clear();
	updatePlaylist();
}

void MainWindow::onTreeViewContextMenuEvent(QContextMenuEvent *)
{
	QPoint pos = ui->treeWidget->mapFromGlobal(QCursor::pos());
	QTreeWidgetItem *treeitem = ui->treeWidget->itemAt(pos);
	if (!treeitem) return;
	QMenu menu;
	QAction a_AddToPlaylist(tr("Add to play list"), 0);
	QAction a_Property(tr("Property"), 0);
	menu.addAction(&a_AddToPlaylist);
	menu.addAction(&a_Property);
	QAction *act = menu.exec(QCursor::pos() + QPoint(8, -8));
	QString path = treeitem->data(0, ITEM_PathRole).toString();
	if (act == &a_AddToPlaylist) {
		addToPlaylist(path, -1, true);
	} else if (act == &a_Property) {
		execSongProperty(path, true);
	}
}

void MainWindow::onListViewContextMenuEvent(QContextMenuEvent *)
{
	QPoint pos = ui->listWidget_playlist->mapFromGlobal(QCursor::pos());
	if (ui->listWidget_playlist->selectedItems().isEmpty()) {
		return;
	}
	QMenu menu;
	QAction a_PlayFromHere(tr("Play from here"), 0);
	QAction a_Cut(tr("Cut"), 0);
	QAction a_Copy(tr("Copy"), 0);
	QAction a_PasteInsert(tr("Paste (Insert)"), 0);
	QAction a_PasteAppend(tr("Paste (Append)"), 0);
	QAction a_Delete(tr("Delete"), 0);
	QAction a_Clear(tr("Clear play list"), 0);
	QAction a_Property(tr("Property"), 0);
	menu.addAction(&a_PlayFromHere);
	menu.addSeparator();
	menu.addAction(&a_Cut);
	menu.addAction(&a_Copy);
	menu.addAction(&a_PasteInsert);
	menu.addAction(&a_PasteAppend);
	menu.addAction(&a_Delete);
	menu.addAction(&a_Clear);
	menu.addSeparator();
	menu.addAction(&a_Property);
	QAction *act = menu.exec(QCursor::pos() + QPoint(8, -8));
	if (act == &a_PlayFromHere) {
		int i = ui->listWidget_playlist->currentIndex().row();
		if (i >= 0) {
            impl->mpc.do_play(i);
		}
	} else if (act == &a_Cut) {
		ui->action_edit_cut->trigger();
	} else if (act == &a_Copy) {
		ui->action_edit_copy->trigger();
	} else if (act == &a_PasteInsert) {
		ui->action_edit_paste_insert->trigger();
	} else if (act == &a_PasteAppend) {
		ui->action_edit_paste_bottom->trigger();
	} else if (act == &a_Delete) {
		deleteSelectedSongs();
		return;
	} else if (act == &a_Clear) {
		clearPlaylist();
	} else if (act == &a_Property) {
		int i = ui->listWidget_playlist->currentIndex().row();
		QListWidgetItem *listitem = ui->listWidget_playlist->item(i);
		if (listitem) {
			QString path = listitem->data(ITEM_PathRole).toString();
			execSongProperty(path, false);
		}
	}
}

void MainWindow::addToPlaylist(QString const &path, int to, bool update)
{
	if (path.isEmpty()) return;

	std::vector<MusicPlayerClient::Item> mpcitems;
    if (impl->mpc.do_listall(path, &mpcitems)) {
        for (MusicPlayerClient::Item const &mpcitem : mpcitems) {
			if (mpcitem.kind == "file") {
				if (to < 0) {
                    impl->mpc.do_add(mpcitem.text);
				} else {
                    impl->mpc.do_addid(mpcitem.text, to);
					to++;
				}
			}
		}
	}
	if (update) {
		updatePlaylist();
	}
}

void MainWindow::onDropEvent(bool done)
{
	if (!done) {
        impl->drop_before.clear();
		int n = ui->listWidget_playlist->count();
		for (int i = 0; i < n; i++) {
			QListWidgetItem *listitem = ui->listWidget_playlist->item(i);
			listitem->setData(ITEM_RowRole, i);
			QString path = listitem->data(ITEM_PathRole).toString();
            impl->drop_before.push_back(SongItem(i, path));
		}
	} else {
		std::vector<SongItem> drop_after;
		int n = ui->listWidget_playlist->count();
		for (int i = 0; i < n; i++) {
			QListWidgetItem *listitem = ui->listWidget_playlist->item(i);
			bool ok = false;
			int row = listitem->data(ITEM_RowRole).toInt(&ok);
			if (!ok) row = -1;
			QString path = listitem->data(ITEM_PathRole).toString();
			if (path.isEmpty()) continue;
			drop_after.push_back(SongItem(row, path));
		}
        std::vector<SongItem> drop_before = impl->drop_before;
		for (size_t i = 0; i < drop_after.size(); i++) {
			SongItem a = drop_after[i];
			if (a.index == -1) {
				std::vector<MusicPlayerClient::Item> mpcitems;
                if (impl->mpc.do_listall(a.path, &mpcitems)) {
					size_t j = i;
                    for (MusicPlayerClient::Item const &item : mpcitems) {
						if (item.kind == "file") {
							QString path = item.text;
                            impl->mpc.do_addid(path, (int)j);
							SongItem item(-1, path);
							drop_before.insert(drop_before.begin() + j, item);
							j++;
						}
					}
				}
			} else if (i < drop_before.size() && a.index != drop_before[i].index) {
				for (size_t j = i + 1; j < drop_before.size(); j++) {
					if (a.index == drop_before[j].index) {
						std::swap(drop_before[i], drop_before[j]);
                        impl->mpc.do_swap(i, j);
						break;
					}
				}
			}
		}
		updatePlaylist();
	}
}

void MainWindow::on_treeWidgetItem_doubleClicked(QTreeWidgetItem *item, int /*column*/)
{
	if (isFile(item)) {
		execPrimaryCommand(item);
	} else if (isFolder(item)) {
		toggleExpandCollapse(item);
	} else if (isRoot(item)) {
		item->setExpanded(true); // always open
	}
}

void MainWindow::on_listWidget_playlist_doubleClicked(const QModelIndex &index)
{
	if (qApp->keyboardModifiers() & Qt::AltModifier) {
		int i = index.row();
		QListWidgetItem *item = ui->listWidget_playlist->item(i);
		if (item) {
			QString path = item->data(ITEM_PathRole).toString();
			execSongProperty(path, false);
		}
	} else {
		int i = index.row();
		if (i >= 0) {
            impl->mpc.do_play(i);
		}
        invalidateCurrentSongIndicator();
    }
}

void MainWindow::on_toolButton_volume_clicked()
{
    if (impl->volume < 0) {
		return; // setvol not supported
	}
	QRect rc = ui->toolButton_volume->frameGeometry();
	QPoint pt = ((QWidget *)ui->toolButton_volume->parent())->mapToGlobal(rc.bottomRight());
    pt.rx() -= impl->volume_popup.width();
    impl->volume_popup.setValue(impl->volume);
    impl->volume_popup.move(pt);
    impl->volume_popup.show();
}

void MainWindow::onVolumeChanged()
{
    int v = impl->volume_popup.value();
    impl->mpc.do_setvol(v);
}

void MainWindow::onSliderPressed()
{
    if (impl->status.playing == PlayingStatus::Play) {
        impl->mpc.do_pause(true);
	}
}

void MainWindow::onSliderReleased()
{
	int pos = ui->horizontalSlider->value() / 100;
	{
        impl->mpc.do_seek(impl->status.current_song, pos);
        if (impl->status.playing == PlayingStatus::Play) {
            impl->mpc.do_pause(false);
		}
	}
    impl->slider_down_count = 0;
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
	displayProgress(value / 100.0);
}

void MainWindow::on_action_file_close_triggered()
{
	close();
}

void MainWindow::on_action_help_about_triggered()
{
	AboutDialog dlg(this);
	dlg.exec();
}

void MainWindow::play(bool toggle)
{
	if (toggle) {
        if (impl->status.playing == PlayingStatus::Play) {
            impl->mpc.do_pause(true);
		} else {
            impl->mpc.do_play();
		}
	} else {
        if (impl->status.playing != PlayingStatus::Play) {
            impl->mpc.do_play();
		}
	}
}

void MainWindow::on_action_play_triggered()
{
	play(true);
}

void MainWindow::on_action_stop_triggered()
{
    impl->mpc.do_stop();
	invalidateCurrentSongIndicator();
}

void MainWindow::on_action_previous_triggered()
{
    impl->mpc.do_previous();
}

void MainWindow::on_action_next_triggered()
{
    impl->mpc.do_next();
}

void MainWindow::on_action_repeat_triggered()
{
    impl->mpc.do_repeat(!impl->repeat_enabled);
}

void MainWindow::on_action_random_triggered()
{
    impl->mpc.do_random(!impl->random_enabled);
}

void MainWindow::on_action_single_triggered()
{
    impl->mpc.do_single(!impl->single_enabled);
}

void MainWindow::on_action_consume_triggered()
{
    impl->mpc.do_consume(!impl->consume_enabled);
}

void MainWindow::on_action_network_connect_triggered()
{
    ConnectionDialog dlg(this, impl->host);
	if (dlg.exec() == QDialog::Accepted) {
		Host host = dlg.host();
		connectToMPD(host);
	}
	updateServersComboBox();
}

void MainWindow::on_action_network_disconnect_triggered()
{
    impl->mpc.close();
}

void MainWindow::on_toolButton_play_clicked()
{
	ui->action_play->trigger();
}

void MainWindow::on_toolButton_stop_clicked()
{
	ui->action_stop->trigger();
}

void MainWindow::on_toolButton_prev_clicked()
{
	ui->action_previous->trigger();
}

void MainWindow::on_toolButton_next_clicked()
{
	ui->action_next->trigger();
}

void MainWindow::on_toolButton_repeat_clicked()
{
	ui->action_repeat->trigger();
}

void MainWindow::on_toolButton_random_clicked()
{
	ui->action_random->trigger();
}

void MainWindow::on_toolButton_single_clicked()
{
	ui->action_single->trigger();

}

void MainWindow::on_toolButton_consume_clicked()
{
	ui->action_consume->trigger();
}

//

bool isValidPlaylistName(QString const &name)
{
	if (name.isEmpty()) return false;
	ushort const *p = (ushort const *)name.data();
	while (*p) {
		if (*p < 0x20) return false;
		if (*p < 0x80 && strchr("\"\\/?|<>", *p)) return false;
		p++;
	}
	return true;
}

void MainWindow::loadPlaylist(QString const &name, bool replace)
{
	if (replace) {
        impl->mpc.do_stop();
        impl->mpc.do_clear();
	}
    impl->mpc.do_load(name);
	updatePlaylist();
}

void MainWindow::savePlaylist(QString const &name)
{
	deletePlaylist(name);
    impl->mpc.do_save(name);
}

void MainWindow::deletePlaylist(QString const &name)
{
    impl->mpc.do_rm(name);
}

void MainWindow::on_action_playlist_edit_triggered()
{
    EditPlaylistDialog dlg(this, &impl->mpc);
	if (dlg.exec() != QDialog::Accepted) return;
	QString name = dlg.name();
	if (name.isEmpty()) return;
	if (!isValidPlaylistName(name)) {
		QMessageBox::warning(this, qApp->applicationName(), tr("The name is invalid."));
		return;
	}
	loadPlaylist(name, dlg.forReplace());
}

void MainWindow::on_action_edit_cut_triggered()
{
	QWidget *focus = focusWidget();
	if (focus == ui->listWidget_playlist) {
		on_action_edit_copy_triggered();
		on_action_edit_delete_triggered();
	}
}


void MainWindow::on_action_edit_copy_triggered()
{
	QStringList list;
	QWidget *focus = focusWidget();
	if (focus == ui->treeWidget) {
		QList<QTreeWidgetItem *> items = ui->treeWidget->selectedItems();
        for (QTreeWidgetItem const *item : items) {
			QString path = item->data(0, ITEM_PathRole).toString();
			list.append(path);
		}
	} else if (focus == ui->listWidget_playlist) {
		QList<QListWidgetItem *> items = ui->listWidget_playlist->selectedItems();
        for (QListWidgetItem const *item : items) {
			QString path = item->data(ITEM_PathRole).toString();
			list.append(path);
		}
	}

	QString text;
	for (int i = 0; i < list.size(); i++) {
		text += list[i] + '\n';
	}

	QClipboard *cb = qApp->clipboard();

	cb->setText(text);
}

void MainWindow::on_action_edit_paste_bottom_triggered()
{
	QWidget *focus = focusWidget();
	if (focus == ui->listWidget_playlist) {
		QClipboard *cb = qApp->clipboard();
		QString text = cb->text();
		QStringList list = text.split('\n');
        for (QString const &str : list) {
			QString path = str.trimmed();
			addToPlaylist(path, -1, false);
		}
		updatePlaylist();
	}
}

void MainWindow::on_action_edit_paste_insert_triggered()
{
	QWidget *focus = focusWidget();
	if (focus == ui->listWidget_playlist) {
		int row = ui->listWidget_playlist->currentRow();
		if (row < 0) row = 0;
		QClipboard *cb = qApp->clipboard();
		QString text = cb->text();
		QStringList list = text.split('\n');
        for (QString const &str : list) {
			QString path = str.trimmed();
			addToPlaylist(path, row, false);
			row++;
		}
		updatePlaylist();
	}
}

void MainWindow::on_action_edit_delete_triggered()
{
	QWidget *focus = focusWidget();
	if (focus == ui->listWidget_playlist) {
		deleteSelectedSongs();
	}
}

void MainWindow::on_pushButton_manage_connections_clicked()
{
	ui->action_network_connect->trigger();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
	if (ui->comboBox->updatesEnabled()) {
		QString name = ui->comboBox->itemText(index);
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

void MainWindow::on_action_playlist_add_location_triggered()
{
	AddLocationDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {
		QString loc = dlg.location().trimmed();
		if (loc.isEmpty()) {
			QMessageBox::warning(this, QApplication::applicationName(), tr("Location is empty."));
		} else {
            impl->mpc.do_add(loc);
			updatePlaylist();
		}
	}
}

void MainWindow::on_action_playlist_update_triggered()
{
    impl->mpc.do_update();
	updateTreeTopLevel();
	updatePlaylist();
	updatePlayingStatus();
	updateCurrentSongIndicator();
}


void MainWindow::on_action_playlist_unify_triggered()
{
	QString text;
	std::vector<MusicPlayerClient::Item> vec;
	std::set<MusicPlayerClient::Item> set;
	std::vector<int> dup;
    impl->mpc.do_playlistinfo(QString(), &vec);
	for (int i = 0; i < (int)vec.size(); i++) {
		MusicPlayerClient::Item &item = vec[i];
		if (set.find(item) == set.end()) {
			set.insert(item);
		} else {
			dup.push_back(i);
			text += item.text + '\n';
		}
	}
	if (dup.empty()) {
		QMessageBox::information(this, qApp->applicationName(), tr("Overlapped item was not found."));
	} else {
		AskRemoveDuplicatedFileDialog dlg(this, text);
		if (dlg.exec() == QDialog::Accepted) {
			std::sort(dup.begin(), dup.end());
			int i = (int)dup.size();
			while (i > 0) {
				i--;
				int row = dup[i];
				QListWidgetItem *item = ui->listWidget_playlist->item(row);
				Q_ASSERT(item);
				int id = item->data(ITEM_SongIdRole).toInt();
                impl->mpc.do_deleteid(id);
			}
			updatePlaylist();
			updateCurrentSongIndicator();
		}
	}
}

void MainWindow::showNotify(QString const &text)
{
    impl->status_label->setText(text);
    impl->notify_visible_count = 200;
}

void MainWindow::setVolume_(int v)
{
    impl->mpc.do_setvol(v);

    QString text = QString::number(v) + '%';
    showNotify(text);
}

void MainWindow::on_action_volume_up_triggered()
{
    int v = impl->volume;
	if (v >= 0) {
		if (v < 100) {
			v++;
		}
		setVolume_(v);
	}
}

void MainWindow::on_action_volume_down_triggered()
{
    int v = impl->volume;
	if (v >= 0) {
		if (v > 0) {
			v--;
		}
		setVolume_(v);
	}
}

void MainWindow::on_action_playlist_quick_save_1_triggered()
{
	savePlaylist("_quick_save_1_");
    showNotify(tr("Quick Save 1 was completed."));
}

void MainWindow::on_action_playlist_quick_save_2_triggered()
{
	savePlaylist("_quick_save_2_");
    showNotify(tr("Quick Save 2 was completed."));
}

void MainWindow::on_action_playlist_quick_load_1_triggered()
{
	loadPlaylist("_quick_save_1_", true);
}

void MainWindow::on_action_playlist_quick_load_2_triggered()
{
	loadPlaylist("_quick_save_2_", true);
}

void MainWindow::on_action_playlist_clear_triggered()
{
	clearPlaylist();
}

void MainWindow::on_action_debug_triggered()
{
}

void MainWindow::on_action_play_always_triggered()
{
	play(false);
}

#include "KeyboardCustomizeDialog.h"


void MainWindow::on_action_edit_keyboard_customize_triggered()
{
#if 0
	KeyboardCustomizeDialog dlg(this);
	dlg.exec();
#else
	Command c("play");
	execCommand(c);
#endif
}

#include "platform.h"

void MainWindow::on_action_diagnostic_triggered()
{
	diagnostic();
}

void MainWindow::on_action_test_triggered()
{
}

