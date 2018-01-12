#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "BasicMainWindow.h"
#include "MusicPlayerClient.h"
#include "PlaylistFile.h"
#include <QMainWindow>
#include <QModelIndex>
#include <vector>

namespace Ui {
class MainWindow;
}
class QTreeWidgetItem;
class QListWidgetItem;
class QComboBox;

struct ResultItem;

class MainWindow : public BasicMainWindow
{
	Q_OBJECT
private:
	Ui::MainWindow *ui;
private:
    QIcon folderIcon();
	QIcon songIcon();
	QIcon playlistIcon();
	void updateCurrentSongInfo();
	void updateTree(ResultItem *info);
	void clearTreeAndList();
	void updateServersComboBox();
	QString serverName() const;
	void execPrimaryCommand(QTreeWidgetItem *item);
	QString songPath(QTreeWidgetItem const *item) const;
	QString songPath(QListWidgetItem const *item, bool for_export) const;
	bool isPlaceHolder(QTreeWidgetItem *item) const;
	void on_edit_location();
	void displayProgress(const QString &text);
	void seekProgressSlider(double elapsed, double total);
	static bool isRoot(QTreeWidgetItem *item);
	static bool isFolder(QTreeWidgetItem *item);
	static bool isFile(QTreeWidgetItem *item);
	static bool isPlaylist(QTreeWidgetItem *item);
	QComboBox *serversComboBox();
	void comboboxIndexChanged(QComboBox *cbox, int index);
	void updateWindowTitle();
	void paste(int row);
	void loadPlaylistAndPlay(const QString &path);
	int playlistFileCount() const;
	void execPlaylistPropertyDialog(const QString &path);
	void updatePrimaryStatusLabel();
	void updatePlaylistMenu();
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void setRepeatEnabled(bool f);
	void setSingleEnabled(bool f);
	void setConsumeEnabled(bool f);
	void setRandomEnabled(bool f);

protected:
	virtual bool event(QEvent *event);
	virtual bool eventFilter(QObject *, QEvent *);
	virtual void closeEvent(QCloseEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	void changeEvent(QEvent *e);
	void deletePlaylistItem(QListWidgetItem *item, bool updateplaylist);
	void deleteSelectedSongs();
	void displayCurrentSongLabels(QString const &title, QString const &artist, QString const &disc);
	void refreshTreeItem(QTreeWidgetItem *item);
	void setPageConnected();
	void setPageDisconnected();
	void setVolumeEnabled(bool f);
	void updatePlayIcon();
	void updatePlaylist();
	void updateTreeTopLevel();
	void doUpdateStatus();
	void displayExtraInformation(const QString &text2, const QString &text3);
	void execConnectionDialog();
private slots:
	void on_action_consume_triggered();
	void on_action_debug_triggered();
	void on_action_edit_copy_triggered();
	void on_action_edit_cut_triggered();
	void on_action_edit_delete_triggered();
	void on_action_edit_paste_bottom_triggered();
	void on_action_edit_paste_insert_triggered();
	void on_action_file_close_triggered();
	void on_action_help_about_triggered();
	void on_action_network_connect_triggered();
	void on_action_network_disconnect_triggered();
	void on_action_network_reconnect_triggered();
	void on_action_next_triggered();
	void on_action_play_triggered();
	void on_action_playlist_add_location_triggered();
	void on_action_playlist_clear_triggered();
	void on_action_playlist_edit_triggered();
	void on_action_playlist_quick_load_1_triggered();
	void on_action_playlist_quick_load_2_triggered();
	void on_action_playlist_quick_save_1_triggered();
	void on_action_playlist_quick_save_2_triggered();
	void on_action_playlist_unify_triggered();
	void on_action_playlist_update_triggered();
	void on_action_previous_triggered();
	void on_action_random_triggered();
	void on_action_repeat_triggered();
	void on_action_single_triggered();
	void on_action_sleep_timer_triggered();
	void on_action_stop_triggered();
	void on_horizontalSlider_valueChanged(int value);
	void on_listWidget_playlist_doubleClicked(const QModelIndex &index);
	void on_toolButton_consume_clicked();
	void on_toolButton_next_clicked();
	void on_toolButton_play_clicked();
	void on_toolButton_prev_clicked();
	void on_toolButton_random_clicked();
	void on_toolButton_repeat_clicked();
	void on_toolButton_single_clicked();
	void on_toolButton_sleep_timer_clicked();
	void on_toolButton_stop_clicked();
	void on_toolButton_volume_clicked();
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_treeWidget_itemExpanded(QTreeWidgetItem *item);
	void onDropEvent(bool done);
	void onSliderPressed();
	void onSliderReleased();
	void onTreeViewContextMenuEvent(QContextMenuEvent *);
	void onListViewContextMenuEvent(QContextMenuEvent *);
	void on_comboBox_servers1_currentIndexChanged(int index);
	void on_comboBox_servers2_currentIndexChanged(int index);
	void on_listWidget_playlist_itemSelectionChanged();
};

#endif // MAINWINDOW_H
