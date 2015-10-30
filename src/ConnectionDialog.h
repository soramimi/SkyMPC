#ifndef NEWCONNECTIONDIALOG_H
#define NEWCONNECTIONDIALOG_H

#include "Server.h"
#include <QDialog>
#include <QTimer>

namespace Ui {
class ConnectionDialog;
}

class QTableWidgetItem;

class ConnectionDialog : public QDialog
{
	Q_OBJECT
private:
	std::vector<ServerItem> servers;
	int row_height;
	Host current_host;
	QTimer timer;
	void updateList();
	void loadServers();
	void saveServers();
	ServerItem *selectedServer();
	ServerItem const *selectedServer() const;
	void selectServer(ServerItem const *server);
	void updateServer(int row, int col);
public:
	explicit ConnectionDialog(QWidget *parent, Host const &host);
	~ConnectionDialog();

	virtual void accept();

	Host host() const;

private slots:
	void on_lineEdit_address_textChanged(const QString &arg1);
	void on_lineEdit_desc_textChanged(const QString &arg1);
	void on_lineEdit_name_textChanged(const QString &arg1);
	void on_lineEdit_port_textChanged(const QString &arg1);
	void on_lineEdit_password_textChanged(const QString &arg1);
	void on_pushButton_delete_clicked();
	void on_pushButton_down_clicked();
	void on_pushButton_new_clicked();
	void on_pushButton_save_and_close_clicked();
	void on_pushButton_test_connection_clicked();
	void on_pushButton_up_clicked();
	void on_tableWidget_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
	void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
private:
	Ui::ConnectionDialog *ui;
};

#endif // NEWCONNECTIONDIALOG_H
