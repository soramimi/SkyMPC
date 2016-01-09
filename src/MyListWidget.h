#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QListWidget>

class MyListWidget : public QListWidget
{
	Q_OBJECT
public:
	explicit MyListWidget(QWidget *parent = 0);
	using QListWidget::itemFromIndex;
	using QListWidget::selectedIndexes;
	virtual void dropEvent(QDropEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *);
signals:
	void onDropEvent(bool done);
	void onContextMenu(QContextMenuEvent *);
public slots:
	
};

#endif // MYLISTWIDGET_H
