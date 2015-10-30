#include "AskRemoveDuplicatedFileDialog.h"
#include "ui_AskRemoveDuplicatedFileDialog.h"

AskRemoveDuplicatedFileDialog::AskRemoveDuplicatedFileDialog(QWidget *parent, QString const &text) :
	QDialog(parent),
	ui(new Ui::AskRemoveDuplicatedFileDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui->plainTextEdit->setPlainText(text);
}

AskRemoveDuplicatedFileDialog::~AskRemoveDuplicatedFileDialog()
{
	delete ui;
}

void AskRemoveDuplicatedFileDialog::changeEvent(QEvent *e)
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
