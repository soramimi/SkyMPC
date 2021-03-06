#ifndef LOADPLAYLISTDIALOG_H
#define LOADPLAYLISTDIALOG_H

#include <QDialog>
#include "MusicPlayerClient.h"

namespace Ui {
class EditPlaylistDialog;
}
class QModelIndex;

class BasicMainWindow;

class EditPlaylistDialog : public QDialog
{
	Q_OBJECT
private:
	Ui::EditPlaylistDialog *ui;
	BasicMainWindow *mainwindow;
	MusicPlayerClient *mpc();

	void updatePlaylistList();
	void saveSettings();
	bool selectItem(const QString &name);
public:
	static bool isTemporaryItem(const QString &name);
public:
	explicit EditPlaylistDialog(BasicMainWindow *parent/*, MusicPlayerClient *mpc*/);
	~EditPlaylistDialog();
	QString name() const;
	bool forReplace() const;
	bool forAppend() const;
	virtual void accept();

	virtual void reject();

protected:
	void changeEvent(QEvent *e);
private slots:
	void on_listWidget_list_itemSelectionChanged();
	void on_listWidget_list_doubleClicked(const QModelIndex &index);
	void on_pushButton_rename_clicked();
	void on_pushButton_delete_clicked();
	void on_pushButton_save_clicked();
	void on_checkBox_show_temporary_clicked();
};

#endif // LOADPLAYLISTDIALOG_H
