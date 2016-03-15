#include "MySettings.h"
#include "main.h"
#include "pathcat.h"
#include <QApplication>
#include <QDir>
#include <QString>
#include <QStandardPaths>

#ifdef Q_OS_WIN32
#include "win32.h"
#else
QString getAppDataLocation()
{
	QString dir;
	char const *p = getenv("HOME");
	if (p) {
		dir = p;
#ifdef Q_OS_LINUX
		dir = dir / ".local/share";
#endif
#ifdef Q_OS_MACX
		dir = dir / "Library/Application Support";
#endif
	} else {
		dir = "/tmp";
	}
	return dir;
}
#endif

char const *KEY_AutoReconnect = "AutoReconnect";

QString makeApplicationDataDir()
{
	QString dir;
#if QT_VERSION >= 0x050400
	dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
	if (dir.isEmpty()) {
		dir = getAppDataLocation();
		dir = dir / qApp->organizationName();
		dir = dir / qApp->applicationName();
	}
	QDir().mkpath(dir);
	return dir;
}

MySettings::MySettings(QObject *)
	: QSettings(pathcat(application_data_dir, "SkyMPC.ini"), QSettings::IniFormat)
{
}

