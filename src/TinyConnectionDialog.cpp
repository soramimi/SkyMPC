#include "MusicPlayerClient.h"
#include "TinyConnectionDialog.h"
#include "ui_TinyConnectionDialog.h"
#include "QMessageBox"
#include "TestConnectResultDialog.h"
#include "TestConnectionThread.h"

enum {
	C_NAME,
	C_ADDR,
	C_PORT,
	C_DESC,
};

TinyConnectionDialog::TinyConnectionDialog(QWidget *parent, Host const &host)
	: QDialog(parent)
	, ui(new Ui::TinyConnectionDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	setWindowState(Qt::WindowFullScreen);

	ui->tableWidget->horizontalHeader()->setVisible(true);

	int size = ui->tableWidget->font().pointSize();
#if defined(Q_OS_WIN)
	size = size * 96 / 72;
#endif
	row_height = size + 8;

	loadServers();

	current_host = host;

	if (servers.empty()) {
		ServerItem item;
		item.name = "New";
		item.host = host;
		if (item.host.port() < 1 || item.host.port() > 65535) {
			item.host.setPort(DEFAULT_MPD_PORT);
		}
		int row = (int)servers.size();
		servers.push_back(item);
		updateList();
		ui->tableWidget->selectRow(row);
		ui->lineEdit_name->setFocus();
		ui->lineEdit_name->selectAll();
	} else {
		int row;
		for (row = 0; row < (int)servers.size(); row++) {
			if (host == servers[row].host) {
				break;
			}
		}
		if (row < (int)servers.size()) {
			ui->tableWidget->selectRow(row);
		}
	}
}

TinyConnectionDialog::~TinyConnectionDialog()
{
	delete ui;
}

void TinyConnectionDialog::accept()
{
	savePresetServers(&servers);
	QDialog::accept();

}

void TinyConnectionDialog::reject()
{
	savePresetServers(&servers);
	QDialog::reject();

}

ServerItem *TinyConnectionDialog::selectedServer()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *s = &servers[row];
		current_host = s->host;
		return s;
	}
	return 0;
}

ServerItem const *TinyConnectionDialog::selectedServer() const
{
	return const_cast<TinyConnectionDialog *>(this)->selectedServer();
}


Host TinyConnectionDialog::host() const
{
	ServerItem const *s = selectedServer();
	if (s) {
		return s->host;
	}
	return Host();
}

void TinyConnectionDialog::loadServers()
{
	loadPresetServers(&servers);
	updateList();
}

void TinyConnectionDialog::updateList()
{
	int row = ui->tableWidget->currentRow();
	int rowcount = (int)servers.size();
	if (row < 0) row = 0;
	ui->tableWidget->clearContents();

	ui->tableWidget->setColumnCount(4);
	ui->tableWidget->setHorizontalHeaderItem(C_NAME, new QTableWidgetItem(tr("Name")));
	ui->tableWidget->setHorizontalHeaderItem(C_ADDR, new QTableWidgetItem(tr("Address")));
	ui->tableWidget->setHorizontalHeaderItem(C_PORT, new QTableWidgetItem(tr("Port")));
	ui->tableWidget->setHorizontalHeaderItem(C_DESC, new QTableWidgetItem(tr("Description")));

	ui->tableWidget->setRowCount(rowcount);
	QTableWidgetItem *sel = 0;
	for (int i = 0; i < rowcount; i++) {
		ServerItem const &server = servers[i];
		ui->tableWidget->setRowHeight(i, row_height);
		QTableWidgetItem *item;

		item = new QTableWidgetItem(server.name);
		ui->tableWidget->setItem(i, C_NAME, item);
		if (i == row) sel = item;

		item = new QTableWidgetItem(server.host.address());
		ui->tableWidget->setItem(i, C_ADDR, item);

		item = new QTableWidgetItem(QString::number(server.host.port()));
		ui->tableWidget->setItem(i, C_PORT, item);

		item = new QTableWidgetItem(server.description);
		ui->tableWidget->setItem(i, C_DESC, item);
	}
	if (row >= rowcount) {
		row = rowcount - 1;
	}
	ui->tableWidget->resizeColumnsToContents();
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget->setCurrentItem(sel);
}

void TinyConnectionDialog::selectServer(ServerItem const *server)
{
	ui->lineEdit_name->setText(server->name);
	ui->lineEdit_address->setText(server->host.address());
	ui->lineEdit_port->setText(QString::number(server->host.port()));
	ui->lineEdit_password->setText(server->host.password());
//	ui->lineEdit_desc->setText(server->description);
}

void TinyConnectionDialog::updateServer(int row, int col)
{
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *server = &servers[row];
		QTableWidgetItem *item = ui->tableWidget->item(row, col);
		if (item) {
			switch (col) {
			case C_NAME:
				server->name = ui->lineEdit_name->text();
				if (server->name != item->text()) {
					item->setText(server->name);
				}
				break;
			case C_ADDR:
				server->host.setAddress(ui->lineEdit_address->text());
				if (server->host.address() != item->text()) {
					item->setText(server->host.address());
				}
				break;
			case C_PORT:
				server->host.setPort(ui->lineEdit_port->text().toInt());
				item->setText(QString::number(server->host.port()));
				break;
			case C_DESC:
//				server->description = ui->lineEdit_desc->text();
//				item->setText(server->description);
				break;
			}
			ui->tableWidget->resizeColumnsToContents();
			ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
		}
	}
}

void TinyConnectionDialog::on_tableWidget_currentItemChanged(QTableWidgetItem * /*current*/, QTableWidgetItem * /*previous*/)
{
	ServerItem const *s = selectedServer();
	if (s) {
		selectServer(s);
	}
}

void TinyConnectionDialog::on_lineEdit_name_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_NAME);
}

void TinyConnectionDialog::on_lineEdit_address_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_ADDR);
}

void TinyConnectionDialog::on_lineEdit_port_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_PORT);
}

void TinyConnectionDialog::on_lineEdit_password_textChanged(const QString &str)
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *server = &servers[row];
		server->host.setPassword(str);
	}
}

void TinyConnectionDialog::on_pushButton_delete_clicked()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem const &server = servers[row];
		if (QMessageBox::warning(this, qApp->applicationName(), tr("Are you sure you want to delete '%1' ?").arg(server.name), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
			servers.erase(servers.begin() + row);
			updateList();
			if (row >= (int)servers.size()) {
				row = servers.size() - 1;
			}
			if (row >= 0) {
				ui->tableWidget->selectRow(row);
			}
		}
	}
}

void TinyConnectionDialog::on_pushButton_new_clicked()
{
	ServerItem item;
	item.host = current_host;
	if (item.host.port() < 1 || item.host.port() > 65535) {
		item.host.setPort(DEFAULT_MPD_PORT);
	}
	int row = (int)servers.size();
	servers.push_back(item);
	updateList();
	ui->tableWidget->selectRow(row);
	ui->lineEdit_name->setFocus();
	ui->lineEdit_name->selectAll();
}

void TinyConnectionDialog::on_pushButton_test_connection_clicked()
{
	ServerItem const *s = selectedServer();
	if (s) {
		TestConnectResultDialog dlg(this);
		dlg.show();
		TestConnectionThread th(s, &dlg);
		connect(&th, SIGNAL(updateMessage(QString)), &dlg, SLOT(setMessage(QString)));
		th.start();
		dlg.exec();
	}
}

void TinyConnectionDialog::on_tableWidget_itemDoubleClicked(QTableWidgetItem * /*item*/)
{
	accept();
}

