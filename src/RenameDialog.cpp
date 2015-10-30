#include "RenameDialog.h"
#include "ui_RenameDialog.h"

RenameDialog::RenameDialog(QWidget *parent, QString const &curname, QString const &newname) :
	QDialog(parent),
	ui(new Ui::RenameDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui->lineEdit_curname->setText(curname);
	ui->lineEdit_newname->setText(newname);
	ui->lineEdit_newname->setFocus();
	ui->lineEdit_newname->selectAll();
}

RenameDialog::~RenameDialog()
{
	delete ui;
}

QString RenameDialog::name() const
{
	return ui->lineEdit_newname->text();
}

void RenameDialog::changeEvent(QEvent *e)
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
