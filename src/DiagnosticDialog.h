#ifndef DIAGNOSTICDIALOG_H
#define DIAGNOSTICDIALOG_H

#include <QDialog>

namespace Ui {
class DiagnosticDialog;
}

class DiagnosticDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DiagnosticDialog(QWidget *parent = 0);
	~DiagnosticDialog();

	void setText(QString const &text);

protected:
	void changeEvent(QEvent *e);

private slots:
	void on_pushButton_saveas_clicked();

private:
	Ui::DiagnosticDialog *ui;
};

#endif // DIAGNOSTICDIALOG_H
