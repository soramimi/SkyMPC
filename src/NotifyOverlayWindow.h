#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QMainWindow>


namespace Ui {
class Dialog;
}

class NotifyOverlayWindow : public QDialog
{
	Q_OBJECT
private:
	QTimer timer;
	int alpha;
public:
	explicit NotifyOverlayWindow(QWidget *parent = 0);
	~NotifyOverlayWindow();

	void start(QMainWindow *parent, int y, int h, QString const &text);

private:
	Ui::Dialog *ui;
public slots:
	void onTimeout();
};

#endif // DIALOG_H
