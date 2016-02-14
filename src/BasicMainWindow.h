#ifndef BASICMAINWINDOW_H
#define BASICMAINWINDOW_H

#include <QMainWindow>
#include <QEvent>

class Host;
class Command;

class BasicMainWindow : public QMainWindow {
	Q_OBJECT
protected:
	struct Private;
	Private *pv;
	static QString makeStyleSheetText();
	void releaseMouseIfGrabbed();
	void stopSleepTimer();
	void stopStatusThread();
	void execSleepTimerDialog();

	void updatePlayingStatus();
	virtual void updateServersComboBox() {}
	virtual void setPageConnected() {}
	virtual void updateTreeTopLevel() {}
	virtual void setPageDisconnected() {}
	virtual void setVolumeEnabled(bool) {}
	virtual void clearTreeAndList() {}
	virtual void displayStopStatus() {}

	virtual void setRepeatEnabled(bool f);
	virtual void setSingleEnabled(bool f);
	virtual void setConsumeEnabled(bool f);
	virtual void setRandomEnabled(bool f);

	virtual void displayPlayStatus(const QString & /*title*/, const QString & /*artist*/, const QString & /*disc*/) = 0;
	virtual void seekProgressSlider(double /*elapsed*/, double /*total*/) {}
	virtual void displayProgress(double /*elapsed*/) {}
	virtual void updatePlayIcon() {}
	virtual void updatePlaylist() {}
	virtual void updateCurrentSongIndicator() {}
	void invalidateCurrentSongIndicator();

	static QString makeServerText(const Host &host);
	QString serverName() const;
	void showNotify(const QString &text);
	void showError(const QString &text);
	void update(bool mpdupdate);
	void checkDisconnected();
	void execSongProperty(const QString &path, bool addplaylist);
	void clearPlaylist();
	void startSleepTimer(int mins);
	void doQuickSave1();
	void doQuickSave2();
	void doQuickLoad1();
	void doQuickLoad2();
	virtual void doUpdateStatus();
public:
	BasicMainWindow(QWidget *parent);
	~BasicMainWindow();
	virtual void eatMouse();
	virtual bool isAutoReconnectAtStartup();
	virtual void preexec();
	bool execCommand(const Command &c);
	void connectToMPD(const Host &host);
	bool isPlaying() const;
	void startStatusThread();
	void addToPlaylist(const QString &path, int to, bool update);
	void play();
	void pause();
	void stop();
	void play(bool toggle);
	void disconnectNetwork();
	void loadPlaylist(const QString &name, bool replace);
	bool savePlaylist(const QString &name);
	void execAddLocationDialog();

	static BasicMainWindow *findMainWindow(QObject *hint = nullptr);
private slots:
	void onVolumeChanged();
	void onUpdateStatus();
};

enum {
	EVENT_FocusChanged = QEvent::User,
	EVENT_QueryInfo,
};

#endif // BASICMAINWINDOW_H
