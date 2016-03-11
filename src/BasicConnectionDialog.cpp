#include "BasicConnectionDialog.h"
#include "TestConnectResultDialog.h"
#include "TestConnectionThread.h"
#include "MusicPlayerClient.h"
#include <QApplication>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>

BasicConnectionDialog::BasicConnectionDialog(QWidget *parent, Host const &host)
	: QDialog(parent)
{
	current_host = host;
}

void BasicConnectionDialog::accept()
{
	savePresetServers(&servers);
	QDialog::accept();
}

void BasicConnectionDialog::reject()
{
	savePresetServers(&servers);
	QDialog::reject();
}

Host BasicConnectionDialog::host() const
{
	ServerItem const *s = selectedServer();
	if (s) return s->host;
	return Host();
}

void BasicConnectionDialog::init()
{
	new_connection = tr("New connection");

	if (ctrls.tableWidget) connect(ctrls.tableWidget, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)), this, SLOT(on_tableWidget_currentItemChanged_(QTableWidgetItem*,QTableWidgetItem*)));
	if (ctrls.tableWidget) connect(ctrls.tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(on_tableWidget_itemDoubleClicked_(QTableWidgetItem*)));
	if (ctrls.lineEdit_port) connect(ctrls.lineEdit_port, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_port_textChanged_(QString)));
	if (ctrls.lineEdit_address) connect(ctrls.lineEdit_address, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_address_textChanged_(QString)));
	if (ctrls.lineEdit_name) connect(ctrls.lineEdit_name, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_name_textChanged_(QString)));
	if (ctrls.lineEdit_password) connect(ctrls.lineEdit_password, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_password_textChanged_(QString)));
	if (ctrls.lineEdit_desc) connect(ctrls.lineEdit_desc, SIGNAL(textChanged(QString)), this, SLOT(on_lineEdit_desc_textChanged_(QString)));
	if (ctrls.pushButton_up) connect(ctrls.pushButton_up, SIGNAL(clicked(bool)), SLOT(on_pushButton_up_clicked_()));
	if (ctrls.pushButton_down) connect(ctrls.pushButton_down, SIGNAL(clicked(bool)), SLOT(on_pushButton_down_clicked_()));
	if (ctrls.pushButton_new) connect(ctrls.pushButton_new, SIGNAL(clicked(bool)), SLOT(on_pushButton_new_clicked_()));
	if (ctrls.pushButton_delete) connect(ctrls.pushButton_delete, SIGNAL(clicked(bool)), SLOT(on_pushButton_delete_clicked_()));
	if (ctrls.pushButton_test_connection) connect(ctrls.pushButton_test_connection, SIGNAL(clicked(bool)), SLOT(on_pushButton_test_connection_clicked_()));

	if (ctrls.tableWidget) {
		ctrls.tableWidget->horizontalHeader()->setVisible(true);
		ctrls.tableWidget->verticalHeader()->setVisible(false);
		ctrls.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ctrls.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		ctrls.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
		ctrls.tableWidget->horizontalHeader()->setStretchLastSection(true);
	}

	int size = ctrls.tableWidget->font().pointSize();
#if defined(Q_OS_WIN)
	size = size * 96 / 72;
#endif
	table_row_height = size + 8;

	loadServers();
}

void BasicConnectionDialog::loadServers()
{
	loadPresetServers(&servers);
	updateList();
}

void BasicConnectionDialog::updateList(bool add_new_connection)
{
	int current_row = ctrls.tableWidget->currentRow();

	if (add_new_connection) {
		int i = (int)servers.size();
		while (i > 0) {
			i--;
			ServerItem &item = servers[i];
			if (item.name == new_connection && item.host.address().isEmpty()) {
				servers.erase(servers.begin() + i);
			}
		}
		ServerItem newitem;
		newitem.name = new_connection;
		newitem.host.setPort(DEFAULT_MPD_PORT);
		current_row = servers.size(); // select this
		servers.push_back(newitem);
	}

	int rowcount = (int)servers.size();
	if (current_row < 0) current_row = 0;
	ctrls.tableWidget->clearContents();

	ctrls.tableWidget->setColumnCount(4);
	ctrls.tableWidget->setHorizontalHeaderItem(C_NAME, new QTableWidgetItem(tr("Name")));
	ctrls.tableWidget->setHorizontalHeaderItem(C_ADDR, new QTableWidgetItem(tr("Address")));
	ctrls.tableWidget->setHorizontalHeaderItem(C_PORT, new QTableWidgetItem(tr("Port")));
	ctrls.tableWidget->setHorizontalHeaderItem(C_DESC, new QTableWidgetItem(tr("Description")));

	ctrls.tableWidget->setRowCount(rowcount);
	QTableWidgetItem *sel = 0;
	for (int i = 0; i < rowcount; i++) {
		ServerItem const &server = servers[i];
		ctrls.tableWidget->setRowHeight(i, table_row_height);
		QTableWidgetItem *item;

		item = new QTableWidgetItem(server.name);
		ctrls.tableWidget->setItem(i, C_NAME, item);
		if (i == current_row) sel = item;

		item = new QTableWidgetItem(server.host.address());
		ctrls.tableWidget->setItem(i, C_ADDR, item);

		item = new QTableWidgetItem(QString::number(server.host.port()));
		ctrls.tableWidget->setItem(i, C_PORT, item);

		item = new QTableWidgetItem(server.description);
		ctrls.tableWidget->setItem(i, C_DESC, item);
	}
	if (current_row >= rowcount) {
		current_row = rowcount - 1;
	}
	ctrls.tableWidget->resizeColumnsToContents();
	ctrls.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ctrls.tableWidget->setCurrentItem(sel);
}

ServerItem *BasicConnectionDialog::selectedServer()
{
	int row = ctrls.tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *s = &servers[row];
		current_host = s->host;
		return s;
	}
	return 0;
}

const ServerItem *BasicConnectionDialog::selectedServer() const
{
	return const_cast<BasicConnectionDialog *>(this)->selectedServer();
}

void BasicConnectionDialog::selectServer(const ServerItem *server)
{
	if (ctrls.lineEdit_name) ctrls.lineEdit_name->setText(server->name);
	if (ctrls.lineEdit_address) ctrls.lineEdit_address->setText(server->host.address());
	if (ctrls.lineEdit_port) ctrls.lineEdit_port->setText(QString::number(server->host.port()));
	if (ctrls.lineEdit_password) ctrls.lineEdit_password->setText(server->host.password());
	if (ctrls.lineEdit_desc) ctrls.lineEdit_desc->setText(server->description);
}

void BasicConnectionDialog::updateServer(int row, int col)
{
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *server = &servers[row];
		QTableWidgetItem *item = ctrls.tableWidget->item(row, col);
		if (item) {
			switch (col) {
			case C_NAME:
				server->name = ctrls.lineEdit_name->text();
				if (server->name != item->text()) {
					item->setText(server->name);
				}
				break;
			case C_ADDR:
				server->host.setAddress(ctrls.lineEdit_address->text());
				if (server->host.address() != item->text()) {
					item->setText(server->host.address());
				}
				break;
			case C_PORT:
				server->host.setPort(ctrls.lineEdit_port->text().toInt());
				item->setText(QString::number(server->host.port()));
				break;
			case C_DESC:
				server->description = ctrls.lineEdit_desc->text();
				item->setText(server->description);
				break;
			}
			ctrls.tableWidget->resizeColumnsToContents();
			ctrls.tableWidget->horizontalHeader()->setStretchLastSection(true);
		}
	}
}

void BasicConnectionDialog::saveServers()
{
	if (!savePresetServers(&servers)) {
		QMessageBox::critical(this, qApp->applicationName(), tr("Could not create the file."));
	}
}

void BasicConnectionDialog::testConnection()
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



void BasicConnectionDialog::addNewConnection()
{
	updateList(true);
	ctrls.lineEdit_name->setFocus();
	ctrls.lineEdit_name->selectAll();
}

void BasicConnectionDialog::deleteConnection()
{
	int row = ctrls.tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem const &server = servers[row];
		auto DO = [&](){
			servers.erase(servers.begin() + row);
			updateList();
			if (row >= (int)servers.size()) {
				row = servers.size() - 1;
			}
			if (row >= 0) {
				ctrls.tableWidget->selectRow(row);
			}
		};
		if (server.name == new_connection && server.host.address().isEmpty()) {
			DO();
		} else if (QMessageBox::warning(this, qApp->applicationName(), tr("Are you sure you want to delete '%1' ?").arg(server.name), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
			DO();
		}
	}
}

void BasicConnectionDialog::moveUpConnection()
{
	int row = ctrls.tableWidget->currentRow();
	if (row > 0 && row < (int)servers.size()) {
		std::swap(servers[row - 1], servers[row]);
		updateList();
		ctrls.tableWidget->selectRow(row - 1);
	}
}

void BasicConnectionDialog::moveDownConnection()
{
	int row = ctrls.tableWidget->currentRow();
	if (row >= 0 && row + 1 < (int)servers.size()) {
		std::swap(servers[row], servers[row + 1]);
		updateList();
		ctrls.tableWidget->selectRow(row + 1);
	}
}

void BasicConnectionDialog::onPasswordChanged(const QString &str)
{
	int row = ctrls.tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *server = &servers[row];
		server->host.setPassword(str);
	}
}

void BasicConnectionDialog::on_lineEdit_address_textChanged_(const QString &)
{
	updateServer(ctrls.tableWidget->currentRow(), C_ADDR);
}

void BasicConnectionDialog::on_lineEdit_desc_textChanged_(const QString &)
{
	updateServer(ctrls.tableWidget->currentRow(), C_DESC);
}

void BasicConnectionDialog::on_lineEdit_name_textChanged_(const QString &)
{
	updateServer(ctrls.tableWidget->currentRow(), C_NAME);
}

void BasicConnectionDialog::on_lineEdit_port_textChanged_(const QString &)
{
	updateServer(ctrls.tableWidget->currentRow(), C_PORT);
}

void BasicConnectionDialog::on_lineEdit_password_textChanged_(const QString &str)
{
	onPasswordChanged(str);
}

void BasicConnectionDialog::on_pushButton_delete_clicked_()
{
	deleteConnection();
}

void BasicConnectionDialog::on_pushButton_new_clicked_()
{
	addNewConnection();
}

void BasicConnectionDialog::on_pushButton_test_connection_clicked_()
{
	testConnection();
}

void BasicConnectionDialog::on_tableWidget_currentItemChanged_(QTableWidgetItem *, QTableWidgetItem *)
{
	ServerItem const *s = selectedServer();
	if (s) selectServer(s);
}

void BasicConnectionDialog::on_tableWidget_itemDoubleClicked_(QTableWidgetItem *)
{
	if (current_host.address().isEmpty()) {
		QLineEdit *p = ctrls.lineEdit_name;
		if (p->text() != new_connection) {
			p = ctrls.lineEdit_address;
		}
		p->setFocus();
		p->home(false);
		p->end(true);
	} else {
		accept();
	}
}

void BasicConnectionDialog::on_pushButton_up_clicked_()
{
	moveUpConnection();
}

void BasicConnectionDialog::on_pushButton_down_clicked_()
{
	moveDownConnection();
}

