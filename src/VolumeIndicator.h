#ifndef VOLUMEINDICATOR_H
#define VOLUMEINDICATOR_H

#include <QSlider>

class VolumeIndicator : public QSlider
{
	Q_OBJECT
public:
	explicit VolumeIndicator(QWidget *parent = 0);
	virtual void paintEvent(QPaintEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
signals:

public slots:

};

#endif // VOLUMEINDICATOR_H
