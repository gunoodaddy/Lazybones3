#ifndef COMMANDDATA_H
#define COMMANDDATA_H

#include <QObject>
#include <QVector>

#include "inputaction.h"

class CommandData : public QObject
{
	Q_OBJECT
public:
	explicit CommandData(const QString &line);

	const QString & command(void)
	{
		return m_command;
	}
	void setCommand(const QString &command)
	{
		m_command = command;
	}

	void addArg(QString arg)
	{
		m_argList.push_back(arg);
	}

	int argCount(void)
	{
		return m_argList.count();
	}

	QString arg(int index)
	{
		return m_argList[index];
	}

	void setLine(const QString &line);

	bool doit(void);

private:

signals:

public slots:

private:
	QString m_command;
	QVector<QString> m_argList;
};

#endif // COMMANDDATA_H
