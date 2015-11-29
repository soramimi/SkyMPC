#ifndef TESTCONNECTIONTHREAD_H
#define TESTCONNECTIONTHREAD_H

#include <QThread>

#include "Server.h"
#include "TestConnectResultDialog.h"
#include "MusicPlayerClient.h"

class TestConnectionThread : public QThread, public MusicPlayerClient::Logger {
	Q_OBJECT
private:
	struct {
		ServerItem const *server;
		TestConnectResultDialog *dlg;
		QString message;
	} data;
protected:
	virtual void append(const QString &text);
	virtual void run();
public:
	TestConnectionThread(ServerItem const *server, TestConnectResultDialog *dlg);
signals:
	void updateMessage(QString const &msg);
};

#endif // TESTCONNECTIONTHREAD_H
