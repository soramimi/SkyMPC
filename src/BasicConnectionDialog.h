#ifndef BASICCONNECTIONDIALOG_H
#define BASICCONNECTIONDIALOG_H

#include "Server.h"

#include <QDialog>
#include <QLineEdit>
#include <QTableWidget>

class BasicConnectionDialog : public QDialog
{
	Q_OBJECT
protected:
	struct Controls {
		QTableWidget *tableWidget = 0;
		QLineEdit *lineEdit_port = 0;
		QLineEdit *lineEdit_address = 0;
		QLineEdit *lineEdit_name = 0;
		QLineEdit *lineEdit_password = 0;
		QLineEdit *lineEdit_desc = 0;
		QPushButton *pushButton_up = 0;
		QPushButton *pushButton_down = 0;
		QPushButton *pushButton_new = 0;
		QPushButton *pushButton_delete = 0;
		QPushButton *pushButton_test_connection = 0;
	};

	Controls ctrls;

	enum {
		C_NAME,
		C_ADDR,
		C_PORT,
		C_DESC,
	};

	std::vector<ServerItem> servers;
	int table_row_height;
	Host current_host;
	QString new_connection;

	void init();
	void loadServers();
	void updateList(bool add_new_connection = false);
	ServerItem *selectedServer();
	const ServerItem *selectedServer() const;
	void selectServer(const ServerItem *server);
	void updateServer(int row, int col);
	void saveServers();
	void testConnection();
	void addNewConnection();
	void deleteConnection();
	void moveUpConnection();
	void moveDownConnection();
	void onPasswordChanged(const QString &str);
public:
	explicit BasicConnectionDialog(QWidget *parent, Host const &host);

	virtual void accept();
	virtual void reject();
	Host host() const;
signals:

public slots:
private slots:
	virtual void on_lineEdit_address_textChanged_(const QString &);
	virtual void on_lineEdit_desc_textChanged_(const QString &);
	virtual void on_lineEdit_name_textChanged_(const QString &);
	virtual void on_lineEdit_port_textChanged_(const QString &);
	virtual void on_lineEdit_password_textChanged_(const QString &str);
	virtual void on_pushButton_delete_clicked_();
	virtual void on_pushButton_new_clicked_();
	virtual void on_pushButton_test_connection_clicked_();
	virtual void on_tableWidget_currentItemChanged_(QTableWidgetItem *, QTableWidgetItem *);
	virtual void on_tableWidget_itemDoubleClicked_(QTableWidgetItem *);
	virtual void on_pushButton_up_clicked_();
	virtual void on_pushButton_down_clicked_();
};

#endif // BASICCONNECTIONDIALOG_H
