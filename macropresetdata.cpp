#include "MacroPresetData.h"
#include <QQmlEngine>
#include <QDateTime>
#include <QThread>
#include <QMessageBox>
#include <QStack>
#include "Lazybones.h"
#include "macropresetlistmodel.h"
#include "Logger.h"
#include "Defines.h"

#define kForceGapIntervalMsec 100
#define kHangupTimeout 45

class MacroThreadWorker : public QThread
{
public:
	MacroThreadWorker(MacroPresetData *owner) : QThread(), m_owner(owner), m_stop(false)
	{
	}

	void run() Q_DECL_OVERRIDE
	{
		int index = m_owner->m_currentIndex;

		int lastLoopStartIndex = -1;
		int lastLoopEndIndex = -1;
		QSharedPointer<MacroAction> action;
		MacroAction::ConditionType currBlockStartType;
		bool firstLoop = true;
		while(!m_stop)
		{
			bool needToJumpToEnd = false;

			//------------------------------------
			m_owner->lock();
			//------------------------------------

			if(firstLoop || index < 0 || index >= m_owner->m_model.count())
			{
				for(int i = 0; i < m_owner->m_model.count(); i++)
				{
					QSharedPointer<MacroAction> action = m_owner->m_model.at(i);
					action->init();
				}

				if(!firstLoop)
				{
					if(!m_owner->m_repeat)
					{
						m_stop = true;
						emit m_owner->runningChanged();
						break;
					}
					index = 0;	// go to first

					m_owner->processLoopEnd();
				}
				firstLoop = false;
				lastLoopStartIndex = -1;
				lastLoopEndIndex = -1;

				m_owner->processLoopStart();
			}
			m_owner->setCurrentIndex(index);

			if(action)
				action->setRunning(false);

			action = m_owner->m_model.at(index);
			action->setRunning(true);

			if(action->blockStartType())
			{
				currBlockStartType = (MacroAction::ConditionType)action->conditionType();
				lastLoopStartIndex = index;
			}
			else if(action->blockEndType())
			{
				lastLoopEndIndex = index;
			}

			//------------------------------------
			m_owner->unlock();
			//------------------------------------

			if(action->doit())
			{
				LOG_INFO() << "MACRO ACTION RUNNING => OK : " << index << " : " << action->description();

				// If Loop End, then go to Loop Start
				if(action->blockLoopEndType())
				{
					if(lastLoopStartIndex >= 0)
					{
						index = lastLoopStartIndex;	// go to start index
						continue;
					}
					else
					{
						qDebug() << "!Error unexpacted case.. not found start block!";
					}
				}

				if(action->conditionType() == MacroAction::Else)
				{
					needToJumpToEnd = true;
				}
			}
			else
			{
				// if it's not condition type, we do not need to find end block.
				// it always be ignored.
				if(action->blockIfType() || action->blockLoopStartType())
					needToJumpToEnd = true;

				LOG_INFO() << "MACRO ACTION RUNNING => FAIL : " << index << " : " << action->description() << ", JUMP :" << needToJumpToEnd;
			}

			// index can be changed by Goto statement. but it almost be same with current index.
			if(m_owner->currentIndex() != index)
			{
				index = m_owner->currentIndex();
				LOG_INFO() << "JUMP TO INDEX => " << index;
			}

			if(needToJumpToEnd)
			{
				// Not reached to end block, so we need to find this end block or else type.
				if(lastLoopEndIndex < 0 || index > lastLoopEndIndex)
				{
					index ++;

					for(; index < m_owner->m_model.count(); index++)
					{
						QSharedPointer<MacroAction> tempAction = m_owner->m_model.at(index);

						// if current action is if statement, then go to else statement.
						// if not found, then go to end statement
						if(MacroAction::checkBlockIfType(currBlockStartType))
						{
							// Find a else type
							if(tempAction->conditionType() == MacroAction::Else)
							{
								lastLoopEndIndex = index;
								break;
							}
						}

						// Find a end block (IfEnd, LoopEnd)
						if(tempAction->blockEndType())
						{
							lastLoopEndIndex = index;
							break;
						}
					}
				}

				// Jump to there
				if(lastLoopEndIndex >= 0)
				{
					index = lastLoopEndIndex + 1;	// go to the next of the end index
					continue;
				}
			}

			index++;
		}
	}

	MacroPresetData *m_owner;
	volatile bool m_stop;
};

MacroPresetData::MacroPresetData(MacroPresetListModel *model) : QObject(NULL)
  , m_mutex(QMutex::Recursive), m_threadWorker(NULL), m_presetModel(model)
  , m_model(this), m_currentIndex(0), m_repeat(true), m_maxIndex(-1), m_hangupSeconds(kHangupTimeout), m_remainHangUpSeconds(kHangupTimeout)
  , m_statsLoopCount(0), m_statsHangCount(0), m_statsLegendaryCount(0)
  , m_statsStartRunningTime(0)
{
	m_winRect.setRect(0, 0, 1024, 768);
	QQmlEngine::setObjectOwnership(&m_model, QQmlEngine::CppOwnership);

	m_hangupTimer.setInterval(1000);
	connect(&m_hangupTimer, SIGNAL(timeout()), this, SLOT(onHangupCheckTimer()));
}

MacroPresetData::~MacroPresetData(void)
{
	stop();
}

void MacroPresetData::read(QTextStream &in)
{
	while ( !in.atEnd() )
	{
		MacroAction *action = MacroAction::fromTextStream(in);
		if(action)
		{
			addAction(action);
			delete action;
		}
	}
}

void MacroPresetData::write(QTextStream &out)
{
	QStack<QString> tabStack;
	for(int i = 0; i < count(); i++)
	{
		QSharedPointer<MacroAction> macroAction = model()->at(i);

		if(macroAction->blockEndType() || macroAction->blockRestartType())
			tabStack.pop();

		QString currTab = !tabStack.isEmpty() ? tabStack.top() : "";

		macroAction->writeTextStream(currTab, out);

		if(macroAction->blockStartType() || macroAction->blockRestartType())
			tabStack.push(currTab + "\t");
	}
}

QString MacroPresetData::codeText(void)
{
	QString codeText;
	QTextStream out(&codeText, QIODevice::WriteOnly);
	write(out);
	return codeText;
}

void MacroPresetData::setCodeText(QString codeText)
{
	clear();
	QTextStream in(&codeText, QIODevice::ReadOnly);
	read(in);
}

void MacroPresetData::copy(QSharedPointer<MacroPresetData> presetData)
{
	setTitle(presetData->title());
	setWindowTitle(presetData->windowTitle());
	setWindowRect(presetData->windowRect());
	setHangUpSeconds(presetData->hangUpSeconds());

	m_model.copy(presetData->model());
}

void MacroPresetData::applyWindowRect(void)
{
	Lazybones::inputEventManager()->worker()->doWindowGeometry(m_winTitle, m_winRect);
}

int MacroPresetData::verify(void)
{
	int depth = -1;

	QStack<int> indexStack;
	for(int i = 0; i < m_model.count(); i++)
	{
		QSharedPointer<MacroAction> action = m_model.at(i);
		action->setRunning(false);

		if(action->blockStartType() || depth < 0)
			depth++;

		if(action->blockStartType())
		{
			indexStack.push(i);
		}

		if(action->blockEndType())
		{
			if(indexStack.size() <= 0)
				return i;

			indexStack.pop();
		}
	}

	if(indexStack.size() > 0)
		return indexStack.pop();
	return -1;
}

void MacroPresetData::insertAction(int index, MacroAction *newAction)
{
	QMutexLocker locker(&m_mutex);
	QSharedPointer<MacroAction> action =
			QSharedPointer<MacroAction>(
				new MacroAction(newAction));

	__insertAction(index, action);
}

void MacroPresetData::addAction(MacroAction *newAction)
{
	QMutexLocker locker(&m_mutex);
	QSharedPointer<MacroAction> action =
			QSharedPointer<MacroAction>(
				new MacroAction(newAction));

	__addAction(action);
}

void MacroPresetData::removeAction(int index)
{
	QMutexLocker locker(&m_mutex);
	m_model.remove(index);
}

void MacroPresetData::moveAction(int index, MacroAction *action)
{
	QMutexLocker locker(&m_mutex);

	int currIndex = m_model.indexOf(action);
	int newIndex = index;
	if(currIndex < 0)
		return;

	if(currIndex < index)
	{
		newIndex--;
	}

	if(newIndex < 0 || newIndex >= (m_model.count() - 1))	// 1 will be removed below
		return;

	QSharedPointer<MacroAction> currAction = m_model.at(currIndex);
	m_model.remove(currIndex);
	m_model.insert(newIndex, currAction);
}

void MacroPresetData::setTitle(const QString &title)
{
	m_title = title;
	emit titleChanged();
	updatePresetModel();
}

void MacroPresetData::updatePresetModel(void)
{
	QMutexLocker locker(&m_mutex);
	if(m_presetModel == NULL)
		return;

	m_presetModel->update(this);
}

bool MacroPresetData::running(void)
{
	QMutexLocker locker(&m_mutex);
	if(!m_threadWorker)
		return false;
	return !m_threadWorker->m_stop;
}

void MacroPresetData::setRunning(bool running)
{
	if(running)
	{
		int index = verify();
		if(index < 0)
		{
			start();
		}
		else
		{
			QString message = "The syntax for index [" + QString::number(index) + "] action is not valid.";
			QMessageBox::warning(NULL, "", message);
			return;	// failed
		}
	}
	else
	{
		stop();
	}
	emit runningChanged();
}

void MacroPresetData::start(void)
{
	QMutexLocker locker(&m_mutex);
	if(m_threadWorker)
		stop();

	m_maxIndex = -1;
	emit remainHangUpSecondsChanged();
	emit statsStartRunningTimeChanged();
	m_hangupTimer.start();

	m_threadWorker = new MacroThreadWorker(this);
	m_threadWorker->start();

	Lazybones::setEmergencyStop(false);
}

void MacroPresetData::stop(void)
{
	m_hangupTimer.stop();

	if(!m_threadWorker)
		return;

	m_threadWorker->m_stop = true;
	m_threadWorker->quit();
	m_threadWorker->wait();
	delete m_threadWorker;
	m_threadWorker = NULL;

	m_remainHangUpSeconds = m_hangupSeconds;
	emit remainHangUpSecondsChanged();
}

void MacroPresetData::clear(void)
{
	QMutexLocker locker(&m_mutex);
	m_model.clear();
	emit currentIndexChanged();
}

void MacroPresetData::__addAction(QSharedPointer<MacroAction> newAction)
{
	QMutexLocker locker(&m_mutex);
	m_model.add(newAction);
}

void MacroPresetData::__insertAction(int index, QSharedPointer<MacroAction> newAction)
{
	QMutexLocker locker(&m_mutex);
	m_model.insert(index, newAction);
}

void MacroPresetData::onHangupCheckTimer(void)
{
	if(m_maxIndex < 0)
		m_maxIndex = m_currentIndex;

	//qDebug() << m_currentIndex << m_maxIndex << m_remainHangUpSeconds;

	if(m_currentIndex <= m_maxIndex)
	{
		m_remainHangUpSeconds--;
	}
	else
	{
		m_remainHangUpSeconds = m_hangupSeconds;
		m_maxIndex = m_currentIndex;
	}

	emit remainHangUpSecondsChanged();
	emit statsCurrLoopTimeChanged();
	emit statsStartRunningTimeChanged();

	if(m_remainHangUpSeconds <= 0)
	{
		SETTING_MANAGER->dropBox()->doScreenShot("Hangup_", false, false);

		increaseHangCount();

		m_hangupTimer.stop();

		goToMainScreen();
	}
}

void MacroPresetData::goToMainScreen(void)
{
	LOG_INFO() << "HANGUP DETECTED :" << currentIndex();
	stop();

	int loopLimitCount = 10;

	// go to game menu
	while(!Lazybones::gameScreenManager()->isCurrentScreen(kScreenName_GameMenu, true))
	{
		Lazybones::inputEventManager()->worker()->doKeyboardInput(Qt::Key_Escape);
		QThread::msleep(400);
		if(Lazybones::isEmergencyStop() || --loopLimitCount <= 0)
			break;
	}

	if(Lazybones::isEmergencyStop())
		return;

	QThread::msleep(400);
	Lazybones::inputEventManager()->worker()->doMouseLeftClick(142, 297);

	// restart from 0 index
	LOG_INFO() << "RESTART";
	setCurrentIndex(0);
	start();
}

int MacroPresetData::statsStartRunningTime(void)
{
	if(running() == false)
		return 0;

	return QDateTime::currentMSecsSinceEpoch() - m_statsStartRunningTime;
}

int MacroPresetData::statsCurrLoopTime(void)
{
	if(running() == false)
		return 0;

	return QDateTime::currentMSecsSinceEpoch() - m_tempStartLoopTimeMsec;
}

int MacroPresetData::statsAvgLoopTime(void)
{
	if(statsLoopCount() <= 0)
		return 0;

	return m_statsTotalLoopTime / statsLoopCount();
}

void MacroPresetData::updateAvgLoopTime(int currTime)
{
	m_statsTotalLoopTime += currTime;
	emit statsAvgLoopTimeChanged();
}
