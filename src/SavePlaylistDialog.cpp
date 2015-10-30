#include "SavePlaylistDialog.h"
#include "ui_SavePlaylistDialog.h"
#include <QLineEdit>

SavePlaylistDialog::SavePlaylistDialog(QWidget *parent, QString const &initname) :
	QDialog(parent),
	ui(new Ui::SavePlaylistDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui->lineEdit->setText(initname);

	ui->lineEdit->setFocus();
	ui->lineEdit->selectAll();
}

SavePlaylistDialog::~SavePlaylistDialog()
{
	delete ui;
}

void SavePlaylistDialog::changeEvent(QEvent *e)
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

QString SavePlaylistDialog::name() const
{
	return ui->lineEdit->text();
}

