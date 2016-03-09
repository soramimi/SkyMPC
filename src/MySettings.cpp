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
		dir = dir / ".local/share";
	} else {
		dir = "/tmp";
	}
	return dir;
}
#endif

char const *KEY_AutoReconnect = "AutoReconnect";

QString makeApplicationDataDir()
{
	QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
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

