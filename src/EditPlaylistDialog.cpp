#include "EditPlaylistDialog.h"
#include "ui_EditPlaylistDialog.h"
#include "RenameDialog.h"
#include <QMessageBox>

void removeKeyAcceleratorText(QObject *obj);

EditPlaylistDialog::EditPlaylistDialog(QWidget *parent, MusicPlayerClient *mpc) :
	QDialog(parent),
	ui(new Ui::EditPlaylistDialog),
	mpc(mpc)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

#if defined(Q_OS_MAC)
	removeKeyAcceleratorText(this);
#endif

	updatePlaylistList();

	ui->listWidget_list->setFocus();
}

EditPlaylistDialog::~EditPlaylistDialog()
{
	delete ui;
}

void EditPlaylistDialog::updatePlaylistList()
{
	ui->listWidget_list->clear();
	ui->listWidget_songs->clear();
	std::vector<MusicPlayerClient::Item> items;
	mpc->do_lsinfo(QString(), &items);
	std::sort(items.begin(), items.end());
	for (MusicPlayerClient::Item const &item : items) {
		if (item.kind == "playlist") {
			QString name = item.text;
			ui->listWidget_list->addItem(name);
		}
	}
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

void EditPlaylistDialog::on_listWidget_list_itemSelectionChanged()
{
	QListWidgetItem *listitem = ui->listWidget_list->currentItem();
	if (!listitem) return;
	QString name = listitem->text();
	ui->listWidget_songs->clear();
	std::vector<MusicPlayerClient::Item> songs;
	mpc->do_listplaylistinfo(name, &songs);
	for (MusicPlayerClient::Item const &song : songs) {
		QString path = song.text;
		QString text = song.map.get("Title");
		QString artist = song.map.get("Artist");
		QString album = song.map.get("Album");
		QString dir;
		if (text.isEmpty()) {
			ushort const *str = path.utf16();
			ushort const *ptr = ucsrchr(str, '/');
			if (ptr) {
				dir = QString::fromUtf16(str, ptr - str);
				text = QString::fromUtf16(ptr + 1);
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
	QListWidgetItem *listitem = ui->listWidget_list->currentItem();
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


#include "SavePlaylistDialog.h"
bool isValidPlaylistName(QString const &name);


void EditPlaylistDialog::on_pushButton_save_clicked()
{
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
	if (!isValidPlaylistName(name)) {
		QMessageBox::warning(this, qApp->applicationName(), tr("The name is invalid."));
		return;
	}
	mpc->do_save(name);
	updatePlaylistList();
}
