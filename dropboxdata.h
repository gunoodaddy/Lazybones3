#ifndef DROPBOXDATA_H
#define DROPBOXDATA_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "dropboxcommandreader.h"

class DropBoxData : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString defaultDropBoxPath READ defaultDropBoxPath NOTIFY defaultDropBoxPathChanged)
	Q_PROPERTY(QString dropBoxPath READ dropBoxPath WRITE setDropBoxPath NOTIFY dropBoxPathChanged)
	Q_PROPERTY(int screenShotIntervalSec READ screenShotIntervalSec WRITE setScreenShotIntervalSec NOTIFY screenShotIntervalSecChanged)
	Q_PROPERTY(int screenShotFileRotateCount READ screenShotFileRotateCount WRITE setScreenShotFileRotateCount NOTIFY screenShotFileRotateCountChanged)
	Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
	explicit DropBoxData(QObject *parent = 0);

	QString defaultDropBoxPath(void);

	const QString& dropBoxPath(void) { return m_dropBoxPath; }
	void setDropBoxPath(const QString &path) { m_dropBoxPath = path; emit dropBoxPathChanged(); }

	int screenShotIntervalSec(void) { return m_screenShotIntervalSec; }
	void setScreenShotIntervalSec(int msec) { m_screenShotIntervalSec = msec; emit screenShotIntervalSecChanged(); }

	int screenShotFileRotateCount(void) { return m_screenShotFileRotateCount; }
	void setScreenShotFileRotateCount(int count) { m_screenShotFileRotateCount = count; emit screenShotFileRotateCountChanged(); }

	bool running(void) { return m_running; }
	void setRunning(bool running);

	bool findFile(QString fileName);

	bool doScreenShotWithFilePath(QString filePath);
	void doScreenShot(QString prefix = "", bool showErrorPopup = false, bool checkMaxCount = true);
	Q_INVOKABLE void removeAllScreenShotFiles(void);

	DropoxCommandReader *commandReader(void) { return &m_dropoxCommandReader; }

signals:
	void defaultDropBoxPathChanged(void);
	void dropBoxPathChanged(void);
	void screenShotIntervalSecChanged(void);
	void screenShotFileRotateCountChanged(void);
	void runningChanged(void);

public slots:
	void onTimerScreenShot(void);

private:
	QString m_dropBoxPath;
	int m_screenShotIntervalSec;
	int m_screenShotFileRotateCount;

	bool m_running;
	QTimer m_timerScreenShot;
	QVector<QString> m_shotPathList;

	DropoxCommandReader m_dropoxCommandReader;
};

#endif // DROPBOXDATA_H
