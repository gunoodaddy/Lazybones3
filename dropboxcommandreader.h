#ifndef DROPBOXCOMMANDREADER_H
#define DROPBOXCOMMANDREADER_H

#include <QObject>
#include <QMutex>
#include "commanddata.h"

class CommandReaderWorker;

class DropoxCommandReader : public QObject
{
	Q_OBJECT
public:
	explicit DropoxCommandReader(QObject *parent = 0);

	void start(void);
	void stop(void);

public slots:

private:
	friend class CommandReaderWorker;

	CommandReaderWorker *m_threadWorker;
	QMutex m_mutex;
};

#endif // DROPBOXCOMMANDREADER_H
