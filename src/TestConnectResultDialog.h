#ifndef TESTCONNECTRESULTDIALOG_H
#define TESTCONNECTRESULTDIALOG_H

#include <QDialog>

namespace Ui {
class TestConnectResultDialog;
}

class TestConnectResultDialog : public QDialog
{
	Q_OBJECT
private:
	QThread *thread;
public:
	explicit TestConnectResultDialog(QWidget *parent);
	~TestConnectResultDialog();
	
	void bindThread(QThread *t)
	{
		thread = t;
	}

	virtual void reject();

private:
	Ui::TestConnectResultDialog *ui;
public slots:
	void setMessage(QString const &message);
};

#endif // TESTCONNECTRESULTDIALOG_H
