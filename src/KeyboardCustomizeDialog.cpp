#include "KeyboardCustomizeDialog.h"
#include "ui_KeyboardCustomizeDialog.h"

KeyboardCustomizeDialog::KeyboardCustomizeDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::KeyboardCustomizeDialog)
{
	ui->setupUi(this);
}

KeyboardCustomizeDialog::~KeyboardCustomizeDialog()
{
	delete ui;
}
