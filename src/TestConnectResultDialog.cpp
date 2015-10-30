#include "TestConnectResultDialog.h"
#include "ui_TestConnectResultDialog.h"

TestConnectResultDialog::TestConnectResultDialog(QWidget *parent, QString const &message) :
	QDialog(parent),
	ui(new Ui::TestConnectResultDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui->plainTextEdit->setPlainText(message);
}

TestConnectResultDialog::~TestConnectResultDialog()
{
	delete ui;
}
