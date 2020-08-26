#include "MySettings.h"
#include "ApplicationGlobal.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
//#include "main.h"
//#include "pathcat.h"
//#include <QApplication>
//#include <QDebug>
//#include <QDir>
//#include <QStandardPaths>
//#include <QString>

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
	: QSettings(global->config_file_path, QSettings::IniFormat)
{
}

