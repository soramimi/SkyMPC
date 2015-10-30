#include "DiagnosticDialog.h"
#include "ui_DiagnosticDialog.h"
#include <QFileDialog>
#include "MainWindow.h"
#include <QFile>
#include <string>

DiagnosticDialog::DiagnosticDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DiagnosticDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
}

DiagnosticDialog::~DiagnosticDialog()
{
	delete ui;
}

void DiagnosticDialog::changeEvent(QEvent *e)
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

void DiagnosticDialog::setText(QString const &text)
{
	ui->textEdit->setPlainText(text);
}

void DiagnosticDialog::on_pushButton_saveas_clicked()
{
	QString path = QFileDialog::getSaveFileName(the_mainwindow, qApp->applicationName(), "diagnostic.txt", "Text files (*.txx)");
	if (path.isEmpty()) return;

	QFile file(path);
	if (file.open(QFile::WriteOnly | QFile::Truncate)) {
		std::string text = ui->textEdit->toPlainText().toStdString();
		file.write(text.c_str(), text.size());
		file.close();
	}
}
