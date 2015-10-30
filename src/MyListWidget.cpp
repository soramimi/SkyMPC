#include "MyListWidget.h"
#include "main.h"
#include <QDropEvent>

MyListWidget::MyListWidget(QWidget *parent) :
	QListWidget(parent)
{
}

void MyListWidget::dropEvent(QDropEvent *event)
{
	emit onDropEvent(false);
	QListWidget::dropEvent(event);
	emit onDropEvent(true);
}

void MyListWidget::contextMenuEvent(QContextMenuEvent *event)
{
	emit onContextMenu(event);
}
