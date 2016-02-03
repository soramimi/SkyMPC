#ifndef SLEEPTIMERDIALOG_H
#define SLEEPTIMERDIALOG_H

#include <QDialog>

namespace Ui {
class SleepTimerDialog;
}

class SleepTimerDialog : public QDialog
{
	Q_OBJECT
public:
	enum {
		Start = 100,
		Stop,
	};
public:
	explicit SleepTimerDialog(QWidget *parent = 0);
	~SleepTimerDialog();

	void setMinutes(int mins);
	int minutes() const;

private slots:
	void on_pushButton_start_clicked();
	void on_pushButton_stop_clicked();

private:
	Ui::SleepTimerDialog *ui;
};

#endif // SLEEPTIMERDIALOG_H
