#ifndef ASKREMOVEOVERLAPPEDFILEDIALOG_H
#define ASKREMOVEOVERLAPPEDFILEDIALOG_H

#include <QDialog>

namespace Ui {
class AskRemoveOverlappedFileDialog;
}

class AskRemoveOverlappedFileDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AskRemoveOverlappedFileDialog(QWidget *parent, QString const &text);
	~AskRemoveOverlappedFileDialog();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::AskRemoveOverlappedFileDialog *ui;
};

#endif // ASKREMOVEOVERLAPPEDFILEDIALOG_H
