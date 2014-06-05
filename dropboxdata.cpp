#include "dropboxdata.h"
#include <QDir>
#include <QScreen>
#include <QDateTime>
#include <QApplication>
#include <QPixmap>
#include <QMessageBox>

static const QString DEFAULT_TITLE = "Screen Shot";
static const QString DEFAULT_SCREEN_SHOT_PREFIX = "ScreenShot_";

DropBoxData::DropBoxData(QObject *parent) :
	QObject(parent), m_running(false)
{
	QObject::connect(&m_timerScreenShot, SIGNAL(timeout()), this, SLOT(onTimerScreenShot()));
}

QString DropBoxData::defaultDropBoxPath(void)
{
	QString defaultDropBoxPath = QDir::toNativeSeparators(QDir::homePath() + "/Dropbox");
	return defaultDropBoxPath;
}

void DropBoxData::setRunning(bool running)
{
	m_running = running;
	emit runningChanged();

	if(m_running)
	{
		m_timerScreenShot.setInterval(m_screenShotIntervalSec * 1000);
		m_timerScreenShot.start();
		doScreenShot("", QRect(), true, true);
	}
	else
	{
		m_timerScreenShot.stop();
	}
}

void DropBoxData::removeAllScreenShotFiles(void)
{
	QDir dir(m_dropBoxPath);
	foreach(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::AllDirs ))
	{
		if (info.isFile())
		{
			//do something
			if(info.fileName().indexOf(DEFAULT_SCREEN_SHOT_PREFIX) == 0)
			{
				QFile::remove(info.filePath());
			}
		}
		if (info.isDir())
		{
			//scan again
		}
	}
}

bool DropBoxData::doScreenShotWithFilePath(QString filePath)
{
	QScreen *screen = qApp->primaryScreen();
	if (screen)
	{
		QPixmap screenShotPixmap = screen->grabWindow(0);
		if(!screenShotPixmap.isNull())
		{
			if(screenShotPixmap.save(filePath, NULL, 100))
			{
				return true;
			}
		}
	}
	return false;
}

void DropBoxData::doScreenShot(QString prefix, QRect rect, bool showErrorPopup, bool checkMaxCount)
{
	QString path = m_dropBoxPath;
	path += QDir::separator();
	if(prefix.isEmpty())
		prefix = DEFAULT_SCREEN_SHOT_PREFIX;
	path += prefix;
	path += QDateTime::currentDateTime().toString("MMdd_hhmmss") + ".jpg";

	QScreen *screen = qApp->primaryScreen();
	if (screen)
	{
		QPixmap screenShotPixmap = screen->grabWindow(0);

		screenShotPixmap = screenShotPixmap.copy(rect);

		if(!screenShotPixmap.isNull())
		{
			if(screenShotPixmap.save(path, NULL, 100))
			{
				if(checkMaxCount)
				{
					m_shotPathList.push_back(path);

					while(m_shotPathList.size() > m_screenShotFileRotateCount)
					{
						QString path = m_shotPathList.first();
						m_shotPathList.pop_front();
						QFile::remove(path);
					}
				}
			}
			else
			{
				if(showErrorPopup)
					QMessageBox::warning(NULL, DEFAULT_TITLE, "Failed to save the screen shot.\r\n" + path);
			}
		}
		else
		{
			if(showErrorPopup)
				QMessageBox::warning(NULL, DEFAULT_TITLE, "Failed to grab the screen shot.");
		}
	}
	else
	{
		if(showErrorPopup)
			QMessageBox::warning(NULL, DEFAULT_TITLE, "Failed to get primary screen.");
	}
}

void DropBoxData::onTimerScreenShot(void)
{
	doScreenShot();
}

bool DropBoxData::findFile(QString fileName)
{
	QString path = m_dropBoxPath;
	path += QDir::separator();
	path += fileName;
	path = QDir::toNativeSeparators(path);
	return QFile::exists(path);
}
