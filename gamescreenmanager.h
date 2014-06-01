#ifndef SCREENCAPTUREWORKER_H
#define SCREENCAPTUREWORKER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <QStringListModel>
#include <QMutex>
#include <QColor>
#include <QSet>
#include <QScreen>
#include "simplestringlistmodel.h"

class GameScreenData;
class GameScreenListModel;
class GameScreenThreadWorker;

class GameScreenManager : public QObject
{
	Q_OBJECT
	Q_PROPERTY(GameScreenListModel *screenModel READ screenModel NOTIFY screenModelChanged)
	Q_PROPERTY(SimpleStringListModel *screenList READ screenList NOTIFY screenListChanged)

public:
	GameScreenManager(void);
	~GameScreenManager(void);
	void start();
	void pause();

	Q_INVOKABLE bool addScreenData(GameScreenData *newScreenData);
	Q_INVOKABLE void removeScreenData(int index);

	QSharedPointer<GameScreenData> findScreenData(const QString &name);

	GameScreenListModel *screenModel(void);

	SimpleStringListModel *screenList(void) { return &m_screenList; }

	bool findColor(QScreen *screen, QImage *screenImage, int x, int y, const QColor &clr);
	void checkCurrentScreen(void);

	Q_INVOKABLE void setScreenMatchResult(const QString &name, bool result);
	Q_INVOKABLE int screenMatchResult(const QString &name);

	Q_INVOKABLE bool isCurrentScreen(const QString &name, bool checkNow = false);

	Q_INVOKABLE QPoint findPixel(QColor clr, int redNoise = 0, int greenNoise = 0, int bluseNoise = 0, QRect hintRect = QRect());

signals:
	void screenModelChanged(void);
	void screenListChanged(void);

private slots:

private:
	QMap<QString, bool> m_screenMatchMap;
	QSet<QString> m_lastScreenNames;

	SimpleStringListModel m_screenList;
	QMutex m_mutex;
};

#endif // SCREENCAPTUREWORKER_H
