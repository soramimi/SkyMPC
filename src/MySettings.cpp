#include "MySettings.h"
#include "main.h"
#include "pathcat.h"
#include <QApplication>
#include <QDir>
#include <QString>
#include <QStandardPaths>

char const *KEY_AutoReconnect = "AutoReconnect";

QString makeApplicationDataDir()
{
	QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	// QStandardPaths::AppDataLocation was added in Qt 5.4
	if (dir.isEmpty()) {
#ifdef Q_OS_WIN32
#else
		QString d;
		char const *p = getenv("HOME");
		if (p) {
			d = p;
			d = d / ".local/share";
		} else {
			d = "/tmp";
		}
		d = d / qApp->organizationName();
		d = d / qApp->applicationName();
		dir = d;
#endif
	}
	QDir().mkpath(dir);
	puts(dir.toStdString().c_str());
	return dir;
}

MySettings::MySettings(QObject *)
    : QSettings(pathcat(application_data_dir, "SkyMPC.ini"), QSettings::IniFormat)
{
}

