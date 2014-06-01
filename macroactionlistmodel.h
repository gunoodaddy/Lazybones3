#ifndef MACROACTIONLISTMODEL_H
#define MACROACTIONLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QSharedPointer>
#include "macroaction.h"

class MacroPresetData;

class MacroActionListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit MacroActionListModel(MacroPresetData *preset);

	void insert(int index, QSharedPointer<MacroAction> action);
	void add(QSharedPointer<MacroAction> action);
	void remove(int index);
	int indexOf(MacroAction *rawPointer);
	QSharedPointer<MacroAction> find(MacroAction *rawPointer);
	QSharedPointer<MacroAction> at(int index) { return m_macroActionList.at(index); }
	void clear(void);
	Q_INVOKABLE int count(void) { return m_macroActionList.size(); }
	void copy(MacroActionListModel *rhs);
	MacroPresetData *presetData(void) { return m_preset; }

protected:
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QHash<int, QByteArray> roleNames() const;

signals:

public slots:

private:
	QVector<QSharedPointer<MacroAction> > m_macroActionList;
	MacroPresetData *m_preset;
};

#endif // MACROACTIONLISTMODEL_H
