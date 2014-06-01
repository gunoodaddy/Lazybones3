#include "gamescreenlistmodel.h"
#include <QQmlEngine>
#include <Qdebug>

GameScreenListModel::GameScreenListModel(void) :
	QAbstractListModel(NULL)
{
	QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

int GameScreenListModel::rowCount(const QModelIndex &/*parent*/) const
{
	return m_list.size();
}

QVariant GameScreenListModel::data(const QModelIndex &index, int /*role*/) const
{
	GameScreenData *data = m_list.at(index.row()).data();
	QQmlEngine::setObjectOwnership(data, QQmlEngine::CppOwnership);
	return qVariantFromValue<GameScreenData *>(data);
}

QHash<int, QByteArray> GameScreenListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole + 1] = "itemData";
	return roles;
}

void GameScreenListModel::add(QSharedPointer<GameScreenData> data, int insertIndex)
{
	int idx = insertIndex;
	if(idx < 0)
		idx = m_list.size();

	beginInsertRows(QModelIndex(), idx, idx);
	m_list.insert(idx, data);
	endInsertRows();
	//qDebug() << "Screen added : idx =" << idx << ", count =" << m_list.size();
}

int GameScreenListModel::firstIndexOfName(const QString &name)
{
	for(int i = 0; i < m_list.size(); i++)
	{
		if(m_list[i]->name() == name)
			return i;
	}
	return -1;
}

QSharedPointer<GameScreenData> GameScreenListModel::find(const QString &name)
{
	for(int i = 0; i < m_list.size(); i++)
	{
		if(m_list[i]->name() == name)
			return m_list[i];
	}
	return QSharedPointer<GameScreenData>();
}

QSharedPointer<GameScreenData> GameScreenListModel::find(GameScreenData *rawPointer, bool withoutColor)
{
	for(int i = 0; i < m_list.size(); i++)
	{
		if(m_list[i].data() == rawPointer)
			return m_list[i];

		if(withoutColor)
		{
			if(m_list[i]->equalsWithoutColor(rawPointer))
				return m_list[i];
		}
		else
		{
			if(m_list[i]->equals(rawPointer))
				return m_list[i];
		}
	}
	return QSharedPointer<GameScreenData>();
}

void GameScreenListModel::remove(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	m_list.removeAt(index);
	endRemoveRows();
}

void GameScreenListModel::clear(void)
{
	beginResetModel();
	m_list.clear();
	endResetModel();
}
