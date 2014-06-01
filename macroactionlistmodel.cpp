#include "macroactionlistmodel.h"
#include <QQmlEngine>
#include "Lazybones.h"
#include "macropresetdata.h"

MacroActionListModel::MacroActionListModel(MacroPresetData *preset) :
	QAbstractListModel(NULL),  m_preset(preset)
{
}

int MacroActionListModel::rowCount(const QModelIndex &/*parent*/) const
{
	return m_macroActionList.size();
}

QVariant MacroActionListModel::data(const QModelIndex &index, int /*role*/) const
{
	MacroAction *action = m_macroActionList.at(index.row()).data();
	QQmlEngine::setObjectOwnership(action, QQmlEngine::CppOwnership);
	return qVariantFromValue<MacroAction *>(action);
}

QHash<int, QByteArray> MacroActionListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole + 1] = "itemData";
	return roles;
}

void MacroActionListModel::insert(int index, QSharedPointer<MacroAction> action)
{
	beginInsertRows(QModelIndex(), index, index);
	m_macroActionList.insert(index, action);
	endInsertRows();

	emit m_preset->currentIndexChanged();

	action->setModel(this);
}

void MacroActionListModel::add(QSharedPointer<MacroAction> action)
{
	insert(m_macroActionList.size(), action);
}

int MacroActionListModel::indexOf(MacroAction *rawPointer)
{
	for(int i = 0; i < m_macroActionList.size(); i++)
	{
		if(m_macroActionList[i].data() == rawPointer)
			return i;
	}
	return -1;
}

QSharedPointer<MacroAction> MacroActionListModel::find(MacroAction *rawPointer)
{
	for(int i = 0; i < m_macroActionList.size(); i++)
	{
		if(m_macroActionList[i].data() == rawPointer)
			return m_macroActionList[i];
	}
	return QSharedPointer<MacroAction>();
}

void MacroActionListModel::remove(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	m_macroActionList.removeAt(index);
	endRemoveRows();

	emit m_preset->currentIndexChanged();
}

void MacroActionListModel::clear(void)
{
	beginResetModel();
	m_macroActionList.clear();
	endResetModel();
}

void MacroActionListModel::copy(MacroActionListModel *rhs)
{
	for(int i = 0; i < rhs->count(); i++)
	{
		QSharedPointer<MacroAction> action =
				QSharedPointer<MacroAction>(
					new MacroAction(rhs->at(i).data()));
		add(action);
	}
}
