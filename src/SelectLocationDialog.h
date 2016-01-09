#ifndef SELECTLOCATIONDIALOG_H
#define SELECTLOCATIONDIALOG_H

#include "PlaylistFile.h"
#include <QDialog>
#include <vector>

namespace Ui {
class SelectLocationDialog;
}

class SelectLocationDialog : public QDialog
{
	Q_OBJECT
private:
	std::vector<PlaylistFile::Item> const *items;
public:
	explicit SelectLocationDialog(QWidget *parent = 0);
	~SelectLocationDialog();

	void setItems(std::vector<PlaylistFile::Item> const *items);
	void selectedItems(std::vector<PlaylistFile::Item> *out) const;

private:
	Ui::SelectLocationDialog *ui;
};

#endif // SELECTLOCATIONDIALOG_H
