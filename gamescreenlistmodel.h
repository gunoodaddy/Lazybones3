#ifndef GAMESCREENLISTMODEL_H
#define GAMESCREENLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QSharedPointer>
#include "gamescreendata.h"

class GameScreenListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit GameScreenListModel(void);

	void add(QSharedPointer<GameScreenData> action, int insertIndex = -1);
	void remove(int index);
	int firstIndexOfName(const QString &name);
	QSharedPointer<GameScreenData> find(const QString &name);
	QSharedPointer<GameScreenData> find(GameScreenData *rawPointer, bool withoutColor = false);
	QSharedPointer<GameScreenData> at(int index) { return m_list.at(index); }
	void clear(void);

	int count(void) { return m_list.size(); }

protected:
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QHash<int, QByteArray> roleNames() const;

signals:

public slots:

private:
	QVector<QSharedPointer<GameScreenData> > m_list;
};

#endif // GAMESCREENLISTMODEL_H
