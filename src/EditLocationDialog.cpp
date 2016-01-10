#include "EditLocationDialog.h"
#include "ui_EditLocationDialog.h"
#include <QDesktopServices>
#include <QObjectUserData>
#include <QUrl>

EditLocationDialog::EditLocationDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditLocationDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	flags |= Qt::MSWindowsFixedSizeDialogHint;
	setWindowFlags(flags);

	this->setAcceptDrops(true);

	ui->comboBox->addItem(tr("Open with web browser..."));
	ui->comboBox->addItem("SHOUTcast", "http://www.shoutcast.com/");
	ui->comboBox->addItem("Radionomy", "http://www.radionomy.com/en/style");
	ui->comboBox->setCurrentIndex(0);
}

EditLocationDialog::~EditLocationDialog()
{
	delete ui;
}

QString EditLocationDialog::location() const
{
	return ui->lineEdit->text();
}

void EditLocationDialog::setLocation(const QString &url)
{
	ui->lineEdit->setText(url);
	ui->lineEdit->selectAll();
}

void EditLocationDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void EditLocationDialog::on_comboBox_currentIndexChanged(int index)
{
	QString loc = ui->comboBox->itemData(index).toString();
	if (!loc.isEmpty()) {
		QUrl url(loc);
		QDesktopServices::openUrl(url);
		ui->comboBox->setCurrentIndex(0);
		ui->lineEdit->setFocus();
		ui->lineEdit->selectAll();
	}
}

#include "MainWindow.h"
#include "LocationLineEdit.h"

void EditLocationDialog::accept()
{
	QString loc = location();
	if (loc.startsWith("http://")) {
		QStringList list = loc.split(' ', QString::SkipEmptyParts);
		if (list.size() == 1) {
			loc = list[0];
			getLocations(this, loc, &list);
			bool ret = false;
			QString newtext;
			if (list.isEmpty()) {
				ret = true;
			} else if (list.size() == 1) {
				if (list[0] == loc) {
					// nop (normally return)
				} else {
					newtext = list[0];
					ret = true;
				}
			} else if (list.size() > 1) {
				for (QString const &loc : list) {
					if (newtext.isEmpty()) {
						newtext + ' ';
					}
					newtext += loc;
				}
				ret = true;
			}
			if (ret) {
				ui->lineEdit->setText(newtext);
				return;
			}
		}
	}
	QDialog::accept();
}



