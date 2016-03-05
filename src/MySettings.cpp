#include "MySettings.h"
#include "main.h"
#include <QApplication>
#include <QDir>
#include <QString>
#include <QStandardPaths>

#ifdef QT_VERSION
#include "pathcat.h"
#endif

char const *KEY_AutoReconnect = "AutoReconnect";

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shlobj.h>
#include <QDebug>

QString makeApplicationDataDir()
{
	QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir().mkpath(dir);
	return dir;
}

#else if defined(Q_OS_MACX)

QString makeApplicationDataDir()
{
    QString dir;
    char const *p = getenv("HOME");
    if (p) {
        QString org = QApplication::organizationName();
        QString name = QApplication::applicationName();
        dir = pathcat(p, '.' + org + '/' + name);
        QDir().mkpath(dir);
    }
    return dir;
}

#endif

MySettings::MySettings(QObject *)
    : QSettings(pathcat(application_data_dir, "SkyMPC.ini"), QSettings::IniFormat)
{
}

