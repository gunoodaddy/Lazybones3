#ifndef SIMPLESTRINGLISTMODEL_H
#define SIMPLESTRINGLISTMODEL_H

#include <QAbstractListModel>

class SimpleStringListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit SimpleStringListModel(QObject *parent = 0);

	void add(const QString &str);
	void remove(const QString &str);
	Q_INVOKABLE int indexOf(const QString &str);
	const QString & at(int i)
	{
		return m_list.at(i);
	}

	Q_INVOKABLE QVariant get(int i)
	{
		if(i < 0 || i >= m_list.size())
			return QVariant();

		return m_list.at(i);
	}

	Q_INVOKABLE void updateAll(void);
	Q_INVOKABLE int count(void) { return m_list.size(); }


protected:
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QHash<int, QByteArray> roleNames() const;

signals:

public slots:

private:
	QStringList m_list;
};

#endif // SIMPLESTRINGLISTMODEL_H
