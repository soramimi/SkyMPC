#ifndef RINGSLIDER_H
#define RINGSLIDER_H

#include <QSlider>

class RingSlider : public QSlider {
protected:
	int handle_size_ = 16;
	QRect slider_rect_;
	QRect handle_rect_;
	int temporary_value_ = INT_MIN;
	QImage slider_image_cache_;
	void updateGeometry();
	QSize sliderImageSize() const;
	void offset(int delta);
	void resizeEvent(QResizeEvent *e) override;
	void keyPressEvent(QKeyEvent *e) override;
	void paintEvent(QPaintEvent *) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e) override;
	virtual QImage generateSliderImage() = 0;
public:
	RingSlider(QWidget *parent);
};


#endif // RINGSLIDER_H
