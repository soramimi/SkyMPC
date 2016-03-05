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
	QDir().mkpath(dir);
	return dir;
}

MySettings::MySettings(QObject *)
    : QSettings(pathcat(application_data_dir, "SkyMPC.ini"), QSettings::IniFormat)
{
}

