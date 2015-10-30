#ifndef SAVEPLAYLISTDIALOG_H
#define SAVEPLAYLISTDIALOG_H

#include <QDialog>
#include "MusicPlayerClient.h"

namespace Ui {
class SavePlaylistDialog;
}

class SavePlaylistDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit SavePlaylistDialog(QWidget *parent, QString const &initname);
	~SavePlaylistDialog();
	QString name() const;
protected:
	void changeEvent(QEvent *e);
	
private:
	Ui::SavePlaylistDialog *ui;
};

#endif // SAVEPLAYLISTDIALOG_H
