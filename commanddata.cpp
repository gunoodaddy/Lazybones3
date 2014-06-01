#include "commanddata.h"
#include <QThread>
#include <QDebug>
#include "Logger.h"

CommandData::CommandData(const QString &line) :
	QObject(0)
{
	setLine(line);
}

void CommandData::setLine(const QString &line)
{
	QStringList list = line.split(" ", QString::SkipEmptyParts);

	if(list.size() <= 0)
	{
		setCommand(line.trimmed());
		return;
	}

	setCommand(list[0].trimmed());
	for(int i = 1; i < list.size(); i++)
	{
		addArg(list[i]);
	}
}

bool CommandData::doit(void)
{
	InputAction action;

	InputAction::ActionType type = InputAction::None;
	int mouseX = -1;
	int mouseY = -1;
	int lastArgIndex = 0;
	bool mouseActionFlag = false;
	bool keyActionFlag = false;

	if(m_command.compare("mouseleftclick", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::MouseLeftClick;
		mouseActionFlag = true;
	}
	else if(m_command.compare("mouseleftpress", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::MouseLeftPress;
		mouseActionFlag = true;
	}
	else if(m_command.compare("mouseleftrelease", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::MouseLeftRelease;
		mouseActionFlag = true;
	}
	else if(m_command.compare("mouserightclick", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::MouseRightClick;
		mouseActionFlag = true;
	}
	else if(m_command.compare("mouserightpress", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::MouseRightPress;
		mouseActionFlag = true;
	}
	else if(m_command.compare("mouserightrelease", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::MouseRightRelease;
		mouseActionFlag = true;
	}
	else if(m_command.compare("keyinput", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::KeyInput;
		keyActionFlag = true;
	}
	else if(m_command.compare("keypress", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::KeyPress;
		keyActionFlag = true;
	}
	else if(m_command.compare("keyrelease", Qt::CaseInsensitive) == 0)
	{
		type = InputAction::KeyRelease;
		keyActionFlag = true;
	}
	else
	{
		// not supported command
		LOG_WARNING() << "NOT SUPPORTED LINE COMMAND :" << m_command;
		return false;
	}

	action.setActionType(type);

	if(mouseActionFlag)
	{
		if(argCount() >= 2)
		{
			mouseX = arg(0).toInt();
			mouseY = arg(1).toInt();

			action.setMouseX(mouseX);
			action.setMouseY(mouseY);

			lastArgIndex = 2;
		}
		else
		{
			LOG_WARNING() << "INVALID MOUSE ACTION ARGUMENT :" << m_command << argCount();
			return false;
		}
	}

	if(keyActionFlag)
	{
		if(argCount() >= 1)
		{
			action.setKeyCode(arg(0).toInt());

			lastArgIndex = 1;
		}
		else
		{
			LOG_WARNING() << "INVALID KEY ACTION ARGUMENT :" << m_command << argCount();
			return false;
		}
	}

	// last arg always be "delayedMsec"
	if(lastArgIndex >= 0 && lastArgIndex < argCount())
	{
		action.setDelayedMsec(arg(lastArgIndex).toInt());
	}

	qDebug() << "COMMAND LINE :" << action.description();

	if(action.delayedMsec() > 0)
		QThread::msleep(action.delayedMsec());
	action.doitNow();

	return true;
}
