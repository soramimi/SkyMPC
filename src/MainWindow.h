#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MusicPlayerClient.h"
#include <QMainWindow>
#include <QModelIndex>
#include <vector>

namespace Ui {
class MainWindow;
}
class QTreeWidgetItem;

struct RequestItem {
	QString path;
	QModelIndex index;
	RequestItem()
	{
	}
	RequestItem(QString const &path, QModelIndex const &index)
		: path(path)
		, index(index)
	{
	}
};

struct ResultItem {
	RequestItem req;
	QList<MusicPlayerClient::Item> vec;
};

class Command {
	friend class MainWindow;
private:
	struct {
		QString command;
	} data;
	static int compare(QString const &l, QString const &r)
	{
		return l.compare(r, Qt::CaseInsensitive);
	}
	int compare(Command const &r) const
	{
		return compare(data.command, r.data.command);
	}
public:
	Command()
	{
	}

	Command(QString const &command)
	{
		data.command = command;
	}

	QString const &command() const
	{
		return data.command;
	}

	bool operator < (Command const &r) const
	{
		return compare(r) < 0;
	}
	bool operator == (Command const &r) const
	{
		return compare(r) == 0;
	}
	bool operator != (Command const &r) const
	{
		return compare(r) != 0;
	}
};


class MainWindow : public QMainWindow
{
	Q_OBJECT
private:
    struct Impl;
    Impl *impl;
    QIcon folderIcon();
	void displayProgress(double elapsed);
	void updatePlayingStatus();
	void updateCurrentSongIndicator();
	void updateTreeTopLevel();
	void updateTree(ResultItem *info);
	void clear();
	void updateServersComboBox();
	QString serverName() const;
	void showNotify(QString const &text);
	void setVolume_(int v);
	void loadPlaylist(QString const &name, bool replace);
	void savePlaylist(QString const &name);
	void deletePlaylist(QString const &name);
	void clearPlaylist();
	void invalidateCurrentSongIndicator();
	void execSongProperty(QString const &path, bool addplaylist);
	bool isPlaying() const;
	void execPrimaryCommand(QTreeWidgetItem *item);
	void setDefaultStatusBarText();
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	static bool isAutoReconnectAtStartup();

	bool execCommand(Command const &c);

	static QString fixMenuText(QString const &s);
    static void fixActionText(QList<QAction*> &acts);

	void preExec();
	void connectToMPD(Host const &host);
	void setRepeatEnabled(bool f);
	void setSingleEnabled(bool f);
	void setConsumeEnabled(bool f);
	void setRandomEnabled(bool f);

	void play(bool toggle);

protected:
	virtual bool event(QEvent *event);
	virtual bool eventFilter(QObject *, QEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	void changeEvent(QEvent *e);
	virtual void closeEvent(QCloseEvent *);
	void updatePlaylist();
	void refreshTreeItem(QTreeWidgetItem *item);
	void deleteSelectedSongs();
	void addToPlaylist(QString const &path, int to, bool update);
	virtual void timerEvent(QTimerEvent *);
private slots:
	void on_toolButton_play_clicked();
	void on_treeWidget_itemExpanded(QTreeWidgetItem *item);

	void onVolumeChanged();
	void onSliderPressed();
	void onSliderReleased();

	void onTreeViewContextMenuEvent(QContextMenuEvent *);
	void onListViewContextMenuEvent(QContextMenuEvent *);
	void onDropEvent(bool done);
	void on_listWidget_playlist_doubleClicked(const QModelIndex &index);
	void on_toolButton_stop_clicked();
	void on_toolButton_prev_clicked();
	void on_toolButton_next_clicked();
	void on_toolButton_repeat_clicked();
	void on_toolButton_random_clicked();
	void on_toolButton_volume_clicked();
	void on_action_debug_triggered();
	void on_action_play_triggered();
	void on_action_stop_triggered();
	void on_action_previous_triggered();
	void on_action_next_triggered();
	void on_action_repeat_triggered();
	void on_action_random_triggered();
	void on_action_help_about_triggered();
	void on_action_edit_delete_triggered();
	void on_action_network_connect_triggered();
	void on_action_network_disconnect_triggered();
	void on_horizontalSlider_valueChanged(int value);
	void on_action_single_triggered();
	void on_action_consume_triggered();
	void on_toolButton_single_clicked();
	void on_treeWidgetItem_doubleClicked(QTreeWidgetItem *item, int column);
	void on_action_edit_cut_triggered();
	void on_action_edit_copy_triggered();
	void on_action_edit_paste_bottom_triggered();
	void on_pushButton_manage_connections_clicked();
	void on_action_edit_paste_insert_triggered();
	void on_comboBox_currentIndexChanged(int index);
	void on_action_playlist_add_location_triggered();
	void on_action_playlist_update_triggered();
	void on_action_volume_up_triggered();
	void on_action_volume_down_triggered();
	void on_action_file_close_triggered();
	void on_action_playlist_quick_save_1_triggered();
	void on_action_playlist_quick_save_2_triggered();
	void on_action_playlist_quick_load_1_triggered();
	void on_action_playlist_quick_load_2_triggered();
	void on_action_playlist_clear_triggered();
	void on_action_playlist_edit_triggered();
	void on_action_edit_keyboard_customize_triggered();
	void on_action_play_always_triggered();
	void on_action_playlist_unify_triggered();
	void on_toolButton_consume_clicked();
	void on_action_test_triggered();


private:
	Ui::MainWindow *ui;


public:
	static QString tr_Module_information_could_not_be_acquired()
	{
		return tr("Module information could not be acquired.");
	}
};

extern MainWindow *the_mainwindow;

#endif // MAINWINDOW_H
