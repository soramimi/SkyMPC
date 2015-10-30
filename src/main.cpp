
#include "MainWindow.h"
#include "MySettings.h"
#include "main.h"
#include "LegacyWindowsStyleTreeControl.h"
#include <QApplication>
#include <QTextCodec>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>
#include <QSplashScreen>
#include <QProxyStyle>

#define USE_SPLASH 0

MainWindow *the_mainwindow = 0;

bool start_with_shift_key = false;
QString application_data_dir;

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

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    QStyle *style = new MyStyle();
    QApplication::setStyle(style);

	if (QApplication::queryKeyboardModifiers() & Qt::ShiftModifier) {
		start_with_shift_key = true;
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

	a.setOrganizationName("soramimi.jp");
	a.setApplicationName("SkyMPC");

	application_data_dir = makeApplicationDataDir();
    if (application_data_dir.isEmpty()) {
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

	MainWindow w;
	the_mainwindow = &w;
	w.setWindowIcon(QIcon(":/image/appicon.png"));
	w.show();

#if USE_SPLASH
	splash.finish(&w);
#endif

	w.preExec();

	return a.exec();
}

