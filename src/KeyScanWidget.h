#ifndef KEYSCANWIDGET_H
#define KEYSCANWIDGET_H

#include <QLineEdit>

class KeyScanWidget : public QLineEdit
{
	Q_OBJECT
public:
	explicit KeyScanWidget(QWidget *parent = 0);

	virtual void keyPressEvent(QKeyEvent *);

signals:

public slots:

};

#endif // KEYSCANWIDGET_H
