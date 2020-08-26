#include "RingSlider.h"
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <math.h>

template <typename T> static inline T clamp(T v, T min, T max)
{
	return std::max(min, std::min(max, v));
}

RingSlider::RingSlider(QWidget *parent)
	: QSlider(parent)
{

}


void RingSlider::updateGeometry()
{
	handle_size_ = height();

	int x = handle_size_ / 2;
	int w = width() - handle_size_;
	slider_rect_ = QRect(x, 3, w, height() - 6);

	int val = (temporary_value_ == INT_MIN) ? value() : temporary_value_;
	int max = maximum();
	int handle_x = val * slider_rect_.width() / (max + 1) + slider_rect_.x() - handle_size_ / 2;
	handle_rect_ = QRect(handle_x, 0, handle_size_, handle_size_);

	slider_image_cache_ = QImage();
}

QSize RingSlider::sliderImageSize() const
{
	return slider_rect_.size();
}

void RingSlider::offset(int delta)
{
	setValue(value() + delta);
}

void RingSlider::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
	updateGeometry();
}

void RingSlider::keyPressEvent(QKeyEvent *e)
{
	int k = e->key();

	switch (k) {
	case Qt::Key_Home:
		setValue(minimum());
		return;
	case Qt::Key_End:
		setValue(maximum());
		return;
	case Qt::Key_Left:
		offset(-singleStep());
		return;
	case Qt::Key_Right:
		offset(singleStep());
		return;
	case Qt::Key_PageDown:
		offset(-pageStep());
		return;
	case Qt::Key_PageUp:
		offset(pageStep());
		return;
	}
}

void RingSlider::paintEvent(QPaintEvent *)
{
	updateGeometry();

	slider_image_cache_ = generateSliderImage();

	QPainter pr(this);
	pr.setRenderHint(QPainter::Antialiasing);

	{

		int m = handle_rect_.height() / 2;
		QRectF rf = QRectF(slider_rect_.adjusted(-m, 0, m, 0)).adjusted(0.5, 0.5, -0.5, -0.5);
		double r = rf.height() / 2;

		int x0 = slider_rect_.x();
		int x1 = handle_rect_.x() + handle_rect_.height() / 2;
		QRect r2(x0 - m, slider_rect_.y(), x1 + m, slider_rect_.height());

		pr.save();
		QPainterPath path;
		path.addRoundedRect(rf, r, r);
		pr.setClipPath(path);
		{
			QLinearGradient g(r2.topLeft(), r2.bottomLeft());
			g.setColorAt(0, QColor(160, 160, 160));
			g.setColorAt(1, QColor(192, 192, 192));
			pr.fillPath(path, g);
		}
		pr.restore();

		pr.save();
		QPainterPath path2;
		path2.addRoundedRect(QRectF(r2).adjusted(0.5, 0.5, -0.5, -0.5), r, r);
		{
			QLinearGradient g(r2.topLeft(), r2.bottomLeft());
			g.setColorAt(0, QColor(160, 192, 240));
			g.setColorAt(1, QColor(80, 96, 120));
			pr.fillPath(path2, g);
		}
		pr.restore();
		pr.setPen(QPen(Qt::black, 1));
		pr.drawPath(path);
	}

	// slider handle
	{
		QPainterPath path;
		path.addRect(rect());
		QPainterPath path2;
		path2.addEllipse(handle_rect_.adjusted(6, 6, -6, -6));
		path = path.subtracted(path2);
		pr.setClipPath(path);

		pr.setPen(Qt::NoPen);
		pr.setBrush(Qt::black);
		pr.drawEllipse(handle_rect_);
		pr.setBrush(Qt::white);
		pr.drawEllipse(handle_rect_.adjusted(1, 1, -1, -1));
		pr.setPen(Qt::NoPen);
		pr.setBrush(Qt::black);
		pr.drawEllipse(handle_rect_.adjusted(5, 5, -5, -5));
	}
}

void RingSlider::mousePressEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::LeftButton) {
		grabMouse();
		int x = e->pos().x();
		int v = (x - slider_rect_.x()) * (maximum() - minimum()) / slider_rect_.width() + minimum();
		v = std::max(minimum(), std::min(v, maximum()));
		temporary_value_ = v;
	}
}

void RingSlider::mouseReleaseEvent(QMouseEvent *e)
{
	if (QWidget::mouseGrabber() == this) {
		emit sliderPressed();
		setValue(temporary_value_);
		temporary_value_ = INT_MIN;
		emit sliderReleased();
	}
	releaseMouse();
}

void RingSlider::mouseMoveEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::LeftButton) {
		int x = e->pos().x();
		int v = (x - slider_rect_.x()) * (maximum() - minimum()) / slider_rect_.width() + minimum();
		v = std::max(minimum(), std::min(v, maximum()));
		temporary_value_ = v;
		update();
	}
}

