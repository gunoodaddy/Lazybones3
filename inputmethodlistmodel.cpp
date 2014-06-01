#include "inputmethodlistmodel.h"
#include <QQmlEngine>
#include "Lazybones.h"

InputMethodListModel::InputMethodListModel(void) :
	QAbstractListModel(NULL), m_currentActionIndex(-1), m_running(false), m_repeat(true), m_currentTimeMsec(0)
{
}

int InputMethodListModel::rowCount(const QModelIndex &/*parent*/) const
{
	return m_inputMethods.size();
}

QVariant InputMethodListModel::data(const QModelIndex &index, int /*role*/) const
{
	InputAction *action = m_inputMethods.at(index.row()).data();
	QQmlEngine::setObjectOwnership(action, QQmlEngine::CppOwnership);
	return qVariantFromValue<InputAction *>(action);
}

QHash<int, QByteArray> InputMethodListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole + 1] = "itemData";
	return roles;
}

void InputMethodListModel::add(QSharedPointer<InputAction> action)
{
	beginInsertRows(QModelIndex(), m_inputMethods.size(), m_inputMethods.size());
	m_inputMethods.push_back(action);
	endInsertRows();

	action->setModel(this);

	emit totalTimeMsecChanged();
	emit countChanged();
}

QSharedPointer<InputAction> InputMethodListModel::find(InputAction *rawPointer)
{
	for(int i = 0; i < m_inputMethods.size(); i++)
	{
		if(m_inputMethods[i].data() == rawPointer)
			return m_inputMethods[i];
	}
	return QSharedPointer<InputAction>();
}

void InputMethodListModel::remove(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	m_inputMethods.removeAt(index);
	endRemoveRows();

	emit countChanged();
	emit totalTimeMsecChanged();
}

void InputMethodListModel::stop(void)
{
	m_running = false;
}

void InputMethodListModel::start(int fromIndex)
{
	if(m_inputMethods.size() <= 0)
		return;

	Lazybones::setEmergencyStop(false);

	m_currentTimeMsec = 0;
	m_running = true;
	if(fromIndex < 0 || fromIndex >= m_inputMethods.size())
		m_currentActionIndex = -1;
	else
		m_currentActionIndex = fromIndex - 1;

	emit currentTimeMsecChanged();
	emit currentIndexChanged();

	doNextAction();
}

void InputMethodListModel::doNextAction()
{
	if(!m_running)
		return;

	if(Lazybones::isEmergencyStop())
	{
		stop();
		return;
	}

	int nextIndex = m_currentActionIndex + 1;

	if(nextIndex >= m_inputMethods.size())
	{
		if(!m_repeat)
		{
			stop();
			return;
		}
		nextIndex = 0;
	}

	setCurrentIndex(nextIndex);

	m_inputMethods[nextIndex]->doit();
}

void InputMethodListModel::clear(void)
{
	beginResetModel();
	setCurrentIndex(-1);
	m_inputMethods.clear();
	endResetModel();

	emit currentIndexChanged();
	emit totalTimeMsecChanged();
	emit countChanged();
}

qint64 InputMethodListModel::totalTimeMsec(void)
{
	qint64 totalTime = 0;
	for(int i = 0; i < m_inputMethods.size(); i++)
	{
		totalTime += m_inputMethods[i]->delayedMsec();
	}
	return totalTime;
}

qint64 InputMethodListModel::currentTimeMsec(void)
{
	return m_currentTimeMsec;
}

void InputMethodListModel::setCurrentIndex(int index)
{
	if(m_currentActionIndex == 0)
		m_currentTimeMsec = 0;
	else if(m_currentActionIndex >= 0 && m_currentActionIndex < m_inputMethods.size())
		m_currentTimeMsec += m_inputMethods[m_currentActionIndex]->delayedMsec();

	m_currentActionIndex = index;
	emit currentIndexChanged();
	emit currentTimeMsec();
}
