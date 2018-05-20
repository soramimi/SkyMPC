#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "MySettings.h"

#include <QFileDialog>
//#include "common/misc.h"

static int page_number = 0;


SettingsDialog::SettingsDialog(MainWindow *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	Qt::WindowFlags flags = windowFlags();
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	mainwindow_ = parent;

	loadSettings();

	QTreeWidgetItem *item;

	auto AddPage = [&](QWidget *page){
		QString name = page->windowTitle();
		item = new QTreeWidgetItem();
		item->setText(0, name);
		item->setData(0, Qt::UserRole, QVariant::fromValue((uintptr_t)(QWidget *)page));
		ui->treeWidget->addTopLevelItem(item);
	};
//	AddPage(ui->page_general);
	AddPage(ui->page_example);

	ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(page_number));
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::loadSettings(ApplicationSettings *as)
{
	MySettings s;

	ApplicationSettings def = ApplicationSettings::defaultSettings();

	auto STRING_VALUE_ = [&](QString const &name, QString *v, QString const &def){
		*v = s.value(name, def).toString();
	};

	auto BOOL_VALUE_ = [&](QString const &name, bool *v, bool const &def){
		*v = s.value(name, def).toBool();
	};

	auto INT_VALUE_ = [&](QString const &name, int *v, int const &def){
		*v = s.value(name, def).toInt();
	};

#define STRING_VALUE(NAME, SYMBOL) STRING_VALUE_(NAME, &as->SYMBOL, def.SYMBOL)
#define BOOL_VALUE(NAME, SYMBOL)   BOOL_VALUE_(NAME, &as->SYMBOL, def.SYMBOL)
#define INT_VALUE(NAME, SYMBOL)   INT_VALUE_(NAME, &as->SYMBOL, def.SYMBOL)

	s.beginGroup("Global");
	BOOL_VALUE("SaveWindowPosition", remember_and_restore_window_position);
	s.endGroup();

	s.beginGroup("UI");
	BOOL_VALUE("EnableHighDpiScaling", enable_high_dpi_scaling);
	s.endGroup();
}

void SettingsDialog::saveSettings(ApplicationSettings const *as)
{
	MySettings s;

	s.beginGroup("Global");
	s.setValue("SaveWindowPosition", as->remember_and_restore_window_position);
	s.endGroup();

	s.beginGroup("UI");
	s.setValue("EnableHighDpiScaling", as->enable_high_dpi_scaling);
	s.endGroup();
}

void SettingsDialog::saveSettings()
{
	saveSettings(&set);
}

void SettingsDialog::exchange(bool save)
{
	QList<AbstractSettingForm *> forms = ui->stackedWidget->findChildren<AbstractSettingForm *>();
	for (AbstractSettingForm *form : forms) {
		form->exchange(save);
	}
}

void SettingsDialog::loadSettings()
{
	loadSettings(&set);
	exchange(false);
}

void SettingsDialog::done(int r)
{
	page_number = ui->stackedWidget->currentIndex();
	QDialog::done(r);
}

void SettingsDialog::accept()
{
	exchange(true);
	saveSettings();
	done(QDialog::Accepted);
}

void SettingsDialog::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	(void)previous;
	if (current) {
		uintptr_t p = current->data(0, Qt::UserRole).value<uintptr_t>();
		QWidget *w = reinterpret_cast<QWidget *>(p);
		Q_ASSERT(w);
		ui->stackedWidget->setCurrentWidget(w);
	}
}

