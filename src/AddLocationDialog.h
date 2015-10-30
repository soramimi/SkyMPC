#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>

namespace Ui {
class AddLocationDialog;
}

class AddLocationDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit AddLocationDialog(QWidget *parent = 0);
	~AddLocationDialog();
	QString location() const;
protected:
	void changeEvent(QEvent *e);
	
private:
	Ui::AddLocationDialog *ui;
};

#endif // ADDDIALOG_H
