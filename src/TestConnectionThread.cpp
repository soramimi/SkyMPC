#include "TestConnectionThread.h"

TestConnectionThread::TestConnectionThread(ServerItem const *server, TestConnectResultDialog *dlg)
{
	data.server = server;
	data.dlg = dlg;
	dlg->bindThread(this);
}

void TestConnectionThread::append(const QString &text)
{
	data.message.append(text);
	emit updateMessage(data.message);
}

void TestConnectionThread::run()
{
	QTcpSocket sock;
	try {
		MusicPlayerClient mpc;
		MusicPlayerClient::OpenResult r = mpc.open(&sock, data.server->host, this);
		if (r.success) {
			append(tr("Connection was successfully established.") + '\n');
		}
		if (r.incorrect_password) {
			append(tr("Authentication failure.") + '\n');
		} else if (!r.success) {
			append(tr("Unexplained connection failure.") + '\n');
		}
	} catch (QString const &e) {
		append(e);
	}
	emit updateMessage(data.message);
	sock.close();
}

