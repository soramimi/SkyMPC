#include "VerticalVolumePopup.h"
#include "ui_VerticalVolumePopup.h"
#include "misc.h"
#include <QPainter>
#include "MainWindow.h"

VerticalVolumePopup::VerticalVolumePopup(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::VerticalVolumePopup)
{
	ui->setupUi(this);
	auto flags = Qt::Popup | Qt::FramelessWindowHint;
	setWindowFlags(flags);
}

VerticalVolumePopup::~VerticalVolumePopup()
{
	delete ui;
}

void VerticalVolumePopup::on_spinBox_valueChanged(int)
{
	emit valueChanged();
}

int VerticalVolumePopup::value()
{
	return ui->spinBox->value();
}

void VerticalVolumePopup::setValue(int n)
{
	ui->spinBox->setValue(n);
	ui->verticalSlider->setValue(n);
}

void VerticalVolumePopup::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	drawBox(&painter, 0, 0, width(), height(), QColor(128, 128, 128));
}

void VerticalVolumePopup::mousePressEvent(QMouseEvent *e)
{
	the_mainwindow->eatMouse();
	QWidget::mousePressEvent(e);
}
