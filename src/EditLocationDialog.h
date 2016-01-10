#ifndef EDITLOCATIONDIALOG_H
#define EDITLOCATIONDIALOG_H

#include <QDialog>

namespace Ui {
class EditLocationDialog;
}

class EditLocationDialog : public QDialog
{
	Q_OBJECT
public:
	explicit EditLocationDialog(QWidget *parent = 0);
	~EditLocationDialog();
	QString location() const;
	void setLocation(QString const &url);
protected:
	void changeEvent(QEvent *e);
	
private:
	Ui::EditLocationDialog *ui;
};

#endif // EDITLOCATIONDIALOG_H

