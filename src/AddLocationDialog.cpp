#include "AddLocationDialog.h"
#include "ui_AddLocationDialog.h"

AddLocationDialog::AddLocationDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AddLocationDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	setWindowFlags(flags);
}

AddLocationDialog::~AddLocationDialog()
{
	delete ui;
}

QString AddLocationDialog::location() const
{
	return ui->lineEdit->text();
}

void AddLocationDialog::changeEvent(QEvent *e)
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
