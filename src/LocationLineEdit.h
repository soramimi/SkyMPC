#ifndef LOCATIONLINEEDIT_H
#define LOCATIONLINEEDIT_H

#include <QLineEdit>

class LocationLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	explicit LocationLineEdit(QWidget *parent = 0);
	void dropEvent(QDropEvent *);

signals:

public slots:
};

#endif // LOCATIONLINEEDIT_H
