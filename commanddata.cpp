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
			int keyCode = parseKeyCode(arg(0));
			if(keyCode < 0)
			{
				LOG_WARNING() << "INVALID KEY CODE :" << m_command << arg(0);
				return false;
			}
			LOG_WARNING() << "KEY CODE :" << m_command << keyCode;
			action.setKeyCode(keyCode);

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
	else
	{
		action.setDelayedMsec(0);
	}

	qDebug() << "COMMAND LINE :" << action.description();

	if(action.delayedMsec() > 0)
		QThread::msleep(action.delayedMsec());
	action.doitNow();

	return true;
}

int CommandData::parseKeyCode(const QString &str)
{
	QString keyName = str.toUpper();

	if(str[0] == '\'')
		keyName = str.mid(1, str.length() - 2).toUpper();

	static QMap<QString, Qt::Key> KeyMap;
	if(KeyMap.size() == 0)
	{
		KeyMap["ESC"] = Qt::Key_Escape;
		KeyMap["ALT"] = Qt::Key_Alt;
		KeyMap["CTRL"] = Qt::Key_Control;
		KeyMap["SHIFT"] = Qt::Key_Shift;
		KeyMap["SPACE"] = Qt::Key_Space;
		KeyMap["ENTER"] = Qt::Key_Enter;
		KeyMap["RETURN"] = Qt::Key_Return;
		KeyMap["F1"] = Qt::Key_F1;
		KeyMap["F2"] = Qt::Key_F2;
		KeyMap["F3"] = Qt::Key_F3;
		KeyMap["F4"] = Qt::Key_F4;
		KeyMap["F5"] = Qt::Key_F5;
		KeyMap["F6"] = Qt::Key_F6;
		KeyMap["F7"] = Qt::Key_F7;
		KeyMap["F8"] = Qt::Key_F8;
		KeyMap["F9"] = Qt::Key_F9;
		KeyMap["F10"] = Qt::Key_F10;
		KeyMap["F11"] = Qt::Key_F11;
		KeyMap["F12"] = Qt::Key_F12;
	}

	if(keyName.length() == 1
			&& (keyName[0] >= '0' && keyName[0] <= '9'
			|| keyName[0] >= 'A' && keyName[0] <= 'Z'))
	{
		return (int)keyName[0].toLatin1();
	}

	int res = KeyMap[keyName];
	if(res > 0)
		return res;

	return -1;
}
