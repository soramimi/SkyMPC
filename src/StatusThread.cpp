#include "MusicPlayerClient.h"
#include "StatusThread.h"

#include <QMutex>

struct StatusThread::Private {
	QMutex mutex;
	Host host;
	MusicPlayerClient mpc;
	bool f_status;
	bool f_currentsong;
	PlayingInfo info;
};

StatusThread::StatusThread()
{
	pv = new Private();
	pv->f_status = false;
	pv->f_currentsong = false;
}

StatusThread::~StatusThread()
{
	delete pv;
}

void StatusThread::data(PlayingInfo *out) const
{
	QMutexLocker lock(&pv->mutex);
	*out = pv->info;
}

void StatusThread::setHost(Host const &host)
{
	pv->host = host;
}

bool StatusThread::isOpen() const
{
	return pv->mpc.isOpen();
}

void StatusThread::run()
{
	pv->mpc.open(pv->host);
	while (1) {
		if (isInterruptionRequested()) {
			break;
		}
		if (isOpen()) {
			PlayingInfo info;
			pv->f_status = pv->mpc.do_status(&info.status);
			pv->f_currentsong = pv->mpc.do_currentsong(&info.property);
			{
				QMutexLocker lock(&pv->mutex);
				pv->info = info;
			}
			emit onUpdate();
		}
		QThread::msleep(250);
	}
	pv->mpc.close();
}
