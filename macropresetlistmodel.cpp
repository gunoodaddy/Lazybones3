#include "macropresetlistmodel.h"
#include "settingmanager.h"

MacroPresetListModel::MacroPresetListModel(QObject *parent) :
	QAbstractListModel(parent)
{
}

int MacroPresetListModel::rowCount(const QModelIndex &/*parent*/) const
{
	return m_model.size();
}

QVariant MacroPresetListModel::data(const QModelIndex &index, int /*role*/) const
{
	QSharedPointer<MacroPresetData> presetData = m_model.at(index.row());
	return presetData->title();
}

QHash<int, QByteArray> MacroPresetListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole + 1] = "modelData";
	return roles;
}

QSharedPointer<MacroPresetData> MacroPresetListModel::find(QString title, int exceptIndex)
{
	for(int i = 0; i < m_model.size(); i++)
	{
		if(i == exceptIndex)
			continue;

		if(m_model[i]->title() == title)
		{
			return m_model[i];
		}
	}
	return QSharedPointer<MacroPresetData>();
}

void MacroPresetListModel::add(QSharedPointer<MacroPresetData> presetData)
{
	beginInsertRows(QModelIndex(), m_model.size(), m_model.size());
	m_model.push_back(presetData);

	presetData->setMacroPresetListModel(this);
	endInsertRows();
}

void MacroPresetListModel::removeAt(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	m_model.removeAt(index);
	endRemoveRows();
}

void MacroPresetListModel::deleteAt(int index)
{
	QSharedPointer<MacroPresetData> presetData = m_model.at(index);
	SETTING_MANAGER->deleteMacroPreset(presetData);
	removeAt(index);
}

void MacroPresetListModel::copy(int index, QSharedPointer<MacroPresetData> presetData)
{
	QSharedPointer<MacroPresetData> sourceData = at(index);
	presetData->copy(sourceData);
	add(presetData);
}

void MacroPresetListModel::update(MacroPresetData *data)
{
	for(int i = 0; i < m_model.size(); i++)
	{
		if(m_model[i].data() == data)
		{
			QModelIndex modelIndex = createIndex(i, 0);
			emit dataChanged(modelIndex, modelIndex);
			break;
		}
	}
}

void MacroPresetListModel::resetModel(void)
{
	beginResetModel();
	endResetModel();
}

void MacroPresetListModel::clear(void)
{
	beginResetModel();
	m_model.clear();
	endResetModel();
}
