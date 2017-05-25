#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "Server.h"
#include <QDialog>
#include <QTimer>
#include "BasicConnectionDialog.h"

namespace Ui {
class ConnectionDialog;
}

class QTableWidgetItem;

class ConnectionDialog : public BasicConnectionDialog
{
	Q_OBJECT
private:
	bool isAutoReconnect() const;
	void saveAutoReconnect();
public:
	explicit ConnectionDialog(QWidget *parent, Host const &host);
	~ConnectionDialog();

	virtual void accept();
private slots:
	void on_pushButton_close_clicked();

private:
	Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
