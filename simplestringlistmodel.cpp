#include "simplestringlistmodel.h"

SimpleStringListModel::SimpleStringListModel(QObject *parent) :
	QAbstractListModel(parent)
{
}

int SimpleStringListModel::indexOf(const QString &str)
{
	int i = 0;
	for(; i < m_list.size(); i++)
	{
		if(m_list[i] == str)
		{
			return i;
		}
	}
	return -1;
}

void SimpleStringListModel::add(const QString &str)
{
	beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
	m_list.push_back(str);
	endInsertRows();
}

void SimpleStringListModel::remove(const QString &str)
{
	while(true)
	{
		int i = 0;
		for(; i < m_list.size(); i++)
		{
			if(m_list[i] == str)
			{
				beginRemoveRows(QModelIndex(), i, i);
				m_list.removeAt(i);
				endInsertRows();
				break;
			}
		}
		if(i == m_list.size())
			break;
	}
}

int SimpleStringListModel::rowCount(const QModelIndex &/*parent*/) const
{
	return m_list.size();
}

QVariant SimpleStringListModel::data(const QModelIndex &index, int /*role*/) const
{
	return m_list.at(index.row());
}

QHash<int, QByteArray> SimpleStringListModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Qt::UserRole + 1] = "modelData";
	return roles;
}

void SimpleStringListModel::updateAll(void)
{
	int i = 0;
	for(; i < m_list.size(); i++)
	{
		QModelIndex index = createIndex(i, 0);
		emit dataChanged(index, index);
	}
}
