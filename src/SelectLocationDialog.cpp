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
		QTableWidgetItem *ti;
		ti = new QTableWidgetItem();
		ti->setText(item.title);
		ui->tableWidget->setItem(i, 0, ti);
		ti = new QTableWidgetItem();
		ti->setText(item.file);
		ui->tableWidget->setItem(i, 1, ti);
		i++;
	}

	ui->tableWidget->resizeColumnsToContents();
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

	ui->tableWidget->selectRow(0);
}

void SelectLocationDialog::selectedItems(std::vector<PlaylistFile::Item> *out) const
{
	out->clear();
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < items->size()) {
		out->push_back(items->at(row));
	}
}



