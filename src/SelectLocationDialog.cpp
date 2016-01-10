#include "SelectLocationDialog.h"
#include "ui_SelectLocationDialog.h"

SelectLocationDialog::SelectLocationDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SelectLocationDialog)
{
	ui->setupUi(this);
}

SelectLocationDialog::~SelectLocationDialog()
{
	delete ui;
}

void SelectLocationDialog::setItems(const std::vector<PlaylistFile::Item> *items)
{
	this->items = items;
	ui->tableWidget->setRowCount(items->size());
	ui->tableWidget->setColumnCount(2);
	ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Title")));
	ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Location")));
	int i = 0;
	for (PlaylistFile::Item const &item : *items) {
		auto ti0 = new QTableWidgetItem();
		ti0->setText(item.title);
		ui->tableWidget->setItem(i, 0, ti0);
		auto ti1 = new QTableWidgetItem();
		ti1->setText(item.file);
		ui->tableWidget->setItem(i, 1, ti1);
		i++;
	}

	ui->tableWidget->resizeColumnsToContents();
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

	ui->tableWidget->selectRow(0);
}

void SelectLocationDialog::selectedItems(std::vector<PlaylistFile::Item> *out) const
{
	Q_ASSERT(items);
	out->clear();
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)items->size()) {
		out->push_back(items->at(row));
	}
}



