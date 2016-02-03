#include "SleepTimerDialog.h"
#include "ui_SleepTimerDialog.h"

#include <QLineEdit>

SleepTimerDialog::SleepTimerDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SleepTimerDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);
	setFixedSize(width(), height());

	ui->comboBox_timeout->addItem("5");
	ui->comboBox_timeout->addItem("10");
	ui->comboBox_timeout->addItem("15");
	ui->comboBox_timeout->addItem("30");
	ui->comboBox_timeout->addItem("45");
	ui->comboBox_timeout->addItem("60");
	ui->comboBox_timeout->addItem("90");
	ui->comboBox_timeout->addItem("120");
	ui->comboBox_timeout->addItem("150");
	ui->comboBox_timeout->addItem("180");

	ui->pushButton_start->setFocus();
}

SleepTimerDialog::~SleepTimerDialog()
{
	delete ui;
}

void SleepTimerDialog::setMinutes(int mins)
{
	QString text = QString::number(mins);
	ui->comboBox_timeout->setEditText(text);
}

int SleepTimerDialog::minutes() const
{
	return ui->comboBox_timeout->currentText().toInt();
}

void SleepTimerDialog::on_pushButton_start_clicked()
{
	done(Start);
}

void SleepTimerDialog::on_pushButton_stop_clicked()
{
	done(Stop);
}





