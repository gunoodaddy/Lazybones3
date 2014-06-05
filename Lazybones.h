#ifndef AUTODIABLO3_H
#define AUTODIABLO3_H

#include <QObject>
#include <QQmlEngine>
#include <QStringListModel>
#include "settingmanager.h"
#include "gamescreenmanager.h"
#include "dropboxdata.h"
#include "macroaction.h"
#include "macropresetlistmodel.h"
#include <QDateTime>
#include <QDebug>
#include <QScreen>
#include <QApplication>
#include "InputEvent/InputEventManager.h"
#include "Defines.h"

class Lazybones : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int currentMacroPresetIndex READ currentMacroPresetIndex WRITE setCurrentMacroPresetIndex NOTIFY currentMacroPresetIndexChanged)
	Q_PROPERTY(MacroPresetListModel *macroPresetList READ macroPresetList NOTIFY macroPresetListChanged)
	Q_PROPERTY(MacroPresetData *currentMacroPreset READ currentMacroPreset NOTIFY currentMacroPresetChanged)
	Q_PROPERTY(DropBoxData *dropBox READ dropBox NOTIFY dropBoxChanged())
	Q_PROPERTY(GameScreenManager *gameScreenManager READ gameScreenManagerPrivate NOTIFY gameScreenManagerChanged)

public:
	explicit Lazybones(QObject *parent = 0);

	static void initialize(void);

	Q_INVOKABLE InputAction * createInputAction(void)
	{
		InputAction *newAction = new InputAction();
		QQmlEngine::setObjectOwnership(newAction, QQmlEngine::JavaScriptOwnership);
		return newAction;
	}

	Q_INVOKABLE MacroAction * createMacroAction(void)
	{
		MacroAction *newAction = new MacroAction();
		QQmlEngine::setObjectOwnership(newAction, QQmlEngine::JavaScriptOwnership);
		return newAction;
	}

	Q_INVOKABLE GameScreenData * createGameScreenData(void)
	{
		GameScreenData *newData = new GameScreenData();
		QQmlEngine::setObjectOwnership(newData, QQmlEngine::JavaScriptOwnership);
		return newData;
	}

	Q_INVOKABLE qint64 currentMSecsSinceEpoch(void)
	{
		return QDateTime::currentMSecsSinceEpoch();
	}

	Q_INVOKABLE void addNewMacroPreset(QString title)
	{
		QSharedPointer<MacroPresetData> newData = QSharedPointer<MacroPresetData>(new MacroPresetData());
		newData->setTitle(title);
		macroPresetList()->add(newData);
		qDebug() << "addNewMacroPreset :" << title << newData->remainHangUpSeconds();
	}

	Q_INVOKABLE bool copyMacroPreset(int index, QString title)
	{
		if(!macroPresetList()->find(title))
		{
			QSharedPointer<MacroPresetData> newData = QSharedPointer<MacroPresetData>(new MacroPresetData());
			macroPresetList()->copy(index, newData);
			newData->setTitle(title);
			qDebug() << "copyMacroPreset :" << index << title;
			return true;
		}
		return false;
	}

	Q_INVOKABLE void removeMacroPreset(int index)
	{
		macroPresetList()->deleteAt(index);

		if(macroPresetList()->count() <= 0)
		{
			SETTING_MANAGER->addDefaultMacroPreset();
			setCurrentMacroPresetIndex(0);
		}
	}

	Q_INVOKABLE bool findMacroPreset(QString title, bool exceptCurrent = false)
	{
		return macroPresetList()->find(title, exceptCurrent ? m_currentMacroPresetIndex : -1);
	}

	Q_INVOKABLE QString makeNewMacroTitle(void)
	{
		int postfix = 0;
		QString title = "NewMacro";
		while(macroPresetList()->count() > 0)
		{
			QString candidate = title;
			if(postfix > 0)
				candidate = title + "_" + QString::number(postfix);
			int i = 0;
			for( ; i < macroPresetList()->count(); i++)
			{
				QSharedPointer<MacroPresetData> presetData = macroPresetList()->at(i);
				if(presetData->title() == candidate)
				{
					break;
				}
			}
			if(i >= macroPresetList()->count())
				return candidate;

			postfix++;
		}
		return title;
	}

	Q_INVOKABLE QString toRectString(QRect rect) {
		QString res;
		if(rect.isNull())
			return "";
		res =  QString::number(rect.x())
				+ ", " + QString::number(rect.y())
				+ ", " + QString::number(rect.width())
				+ ", " + QString::number(rect.height());
		return res;
	}

	Q_INVOKABLE QRect toStringRect(QString str) {
		QStringList list = str.split(",", QString::SkipEmptyParts);
		if(list.size() < 4)
			return QRect();

		QRect res;
		res.setX(list[0].toInt());
		res.setY(list[1].toInt());
		res.setWidth(list[2].toInt());
		res.setHeight(list[3].toInt());
		return res;
	}

	Q_INVOKABLE void save(void)
	{
		SETTING_MANAGER->save();
	}

	Q_INVOKABLE QRect availableGeometry(void)
	{
		QScreen *screen = QGuiApplication::primaryScreen();
		return screen->availableGeometry();
	}

	Q_INVOKABLE qreal screenX(qreal x)
	{
		QScreen *screen = QGuiApplication::primaryScreen();
		return x + screen->availableGeometry().x();
	}

	Q_INVOKABLE qreal screenY(qreal y)
	{
		QScreen *screen = QGuiApplication::primaryScreen();
		return y + screen->availableGeometry().y();
	}

	Q_INVOKABLE qreal localX(qreal screenX)
	{
		QScreen *screen = QGuiApplication::primaryScreen();
		return screenX - screen->availableGeometry().x();
	}

	Q_INVOKABLE qreal localY(qreal screenY)
	{
		QScreen *screen = QGuiApplication::primaryScreen();
		return screenY - screen->availableGeometry().y();
	}

	MacroPresetListModel * macroPresetList(void) { return SETTING_MANAGER->macroPresetList(); }
	MacroPresetData *currentMacroPreset(void) { return m_currentMacroPresetData.data(); }

	int currentMacroPresetIndex(void) { return m_currentMacroPresetIndex; }
	void setCurrentMacroPresetIndex(int presetIndex);

	GameScreenManager *gameScreenManagerPrivate(void) { return gameScreenManager(); }

	DropBoxData *dropBox(void) { return SETTING_MANAGER->dropBox(); }

	Q_INVOKABLE QColor pixelColor(int pixelX, int pixelY);

	static const QPoint & registeredLastMousePos(void) { return s_registeredLastMousePos; }
	static void setRegisteredLastMousePos(QPoint pos) { s_registeredLastMousePos = pos; }

	static int randInt(int low, int high)
	{
		QTime time = QTime::currentTime();
		qsrand((uint)time.msec());

		// Random number between low and high
		return qrand() % ((high + 1) - low) + low;
	}

	static QPoint toRealPos(QImage *image, int imageX, int imageY);

	static QColor getPixelColor(int pixelX, int pixelY);
	static QColor getPixelColor(QScreen *screen, QImage *image, int pixelX, int pixelY);
	static InputEventManager *inputEventManager(void) { return s_inputEventManager; }
	static GameScreenManager *gameScreenManager(void) { return s_gameScreenManager; }
	static bool isEmergencyStop(void) { return s_forceStopFlag; }
	static void setEmergencyStop(bool stop) { s_forceStopFlag = stop; }
	static QString toMMss(qint64 msec) {
		if(msec <= 0)
			return "00:00";
		int sec = int(msec / 1000);
		int MM = int(sec / 60);
		int ss = int(sec % 60);

		QString strMM = MM < 10 ? ("0" + QString::number(MM)) : ("" + QString::number(MM));
		QString strss = ss < 10 ? ("0" + QString::number(ss)) : ("" + QString::number(ss));
		return strMM + ":" + strss;
	}

private slots:
	void onKeyEvent(QKeyEvent *event);
	void onMouseEvent(QMouseEvent *event);

signals:
	void currentMacroPresetIndexChanged(void);
	void currentMacroPresetChanged(void);
	void macroPresetListChanged(void);
	void dropBoxChanged(void);
	void gameScreenManagerChanged(void);

public slots:
	void onMacroPresetListChanged(void);

private:
	bool m_running;

	QSharedPointer<MacroPresetData> m_currentMacroPresetData;
	int m_currentMacroPresetIndex;

	static bool s_forceStopFlag;
	static InputEventManager *s_inputEventManager;
	static GameScreenManager *s_gameScreenManager;
	static QPoint s_registeredLastMousePos;
};

#endif // AUTODIABLO3_H
