#include "AskRemoveOverlappedFileDialog.h"
#include "ui_AskRemoveOverlappedFileDialog.h"

AskRemoveOverlappedFileDialog::AskRemoveOverlappedFileDialog(QWidget *parent, QString const &text) :
	QDialog(parent),
	ui(new Ui::AskRemoveOverlappedFileDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui->plainTextEdit->setPlainText(text);
}

AskRemoveOverlappedFileDialog::~AskRemoveOverlappedFileDialog()
{
	delete ui;
}

void AskRemoveOverlappedFileDialog::changeEvent(QEvent *e)
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
