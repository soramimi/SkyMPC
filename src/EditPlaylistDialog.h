#ifndef LOADPLAYLISTDIALOG_H
#define LOADPLAYLISTDIALOG_H

#include <QDialog>
#include "MusicPlayerClient.h"

namespace Ui {
class EditPlaylistDialog;
}
class QModelIndex;

class EditPlaylistDialog : public QDialog
{
	Q_OBJECT
private:
	MusicPlayerClient *mpc;
	void updatePlaylistList();
public:
	explicit EditPlaylistDialog(QWidget *parent, MusicPlayerClient *mpc);
	~EditPlaylistDialog();
	QString name() const;
	bool forReplace() const;
	bool forAppend() const;
protected:
	void changeEvent(QEvent *e);
private slots:
	void on_listWidget_list_itemSelectionChanged();

	void on_listWidget_list_doubleClicked(const QModelIndex &index);

	void on_pushButton_rename_clicked();

	void on_pushButton_delete_clicked();

	void on_pushButton_save_clicked();

private:
	Ui::EditPlaylistDialog *ui;
};

#endif // LOADPLAYLISTDIALOG_H
