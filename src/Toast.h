#ifndef TOAST_H
#define TOAST_H

#include <QLabel>
#include <QTimer>

class Toast : public QLabel
{
	Q_OBJECT
public:
	enum Length {
		LENGTH_SHORT = 2000,
		LENGTH_LONG = 3500,
	};
private:
	QTimer timer;
	int delay;
	explicit Toast(QWidget *parent, QString const &text, Length length);
public:
	static void show(QWidget *parent, QString const &text, Length length);
public slots:
	void onTimeout();
};

#endif // TOAST_H
