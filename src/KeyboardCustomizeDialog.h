#ifndef KEYBOARDCUSTOMIZEDIALOG_H
#define KEYBOARDCUSTOMIZEDIALOG_H

#include <QDialog>

namespace Ui {
class KeyboardCustomizeDialog;
}

class KeyboardCustomizeDialog : public QDialog
{
	Q_OBJECT

public:
	explicit KeyboardCustomizeDialog(QWidget *parent = 0);
	~KeyboardCustomizeDialog();

private:
	Ui::KeyboardCustomizeDialog *ui;
};

#endif // KEYBOARDCUSTOMIZEDIALOG_H
