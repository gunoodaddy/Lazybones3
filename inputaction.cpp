#include "inputaction.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include "inputmethodlistmodel.h"
#include "Lazybones.h"

static QMap<InputAction::ActionType, QString> ACTION_MAPPING;
static QMap<int, QString> KEY_MAPPING;

static InputAction::ActionType findActionName(QString name)
{
	QMap<InputAction::ActionType, QString>::iterator it = ACTION_MAPPING.begin();
	for(; it != ACTION_MAPPING.end(); it++)
	{
		if(it.value() == name)
			return it.key();
	}
	return InputAction::None;
}

static int toKeyCode(QString keyName)
{
	keyName = keyName.toUpper();

	QMap<int, QString>::iterator it = KEY_MAPPING.begin();
	for(; it != KEY_MAPPING.end(); it++)
	{
		if(it.value() == keyName)
			return it.key();
	}

	if(keyName.length() == 1)
	{
		int sz = (int)keyName[0].toLatin1();
		return sz;
	}

	return -1;
}

static QString toKeyName(int code)
{
	QString name = KEY_MAPPING[code];
	if(name.length() > 0)
		return name;

	QKeySequence seq(code);
	return seq.toString();
}

InputAction::InputAction(QObject *parent) :
	QObject(parent), m_action(InputAction::None), m_key(-1), m_xPos(-1), m_yPos(-1), m_delayedMsec(1000)
  , m_running(false), m_keyRecording(false), m_useLastPosition(false), m_model(NULL)
{
	__initTimer();
}

InputAction::InputAction(InputAction::ActionType action, int keyCode, int xPos, int yPos, int delayed)
	: QObject(NULL), m_action(action), m_key(keyCode), m_xPos(xPos), m_yPos(yPos), m_delayedMsec(delayed)
	, m_running(false), m_keyRecording(false), m_useLastPosition(false), m_model(NULL)
{
	__initTimer();
}

InputAction::~InputAction(void)
{
	stop();
}

QString InputAction::keyName()
{
	return toKeyName(m_key);
}

/*static*/
void InputAction::initialize(void)
{
	ACTION_MAPPING[MouseLeftClick] = "MouseLeftClick";
	ACTION_MAPPING[MouseLeftPress] = "MouseLeftPress";
	ACTION_MAPPING[MouseLeftRelease] = "MouseLeftRelease";
	ACTION_MAPPING[MouseRightClick] = "MouseRightClick";
	ACTION_MAPPING[MouseRightPress] = "MouseRightPress";
	ACTION_MAPPING[MouseRightRelease] = "MouseRightRelease";
	ACTION_MAPPING[MouseMove] = "MouseMove";
	ACTION_MAPPING[KeyPress] = "KeyPress";
	ACTION_MAPPING[KeyRelease] = "KeyRelease";
	ACTION_MAPPING[KeyInput] = "KeyInput";

	KEY_MAPPING[Qt::Key_Shift] = "SHIFT";
	KEY_MAPPING[Qt::Key_Alt] = "ALT";
	KEY_MAPPING[Qt::Key_Control] = "CONTROL";
	KEY_MAPPING[Qt::Key_Escape] = "ESC";
	KEY_MAPPING[Qt::Key_Enter] = "ENTER";
	KEY_MAPPING[Qt::Key_Return] = "RETURN";
	KEY_MAPPING[Qt::Key_F1] = "F1";
	KEY_MAPPING[Qt::Key_F2] = "F2";
	KEY_MAPPING[Qt::Key_F3] = "F3";
	KEY_MAPPING[Qt::Key_F4] = "F4";
	KEY_MAPPING[Qt::Key_F5] = "F5";
	KEY_MAPPING[Qt::Key_F6] = "F6";
	KEY_MAPPING[Qt::Key_F7] = "F7";
	KEY_MAPPING[Qt::Key_F8] = "F8";
	KEY_MAPPING[Qt::Key_F9] = "F9";
	KEY_MAPPING[Qt::Key_F10] = "F10";
	KEY_MAPPING[Qt::Key_F11] = "F11";
	KEY_MAPPING[Qt::Key_F12] = "F12";
}

InputAction *InputAction::fromLineCode(QString line, int &doneIndex)
{
	line = line.trimmed();

	int startBraceIndex = line.indexOf('(');
	if(startBraceIndex < 0)
		return NULL;

	int endBraceIndex = line.indexOf(')', startBraceIndex);
	if(endBraceIndex < 0)
		return NULL;

	QString name = line.left(startBraceIndex).trimmed();
	InputAction::ActionType actionType = findActionName(name);
	if(actionType == InputAction::None)
		return NULL;

	if(endBraceIndex - startBraceIndex <= 1)
		return NULL;

	QString arguments = line.mid(startBraceIndex+1, (endBraceIndex - startBraceIndex - 1));
	if(arguments.length() <= 0)
		return NULL;

	QStringList tokens = arguments.split(",", QString::SkipEmptyParts);
	if(tokens.size() < 1)
		return NULL;

	doneIndex = endBraceIndex + 1;

	InputAction *newAction = new InputAction();
	newAction->setActionType(actionType);
	if(newAction->isKeyAction())
	{
		QString keyName = tokens[0];
		newAction->setKeyCode(toKeyCode(keyName));
		if(tokens.length() > 1)
			newAction->setDelayedMsec(tokens[1].toInt());
		return newAction;
	}
	else if(newAction->isMouseAction())
	{
		int pi = 0;
		if(tokens[0] == kReservedToken_LastPos)
		{
			newAction->setUseLastPos(true);
			pi = 1;
		}
		else
		{
			newAction->setMouseX(tokens[0].toInt());
			newAction->setMouseY(tokens[1].toInt());
			newAction->setUseLastPos(false);
			pi = 2;
		}

		if(tokens.length() > pi)
			newAction->setDelayedMsec(tokens[pi].toInt());
		return newAction;
	}

	return NULL;
}

QString InputAction::toLineCode(void)
{
	QString codes;
	QString args = argumentString();
	codes += ACTION_MAPPING[m_action];
	codes += "(";
	codes += args;
	if(m_delayedMsec > 0)
	{
		if(args.length() > 0)
			codes += ", ";
		codes += QString::number(m_delayedMsec);
	}

	codes += ")";
	return codes;
}

QString InputAction::argumentString(void)
{
	switch(m_action)
	{
	case MouseMove:
	case MouseLeftClick:
	case MouseLeftPress:
	case MouseLeftRelease:
	case MouseRightClick:
	case MouseRightPress:
	case MouseRightRelease:
		if(m_useLastPosition)
			return kReservedToken_LastPos;
		return QString::number(m_xPos) + ", " + QString::number(m_yPos);
	case KeyPress:
	case KeyRelease:
	case KeyInput:
		return keyName();
	default:
		return "";
	}
	return "";
}

void InputAction::copy(InputAction *action)
{
	if(action == NULL)
		return;

	m_action = (InputAction::ActionType)action->actionType();
	m_key = action->keyCode();
	m_xPos = action->mouseX();
	m_yPos = action->mouseY();
	m_delayedMsec = action->delayedMsec();
	m_useLastPosition = action->useLastPos();
}

void InputAction::__perform(void)
{
	//qDebug() << "InputAction action : " << m_action << m_key << m_xPos << m_yPos << m_delayedMsec;

	int xPos = m_xPos;
	int yPos = m_yPos;

	if(m_useLastPosition)
	{
		xPos = Lazybones::registeredLastMousePos().x();
		xPos = Lazybones::registeredLastMousePos().y();
	}

	switch(m_action)
	{
	case InputAction::MouseLeftClick:
		Lazybones::inputEventManager()->worker()->doMouseLeftClick(xPos, yPos);
		break;
	case InputAction::MouseLeftPress:
		Lazybones::inputEventManager()->worker()->doMouseLeftPress(xPos, yPos);
		break;
	case InputAction::MouseLeftRelease:
		Lazybones::inputEventManager()->worker()->doMouseLeftRelease(xPos, yPos);
		break;
	case InputAction::MouseRightClick:
		Lazybones::inputEventManager()->worker()->doMouseRightClick(xPos, yPos);
		break;
	case InputAction::MouseRightPress:
		Lazybones::inputEventManager()->worker()->doMouseRightPress(xPos, yPos);
		break;
	case InputAction::MouseRightRelease:
		Lazybones::inputEventManager()->worker()->doMouseRightRelease(xPos, yPos);
		break;
	case InputAction::MouseMove:
		Lazybones::inputEventManager()->worker()->doMouseMove(xPos, yPos);
		break;
	case InputAction::KeyPress:
		if(m_xPos >= 0 && m_yPos >= 0)
			Lazybones::inputEventManager()->worker()->doMouseMove(m_xPos, m_yPos);
		Lazybones::inputEventManager()->worker()->doKeyboardPress((Qt::Key)m_key);
		break;
	case InputAction::KeyRelease:
		if(m_xPos >= 0 && m_yPos >= 0)
			Lazybones::inputEventManager()->worker()->doMouseMove(m_xPos, m_yPos);
		Lazybones::inputEventManager()->worker()->doKeyboardRelease((Qt::Key)m_key);
		break;
	case InputAction::KeyInput:
		if(m_xPos >= 0 && m_yPos >= 0)
			Lazybones::inputEventManager()->worker()->doMouseMove(m_xPos, m_yPos);
		Lazybones::inputEventManager()->worker()->doKeyboardInput((Qt::Key)m_key);
		break;
	default:
		break;
	}
}

QString InputAction::description(void)
{
	QString str;
	str = ACTION_MAPPING[m_action];
	str += " ";
	QString args = argumentString();
	str += args;
	if(args.length() > 0)
		str += " ";
	str += ("Delay : " + QString::number(m_delayedMsec) + "msec");
	return str;
}

void InputAction::stop(void)
{
	m_timer.stop();
	setRunning(false);
}

void InputAction::test(void)
{
	m_doNextAction = false;

	__doit();
}

void InputAction::doit(void)
{
	m_doNextAction = true;

	__doit();
}

void InputAction::doitNow(void)
{
	__perform();
}

void InputAction::__doit(void)
{
	setRunning(true);

	if(m_delayedMsec > 10)
	{
		m_timer.setInterval(m_delayedMsec);
		m_timer.start();
	}
	else
	{
		onTriggered();
	}
}

void InputAction::setActionType(int actionType)
{
	m_action = (InputAction::ActionType)actionType;
	emit actionTypeChanged();
	emit descriptionChanged();
}

void InputAction::setKeyCode(int keyCode)
{
	m_key = keyCode;
	emit keyCodeChanged();
	emit descriptionChanged();
}

void InputAction::setMouseX(int mouseX)
{
	m_xPos = mouseX;
	emit mouseXChanged();
	emit descriptionChanged();
}

void InputAction::setMouseY(int mouseY)
{
	m_yPos = mouseY;
	emit mouseYChanged();
	emit descriptionChanged();
}

void InputAction::setDelayedMsec(int delayedMsec)
{
	m_delayedMsec = delayedMsec;
	emit delayedMsecChanged();
	emit descriptionChanged();

	if(m_model)
	{
		emit m_model->totalTimeMsecChanged();
	}
}

bool InputAction::running(void)
{
	return m_running;
}

void InputAction::setRunning(bool running)
{
	m_running = running;
	emit runningChanged();
}

bool InputAction::keyRecording(void)
{
	return m_keyRecording;
}

void InputAction::setKeyRecording(bool keyRecording)
{
	m_keyRecording = keyRecording;
	if(keyRecording)
	{
		connect(Lazybones::inputEventManager(), SIGNAL(keyEvent(QKeyEvent*)), this, SLOT(onKeyEvent(QKeyEvent*)));
		Lazybones::inputEventManager()->startFiltering(InputEventManager::KeyboardHook);
	}
	else
	{
		disconnect(Lazybones::inputEventManager(), SIGNAL(keyEvent(QKeyEvent*)), this, SLOT(onKeyEvent(QKeyEvent*)));
	}
	emit keyRecordingChanged();
}

void InputAction::onTriggered(void)
{
	__perform();

	setRunning(false);
	if(m_doNextAction)
	{
		m_doNextAction = false;
		if(m_model)
		{
			m_model->doNextAction();
		}
	}
}

void InputAction::setModel(InputMethodListModel *model)
{
	m_model = model;
}

QSharedPointer<InputAction> InputAction::fromKeyEvent(const QKeyEvent *event, qint64 delayMsec)
{
	QSharedPointer<InputAction> inputAction =
			QSharedPointer<InputAction>(
				new InputAction(
					event->type() == QEvent::KeyPress ? InputAction::KeyPress : InputAction::KeyRelease,
					event->key(),
					-1,
					-1,
					delayMsec));
	return inputAction;
}

QSharedPointer<InputAction> InputAction::fromMouseEvent(const QMouseEvent *event, qint64 delayMsec)
{
	InputAction::ActionType type = InputAction::None;
	if(event->button() == Qt::LeftButton)
	{
		if(event->type() == QEvent::MouseButtonPress)
			type = InputAction::MouseLeftPress;
		if(event->type() == QEvent::MouseButtonRelease)
			type = InputAction::MouseLeftRelease;
	}
	else if(event->button() == Qt::RightButton)
	{
		if(event->type() == QEvent::MouseButtonPress)
			type = InputAction::MouseRightPress;
		if(event->type() == QEvent::MouseButtonRelease)
			type = InputAction::MouseRightRelease;
	}

	if(event->type() == QEvent::MouseMove)
	{
		if(event->type() == QEvent::MouseMove)
			type = InputAction::MouseMove;
	}

	if(type == InputAction::None)
		return QSharedPointer<InputAction>(); // this mouse event not supported

	QPoint pos = event->pos();

	QSharedPointer<InputAction> inputAction =
			QSharedPointer<InputAction>(
				new InputAction(
					type,
					0,
					pos.x(),
					pos.y(),
					delayMsec));
	return inputAction;
}

void InputAction::onKeyEvent(QKeyEvent *event)
{
	if(!keyRecording())
		return;

	setKeyCode(event->key());
	setKeyRecording(false);
}

