#include "ConnectionDialog.h"
#include "ui_ConnectionDialog.h"
#include "BasicMainWindow.h"
#include "MySettings.h"
#include <QMessageBox>

ConnectionDialog::ConnectionDialog(QWidget *parent, Host const &host)
	: BasicConnectionDialog(parent, host)
	, ui(new Ui::ConnectionDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ctrls.tableWidget = ui->tableWidget;
	ctrls.lineEdit_port = ui->lineEdit_port;
	ctrls.lineEdit_address = ui->lineEdit_address;
	ctrls.lineEdit_name = ui->lineEdit_name;
	ctrls.lineEdit_password = ui->lineEdit_password;
	ctrls.lineEdit_desc = ui->lineEdit_desc;
	ctrls.pushButton_up = ui->pushButton_up;
	ctrls.pushButton_down = ui->pushButton_down;
	ctrls.pushButton_new = ui->pushButton_new;
	ctrls.pushButton_delete = ui->pushButton_delete;
	ctrls.pushButton_test_connection = ui->pushButton_test_connection;

	init();

	BasicMainWindow *mw = BasicMainWindow::findMainWindow(this);
	if (mw) {
		bool f = mw->isAutoReconnectAtStartup();
		ui->checkBox_auto_reconnect->setChecked(f);
	}
}

ConnectionDialog::~ConnectionDialog()
{
	delete ui;
}

void ConnectionDialog::accept()
{
	saveAutoReconnect();
	BasicConnectionDialog::accept();
}

void ConnectionDialog::on_pushButton_close_clicked()
{
	saveAutoReconnect();
	BasicConnectionDialog::close();
}

bool ConnectionDialog::isAutoReconnect() const
{
	return ui->checkBox_auto_reconnect->isChecked();
}

void ConnectionDialog::saveAutoReconnect()
{
	MySettings settings;
	settings.beginGroup("Connection");
	bool f = isAutoReconnect();
	settings.setValue(KEY_AutoReconnect, f);
	settings.endGroup();
}

