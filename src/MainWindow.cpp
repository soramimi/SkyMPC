#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AboutDialog.h"
#include "BasicMainWindow.h"
#include "Common.h"
#include "ConnectionDialog.h"
#include "EditLocationDialog.h"
#include "EditPlaylistDialog.h"
#include "main.h"
#include "MainWindowPrivate.h"
#include "MySettings.h"
#include "Server.h"
#include "SongPropertyDialog.h"
#include "SettingsDialog.h"
#include "ApplicationGlobal.h"
#include <QClipboard>
#include <QKeyEvent>
#include <QMessageBox>

#define DISPLAY_TIME 0

struct ResultItem {
	RequestItem req;
	QList<MusicPlayerClient::Item> vec;
};

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
	BasicMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m->release_mouse_event = false;

	m->status_label1 = new StatusLabel();
	m->status_label2 = new StatusLabel();
	m->status_label3 = new StatusLabel();
	ui->statusBar->addWidget(m->status_label1, 1);
	ui->statusBar->addWidget(m->status_label2, 0);
	ui->statusBar->addWidget(m->status_label3, 0);

	QFormLayout *layout = static_cast<QFormLayout *>(ui->widget_information_area->layout());
	layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

#if 0 //def Q_OS_WIN
	priv->folder_icon = QIcon(":/image/winfolder.png");
#else
	m->folder_icon = QIcon(":/image/macfolder.png");
#endif

	m->song_icon = QIcon(":/image/song.png");
	m->playlist_icon = QIcon(":/image/playlist.png");

#if 0
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
#else
	{
		QString ss = makeStyleSheetText();
		setStyleSheet(ss);
	}
#endif

#ifdef Q_OS_MAC
#else
	ui->action_help_about->setText(tr("&About %1").arg(qApp->applicationName()));
#endif

#ifdef QT_NO_DEBUG
	ui->toolButton_menu->hide();
#else
#endif
	m->menu.addAction(ui->action_help_about);
	m->menu.addAction(ui->action_debug);
	ui->toolButton_menu->setMenu(&m->menu);
	ui->toolButton_menu->setPopupMode(QToolButton::InstantPopup);

	connect(ui->menu_playlist_load, &QMenu::aboutToShow, [&](){
		updatePlaylistMenu();
	});
	connect(ui->menu_playlist_load, &QMenu::triggered, [&](QAction *action){
		loadPlaylist(action->text(), true);
	});

	connect(ui->treeWidget, SIGNAL(onContextMenuEvent(QContextMenuEvent*)), this, SLOT(onTreeViewContextMenuEvent(QContextMenuEvent*)));
	connect(ui->listWidget_playlist, SIGNAL(onContextMenu(QContextMenuEvent*)), this, SLOT(onListViewContextMenuEvent(QContextMenuEvent*)));
	connect(ui->listWidget_playlist, SIGNAL(onDropEvent(bool)), this, SLOT(onDropEvent(bool)));
	connect(ui->horizontalSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
	connect(ui->horizontalSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));

	setRepeatEnabled(false);
	setRandomEnabled(false);

	MySettings settings;
	if (!global->start_with_shift_key && m->appsettings.remember_and_restore_window_position) {
		Qt::WindowStates state = windowState();

		settings.beginGroup("MainWindow");
		bool maximized = settings.value("Maximized").toBool();
		restoreGeometry(settings.value("Geometry").toByteArray());
		ui->splitter->restoreState(settings.value("SplitterState").toByteArray());
		settings.endGroup();
		if (maximized) {
			state |= Qt::WindowMaximized;
			setWindowState(state);
		}
	}
	{
		settings.beginGroup("Connection");
		QString addr = settings.value("Address").toString();
		int port = settings.value("Port").toInt();
		QString password = settings.value("Password").toString();
		settings.endGroup();
		m->host = Host(addr, port);
		m->host.setPassword(password);
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

	ui->treeWidget->setFocus();
}

MainWindow::~MainWindow()
{
	delete ui;
}

QString MainWindow::songPath(QTreeWidgetItem const *item) const
{
	return item->data(0, ITEM_PathRole).toString();
}

QString MainWindow::songPath(QListWidgetItem const *item, bool for_export) const
{
	QString text = item->data(ITEM_PathRole).toString();
	if (for_export) {
		QString range = item->data(ITEM_RangeRole).toString();
		if (!range.isEmpty()) {
			text += "/.../";
			text += "range=" + range;
		}
	}
	return text;
}

QIcon MainWindow::folderIcon()
{
	return m->folder_icon;
}

QIcon MainWindow::songIcon()
{
	return m->song_icon;
}

QIcon MainWindow::playlistIcon()
{
	return m->playlist_icon;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (m->sleep_time.isValid()) {
		QString text;
		text += tr("Now sleep timer is working.") + '\n';
		text += tr("If this program is closed, the sleep timer will be canceled.") + '\n';
		text += tr("Are you sure you want to close ?");
		if (QMessageBox::warning(this, qApp->applicationName(), text, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes) {
			event->ignore();
			return;
		}
	}

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
		settings.setValue("Address", m->host.address());
		settings.setValue("Port", m->host.port());
		settings.setValue("Password", m->host.password());
		settings.endGroup();
	}
	QMainWindow::closeEvent(event);
}

QComboBox *MainWindow::serversComboBox()
{
	if (ui->stackedWidget->currentWidget() == ui->page_connected) {
		return ui->comboBox_servers1;
	}
	return ui->comboBox_servers2;
}

void MainWindow::updateServersComboBox()
{
	ui->comboBox_servers1->resetContents(m->host, false, true);
	ui->comboBox_servers2->resetContents(m->host, true, true);
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
			if (mpc()->do_lsinfo(item.req.path, &item.vec)) {
				updateTree(&item);
			}
		}
		event->accept();
		return true;
	}
	return QMainWindow::event(event);
}

bool MainWindow::isRoot(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsRoot).toBool();
}

bool MainWindow::isFolder(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsFolder).toBool();
}

bool MainWindow::isFile(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsFile).toBool();
}

bool MainWindow::isPlaylist(QTreeWidgetItem *item)
{
	return item && item->data(0, ITEM_IsPlaylist).toBool();
}

int MainWindow::playlistFileCount() const
{
	return ui->listWidget_playlist->count();
}

void MainWindow::execPrimaryCommand(QTreeWidgetItem *item)
{
	bool altmod = (qApp->keyboardModifiers() & Qt::AltModifier) != 0;

	QString path = songPath(item);

	if (isFile(item)) {
		if (altmod) {
			execSongProperty(path, -1, true);
		} else {
			int i = playlistFileCount();
			if (i < 0) {
				i = 0;
			}
			addToPlaylist(path, -1, true);
			if (!isPlaying()) {
				mpc()->do_play(i);
				invalidateCurrentSongIndicator();
				updateCurrentSongInfo();
			}
		}
	} else if (isPlaylist(item)) {
		if (altmod) {
			execPlaylistPropertyDialog(path);
		} else {
			loadPlaylistAndPlay(path);
		}
	}
}

void MainWindow::loadPlaylistAndPlay(QString const &path)
{
	if (path.isEmpty()) return;

	int index = playlistFileCount();
	if (index < 0) index = 0;

	if (mpc()->do_load(path)) {
		if (!isPlaying()) {
			mpc()->do_play(index);
			invalidateCurrentSongIndicator();
			updateCurrentSongInfo();
		}
		updatePlaylist();
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
			if (i < ui->comboBox_servers1->count()) {
				ui->comboBox_servers1->setCurrentIndex(i);
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
			if (isFile(item) || isPlaylist(item)) {
				execPrimaryCommand(item);
			} else if (isFolder(item)) {
				toggleExpandCollapse(item);
			}
		} else if (focus == ui->listWidget_playlist) {
			if (event->modifiers() & Qt::AltModifier) {
				QListWidgetItem *item = ui->listWidget_playlist->currentItem();
				QString path = songPath(item, false);
				bool ok = false;
				int pos = item->data(ITEM_PosRole).toInt(&ok);
				if (!ok) pos = -1;
				execSongProperty(path, pos, false);
			} else {
				int i = ui->listWidget_playlist->currentRow();
				mpc()->do_play(i);
			}
		}
		event->accept();
		return;
	case Qt::Key_Insert:
		if (focus == ui->treeWidget) {
			if (key == Qt::Key_Insert) {
				QList<QTreeWidgetItem *> items = ui->treeWidget->selectedItems();
				for (QTreeWidgetItem *item : items) {
					QString path = songPath(item);
					if (path.isEmpty()) continue;
					if (isFile(item)) {
						addToPlaylist(path, -1, false);
					} else if (isPlaylist(item)) {
						mpc()->do_load(path);
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
		return;
#ifdef Q_OS_MAC
	case Qt::Key_Delete:
		ui->action_edit_delete->trigger();
		event->accept();
		return;
#endif
	}
}

void MainWindow::displayExtraInformation(QString const &text2, QString const &text3)
{
	if (text2.isEmpty()) {
		m->status_label2->setVisible(false);
	} else {
		m->status_label2->setText(text2);
		m->status_label2->setVisible(true);
	}

	m->status_label3->setText(text3);
}

void MainWindow::setPageConnected()
{
	ui->stackedWidget->setCurrentWidget(ui->page_connected);
}

void MainWindow::setPageDisconnected()
{
	ui->stackedWidget->setCurrentWidget(ui->page_disconnecccted);
	ui->comboBox_servers2->setCurrentIndex(0);
}

void MainWindow::setVolumeEnabled(bool f)
{
	if (f) {
		ui->toolButton_volume->setEnabled(true);
		ui->toolButton_volume->setToolTip(tr("Volume"));
	} else {
		ui->toolButton_volume->setEnabled(false);
		ui->toolButton_volume->setToolTip(tr("Volume change is not supported"));
	}
}

void MainWindow::clearTreeAndList()
{
	ui->treeWidget->clear();
	ui->listWidget_playlist->clear();
}

void MainWindow::setRepeatEnabled(bool f)
{
	if (m->repeat_enabled != f) {
		ui->action_repeat->setChecked(f);
		ui->toolButton_repeat->setChecked(f);
	}
	BasicMainWindow::setRepeatEnabled(f);
}

void MainWindow::setSingleEnabled(bool f)
{
	if (m->single_enabled != f) {
		ui->action_single->setChecked(f);
		ui->toolButton_single->setChecked(f);
	}
	BasicMainWindow::setSingleEnabled(f);
}

void MainWindow::setConsumeEnabled(bool f)
{
	if (m->consume_enabled != f) {
		ui->action_consume->setChecked(f);
		ui->toolButton_consume->setChecked(f);
	}
	BasicMainWindow::setConsumeEnabled(f);
}

void MainWindow::setRandomEnabled(bool f)
{
	if (m->random_enabled != f) {
		ui->action_random->setChecked(f);
		ui->toolButton_random->setChecked(f);
	}
	BasicMainWindow::setRandomEnabled(f);
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

void MainWindow::displayProgress(QString const &text)
{
	ui->label_progress->setText(text);
}

void MainWindow::seekProgressSlider(double elapsed, double total)
{
//	if (QWidget::mouseGrabber() == ui->horizontalSlider) return;

	bool b = ui->horizontalSlider->blockSignals(true);
	ui->horizontalSlider->setUpdatesEnabled(false);
	ui->horizontalSlider->setMaximum((int)(total * 100));
	ui->horizontalSlider->setValue((int)(elapsed * 100));
	ui->horizontalSlider->setUpdatesEnabled(true);
	ui->horizontalSlider->blockSignals(b);
}

void MainWindow::displayCurrentSongLabels(QString const &title, QString const &artist, QString const &disc)
{
	ui->label_title->setText(title);
	ui->label_artist->setText(artist);
	ui->label_disc->setText(disc);
}

void MainWindow::updatePlayIcon()
{
	BasicMainWindow::updatePlayIcon(m->status.now.status, ui->toolButton_play, ui->action_play);
}

void MainWindow::updateWindowTitle()
{
	QString text = currentSongTitle();
	if (text.isEmpty()) {
		text = qApp->applicationName();
	} else {
		text = text + " - " + qApp->applicationName();
	}
	setWindowTitle(text);
}

void MainWindow::updateCurrentSongInfo()
{
	int count = playlistFileCount();
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			QListWidgetItem *item = ui->listWidget_playlist->item(i);
			Q_ASSERT(item);
			char const *s = ":image/notplaying.png";
			if (i == m->status.now.index) {
				if (m->status.now.status == PlayingStatus::Play) {
					s = ":image/playing.svgz";
				} else if (m->status.now.status == PlayingStatus::Pause) {
					s = ":image/pause.png";
				}
			}
			item->setIcon(QIcon(s));
		}

		displayCurrentSongLabels(m->status.now.title, m->status.now.artist, m->status.now.disc);

		updateWindowTitle();
	} else {
		invalidateCurrentSongIndicator();
	}
}

void MainWindow::updateTreeTopLevel()
{
	ui->treeWidget->clear();
	QList<MusicPlayerClient::Item> vec;
	mpc()->do_lsinfo(QString(), &vec);
	MusicPlayerClient::sort(&vec);

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

void MainWindow::doUpdateStatus()
{
	if (!ui->horizontalSlider->isSliderDown()) {
		BasicMainWindow::doUpdateStatus();
	}
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
		MusicPlayerClient::sort(&info->vec);
		using mpcitem_t = MusicPlayerClient::Item;
		for (mpcitem_t const &mpcitem : info->vec) {
			int i = mpcitem.text.lastIndexOf('/');
			if (i >= 0) {
				QString name = mpcitem.text.mid(i + 1);
				int kind = -1;
				if (mpcitem.kind == "directory") {
					kind = ITEM_IsFolder;
				} else if (mpcitem.kind == "file") {
					kind = ITEM_IsFile;
				} else if (mpcitem.kind == "playlist") {
					kind = ITEM_IsPlaylist;
				}
				if (kind == ITEM_IsFolder) {
					QTreeWidgetItem *child = new_QTreeWidgetItem(treeitem);
					child->setText(0, name);
					child->setIcon(0, folderIcon());
					child->setData(0, kind, true);
					child->setData(0, ITEM_PathRole, mpcitem.text);
					QTreeWidgetItem *g = new_QTreeWidgetItem(child);
					g->setText(0, "Reading...");
					child->addChild(g);
					treeitem->addChild(child);
				} else if (kind == ITEM_IsFile || kind == ITEM_IsPlaylist) {
					QString path = mpcitem.text;
					QString text;
					QList<MusicPlayerClient::Item> v;
					if (mpc()->do_listallinfo(path, &v)) {
						if (v.size() == 1) {
							MusicPlayerClient::StringMap const &map = v.front().map;
							int trk = map.get("Track").toInt();
							if (trk > 0) {
								char tmp[10];
								sprintf(tmp, "%02d ", trk);
								text += tmp;
							}
							text += map.get("Title");
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
						int i = path.lastIndexOf('/');
						if (i >= 0) {
							text = path.mid(i + 1);
						}
					}

					QIcon icon;
					if (kind == ITEM_IsFile) {
						icon = songIcon();
					} else if (kind == ITEM_IsPlaylist) {
						icon = playlistIcon();
					}

					QTreeWidgetItem *child = new_QTreeWidgetItem(treeitem);
					child->setText(0, text);
					child->setIcon(0, icon);
					child->setData(0, kind, true);
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

bool MainWindow::isPlaceHolder(QTreeWidgetItem *item) const
{
	if (item->childCount() == 1) {
		QTreeWidgetItem *child = item->child(0);
		if (songPath(child).isNull()) {
			return true;
		}
	}
	return false;
}

void MainWindow::refreshTreeItem(QTreeWidgetItem *item)
{
	QString path = songPath(item);
	QModelIndex index = ui->treeWidget->indexFromItem(item);
	QApplication::postEvent(this, new QueryInfoEvent(RequestItem(path, index)));
}

void MainWindow::on_treeWidget_itemExpanded(QTreeWidgetItem *item)
{
	if (isPlaceHolder(item)) {
		refreshTreeItem(item);
	}
}

void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
	if (isFile(item) || isPlaylist(item)) {
		execPrimaryCommand(item);
	} else if (isFolder(item)) {
		toggleExpandCollapse(item);
	} else if (isRoot(item)) {
		item->setExpanded(true); // always open
	}
}

void MainWindow::deletePlaylistItem(QListWidgetItem *item, bool updateplaylist)
{
	int id = item->data(ITEM_SongIdRole).toInt();
	mpc()->do_deleteid(id);

	if (updateplaylist) updatePlaylist();
}

void MainWindow::deleteSelectedSongs()
{
	int row = ui->listWidget_playlist->currentRow();

	auto list = ui->listWidget_playlist->selectedItems();
	for (int i = 0; i < list.size(); i++) {
		QListWidgetItem *item = list.at(i);
		deletePlaylistItem(item, false);
	}
	updatePlaylist();

	int count = playlistFileCount();
	if (count <= row) {
		row = count - 1;
	}
	if (row >= 0) {
		ui->listWidget_playlist->setCurrentRow(row);
		updateCurrentSongInfo();
	}
}

void BasicMainWindow::execSongProperty(QString const &path, int listrow, bool addplaylist)
{
	if (path.isEmpty()) {
		return;
	}
	using KeyValue = MusicPlayerClient::KeyValue;
	std::vector<KeyValue> vec;
	mpc()->do_listallinfo(path, &vec);
	if (vec.size() > 0) {
		if (vec[0].key == "file") {
			{
				std::map<QString, QString> map;
				for (int i = 1; i < (int)vec.size(); i++) {
					map[vec[i].key] = vec[i].value;
				}
				if (listrow >= 0){
					QList<MusicPlayerClient::Item> items;
					if (mpc()->do_playlistinfo(QString(), &items)) {
						if (listrow < items.size()) {
							std::map<QString, QString> const &m = items[listrow].map.map;
							for (auto const &pair : m) {
								map[pair.first] = pair.second;
							}
						}
					}
				}
				vec.resize(1);
				for (auto const &pair : map) {
					vec.push_back(KeyValue(pair.first, pair.second));
				}
			}
			std::sort(vec.begin() + 1, vec.end(), [](KeyValue const &l, KeyValue const &r){
				return l.key < r.key;
			});

			SongPropertyDialog dlg(this, &vec, addplaylist);
			if (dlg.exec() == QDialog::Accepted) {
				if (addplaylist && dlg.addToPlaylistClicked()) {
					QString path = vec[0].value;
					mpc()->do_add(path);
					updatePlaylist();
				}
			}
		}
	}
}

void MainWindow::execPlaylistPropertyDialog(QString const &path)
{
	QList<MusicPlayerClient::Item> items;
	mpc()->do_listplaylistinfo(path, &items);

	std::vector<MusicPlayerClient::KeyValue> vec;
	vec.push_back(MusicPlayerClient::KeyValue("playlist", path));

	for (int i = 0; i < items.size(); i++) {
		QString title = items[i].map.get("Title");
		vec.push_back(MusicPlayerClient::KeyValue(QString::number(i + 1), title));
	}

	SongPropertyDialog dlg(this, &vec, true);
	if (dlg.exec() == QDialog::Accepted) {
		loadPlaylist(path, false);
	}
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
	QString path = songPath(treeitem);
	if (act == &a_AddToPlaylist) {
		addToPlaylist(path, -1, true);
	} else if (act == &a_Property) {
		if (isFile(treeitem)) {
			execSongProperty(path, -1, true);
		} else if (isPlaylist(treeitem)) {
			execPlaylistPropertyDialog(path);
		}
	}
}

void MainWindow::onListViewContextMenuEvent(QContextMenuEvent *)
{
	QMenu menu;
	QAction a_PlayFromHere(tr("Play from here"), 0);
	QAction a_Edit(tr("Edit"), 0);
	QAction a_Cut(tr("Cut"), 0);
	QAction a_Copy(tr("Copy"), 0);
	QAction a_PasteInsert(tr("Paste (Insert)"), 0);
	QAction a_PasteAppend(tr("Paste (Append)"), 0);
	QAction a_Delete(tr("Delete"), 0);
	QAction a_Clear(tr("Clear play list"), 0);
	QAction a_Property(tr("Property"), 0);
	QAction a_AddLocation(tr("Add location"), 0);
	if (ui->listWidget_playlist->selectedItems().isEmpty()) {
		menu.addAction(&a_AddLocation);
	} else {
		menu.addAction(&a_PlayFromHere);
		menu.addSeparator();
		menu.addAction(&a_Edit);
		menu.addAction(&a_AddLocation);
		menu.addSeparator();
		menu.addAction(&a_Cut);
		menu.addAction(&a_Copy);
		menu.addAction(&a_PasteInsert);
		menu.addAction(&a_PasteAppend);
		menu.addAction(&a_Delete);
		menu.addAction(&a_Clear);
		menu.addSeparator();
		menu.addAction(&a_Property);
	}
	QAction *act = menu.exec(QCursor::pos() + QPoint(8, -8));
	if (act == &a_PlayFromHere) {
		int i = ui->listWidget_playlist->currentIndex().row();
		if (i >= 0) {
			mpc()->do_play(i);
		}
	} else if (act == &a_Edit) {
		on_edit_location();
	} else if (act == &a_AddLocation) {
		execAddLocationDialog();
		return;
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
		int row = ui->listWidget_playlist->currentIndex().row();
		QListWidgetItem *listitem = ui->listWidget_playlist->item(row);
		if (listitem) {
			QString path = songPath(listitem, false);
			execSongProperty(path, row, false);
		}
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
	releaseMouseIfGrabbed();
	QMainWindow::mouseReleaseEvent(e);
}

void MainWindow::onDropEvent(bool done)
{
	if (!done) {
		m->drop_before.clear();
		int n = playlistFileCount();
		for (int i = 0; i < n; i++) {
			QListWidgetItem *listitem = ui->listWidget_playlist->item(i);
			listitem->setData(ITEM_RowRole, i);
			QString path = songPath(listitem, false);
			m->drop_before.push_back(SongItem(i, path));
		}
	} else {
		std::vector<SongItem> drop_after;
		int n = playlistFileCount();
		for (int i = 0; i < n; i++) {
			QListWidgetItem const *listitem = ui->listWidget_playlist->item(i);
			bool ok = false;
			int row = listitem->data(ITEM_RowRole).toInt(&ok);
			if (!ok) row = -1;
			QString path = songPath(listitem, false);
			if (path.isEmpty()) continue;
			drop_after.push_back(SongItem(row, path));
		}
		std::vector<SongItem> drop_before = m->drop_before;
		for (size_t i = 0; i < drop_after.size(); i++) {
			SongItem const &a = drop_after[i];
			if (a.index == -1) {
				QList<MusicPlayerClient::Item> fileitems;
				QList<MusicPlayerClient::Item> playlist;
				if (mpc()->do_listall(a.path, &fileitems)) {
					size_t j = i;
					for (MusicPlayerClient::Item const &item : fileitems) {
						if (item.kind == "file") {
							QString path = item.text;
							mpc()->do_addid(path, (int)j);
							SongItem item(-1, path);
							drop_before.insert(drop_before.begin() + j, item);
							j++;
						}
					}
				} else if (mpc()->do_listplaylistinfo(a.path, &playlist)) {
					addPlaylsitToPlaylist(a.path, i);
				}
			} else if (i < drop_before.size() && a.index != drop_before[i].index) {
				for (size_t j = i + 1; j < drop_before.size(); j++) {
					if (a.index == drop_before[j].index) {
						std::swap(drop_before[i], drop_before[j]);
						mpc()->do_swap(i, j);
						break;
					}
				}
			}
		}
		updatePlaylist();
	}
}

void MainWindow::on_listWidget_playlist_doubleClicked(const QModelIndex &index)
{
	if (qApp->keyboardModifiers() & Qt::AltModifier) {
		int row = index.row();
		QListWidgetItem *item = ui->listWidget_playlist->item(row);
		if (item) {
			QString path = songPath(item, false);
			execSongProperty(path, row, false);
		}
	} else {
		int i = index.row();
		if (i >= 0) {
			mpc()->do_play(i);
		}
		invalidateCurrentSongIndicator();
	}
}

void MainWindow::on_toolButton_volume_clicked()
{
	if (m->volume < 0) {
		return; // setvol not supported
	}
	QRect rc = ui->toolButton_volume->frameGeometry();
	QPoint pt = ((QWidget *)ui->toolButton_volume->parent())->mapToGlobal(rc.bottomRight());
	pt.rx() -= m->volume_popup.width();
	m->volume_popup.setValue(m->volume);
	m->volume_popup.move(pt);
	m->volume_popup.show();
}

void MainWindow::updatePrimaryStatusLabel()
{
	QString text;
	int selected = ui->listWidget_playlist->selectedItems().size();
	if (selected < 2) {
		int count = playlistFileCount();
		text = tr("%1 songs in playlist").arg(count);
	} else {
		text = tr("%1 songs selected").arg(selected);
	}
	m->status_label1->setText(text);
}

void MainWindow::updatePlaylist()
{
	QList<MusicPlayerClient::Item> vec;

	if (!mpc()->do_playlistinfo(QString(), &vec)) {
		return;
	}

	BasicMainWindow::updatePlaylist(ui->listWidget_playlist, &vec);

	updateCurrentSongInfo();

	updatePrimaryStatusLabel();
}

void MainWindow::execConnectionDialog()
{
	ConnectionDialog dlg(this, m->host);
	if (dlg.exec() == QDialog::Accepted) {
		Host host = dlg.host();
		connectToMPD(host);
	}
	updateServersComboBox();
}

void BasicMainWindow::stopSleepTimer()
{
	startSleepTimer(0);
}

void MainWindow::updatePlaylistMenu()
{
	const bool showtemp = false;

	QStringList list;
	{
		QList<MusicPlayerClient::Item> items;
		mpc()->do_lsinfo(QString(), &items);
		std::sort(items.begin(), items.end());
		for (MusicPlayerClient::Item const &item : items) {
			if (item.kind == "playlist") {
				QString name = item.text;
				if (!showtemp && name.size() > 1 && EditPlaylistDialog::isTemporaryItem(name)) {
					// nop
				} else {
					list.push_back(name);
				}
			}
		}
	}

	std::sort(list.begin(), list.end(), [](QString const &l, QString const &r){
		int i = l.compare(r, Qt::CaseInsensitive);
		return i < 0;
	});

	ui->menu_playlist_load->clear();
	for (QString const &name : list) {
		ui->menu_playlist_load->addAction(name);
	}
}

void MainWindow::on_toolButton_sleep_timer_clicked()
{
	execSleepTimerDialog();
}

void MainWindow::onSliderPressed()
{
	if (m->status.now.status == PlayingStatus::Play) {
		mpc()->do_pause(true);
	}
}

void MainWindow::onSliderReleased()
{
	int pos = ui->horizontalSlider->value() / 100;
	{
		mpc()->do_seek(m->status.now.index, pos);
		if (m->status.now.status == PlayingStatus::Play) {
			mpc()->do_pause(false);
		}
	}
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
	BasicMainWindow::displayProgress(value / 100.0);
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

void MainWindow::on_action_play_triggered()
{
	play(true);
}

void MainWindow::on_action_stop_triggered()
{
	stop();
}

void MainWindow::on_action_previous_triggered()
{
	mpc()->do_previous();
}

void MainWindow::on_action_next_triggered()
{
	mpc()->do_next();
}

void MainWindow::on_action_repeat_triggered()
{
	mpc()->do_repeat(!m->repeat_enabled);
}

void MainWindow::on_action_random_triggered()
{
	mpc()->do_random(!m->random_enabled);
}

void MainWindow::on_action_single_triggered()
{
	mpc()->do_single(!m->single_enabled);
}

void MainWindow::on_action_consume_triggered()
{
	mpc()->do_consume(!m->consume_enabled);
}

void MainWindow::on_action_network_connect_triggered()
{
	execConnectionDialog();
}

void MainWindow::on_action_network_disconnect_triggered()
{
	disconnectNetwork();
}

void MainWindow::on_action_network_reconnect_triggered()
{
	connectToMPD(m->host);
	update(false);

	showNotify(tr("Reconnected"));
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

void MainWindow::on_action_playlist_edit_triggered()
{
	EditPlaylistDialog dlg(this);
	if (dlg.exec() != QDialog::Accepted) return;
	QString name = dlg.name();
	if (name.isEmpty()) return;
	if (!MusicPlayerClient::isValidPlaylistName(name)) {
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

void MainWindow::on_edit_location()
{
	int row = ui->listWidget_playlist->currentRow();
	QListWidgetItem *item = ui->listWidget_playlist->item(row);
	if (item) {
		QString path = songPath(item, false);
		EditLocationDialog dlg(this);
		dlg.setLocation(path);
		if (dlg.exec() == QDialog::Accepted) {
			deletePlaylistItem(item, false);
			QString path = dlg.location();
			addToPlaylist(path, row, true);
		}
	} else {
		QMessageBox::warning(this, qApp->applicationName(), tr("Please select only one item before use this command."));
	}
}

void MainWindow::on_action_edit_copy_triggered()
{
	QStringList list;
	QWidget *focus = focusWidget();
	if (focus == ui->treeWidget) {
		auto items = ui->treeWidget->selectedItems();
		for (QTreeWidgetItem const *item : items) {
			QString path = songPath(item);
			list.append(path);
		}
	} else if (focus == ui->listWidget_playlist) {
		QList<QListWidgetItem *> items = ui->listWidget_playlist->selectedItems();
		for (QListWidgetItem const *item : items) {
			QString path = songPath(item, true);
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

void MainWindow::paste(int row)
{
	QWidget *focus = focusWidget();
	if (focus == ui->listWidget_playlist) {
		QClipboard *cb = qApp->clipboard();
		QString text = cb->text();
		QStringList list = text.split('\n');
		for (QString const &str : list) {
			QString path = str.trimmed();
			int i = path.indexOf("/.../");
			if (i >= 0) {
				QString args = path.mid(i + 3);
				path = path.mid(0, i);
				QStringList argv = args.split('/');
				for (QString const &line : argv) {
					int i = line.indexOf('=');
					if (i > 0) {
//						QString key = line.mid(0, i);
//						QString val = line.mid(i + 1);
					}
				}
			}
			if (path.isEmpty()) continue;
			addToPlaylist(path, row, false);
			if (row >= 0) row++;
		}
		updatePlaylist();
	}
}

void MainWindow::on_action_edit_paste_bottom_triggered()
{
	paste(-1);
}

void MainWindow::on_action_edit_paste_insert_triggered()
{
	int row = ui->listWidget_playlist->currentRow();
	if (row < 0) row = -1;
	paste(row);
}

void MainWindow::on_action_edit_delete_triggered()
{
	QWidget *focus = focusWidget();
	if (focus == ui->listWidget_playlist) {
		deleteSelectedSongs();
	}
}

void MainWindow::comboboxIndexChanged(QComboBox *cbox, int index)
{
	onServersComboBoxIndexChanged(cbox, index);
}

void MainWindow::on_comboBox_servers1_currentIndexChanged(int index)
{
	comboboxIndexChanged(ui->comboBox_servers1, index);
}

void MainWindow::on_comboBox_servers2_currentIndexChanged(int index)
{
	comboboxIndexChanged(ui->comboBox_servers2, index);
}

void MainWindow::on_action_playlist_add_location_triggered()
{
	execAddLocationDialog();
}

void MainWindow::on_action_playlist_update_triggered()
{
	update(true);
}

void MainWindow::on_action_playlist_unify_triggered()
{
	unify();
}

void MainWindow::on_action_playlist_quick_save_1_triggered()
{
	doQuickSave1();
}

void MainWindow::on_action_playlist_quick_save_2_triggered()
{
	doQuickSave2();
}

void MainWindow::on_action_playlist_quick_load_1_triggered()
{
	doQuickLoad1();
}

void MainWindow::on_action_playlist_quick_load_2_triggered()
{
	doQuickLoad2();
}

void MainWindow::on_action_playlist_clear_triggered()
{
	clearPlaylist();
}

void MainWindow::on_action_sleep_timer_triggered()
{
	execSleepTimerDialog();
}

void MainWindow::on_listWidget_playlist_itemSelectionChanged()
{
	updatePrimaryStatusLabel();
}

void MainWindow::on_action_debug_triggered()
{
}

void MainWindow::on_action_settings_triggered()
{
	SettingsDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {

	}
}
