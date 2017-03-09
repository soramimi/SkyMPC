#include "AboutDialog.h"
#include "ui_AboutDialog.h"
#include "main.h"
#include <QPainter>

extern "C" int copyright_year;
extern "C" char const product_version[];
extern "C" char const source_revision[];

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
	auto flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	pixmap.load(":/image/about.png");
	setFixedSize(pixmap.width(), pixmap.height());

	setWindowTitle(tr("About SkyMPC"));

	ui->label_title->setText(QString("SkyMPC, v%1 (%2)").arg(product_version).arg(source_revision));
	ui->label_copyright->setText(QString("Copyright (C) %1 S.Fuchita").arg(copyright_year));
	ui->label_twitter->setText("(@soramimi_jp)");
	QString t = QString("Qt %1").arg(qVersion());
#if defined(_MSC_VER)
	t += QString(", msvc=%1").arg(_MSC_VER);
#elif defined(__clang__)
    t += QString(", clang=%1.%2").arg(__clang_major__).arg(__clang_minor__);
#elif defined(__GNUC__)
	t += QString(", gcc=%1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#endif
	ui->label_qt->setText(t);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

void AboutDialog::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, pixmap);
}

void AboutDialog::mouseReleaseEvent(QMouseEvent *)
{
    accept();
}

