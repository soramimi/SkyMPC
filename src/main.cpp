
#include "MainWindow.h"
#include "TinyMainWindow.h"
#include "MySettings.h"
#include "main.h"
#include "LegacyWindowsStyleTreeControl.h"
#include "ApplicationGlobal.h"
#include <QApplication>
#include <QTextCodec>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>
#include <QSplashScreen>
#include <QProxyStyle>
#include <QStandardPaths>


#define USE_SPLASH 0


ApplicationGlobal *global;

class MyStyle : public QProxyStyle {
private:
    LegacyWindowsStyleTreeControl legacy_windows_;
public:
    MyStyle()
        : QProxyStyle(0)
    {
    }
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const
    {
        if (element == QStyle::PE_IndicatorBranch) {
            if (legacy_windows_.drawPrimitive(element, option, painter, widget)) {
                return;
            }
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};

static bool isHighDpiScalingEnabled(int argc, char *argv[])
{
	QApplication dummy(argc, argv);
	dummy.setOrganizationName(ORGANIZTION_NAME);
	dummy.setApplicationName(APPLICATION_NAME);
	MySettings s;
	s.beginGroup("UI");
	QVariant v = s.value("EnableHighDpiScaling");
	return v.isNull() || v.toBool();
}

int main(int argc, char *argv[])
{
	ApplicationGlobal g;
	global = &g;

	if (isHighDpiScalingEnabled(argc, argv)){
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	}

	QApplication a(argc, argv);

	if (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) {
		global->start_with_shift_key = true;
	}

	bool tiny = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--tiny") == 0) {
			tiny = true;
		}
	}

    QStyle *style = new MyStyle();
    QApplication::setStyle(style);

	if (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) {
		global->start_with_shift_key = true;
	}

#if defined(Q_OS_WIN)
	QApplication::addLibraryPath(qApp->applicationDirPath());
#endif

#if USE_SPLASH
	QSplashScreen splash(QPixmap(":/image/about.png"));
	splash.show();
	qApp->processEvents();
#endif

	QSettings::setDefaultFormat(QSettings::IniFormat);

	a.setOrganizationName(ORGANIZTION_NAME);
	a.setApplicationName(APPLICATION_NAME);

	global->application_data_dir = makeApplicationDataDir();
	if (global->application_data_dir.isEmpty()) {
        QMessageBox::warning(0, qApp->applicationName(), "Preparation of data storage folder failed.");
        return 1;
    }

    QTranslator translator;
    {
#if defined(Q_OS_MACX)
        QString path = "../Resources/SkyMPC_ja";
#else
        QString path = "SkyMPC_ja";
#endif
        translator.load(path, a.applicationDirPath());
        a.installTranslator(&translator);
    }

	BasicMainWindow *mw;
	if (tiny) {
		mw = new TinyMainWindow();
	} else {
		mw = new MainWindow();
	}

	mw->setWindowIcon(QIcon(":/image/appicon.png"));
	mw->show();

#if USE_SPLASH
	splash.finish(&w);
#endif

	mw->preexec();

	int r = a.exec();

	delete mw;

	return r;
}

