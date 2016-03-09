#ifndef TINYCONNECTIONDIALOG_H
#define TINYCONNECTIONDIALOG_H

#include "Server.h"

#include "BasicConnectionDialog.h"

namespace Ui {
class TinyConnectionDialog;
}

class TinyConnectionDialog : public BasicConnectionDialog
{
	Q_OBJECT
public:
	explicit TinyConnectionDialog(QWidget *parent, const Host &host);
	~TinyConnectionDialog();

private:
	Ui::TinyConnectionDialog *ui;
};

#endif // TINYCONNECTIONDIALOG_H
