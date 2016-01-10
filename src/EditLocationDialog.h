#ifndef EDITLOCATIONDIALOG_H
#define EDITLOCATIONDIALOG_H

#include "PlaylistFile.h"

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
	
private slots:
	void on_comboBox_currentIndexChanged(int index);

private:
	Ui::EditLocationDialog *ui;

	// QDialog interface
	static void getLocations(QWidget *parent, const std::vector<PlaylistFile::Item> *locations, QStringList *out);
	static void getLocations(QWidget *parent, const QString &loc, QStringList *out);
public slots:
	void accept();
};

#endif // EDITLOCATIONDIALOG_H

