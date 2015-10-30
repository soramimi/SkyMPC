#include "MySettings.h"
#include "main.h"
#include <QApplication>
#include <QDir>
#include <QString>

#ifdef QT_VERSION
#include "pathcat.h"
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shlobj.h>

QString makeApplicationDataDir()
{
	QString dir;
	ushort tmp[MAX_PATH];
	if (SHGetSpecialFolderPath(0, (wchar_t *)tmp, CSIDL_APPDATA, FALSE)) {
		QString org = QApplication::organizationName();
		QString name = QApplication::applicationName();
		dir = pathcat(QString::fromUtf16((ushort const *)tmp), org + '/' + name);
		QDir().mkpath(dir);
	}
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

