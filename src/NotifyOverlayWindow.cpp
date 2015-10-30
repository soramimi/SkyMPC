#include "NotifyOverlayWindow.h"
#include "ui_NotifyOverlayWindow.h"
#include <QMenuBar>

NotifyOverlayWindow::NotifyOverlayWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
	ui->setupUi(this);
	this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	connect(&timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

NotifyOverlayWindow::~NotifyOverlayWindow()
{
	delete ui;
}

void NotifyOverlayWindow::start(QMainWindow *parent, int y, int h, QString const &text)
{
	QPoint pt0 = parent->mapToGlobal(QPoint(0, y));
	QPoint pt1 = parent->mapToGlobal(QPoint(parent->width(), h));
	setGeometry(QRect(pt0.x(), pt0.y(), pt1.x() - pt0.x(), pt1.y() - pt0.y()));
	ui->label->setText(text);
	show();
	parent->activateWindow();
	alpha = 200;
	setWindowOpacity(1);
	timer.setInterval(10);
	timer.start();

}

void NotifyOverlayWindow::onTimeout()
{
	if (alpha > 0) {
		alpha--;
		if (alpha < 100) {
			setWindowOpacity(alpha / 100.0);
		}
	} else {
		timer.stop();
		alpha = 0;
		hide();
	}
}

