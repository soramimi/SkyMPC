#include "SongPropertyDialog.h"
#include "ui_SongPropertyDialog.h"

SongPropertyDialog::SongPropertyDialog(QWidget *parent, std::vector<MusicPlayerClient::KeyValue> const *data, bool use_add_to_playlist) :
	QDialog(parent),
	ui(new Ui::SongPropertyDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui->pushButton_add_to_playlist->setVisible(use_add_to_playlist);
	add_to_playlist_clicked = false;

	int fontsize = ui->tableWidget->font().pointSize();
#if defined(Q_OS_WIN)
	fontsize = fontsize * 96 / 72;
#endif

	int n = data->size();
	ui->tableWidget->setColumnCount(2);
	ui->tableWidget->setRowCount(n);
	QTableWidgetItem *item;
	item = new QTableWidgetItem(tr("Name"));
	ui->tableWidget->setHorizontalHeaderItem(0, item);
	item = new QTableWidgetItem(tr("Value"));
	ui->tableWidget->setHorizontalHeaderItem(1, item);
	for (int i = 0; i < n; i++) {
		QString key = data->at(i).key;
		QString val = data->at(i).value;
		if (key == "Time") {
			unsigned int t = val.toUInt();
			if (t > 0) {
				char tmp[100];
				sprintf(tmp, " (%u:%02u)", t / 60, t % 60);
				val += tmp;
			}
		}
		item = new QTableWidgetItem(key);
		ui->tableWidget->setItem(i, 0, item);
		item = new QTableWidgetItem(val);
		ui->tableWidget->setItem(i, 1, item);
		ui->tableWidget->setRowHeight(i, fontsize + 8);
	}
	ui->tableWidget->resizeColumnsToContents();
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

SongPropertyDialog::~SongPropertyDialog()
{
	delete ui;
}

bool SongPropertyDialog::addToPlaylistClicked() const
{
	return add_to_playlist_clicked;
}

void SongPropertyDialog::changeEvent(QEvent *e)
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

void SongPropertyDialog::on_pushButton_add_to_playlist_clicked()
{
	add_to_playlist_clicked = true;
	accept();
}
