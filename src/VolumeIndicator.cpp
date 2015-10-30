#include "VolumeIndicator.h"
#include <QPainter>
VolumeIndicator::VolumeIndicator(QWidget *parent) :
    QSlider(parent)
{
	setFocusPolicy(Qt::NoFocus);
}

void VolumeIndicator::mousePressEvent(QMouseEvent *)
{
}

void VolumeIndicator::paintEvent(QPaintEvent *)
{
	QPainter pr(this);

	int v = value();
	int w = width();
	int h = height();

	v = (v - minimum()) * w / (maximum() - minimum());

	for (int i = 0; i < w; i++) {
		if (i < v) {
			int t = (h - 1) * i / w + 1;
			int y = h - 1 - t;
			pr.fillRect(i, y, 1, t, Qt::black);
		}
	}
}

