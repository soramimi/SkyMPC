#ifndef TINYMAINWINDOW_H
#define TINYMAINWINDOW_H

#include "Common.h"
#include "MusicPlayerClient.h"
#include "PlaylistFile.h"
#include <QMainWindow>
#include <QModelIndex>
#include <vector>

namespace Ui {
class TinyMainWindow;
}
class QTreeWidgetItem;
class QListWidgetItem;

class TinyMainWindow : public BasicMainWindow
{
	Q_OBJECT
private:
	Ui::TinyMainWindow *ui;
private:
	struct Private;
	Private *pv;
	QIcon folderIcon();
	void updatePlayingStatus();
	QString serverName() const;
	void showNotify(QString const &text);
	void showError(QString const &text);
	void set_volume_(int v);
	void loadPlaylist(QString const &name, bool replace);
	bool savePlaylist(QString const &name);
	bool deletePlaylist(QString const &name);
	void clearPlaylist();
	void invalidateCurrentSongIndicator();
	void execSongProperty(QString const &path, bool addplaylist);
	bool isPlaying() const;
	void updateStatusBar();
	void releaseMouseIfGrabbed();
	void startStatusThread();
	void stopStatusThread();
	void update(bool mpdupdate);
	void displayProgress(double elapsed);
	void checkDisconnected();
	void execAddLocationDialog();
	void startSleepTimer(int mins);
	void stopSleepTimer();
	void execSleepTimerDialog();
	void disconnectNetwork();
	static QString makeStyleSheetText();
	static QString timeText(const MusicPlayerClient::Item &item);
public:
	explicit TinyMainWindow(QWidget *parent = 0);
	~TinyMainWindow();

	bool isAutoReconnectAtStartup();

	bool execCommand(Command const &c);

	void preexec();
	void connectToMPD(Host const &host);
	void setRepeatEnabled(bool f);
	void setSingleEnabled(bool f);
	void setConsumeEnabled(bool f);
	void setRandomEnabled(bool f);

	void play();
	void pause();
	void stop();
	void play(bool toggle);

	void eatMouse();
protected:
	virtual bool event(QEvent *event);
	virtual bool eventFilter(QObject *, QEvent *);
	void changeEvent(QEvent *e);
	virtual void closeEvent(QCloseEvent *);
	bool updatePlaylist();
	void addToPlaylist(QString const &path, int to, bool update);
	virtual void mouseReleaseEvent(QMouseEvent *);
//	virtual void timerEvent(QTimerEvent *);
private slots:
	void on_toolButton_play_clicked();

	void onVolumeChanged();
	void onSliderPressed();

	void on_toolButton_stop_clicked();
	void on_toolButton_prev_clicked();
	void on_toolButton_next_clicked();
	void on_toolButton_sleep_timer_clicked();

	void on_action_debug_triggered();
	void on_action_play_triggered();
	void on_action_stop_triggered();
	void on_action_previous_triggered();
	void on_action_next_triggered();
	void on_action_repeat_triggered();
	void on_action_random_triggered();
	void on_action_help_about_triggered();
	void on_action_network_connect_triggered();
	void on_action_network_disconnect_triggered();
	void on_action_network_reconnect_triggered();
	void on_action_single_triggered();
	void on_action_consume_triggered();
	void on_toolButton_single_clicked();
	void on_pushButton_manage_connections_clicked();
	void on_action_playlist_add_location_triggered();
	void on_action_playlist_update_triggered();
	void on_action_file_close_triggered();
	void on_action_playlist_quick_save_1_triggered();
	void on_action_playlist_quick_save_2_triggered();
	void on_action_playlist_quick_load_1_triggered();
	void on_action_playlist_quick_load_2_triggered();
	void on_action_playlist_clear_triggered();
	void on_action_playlist_edit_triggered();
	void on_action_edit_keyboard_customize_triggered();
	void on_toolButton_consume_clicked();
	void onUpdateStatus();


	void on_action_sleep_timer_triggered();

	void on_toolButton_close_clicked();

public:
	static QString tr_Module_information_could_not_be_acquired()
	{
		return tr("Module information could not be acquired.");
	}

protected:

	// QObject interface
protected:
	void timerEvent(QTimerEvent *);
};

#endif // TINYMAINWINDOW_H
