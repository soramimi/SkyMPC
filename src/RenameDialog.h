#ifndef RENAMEPLAYLISTNAMEDIALOG_H
#define RENAMEPLAYLISTNAMEDIALOG_H

#include <QDialog>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit RenameDialog(QWidget *parent, QString const &curname, QString const &newname);
	~RenameDialog();
	QString name() const;
protected:
	void changeEvent(QEvent *e);
	
private:
	Ui::RenameDialog *ui;
};

#endif // RENAMEPLAYLISTNAMEDIALOG_H
