#include "TinyMainWindow.h"
#include "ui_TinyMainWindow.h"
#include "AboutDialog.h"
#include "AskRemoveOverlappedFileDialog.h"
#include "ConnectionDialog.h"
#include "EditLocationDialog.h"
#include "EditPlaylistDialog.h"
#include "KeyboardCustomizeDialog.h"
#include "main.h"
#include "misc.h"
#include "MySettings.h"
#include "platform.h"
#include "SelectLocationDialog.h"
#include "ServersComboBox.h"
#include "SleepTimerDialog.h"
#include "SongPropertyDialog.h"
#include "TinyMainWindowPrivate.h"
#include "Toast.h"
#include "VolumeIndicatorPopup.h"
#include "webclient.h"
#include "TinyConnectionDialog.h"
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

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#define DISPLAY_TIME 0

TinyMainWindow::TinyMainWindow(QWidget *parent) :
	BasicMainWindow(parent),
	ui(new Ui::TinyMainWindow)
{
	ui->setupUi(this);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_QuitOnClose);

	setWindowState(windowState() | Qt::WindowFullScreen);

	pv->release_mouse_event = false;

	pv->status_label1 = new QLabel();
	ui->statusBar->addWidget(pv->status_label1, 1);
	pv->status_label2 = new QLabel();
	ui->statusBar->addWidget(pv->status_label2, 0);
	pv->status_label3 = new QLabel();
	ui->statusBar->addWidget(pv->status_label3, 0);

#if 0 //def Q_OS_WIN
	priv->folder_icon = QIcon(":/image/winfolder.png");
#else
	pv->folder_icon = QIcon(":/image/macfolder.png");
#endif

	{
		QString ss = makeStyleSheetText();
		setStyleSheet(ss);
	}

#ifdef Q_OS_MAC
#else
	ui->action_help_about->setText(tr("&About SkyMPC"));
#endif

	pv->menu.addAction(ui->action_help_about);
	pv->menu.addAction(ui->action_debug);

	setRepeatEnabled(false);
	setRandomEnabled(false);

	if (!start_with_shift_key) {
		MySettings settings;

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
	delete ui;
}

QIcon TinyMainWindow::folderIcon()
{
	return pv->folder_icon;
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

	{
		MySettings settings;
		settings.beginGroup("Connection");
		settings.setValue("Address", pv->host.address());
		settings.setValue("Port", pv->host.port());
		settings.setValue("Password", pv->host.password());
		settings.endGroup();
	}
	QMainWindow::closeEvent(event);
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
	return QMainWindow::event(event);
}

void TinyMainWindow::displayExtraInformation(QString const &text2, QString const &text3)
{
	if (text2.isEmpty()) {
		pv->status_label2->setVisible(false);
	} else {
		pv->status_label2->setText(text2);
		pv->status_label2->setVisible(true);
	}

	pv->status_label3->setText(text3);
	pv->status_label3->setVisible(true);
}

void TinyMainWindow::displayCurrentSongLabels(const QString &title, const QString &artist, const QString & /*disc*/)
{
	ui->label_title->setText(title);
	ui->label_artist->setText(artist);
}

void TinyMainWindow::updatePlayIcon()
{
	BasicMainWindow::updatePlayIcon(pv->status.now.status, ui->toolButton_play, ui->action_play);
}

void TinyMainWindow::displayProgress(const QString &text)
{
	pv->status_label1->setText(text);
}

void TinyMainWindow::setRepeatEnabled(bool f)
{
	if (pv->repeat_enabled != f) {
		ui->action_repeat->setChecked(f);
	}
	BasicMainWindow::setRepeatEnabled(f);
}

void TinyMainWindow::setSingleEnabled(bool f)
{
	if (pv->single_enabled != f) {
		ui->action_single->setChecked(f);
	}
	BasicMainWindow::setSingleEnabled(f);
}

void TinyMainWindow::setConsumeEnabled(bool f)
{
	if (pv->consume_enabled != f) {
		ui->action_consume->setChecked(f);
	}
	BasicMainWindow::setConsumeEnabled(f);
}

void TinyMainWindow::setRandomEnabled(bool f)
{
	if (pv->random_enabled != f) {
		ui->action_random->setChecked(f);
	}
	BasicMainWindow::setRandomEnabled(f);
}

void TinyMainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	case QEvent::ActivationChange:
		if (!isMinimized() && !isFullScreen()) {
			setWindowState(Qt::WindowFullScreen);
		}
		break;
	default:
		break;
	}
}

void TinyMainWindow::execConnectionDialog()
{
	TinyConnectionDialog dlg(this, pv->host);
	if (dlg.exec() == QDialog::Accepted) {
		Host host = dlg.host();
		connectToMPD(host);
	}
	updateServersComboBox();
}

void TinyMainWindow::updateClock(const QString &text)
{
	ui->label_clock->setText(text);
}

void TinyMainWindow::updateCurrentSongInfo()
{
	displayCurrentSongLabels(pv->status.current_title, pv->status.current_artist, pv->status.current_disc);
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

void TinyMainWindow::mouseReleaseEvent(QMouseEvent *e)
{
	releaseMouseIfGrabbed();
	QMainWindow::mouseReleaseEvent(e);
}

void TinyMainWindow::on_toolButton_sleep_timer_clicked()
{
	execSleepTimerDialog();
}

void TinyMainWindow::onVolumeChanged()
{
	int v = pv->volume_popup.value();
	mpc()->do_setvol(v);
}

void TinyMainWindow::onSliderPressed()
{
	if (pv->status.now.status == PlayingStatus::Play) {
		mpc()->do_pause(true);
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
	mpc()->do_previous();
}

void TinyMainWindow::on_action_next_triggered()
{
	mpc()->do_next();
}

void TinyMainWindow::on_action_repeat_triggered()
{
	mpc()->do_repeat(!pv->repeat_enabled);
}

void TinyMainWindow::on_action_random_triggered()
{
	mpc()->do_random(!pv->random_enabled);
}

void TinyMainWindow::on_action_single_triggered()
{
	mpc()->do_single(!pv->single_enabled);
}

void TinyMainWindow::on_action_consume_triggered()
{
	mpc()->do_consume(!pv->consume_enabled);
}

void TinyMainWindow::on_action_network_connect_triggered()
{
	execConnectionDialog();
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

void TinyMainWindow::on_action_playlist_edit_triggered()
{
	EditPlaylistDialog dlg(this, mpc());
	if (dlg.exec() != QDialog::Accepted) return;
	QString name = dlg.name();
	if (name.isEmpty()) return;
	if (!MusicPlayerClient::isValidPlaylistName(name)) {
		QMessageBox::warning(this, qApp->applicationName(), tr("The name is invalid."));
		return;
	}
	loadPlaylist(name, dlg.forReplace());
}



void TinyMainWindow::on_action_playlist_add_location_triggered()
{
	execAddLocationDialog();
}

void TinyMainWindow::on_action_playlist_update_triggered()
{
	update(true);
}

void TinyMainWindow::on_action_playlist_quick_save_1_triggered()
{
	if (savePlaylist("_quick_save_1_", true)) {
		showNotify(tr("Quick Save 1 was completed"));
	}
}

void TinyMainWindow::on_action_playlist_quick_save_2_triggered()
{
	if (savePlaylist("_quick_save_2_", true)) {
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

void TinyMainWindow::on_toolButton_close_clicked()
{
	close();
}

void TinyMainWindow::updateServersComboBox()
{
	ui->comboBox_servers->resetContents(pv->host, false, true);
}

void TinyMainWindow::on_comboBox_servers_currentIndexChanged(int index)
{
	onServersComboBoxIndexChanged(ui->comboBox_servers, index);
}

void TinyMainWindow::on_toolButton_minimize_clicked()
{
	setWindowState(Qt::WindowMinimized);
}




