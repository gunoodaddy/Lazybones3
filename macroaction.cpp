#include "macroaction.h"
#include "macroactionlistmodel.h"
#include "inputaction.h"
#include "Lazybones.h"
#include <QQmlEngine>
#include <QThread>
#include "Logger.h"
#include "Defines.h"

static QMap<MacroAction::ConditionType, QString> CONDITION_MAPPING;
static QMap<MacroAction::ActionType, QString> ACTION_MAPPING;

static MacroAction::ConditionType findConditionName(QString name)
{
	QMap<MacroAction::ConditionType, QString>::iterator it = CONDITION_MAPPING.begin();
	for(; it != CONDITION_MAPPING.end(); it++)
	{
		if(it.value() == name)
			return it.key();
	}
	return MacroAction::None;
}

static MacroAction::ActionType findActionName(QString name)
{
	QMap<MacroAction::ActionType, QString>::iterator it = ACTION_MAPPING.begin();
	for(; it != ACTION_MAPPING.end(); it++)
	{
		if(it.value() == name)
			return it.key();
	}
	return MacroAction::NoneAction;
}

MacroAction::MacroAction(MacroAction *action) : QObject(0)
  , m_model(NULL), m_running(false), m_recording(false), m_conditionType(MacroAction::None), m_actionType(MacroAction::NoneAction)
  , m_inputActionChunk()
  , m_sleepMsec(0)
  , m_macroIndex(-1)
  , m_active(false), m_currentSleepMsec(-1)
  , m_loopIndex(-1)
  , m_colorOffset(0)
  , m_prevDelayMsec(-1), m_startActionMsec(-1)
{
	copy(action);
	QQmlEngine::setObjectOwnership(&m_inputAction, QQmlEngine::CppOwnership);
	connect(&m_inputAction, SIGNAL(descriptionChanged()), this, SLOT(onInputActionDescriptionChanged()));

	m_inputActionChunk.setRepeat(false);
}

/* static */
void MacroAction::initialize(void)
{
	CONDITION_MAPPING[MacroAction::None] = "";
	CONDITION_MAPPING[MacroAction::LoopIfNotStart] = "LoopIfNotBegin";
	CONDITION_MAPPING[MacroAction::LoopIfStart] = "LoopIfBegin";
	CONDITION_MAPPING[MacroAction::LoopEnd] = "LoopEnd";
	CONDITION_MAPPING[MacroAction::IfStart] = "IfBegin";
	CONDITION_MAPPING[MacroAction::IfNotStart] = "IfNotBegin";
	CONDITION_MAPPING[MacroAction::IfEnd] = "IfEnd";
	CONDITION_MAPPING[MacroAction::Else] = "Else";

	ACTION_MAPPING[MacroAction::NoneAction] = "";
	ACTION_MAPPING[MacroAction::DoInputAction] = "";
	ACTION_MAPPING[MacroAction::DoInputActionChunk] = "";
	ACTION_MAPPING[MacroAction::WaitScreen] = "WaitScreen";
	ACTION_MAPPING[MacroAction::Sleep] = "Sleep";
	ACTION_MAPPING[MacroAction::RandomSleep] = "RandomSleep";
	ACTION_MAPPING[MacroAction::FindLegendaryItem] = "FindLegendaryItem";
	ACTION_MAPPING[MacroAction::Goto] = "Goto";
	ACTION_MAPPING[MacroAction::DisassembleAllItemWithoutLegendary] = "DisassembleAllItemWithoutLegendary";
	ACTION_MAPPING[MacroAction::LoopWaitScreenDoInputActionEnd] = "LoopWaitScreenDoInputActionEnd";
	ACTION_MAPPING[MacroAction::SaveScreenShot] = "SaveScreenShot";
	ACTION_MAPPING[MacroAction::CheckElapsedTime] = "CheckElapsedTime";
	ACTION_MAPPING[MacroAction::CheckDropBoxFileExist] = "CheckDropBoxFileExist";
	ACTION_MAPPING[MacroAction::CheckLoopIndex] = "CheckLoopIndex";
	ACTION_MAPPING[MacroAction::CheckLoopIndexModulus] = "CheckLoopIndexModulus";
	ACTION_MAPPING[MacroAction::CheckScreen] = "CheckScreen";
	ACTION_MAPPING[MacroAction::CheckPixelColor] = "CheckPixelColor";
}

void MacroAction::init(void)
{
	m_startActionMsec = -1;
	m_currentSleepMsec = -1;

	emit descriptionChanged();
}

static QString nextTokenString(const QString &line, int & fromIndex)
{
	int end = -1;

	if(fromIndex >= line.length())
		return "";

	if(line[fromIndex].isSpace())
	{
		for(int i = fromIndex; i < line.length(); i++)
		{
			if(!line[i].isSpace())
			{
				fromIndex = i;
				break;
			}
		}
		if(fromIndex >= line.length())
			return "";
	}

	for(int i = fromIndex; i < line.length(); i++)
	{
		if(line[i].isSpace() || line[i] == '(' || i >= line.length() - 1)
		{
			end = i;
			if(i >= line.length() - 1)
				end++;
			break;
		}
	}

	QString token = line.mid(fromIndex, end - fromIndex).trimmed();
	fromIndex = end;
	return token;
}


static QString getCommentString(const QString &line, int & fromIndex)
{
	if(fromIndex >= line.length())
		return "";

	int commentStartIndex = line.indexOf("//", fromIndex);
	if(commentStartIndex >= 0)
	{
		commentStartIndex += 2;
		if(commentStartIndex < line.length())
			return line.mid(commentStartIndex).trimmed();
	}
	return "";
}

void MacroAction::parseLine(MacroAction *newAction, MacroAction::State &state, int & parsedIndex, QString &line)
{
	QString token;
	switch(state)
	{
	case MacroAction::DoneState:
	case MacroAction::InitState:
		parsedIndex = 0;
		token = nextTokenString(line, parsedIndex);
		// go through
	case MacroAction::ConditionState:
	{
		MacroAction::ConditionType conditionType = findConditionName(token);
		newAction->setConditionType(conditionType);
		state = MacroAction::ActionState;
		if(conditionType != MacroAction::None)
		{
			if(newAction->isOnlyConditionType())
			{
				state = MacroAction::CommentState;
				break;
			}
			token = nextTokenString(line, parsedIndex);
		}
		// go through
	}
	case MacroAction::ActionState:
	{
		MacroAction::ActionType actionType = findActionName(token);
		newAction->setActionType(actionType);

		if(actionType == MacroAction::NoneAction)
		{
			if(token == "ChunkBegin")
			{
				state = MacroAction::InputActionChunkState;
				// comment parsing
				newAction->setActionType(MacroAction::DoInputActionChunk);
				newAction->setComment(getCommentString(line, parsedIndex));
				return;	// parse end
			}
			else
			{
				parsedIndex = 0;
				state = MacroAction::InputActionState;
			}
		}
		else if(actionType == MacroAction::LoopWaitScreenDoInputActionEnd)
		{
			state = MacroAction::InputActionState;
		}
		else
		{
			state = MacroAction::ArgumentState;
		}
		break;
	}
	case MacroAction::InputActionChunkState:
	case MacroAction::InputActionState:
	{
		if(state == MacroAction::InputActionChunkState)
		{
			if(line == "ChunkEnd")
			{
				state = MacroAction::DoneState;
				return;	// parse end
			}
		}

		InputAction *inputAction = InputAction::fromLineCode(parsedIndex <= 0 ? line : line.mid(parsedIndex).trimmed(), parsedIndex);
		if(inputAction != NULL)
		{
			if(state == MacroAction::InputActionState)
			{
				if(newAction->actionType() == MacroAction::NoneAction)
					newAction->setActionType(MacroAction::DoInputAction);
				newAction->inputAction()->copy(inputAction);
				// comment parsing
				newAction->setComment(getCommentString(line, parsedIndex));
				state = MacroAction::DoneState;
			}
			else
			{
				newAction->addInputActionChunk(inputAction);
			}
			delete inputAction;
		}
		else
		{
			state = MacroAction::ErrorState;
		}
		return;	// parse end
	}
	case MacroAction::ArgumentState:
	{
		int startBraceIndex = line.indexOf('(', parsedIndex);
		if(startBraceIndex < 0)
		{
			state = MacroAction::ErrorState;
			break;
		}

		int endBraceIndex = line.indexOf(')', startBraceIndex);
		if(endBraceIndex < 0)
		{
			state = MacroAction::ErrorState;
			break;
		}

		QString arguments = line.mid(startBraceIndex+1, (endBraceIndex - startBraceIndex - 1));
		if(arguments.length() > 0)
		{
			newAction->setArgumentString(arguments);
		}
		parsedIndex = endBraceIndex + 1;
		state = MacroAction::CommentState;
		break;
	}
	case MacroAction::CommentState:
	{
		newAction->setComment(getCommentString(line, parsedIndex));
		state = MacroAction::DoneState;
		return;	// parse end
	}
	case MacroAction::ErrorState:
	{
		LOG_ERROR() << "MacroAction parse failed :" << line;
		return;	// parse end
	}
	}

	parseLine(newAction, state, parsedIndex, line);
}

MacroAction *MacroAction::fromTextStream(QTextStream &ts)
{
	MacroAction::State state = MacroAction::InitState;
	int parsedIndex = 0;
	MacroAction *newAction = new MacroAction();
	while ( !ts.atEnd() )
	{
		QString line = ts.readLine().trimmed();
		parsedIndex = 0;
		parseLine(newAction, state, parsedIndex, line);

		if(state == MacroAction::DoneState)
		{
			return newAction;
		}
		else if(state == MacroAction::ErrorState)
		{
			break;
		}
	}

	return NULL;
}

void MacroAction::writeTextStream(QString prefix, QTextStream &ts)
{
	if(m_actionType == MacroAction::DoInputActionChunk)
	{
		// Write chunk start
		if(!prefix.isEmpty())
			ts << prefix;
		ts << "ChunkBegin";
		if(m_comment.length() > 0)
		{
			ts << "\t//";
			ts << m_comment;
		}
		ts << "\r\n";

		for(int i = 0; i < m_inputActionChunk.count(); i++)
		{
			QString actioncode = prefix;
			QSharedPointer<InputAction> action = m_inputActionChunk.at(i);
			actioncode += action->toLineCode();
			actioncode += "\r\n";
			ts << actioncode;
		}

		// Write chunk end
		if(!prefix.isEmpty())
			ts << prefix;
		ts << "ChunkEnd\r\n";

		//====================================
		// MUST EXIT!
		//====================================
		return;
	}

	QString code = toLineCode();

	QString finalLine;

	if(!prefix.isEmpty())
		finalLine += prefix;

	finalLine += code.trimmed();

	if(m_comment.length() > 0)
	{
		finalLine += "\t//";
		finalLine += m_comment;
	}

	finalLine += "\r\n";

	ts << finalLine;

	//qDebug() << finalLine;
}


QString MacroAction::toLineCode(void)
{
	QString code;

	if(m_actionType == LoopWaitScreenDoInputActionEnd || m_actionType == MacroAction::DoInputAction)
	{
		if(m_actionType == LoopWaitScreenDoInputActionEnd)
		{
			code += ACTION_MAPPING[m_actionType];
			code += " ";
		}

		code += m_inputAction.toLineCode();
	}
	else if(m_actionType == MacroAction::DoInputActionChunk)
	{
		return "DoInputActionChunk";
	}
	else
	{
		// Syntax : [Condition] + <ActionName> + "(" + <Arguments> + ")" + "// Comment"

		code += CONDITION_MAPPING[m_conditionType];

		if(!isOnlyConditionType())
		{
			if(code.length() > 0) code += " ";
			code += ACTION_MAPPING[m_actionType];

			code += "(";
			code += argumentString();
			code += ")";
		}
	}

	return code;
}


void MacroAction::setArgumentString(QString arguments)
{
	QStringList argTokens = arguments.split(",", QString::SkipEmptyParts);
	if(argTokens.size() >= 1)
	{
		switch(m_actionType)
		{
		case MacroAction::CheckPixelColor:
			m_color = argTokens[0];
			if(argTokens.size() > 1) m_colorOffset = argTokens[1].toInt();
			if(argTokens.size() > 2) m_colorSearchRect.setX(argTokens[2].toInt());
			if(argTokens.size() > 3) m_colorSearchRect.setY(argTokens[3].toInt());
			if(argTokens.size() > 4) m_colorSearchRect.setWidth(argTokens[4].toInt());
			if(argTokens.size() > 5) m_colorSearchRect.setHeight(argTokens[5].toInt());
			break;
		case MacroAction::LoopWaitScreenDoInputActionEnd:
		case MacroAction::CheckScreen:
		case MacroAction::WaitScreen:
			m_conditionScreenName = argTokens[0];
			break;
		case MacroAction::CheckElapsedTime:
		case MacroAction::RandomSleep:
		case MacroAction::Sleep:
			m_sleepMsec = argTokens[0].toInt();
			break;
		case MacroAction::Goto:
			m_macroIndex = argTokens[0].toInt();
			break;
		case MacroAction::CheckLoopIndex:
		case MacroAction::CheckLoopIndexModulus:
			m_loopIndex = argTokens[0].toInt();
			break;
		case MacroAction::CheckDropBoxFileExist:
		case MacroAction::SaveScreenShot:
			m_filePath = argTokens[0];
			break;
		default:
			break;
		}
	}
}

QString MacroAction::argumentString(void)
{
	switch(m_actionType)
	{
	case MacroAction::CheckPixelColor:
	{
		return m_color.name()
				+ ", " + QString::number(m_colorOffset)
				+ ", " + QString::number(m_colorSearchRect.x())
				+ ", " + QString::number(m_colorSearchRect.y())
				+ ", " + QString::number(m_colorSearchRect.width())
				+ ", " + QString::number(m_colorSearchRect.height());
	}
	case MacroAction::LoopWaitScreenDoInputActionEnd:
	case MacroAction::CheckScreen:
	case MacroAction::WaitScreen:
		return m_conditionScreenName;
	case MacroAction::CheckElapsedTime:
	case MacroAction::Sleep:
	case MacroAction::RandomSleep:
		return QString::number(m_sleepMsec);
	case MacroAction::Goto:
		return QString::number(m_macroIndex);
	case MacroAction::CheckLoopIndex:
	case MacroAction::CheckLoopIndexModulus:
		return QString::number(m_loopIndex);
	case MacroAction::CheckDropBoxFileExist:
	case MacroAction::SaveScreenShot:
		return m_filePath;
	default:
		return "";
	}
}

void MacroAction::copy(MacroAction *action)
{
	if(action == NULL)
		return;
	m_actionType = (MacroAction::ActionType)action->actionType();
	m_conditionType = (MacroAction::ConditionType)action->conditionType();
	m_comment = action->comment();
	m_conditionScreenName = action->conditionScreen();
	m_inputAction.copy(action->inputAction());
	m_sleepMsec = action->sleepMsec();
	m_macroIndex = action->macroIndex();
	m_loopIndex = action->loopIndex();
	m_filePath = action->filePath();
	m_color = action->color();
	m_colorOffset = action->colorOffset();
	m_colorSearchRect = action->colorSearchRect();
	for(int i = 0; i < action->inputActionChunk()->count(); i++)
	{
		addInputActionChunk(action->inputActionChunk()->at(i).data());
	}
}

void MacroAction::setActive(bool active)
{
	if(active)
	{
		connect(Lazybones::inputEventManager(), SIGNAL(keyEvent(QKeyEvent*)), this, SLOT(onKeyEvent(QKeyEvent*)));
		connect(Lazybones::inputEventManager(), SIGNAL(mouseEvent(QMouseEvent*)), this, SLOT(onMouseEvent(QMouseEvent*)));
	}
	else
	{
		disconnect(Lazybones::inputEventManager(), SIGNAL(keyEvent(QKeyEvent*)), this, SLOT(onKeyEvent(QKeyEvent*)));
		disconnect(Lazybones::inputEventManager(), SIGNAL(mouseEvent(QMouseEvent*)), this, SLOT(onMouseEvent(QMouseEvent*)));
	}
	m_active = active;
	emit activeChanged();
}

MacroPresetData *MacroAction::presetData(void)
{
	if(m_model)
		return m_model->presetData();
	return NULL;
}

int MacroAction::realSleepTime(void)
{
	if(m_currentSleepMsec > 0)
		return m_currentSleepMsec;

	if(m_actionType == MacroAction::RandomSleep)
		return Lazybones::randInt(m_sleepMsec, m_sleepMsec * 2);
	return m_sleepMsec;
}

bool MacroAction::doit()
{
	if(blockEndType()
			|| m_conditionType == MacroAction::Else
			|| m_actionType == MacroAction::NoneAction)
	{
		return true;
	}

	bool res = false;
	switch(m_actionType)
	{
	case MacroAction::NoneAction:
		return true;
	case MacroAction::DoInputAction:
		doInputAction();
		res = true;
		break;
	case MacroAction::DoInputActionChunk:
	{
		doInputActionChunk();
		res = true;
		break;
	}
	case MacroAction::RandomSleep:
	case MacroAction::Sleep:
		QThread::msleep(realSleepTime());
		res = true;
		break;
	case MacroAction::CheckElapsedTime:
		if(m_startActionMsec < 0)
		{
			m_startActionMsec = QDateTime::currentMSecsSinceEpoch();
		}
		if(QDateTime::currentMSecsSinceEpoch() - m_startActionMsec >= m_sleepMsec )
		{
			// time over
			m_startActionMsec = -1; // init
			res = true;
		}
		else
		{
			// not yet..
			res = false;
		}
		break;
	case MacroAction::CheckPixelColor:
	{
		QRect rect = m_colorSearchRect;
		if(rect.isNull() || rect.isEmpty() || !rect.isValid())
			rect = presetData()->windowRect();

		QPoint lastPoint = Lazybones::gameScreenManager()->findPixel(m_color.name()
															   , m_colorOffset, m_colorOffset, m_colorOffset
															   , rect);

		if(lastPoint.isNull() == false)
		{
			Lazybones::setRegisteredLastMousePos(lastPoint);
			return true;
		}

		return false;
	}
	case MacroAction::CheckScreen:
		res = Lazybones::gameScreenManager()->isCurrentScreen(m_conditionScreenName, true);
		break;
	case MacroAction::WaitScreen:
	{
		qint64 prev = QDateTime::currentMSecsSinceEpoch();
		while(presetData()->running())
		{
			if(Lazybones::gameScreenManager()->isCurrentScreen(m_conditionScreenName, true))
			{
				res = true;
				break;
			}

			if(m_sleepMsec > 0 && ((QDateTime::currentMSecsSinceEpoch() - prev) > m_sleepMsec))
			{
				break;
			}
			QThread::msleep(100);
		}
		break;
	}
	case MacroAction::Goto:
		presetData()->setCurrentIndex(m_macroIndex);
		res = true;
		break;
	case MacroAction::FindLegendaryItem:
		doFindLegendaryItem();
		res = true;
		break;
	case MacroAction::DisassembleAllItemWithoutLegendary:
		doDisassembleAllItemWithoutLegendary();
		res = true;
		break;
	case MacroAction::LoopWaitScreenDoInputActionEnd:
		doLoopWaitScreenDoInputActionEnd();
		res = true;
		break;
	case MacroAction::CheckDropBoxFileExist:
	{
		res = SETTING_MANAGER->dropBox()->findFile(m_filePath);
		LOG_INFO() << "DROP BOX FILE SEARCH :" << m_filePath << " :" << res;
		break;
	}
	case MacroAction::CheckLoopIndex:
		res = presetData()->statsLoopCount() == m_loopIndex;
		break;
	case MacroAction::CheckLoopIndexModulus:
		if(presetData()->statsLoopCount() <= 0)
			return false;
		res = (presetData()->statsLoopCount() % m_loopIndex) == 0 ? true : false;
		break;
	case MacroAction::SaveScreenShot:
		doSaveScreenShot();
		res = true;
		break;
	}

	if(negativeConditionType())
		res = !res;

	return res;
}

void MacroAction::setRunning(bool running)
{
	m_running = running;
	emit runningChanged();
}

QString MacroAction::description(void)
{
	QString desc;

	if(m_actionType == MacroAction::DoInputActionChunk)
	{
		desc = "DoInputActionChunk";
		desc += (m_inputActionChunk.count() > 0 ? "Time : " + Lazybones::toMMss(m_inputActionChunk.totalTimeMsec()) : "EMPTY");
	}
	else
	{
		desc = toLineCode();
	}

	return desc;
}

void MacroAction::addInputActionChunk(InputAction* newAction)
{
	QSharedPointer<InputAction> inputAction =
			QSharedPointer<InputAction>(
				new InputAction(
					(InputAction::ActionType)newAction->actionType(),
					newAction->keyCode(),
					newAction->mouseX(),
					newAction->mouseY(),
					newAction->delayedMsec()));

	m_inputActionChunk.add(inputAction);
}

void MacroAction::setRecording(bool recording)
{
	m_recording = recording;
	m_prevDelayMsec = QDateTime::currentMSecsSinceEpoch();
	emit recordingChanged();
	if(recording)
		Lazybones::inputEventManager()->startFiltering(InputEventManager::AllHook);
	else
		Lazybones::inputEventManager()->stopFiltering(InputEventManager::MouseHook);
}

void MacroAction::test(void)
{
	switch(m_actionType)
	{
	case MacroAction::DoInputActionChunk:
		doInputActionChunk(true);
		break;
	case MacroAction::DoInputAction:
		doInputAction(true);
		break;
	case MacroAction::FindLegendaryItem:
		doFindLegendaryItem();
		break;
	case MacroAction::DisassembleAllItemWithoutLegendary:
		doDisassembleAllItemWithoutLegendary(true);
		break;
	case MacroAction::LoopWaitScreenDoInputActionEnd:
		doLoopWaitScreenDoInputActionEnd(true);
		break;
	case MacroAction::SaveScreenShot:
		doSaveScreenShot();
		break;
	default:
		break;
	}
}

qint64 MacroAction::__calculateDelayMsec(qint64 delayMsec)
{
	if(delayMsec > 0)
	{
		m_prevDelayMsec = QDateTime::currentMSecsSinceEpoch();
		return delayMsec;
	}

	qint64 curr = QDateTime::currentMSecsSinceEpoch();

	delayMsec = curr - m_prevDelayMsec;
	m_prevDelayMsec = curr;
	return delayMsec;
}

void MacroAction::doInputAction(bool testMode)
{
	if(testMode)
	{
		m_inputAction.test();
		return;
	}
	if(m_inputAction.delayedMsec() > 0)
		QThread::msleep(m_inputAction.delayedMsec());
	m_inputAction.doitNow();
}

void MacroAction::doInputActionChunk(bool testMode)
{
	if(testMode)
	{
		m_startActionMsec = -1;
		m_inputActionChunk.setRepeat(false);
		m_inputActionChunk.start();
		return;
	}

	for(int i = 0; i < m_inputActionChunk.count(); i++)
	{
		if((!testMode && !presetData()->running()) || Lazybones::isEmergencyStop())
			break;

		m_inputActionChunk.setCurrentIndex(i);
		emit m_inputActionChunk.currentIndexChanged();

		QSharedPointer<InputAction> action = m_inputActionChunk.at(i);
		if(action->delayedMsec() > 0)
			QThread::msleep(action->delayedMsec());
		action->doitNow();
	}
}

void MacroAction::doLoopWaitScreenDoInputActionEnd(bool testMode)
{
	while((testMode || presetData()->running()) && !Lazybones::isEmergencyStop())
	{
		if(Lazybones::gameScreenManager()->isCurrentScreen(m_conditionScreenName, true))
			return;

		doInputAction();

		QThread::msleep(800);
	}
}

void MacroAction::doDisassembleAllItemWithoutLegendary(bool testMode)
{
	if(!Lazybones::gameScreenManager()->isCurrentScreen(kScreenName_BlackSmith, true))
		return;

	QSharedPointer<GameScreenData> topLeft = Lazybones::gameScreenManager()->findScreenData(kScreenName_InventoryTopLeft);
	if(!topLeft)
		return;
	QSharedPointer<GameScreenData> bottomRight = Lazybones::gameScreenManager()->findScreenData(kScreenName_InventoryBottomRight);
	if(!bottomRight)
		return;

	int stepW = (bottomRight->pixelX() - topLeft->pixelX()) / 10;
	int stepH = (bottomRight->pixelY() - topLeft->pixelY()) / 6;
	int x = topLeft->pixelX() + (stepW / 2);
	int y = topLeft->pixelY() + (stepH / 2);

	Lazybones::inputEventManager()->worker()->doMouseLeftClick(158, 194);
	QThread::msleep(300);

	for(int i = 0; i < 6; i++)
	{
		if(!(testMode || presetData()->running()) && !Lazybones::isEmergencyStop())
			break;

		for(int j = 0; j < 10; j++)
		{
			if(!(testMode || presetData()->running()) && !Lazybones::isEmergencyStop())
				break;

			int xx = x + (j * stepW);
			int yy = y + (i * stepH);
			QThread::msleep(50);
			Lazybones::inputEventManager()->worker()->doMouseMove(xx, yy);
			QThread::msleep(150);
			Lazybones::inputEventManager()->worker()->doMouseLeftClick(xx, yy);
			QThread::msleep(130);

			if(Lazybones::gameScreenManager()->isCurrentScreen(kScreenName_Warning, true))
			{
				Lazybones::inputEventManager()->worker()->doMouseLeftClick(502, 239);
				QThread::msleep(150);
				Lazybones::inputEventManager()->worker()->doMouseLeftClick(158, 194);
				QThread::msleep(150);
			}
		}
	}
}

void MacroAction::doFindLegendaryItem(void)
{
	QRect windowRect = presetData()->windowRect();
	int winW = windowRect.width();
	int winH = windowRect.height();

	Lazybones::inputEventManager()->worker()->doKeyboardInput(Qt::Key_Alt);
	QThread::msleep(300);

	SETTING_MANAGER->dropBox()->doScreenShot("BossKill_", false, false);
	QThread::msleep(300);

	for(int i = 0; i < 4; i++)
	{
		int checked = 0;
		QPoint pos = Lazybones::gameScreenManager()->findPixel(kLegendaryItemColor
															   , kDefaultLegendaryColorOffset, kDefaultLegendaryColorOffset, kDefaultLegendaryColorOffset
															   , QRect(QPoint(windowRect.x() + winW/2, windowRect.y() + winH/2), QSize(winW, winH)));
		LOG_INFO() << "FIND ITEM LEGENDARY RESULT : " << pos.x() << "x" << pos.y();
		if(!pos.isNull())
		{
			Lazybones::inputEventManager()->worker()->doMouseLeftClick(pos.x(), pos.y());
			presetData()->increaseLegendaryCount();
			SETTING_MANAGER->dropBox()->doScreenShot("Legendary_", false, false);
			QThread::msleep(1000);
		}
		else
			checked++;

		QThread::msleep(100);

		pos = Lazybones::gameScreenManager()->findPixel(kSetItemColor
														, kDefaultSetColorOffset, kDefaultSetColorOffset, kDefaultSetColorOffset
														, QRect(QPoint(winW/2, winH/2), QSize(winW, winH)));
		LOG_INFO() << "FIND ITEM SET RESULT : " << pos.x() << "x" << pos.y();
		if(!pos.isNull())
		{
			Lazybones::inputEventManager()->worker()->doMouseLeftClick(pos.x(), pos.y());
			presetData()->increaseLegendaryCount();
			SETTING_MANAGER->dropBox()->doScreenShot("Legendary_", false, false);
			QThread::msleep(1000);
		}
		else
			checked++;

		QThread::msleep(100);

		if(checked == 2)
			break;
	}
}

void MacroAction::doSaveScreenShot(void)
{
	SETTING_MANAGER->dropBox()->doScreenShotWithFilePath(m_filePath);
}

void MacroAction::onKeyEvent(QKeyEvent *event)
{
	if(recording())
	{
		if(event->key() == Qt::Key_F8 || event->key() == Qt::Key_F10 || event->key() == Qt::Key_F12 || event->key() == Qt::Key_Pause)
		{
			setRecording(false);
			qDebug() << "EMERGENCY RECORD STOP";
		}
		else
		{
			//qDebug() << "Add KeyEvent" << event->key() << recording();

			QSharedPointer<InputAction> inputAction = InputAction::fromKeyEvent(event, __calculateDelayMsec(-1));
			m_inputActionChunk.add(inputAction);
		}
	}
	else
	{
		if(event->key() == Qt::Key_F7)
		{
			setRecording(true);
			event->setAccepted(true);
			qDebug() << "EMERGENCY RECORD START";
		}
	}
}

void MacroAction::onMouseEvent(QMouseEvent *event)
{
	if(!recording())
		return;
	//qDebug() << "MouseEvent" << m_actionType<< event->pos() << event->button();
	QSharedPointer<InputAction> inputAction = InputAction::fromMouseEvent(event, __calculateDelayMsec(-1));
	m_inputActionChunk.add(inputAction);
}

