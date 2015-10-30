#ifndef TESTCONNECTRESULTDIALOG_H
#define TESTCONNECTRESULTDIALOG_H

#include <QDialog>

namespace Ui {
class TestConnectResultDialog;
}

class TestConnectResultDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit TestConnectResultDialog(QWidget *parent, QString const &message);
	~TestConnectResultDialog();
	
private:
	Ui::TestConnectResultDialog *ui;
};

#endif // TESTCONNECTRESULTDIALOG_H
