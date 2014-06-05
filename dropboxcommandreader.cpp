#include "dropboxcommandreader.h"
#include <QThread>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QDateTime>
#include <QApplication>
#include <QPixmap>
#include <QPainter>
#include "settingmanager.h"
#include "Logger.h"

const QString REQ_FILE_NAME = "req.txt";
const QString RES_FILE_NAME = "res.jpg";

class CommandReaderWorker : public QThread
{
public:
	CommandReaderWorker(DropoxCommandReader *owner) : QThread(), m_owner(owner), m_stop(false)
	{
	}

	void run() Q_DECL_OVERRIDE
	{
		while(!m_stop)
		{
			QThread::msleep(100);

			QString reqFilePath = SETTING_MANAGER->dropBox()->dropBoxPath() + QDir::separator();
			reqFilePath += REQ_FILE_NAME;

			if(!QFile::exists(reqFilePath))
				continue;

			qDebug() << "REQ FILE DETECTED.." << reqFilePath;

			QFile file(reqFilePath);

			if(!file.open(QIODevice::ReadOnly))
			{
				QFile::remove(reqFilePath);
				continue;
			}

			QTextStream in(&file);

			QVector<QSharedPointer<CommandData> > commandList;
			while(!in.atEnd())
			{
				QString line = in.readLine().trimmed();

				if(line.isEmpty())
					continue;

				QSharedPointer<CommandData> data = QSharedPointer<CommandData>(new CommandData(line));
				if(data->doit())
					commandList.append(data);
			}

			QThread::msleep(100);

			saveScreenShotResult(&commandList);

			file.close();

			QFile::remove(reqFilePath);
		}
		LOG_INFO() << "COMMAND READER THREAD FINISHED";
	}

	void saveScreenShotResult(QVector<QSharedPointer<CommandData> > *commandList)
	{
		QString resFilePath = SETTING_MANAGER->dropBox()->dropBoxPath() + QDir::separator();
		resFilePath += RES_FILE_NAME;

		QScreen *screen = qApp->primaryScreen();
		if (screen)
		{
			QPixmap screenShotPixmap = screen->grabWindow(0);
			if(!screenShotPixmap.isNull())
			{
//				QPainter pixPaint(&screenShotPixmap);

//				for(int i = 0; i < commandList->size(); i++)
//				{
//					pixPaint.setBrush(Qt::blue);
//					pixPaint.setPen(QPen(Qt::red, 5));
//					//pixPaint.drawEllipse();
//				}

				screenShotPixmap.save(resFilePath, NULL, 100);
			}
		}
	}

	DropoxCommandReader *m_owner;
	bool m_stop;
};

DropoxCommandReader::DropoxCommandReader(QObject *parent) : QObject(parent), m_threadWorker(NULL)
{
}

void DropoxCommandReader::start(void)
{
	QMutexLocker locker(&m_mutex);
	if(m_threadWorker)
		stop();

	m_threadWorker = new CommandReaderWorker(this);
	m_threadWorker->start();
}

void DropoxCommandReader::stop(void)
{
	if(!m_threadWorker)
		return;

	m_threadWorker->m_stop = true;
	m_threadWorker->quit();
	m_threadWorker->wait();
	delete m_threadWorker;
	m_threadWorker = NULL;
}
