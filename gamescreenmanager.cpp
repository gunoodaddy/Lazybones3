#include "gamescreenmanager.h"
#include <QPixmap>
#include <QRunnable>
#include <QThread>
#include <QScreen>
#include <QGuiApplication>
#include <QThreadPool>
#include <QDebug>
#include <QQmlEngine>
#include "Lazybones.h"
#include "gamescreenlistmodel.h"
#include "settingmanager.h"

#define kPixelThreshold 3

GameScreenManager::GameScreenManager(void) : QObject(0), m_mutex(QMutex::Recursive)
{
	QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);	
}

GameScreenManager::~GameScreenManager(void)
{
}

GameScreenListModel *GameScreenManager::screenModel(void)
{
	return SETTING_MANAGER->gameScreenModel();
}

void GameScreenManager::setScreenMatchResult(const QString &name, bool result)
{
	m_screenMatchMap[name] = result;
}

int GameScreenManager::screenMatchResult(const QString &name)
{
	QMap<QString, bool>::iterator it = m_screenMatchMap.find(name);
	if(it != m_screenMatchMap.end())
		return it.value();
	return -1;
}

void GameScreenManager::removeScreenData(int index)
{
	QSharedPointer<GameScreenData> screenData = screenModel()->at(index);
	if(!screenData)
		return;

	QString name = screenData->name();
	screenModel()->remove(index);

	if(screenModel()->firstIndexOfName(name) < 0)
	{
		m_screenList.remove(name);
	}
}

bool GameScreenManager::addScreenData(GameScreenData *newScreenData)
{
	QSharedPointer<GameScreenData> screenData =
			QSharedPointer<GameScreenData>(
				new GameScreenData(
					newScreenData->name(),
					newScreenData->pixelX(),
					newScreenData->pixelY(),
					newScreenData->color()));

	QMutexLocker locker(&m_mutex);

	if(screenModel()->find(newScreenData, true))
		return false;

	int rowIndex = screenModel()->firstIndexOfName(newScreenData->name());

	screenModel()->add(screenData, rowIndex);

	if(m_screenList.indexOf(newScreenData->name()) < 0)
	{
		m_screenList.add(newScreenData->name());
		m_screenList.sort(0);
	}
	//qDebug() << "m_screenList : " << m_screenList.count();
	return true;
}

QSharedPointer<GameScreenData> GameScreenManager::findScreenData(const QString &name)
{
	return screenModel()->find(name);
}

bool GameScreenManager::isCurrentScreen(const QString &name, bool checkNow)
{
	QMutexLocker locker(&m_mutex);

	if(checkNow)
		checkCurrentScreen();

	if(m_lastScreenNames.find(name) != m_lastScreenNames.end() )
		return true;
	return false;
}

bool GameScreenManager::findColor(QScreen *screen, QImage *screenImage, int x, int y, const QColor &clr)
{
	int beginX = qMax(0, x - kPixelThreshold);
	int endX = qMin(screenImage->width(), x + kPixelThreshold);
	int beginY = qMax(0, y - kPixelThreshold);
	int endY = qMin(screenImage->height(), y + kPixelThreshold);

	QColor color = Lazybones::getPixelColor(screen, screenImage, x, y);
	if(clr.toRgb() == color.toRgb())
	{
		return true;
	}

	for(int i = beginX; i < endX; i++)
	{
		for(int j = beginY; j < endY; j++)
		{
			QColor color = Lazybones::getPixelColor(screen, screenImage, i, j);
			if(clr.toRgb() == color.toRgb())
			{
				return true;
			}
		}
	}
	return false;
}

void GameScreenManager::checkCurrentScreen(void)
{
	GameScreenListModel *model = screenModel();
	QScreen *screen = QGuiApplication::primaryScreen();
	QPixmap screenShotPixmap = screen->grabWindow(0);
	QImage image = screenShotPixmap.toImage();

	QMutexLocker locker(&m_mutex);

	m_lastScreenNames.clear();

	bool match = false;
	bool skip = false;

	for(int i = 0; i < model->count(); i++)
	{
		QSharedPointer<GameScreenData> data = model->at(i);
		QSharedPointer<GameScreenData> nextData;
		if(i < model->count() - 1)
			nextData = model->at(i + 1);

		if(!skip)
		{
			if(findColor(screen, &image, data->pixelX(), data->pixelY(), data->color()))
			{
				match = true;

			}
			else
			{
				match = false;
				skip = true;
			}
		}

		if(!nextData || nextData->name() != data->name())
		{
			if(match)
			{
				m_lastScreenNames.insert(data->name());
				qDebug() << "MATCH : " << data->name();
			}
			match = false;
			skip = false;
		}
	}
}

QPoint GameScreenManager::findPixel(QColor clr, int redNoise, int greenNoise, int blueNoise, QRect hintRect)
{
	QScreen *screen = QGuiApplication::primaryScreen();
	QPixmap screenShotPixmap = screen->grabWindow(0);
	QImage image = screenShotPixmap.toImage();

	qreal ratioH = image.size().width() / screen->size().width();
	qreal ratioV = image.size().height() / screen->size().height();

	int beginX = 0;
	int endX = image.width();
	int beginY = 0;
	int endY = image.height();

	if(!hintRect.isNull())
	{
		beginX = qMax(0, hintRect.x()) * ratioH;
		endX = qMin(image.width(), hintRect.x() + hintRect.width()) * ratioH;
		beginY = qMax(0, hintRect.y()) * ratioV;
		endY = qMin(image.height(), hintRect.y() + hintRect.height()) * ratioV;
	}

	for(int i = beginX; i < endX; i++)
	{
		for(int j = beginY; j < endY; j++)
		{
			QColor color = image.pixel(i, j);

			if(qAbs(clr.red() - color.red()) > redNoise)
				continue;
			if(qAbs(clr.green() - color.green()) > greenNoise)
				continue;
			if(qAbs(clr.blue() - color.blue()) > blueNoise)
				continue;

			qDebug() << "findPixel " << i << j << color.name();

			return Lazybones::toRealPos(&image, i, j);
		}
	}

	return QPoint();
}
