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
	QString dir;
	dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (!QFileInfo(dir).isDir()) {
		QDir().mkpath(dir);
	}
	return dir;
}

MySettings::MySettings(QObject *)
	: QSettings(pathcat(makeApplicationDataDir(), qApp->applicationName() + ".ini"), QSettings::IniFormat)
{
}

