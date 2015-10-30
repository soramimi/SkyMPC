#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QPixmap>
namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
	Q_OBJECT
private:
	QPixmap pixmap;
public:
	explicit AboutDialog(QWidget *parent = 0);
	~AboutDialog();
	virtual void paintEvent(QPaintEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
private:
	Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
