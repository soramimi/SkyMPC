#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"
#include "TestConnectResultDialog.h"
#include <QMessageBox>

void removeKeyAcceleratorText(QObject *obj);

enum {
	C_NAME,
	C_ADDR,
	C_PORT,
	C_DESC,
};

ConnectionDialog::ConnectionDialog(QWidget *parent, Host const &host) :
	QDialog(parent),
	ui(new Ui::ConnectionDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

#if defined(Q_OS_MAC)
	removeKeyAcceleratorText(this);
#endif

	int size = ui->tableWidget->font().pointSize();
#if defined(Q_OS_WIN)
	size = size * 96 / 72;
#endif
	row_height = size + 8;

	loadServers();

	current_host = host;

	int row;
	for (row = 0; row < (int)servers.size(); row++) {
		if (host == servers[row].host) {
			break;
		}
	}
	if (row < (int)servers.size()) {
		ui->tableWidget->selectRow(row);
	} else {
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
	}
}

ConnectionDialog::~ConnectionDialog()
{
	delete ui;
}

void ConnectionDialog::accept()
{
	savePresetServers(&servers);
	QDialog::accept();
}


void ConnectionDialog::on_pushButton_save_and_close_clicked()
{
	savePresetServers(&servers);
	reject();
}

Host ConnectionDialog::host() const
{
	ServerItem const *s = selectedServer();
	if (s) {
		return s->host;
	}
	return Host();
}

void ConnectionDialog::updateList()
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
		QString portstr = QString::number(server.host.port());
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

void ConnectionDialog::loadServers()
{
	loadPresetServers(&servers);
	updateList();
}

void ConnectionDialog::saveServers()
{
	if (!savePresetServers(&servers)) {
		QMessageBox::critical(this, QApplication::applicationName(), tr("Could not create the file."));
	}
}

ServerItem *ConnectionDialog::selectedServer()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *s = &servers[row];
		current_host = s->host;
		return s;
	}
	return 0;
}

ServerItem const *ConnectionDialog::selectedServer() const
{
	return const_cast<ConnectionDialog *>(this)->selectedServer();
}

void ConnectionDialog::selectServer(ServerItem const *server)
{
	ui->lineEdit_name->setText(server->name);
	ui->lineEdit_address->setText(server->host.address());
	ui->lineEdit_port->setText(QString::number(server->host.port()));
	ui->lineEdit_password->setText(server->host.password());
	ui->lineEdit_desc->setText(server->description);
}

void ConnectionDialog::updateServer(int row, int col)
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
				server->description = ui->lineEdit_desc->text();
				item->setText(server->description);
				break;
			}
			ui->tableWidget->resizeColumnsToContents();
			ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
		}
	}
}

void ConnectionDialog::on_pushButton_up_clicked()
{
	int row = ui->tableWidget->currentRow();
	if (row > 0 && row < (int)servers.size()) {
		std::swap(servers[row - 1], servers[row]);
		updateList();
		ui->tableWidget->selectRow(row - 1);
	}
}

void ConnectionDialog::on_pushButton_down_clicked()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row + 1 < (int)servers.size()) {
		std::swap(servers[row], servers[row + 1]);
		updateList();
		ui->tableWidget->selectRow(row + 1);
	}
}

void ConnectionDialog::on_tableWidget_currentItemChanged(QTableWidgetItem * /*current*/, QTableWidgetItem * /*previous*/)
{
	ServerItem const *s = selectedServer();
	if (s) {
		selectServer(s);
	}
}

void ConnectionDialog::on_lineEdit_name_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_NAME);
}

void ConnectionDialog::on_lineEdit_desc_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_DESC);
}

void ConnectionDialog::on_lineEdit_address_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_ADDR);
}

void ConnectionDialog::on_lineEdit_port_textChanged(const QString &)
{
	updateServer(ui->tableWidget->currentRow(), C_PORT);
}

void ConnectionDialog::on_lineEdit_password_textChanged(const QString &str)
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
		ServerItem *server = &servers[row];
		server->host.setPassword(str);
	}
}

void ConnectionDialog::on_pushButton_delete_clicked()
{
	int row = ui->tableWidget->currentRow();
	if (row >= 0 && row < (int)servers.size()) {
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

void ConnectionDialog::on_pushButton_new_clicked()
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

void ConnectionDialog::on_pushButton_test_connection_clicked()
{
	ServerItem const *s = selectedServer();
	if (s) {
		QString message;
		QTcpSocket sock;
		try {
			MusicPlayerClient::OpenResult r = MusicPlayerClient::open(&sock, s->host);
			message = r.log;
			message += "\n---\n";
			if (r.success) {
				message += tr("Connection was successfully established.") + '\n';
			}
			if (r.incorrect_password) {
				message += tr("Authentication failure.") + '\n';
			} else if (!r.success) {
				message += tr("Unexplained connection failure.") + '\n';
			}
		} catch (QString const &e) {
			message = e;
		}
		sock.close();
		TestConnectResultDialog dlg(this, message);
		dlg.exec();
	}
}

void ConnectionDialog::on_tableWidget_itemDoubleClicked(QTableWidgetItem * /*item*/)
{
	accept();
}

