#include "TestConnectResultDialog.h"
#include "ui_TestConnectResultDialog.h"
#include <QThread>

TestConnectResultDialog::TestConnectResultDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TestConnectResultDialog),
	thread(0)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
}

TestConnectResultDialog::~TestConnectResultDialog()
{
	delete ui;
}

void TestConnectResultDialog::setMessage(QString const &message)
{
	ui->plainTextEdit->setPlainText(message);
	QApplication::processEvents();
}

void TestConnectResultDialog::reject()
{
	if (thread) {
		if (thread->isRunning()) {
			ui->plainTextEdit->setPlainText(tr("Wait..."));
			ui->pushButton->setEnabled(false);
			QApplication::processEvents();
		}
		if (thread) thread->wait();
	}
	QDialog::reject();
}
