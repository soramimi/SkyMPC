#include "MyTreeWidget.h"
#include "MainWindow.h"

MyTreeWidget::MyTreeWidget(QWidget *parent) :
	QTreeWidget(parent)
{
}

void MyTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
	emit onContextMenuEvent(event);
}

