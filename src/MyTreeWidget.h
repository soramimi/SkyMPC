#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

#include <QTreeWidget>

class MainWindow;

class MyTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	explicit MyTreeWidget(QWidget *parent = 0);
	QTreeWidgetItem *itemFromIndex(QModelIndex const &index) const
	{
		return QTreeWidget::itemFromIndex(index);
	}
	QModelIndex indexFromItem(QTreeWidgetItem *item, int column = 0) const
	{
		return QTreeWidget::indexFromItem(item, column);
	}
	virtual void contextMenuEvent(QContextMenuEvent *);
signals:
	void onContextMenuEvent(QContextMenuEvent *event);
public slots:
};

#endif // MYTREEWIDGET_H
