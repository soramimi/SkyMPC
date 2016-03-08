#ifndef TINYCONNECTIONDIALOG_H
#define TINYCONNECTIONDIALOG_H

#include "Server.h"

#include <QDialog>

namespace Ui {
class TinyConnectionDialog;
}

class Host;
class QTableWidgetItem;

class TinyConnectionDialog : public QDialog
{
	Q_OBJECT
private:
	std::vector<ServerItem> servers;
	int row_height;
	Host current_host;
public:
	explicit TinyConnectionDialog(QWidget *parent, const Host &host);
	~TinyConnectionDialog();
	virtual void accept();
	virtual void reject();

	Host host() const;

protected slots:
	void on_tableWidget_currentItemChanged(QTableWidgetItem *, QTableWidgetItem *);
	void on_lineEdit_name_textChanged(const QString &);
	void on_lineEdit_address_textChanged(const QString &);
	void on_lineEdit_port_textChanged(const QString &);
	void on_lineEdit_password_textChanged(const QString &str);
	void on_pushButton_delete_clicked();
	void on_pushButton_new_clicked();
	void on_pushButton_test_connection_clicked();
	void on_tableWidget_itemDoubleClicked(QTableWidgetItem *);
private:
	Ui::TinyConnectionDialog *ui;
	void loadServers();
	void updateList();
	const ServerItem *selectedServer() const;
	ServerItem *selectedServer();
	void selectServer(const ServerItem *server);
	void updateServer(int row, int col);
};

#endif // TINYCONNECTIONDIALOG_H
