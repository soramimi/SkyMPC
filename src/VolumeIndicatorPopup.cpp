#include "VolumeIndicatorPopup.h"
#include "ui_VolumeIndicatorPopup.h"

VolumeIndicatorPopup::VolumeIndicatorPopup(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::VolumeIndicatorPopup)
{
	ui->setupUi(this);
	auto flags = Qt::Tool | Qt::FramelessWindowHint;
	setWindowFlags(flags);
}

VolumeIndicatorPopup::~VolumeIndicatorPopup()
{
	delete ui;
}

void VolumeIndicatorPopup::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void VolumeIndicatorPopup::setValue(int v)
{
	ui->horizontalSlider->setValue(v);
	ui->label_value->setText(QString::number(v));
}
