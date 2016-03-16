#include "EditPlaylistDialog.h"
#include "ui_EditPlaylistDialog.h"
#include "RenameDialog.h"
#include <QMessageBox>
#include "BasicMainWindow.h"
#include "MySettings.h"
#include "SavePlaylistDialog.h"

EditPlaylistDialog::EditPlaylistDialog(QWidget *parent, MusicPlayerClient *mpc) :
	QDialog(parent),
	ui(new Ui::EditPlaylistDialog),
	mpc(mpc)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	{
		MySettings s;
		s.beginGroup("Playlist");
		bool showtemp = s.value("ShowTemporaryItems", false).toBool();
		s.endGroup();
		ui->checkBox_show_temporary->setChecked(showtemp);
	}

	updatePlaylistList();

	ui->listWidget_list->setFocus();
}

EditPlaylistDialog::~EditPlaylistDialog()
{
	delete ui;
}

void EditPlaylistDialog::saveSettings()
{
	bool showtemp = ui->checkBox_show_temporary->isChecked();
	{
		MySettings s;
		s.beginGroup("Playlist");
		s.setValue("ShowTemporaryItems", showtemp);
		s.endGroup();
	}
}

bool EditPlaylistDialog::isTemporaryItem(QString const &name)
{
	return name.startsWith('_') && name.endsWith('_');
}

void EditPlaylistDialog::updatePlaylistList()
{
	bool showtemp = ui->checkBox_show_temporary->isChecked();

	ui->listWidget_list->clear();
	ui->listWidget_songs->clear();
	QList<MusicPlayerClient::Item> items;
	mpc->do_lsinfo(QString(), &items);
	std::sort(items.begin(), items.end());
	for (MusicPlayerClient::Item const &item : items) {
		if (item.kind == "playlist") {
			QString name = item.text;
			if (!showtemp && name.size() > 1 && isTemporaryItem(name)) {
				// nop
			} else {
				ui->listWidget_list->addItem(name);
			}
		}
	}
}

bool EditPlaylistDialog::selectItem(QString const &name)
{
	int n = ui->listWidget_list->count();
	for (int i = 0; i < n; i++) {
		QListWidgetItem *item = ui->listWidget_list->item(i);
		if (item->text() == name) {
			ui->listWidget_list->setCurrentItem(item);
			return true;
		}
	}
	return false;
}

void EditPlaylistDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

QString EditPlaylistDialog::name() const
{
	QListWidgetItem *listitem = ui->listWidget_list->currentItem();
	if (!listitem) return QString();
	return listitem->text();
}

bool EditPlaylistDialog::forReplace() const
{
	return ui->radioButton_replace->isChecked();
}

bool EditPlaylistDialog::forAppend() const
{
	return ui->radioButton_append->isChecked();
}

void EditPlaylistDialog::accept()
{
	saveSettings();
	QDialog::accept();
}

void EditPlaylistDialog::reject()
{
	saveSettings();
	QDialog::reject();
}

void EditPlaylistDialog::on_listWidget_list_itemSelectionChanged()
{
	QListWidgetItem *listitem = ui->listWidget_list->currentItem();
	if (!listitem) return;
	QString name = listitem->text();
	ui->listWidget_songs->clear();
	using mpcitem_t = MusicPlayerClient::Item;
	QList<mpcitem_t> songs;
	mpc->do_listplaylistinfo(name, &songs);
	for (mpcitem_t const &song : songs) {
		QString path = song.text;
		QString text = song.map.get("Title");
		QString artist = song.map.get("Artist");
		QString album = song.map.get("Album");
		QString dir;
		if (text.isEmpty()) {
			int i = path.lastIndexOf('/');
			if (i >= 0) {
				dir = path.mid(0, i);
				text = path.mid(i + 1);
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
		if (!suffix.isEmpty()) {
			text += " -- " + suffix;
		}
		ui->listWidget_songs->addItem(text);
	}
}

void EditPlaylistDialog::on_listWidget_list_doubleClicked(const QModelIndex &)
{
	accept();
}

void EditPlaylistDialog::on_pushButton_rename_clicked()
{
	QListWidgetItem const *listitem = ui->listWidget_list->currentItem();
	if (!listitem) return;
	QString curname = listitem->text();
	RenameDialog dlg(this, curname, curname);
	if (dlg.exec() == QDialog::Accepted) {
		QString newname = dlg.name();
		mpc->do_rename(curname, newname);
		updatePlaylistList();
	}
}

void EditPlaylistDialog::on_pushButton_delete_clicked()
{
	QListWidgetItem *listitem = ui->listWidget_list->currentItem();
	if (!listitem) return;
	QString name = listitem->text();
	if (QMessageBox::warning(this, QApplication::applicationName(), tr("Delete the playlist:") + " \"" + name + "\"\n" + tr("Are you sure ?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
		mpc->do_rm(name);
		updatePlaylistList();
	}
}

void EditPlaylistDialog::on_pushButton_save_clicked()
{
	BasicMainWindow *mw = BasicMainWindow::findMainWindow(this);
	if (mw && !mw->validateForSavePlaylist()) {
		return;
	}

	QString name;
	{
		QListWidgetItem *listitem = ui->listWidget_list->currentItem();
		if (listitem) {
			name = listitem->text();
		}
	}
	SavePlaylistDialog dlg(this, name);
	if (dlg.exec() != QDialog::Accepted) return;
	name = dlg.name();
	if (!MusicPlayerClient::isValidPlaylistName(name)) {
		QMessageBox::warning(this, qApp->applicationName(), tr("The name is invalid."));
		return;
	}
	mpc->do_save(name);
	updatePlaylistList();
	selectItem(name);
}

void EditPlaylistDialog::on_checkBox_show_temporary_clicked()
{
	updatePlaylistList();
}
