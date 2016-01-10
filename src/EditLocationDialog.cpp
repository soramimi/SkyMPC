#include "EditLocationDialog.h"
#include "ui_EditLocationDialog.h"

EditLocationDialog::EditLocationDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditLocationDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	setWindowFlags(flags);
}

EditLocationDialog::~EditLocationDialog()
{
	delete ui;
}

QString EditLocationDialog::location() const
{
	return ui->lineEdit->text();
}

void EditLocationDialog::setLocation(const QString &url)
{
	ui->lineEdit->setText(url);
	ui->lineEdit->selectAll();
}

void EditLocationDialog::changeEvent(QEvent *e)
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
