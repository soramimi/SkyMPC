#ifndef TOAST_H
#define TOAST_H

#include <QLabel>
#include <QTimer>

class BasicMainWindow;

class Toast : public QLabel
{
	Q_OBJECT
public:
	enum Length {
		LENGTH_MOMENT = 1500,
		LENGTH_SHORT = 2000,
		LENGTH_LONG = 3500,
	};
private:
	QTimer timer;
	int delay;
	explicit Toast(BasicMainWindow *parent, QString const &text, Length length);
public:
	~Toast();

	static void show(BasicMainWindow *parent, QString const &text, Length length);
public slots:
	void onTimeout();

	// QWidget interface
protected:
	void showEvent(QShowEvent *);
};

#endif // TOAST_H
