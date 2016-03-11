#include "MusicPlayerClient.h"
#include "TinyConnectionDialog.h"
#include "ui_TinyConnectionDialog.h"
#include "QMessageBox"
#include "TestConnectResultDialog.h"
#include "TestConnectionThread.h"

TinyConnectionDialog::TinyConnectionDialog(QWidget *parent, Host const &host)
	: BasicConnectionDialog(parent, host)
	, ui(new Ui::TinyConnectionDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	setWindowState(Qt::WindowFullScreen);

	ctrls.tableWidget = ui->tableWidget;
	ctrls.lineEdit_port = ui->lineEdit_port;
	ctrls.lineEdit_address = ui->lineEdit_address;
	ctrls.lineEdit_name = ui->lineEdit_name;
	ctrls.lineEdit_password = ui->lineEdit_password;
	ctrls.pushButton_new = ui->pushButton_new;
	ctrls.pushButton_delete = ui->pushButton_delete;
	ctrls.pushButton_test_connection = ui->pushButton_test_connection;

	init();
}

TinyConnectionDialog::~TinyConnectionDialog()
{
	delete ui;
}



