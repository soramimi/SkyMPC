#ifndef STATUSTHREAD_H
#define STATUSTHREAD_H

#include "MusicPlayerClient.h"

#include <QThread>

class PlayingInfo {
public:
	MusicPlayerClient::StringMap status;
	MusicPlayerClient::StringMap property;
};

class StatusThread : public QThread {
	Q_OBJECT
private:
	struct Private;
	Private *pv;
protected:
	void run();
public:
	StatusThread();
	~StatusThread();
	void data(PlayingInfo *out) const;
	bool isOpen() const;
	void setHost(const Host &host);
signals:
	void onUpdate();
};

#endif // STATUSTHREAD_H
