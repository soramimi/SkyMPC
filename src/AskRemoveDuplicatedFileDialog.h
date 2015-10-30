#ifndef ASKREMOVEDUPLICATEDFILEDIALOG_H
#define ASKREMOVEDUPLICATEDFILEDIALOG_H

#include <QDialog>

namespace Ui {
class AskRemoveDuplicatedFileDialog;
}

class AskRemoveDuplicatedFileDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AskRemoveDuplicatedFileDialog(QWidget *parent, QString const &text);
	~AskRemoveDuplicatedFileDialog();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::AskRemoveDuplicatedFileDialog *ui;
};

#endif // ASKREMOVEDUPLICATEDFILEDIALOG_H
