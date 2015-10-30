#ifndef SONGPROPERTYDIALOG_H
#define SONGPROPERTYDIALOG_H

#include "MusicPlayerClient.h"
#include <QDialog>

namespace Ui {
class SongPropertyDialog;
}

class SongPropertyDialog : public QDialog
{
	Q_OBJECT
private:
	bool add_to_playlist_clicked;
public:
	explicit SongPropertyDialog(QWidget *parent, std::vector<MusicPlayerClient::KeyValue> const *data, bool use_add_to_playlist);
	~SongPropertyDialog();
	bool addToPlaylistClicked() const;
protected:
	void changeEvent(QEvent *e);
	
private slots:
	void on_pushButton_add_to_playlist_clicked();

private:
	Ui::SongPropertyDialog *ui;
};

#endif // SONGPROPERTYDIALOG_H
