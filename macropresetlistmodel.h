#ifndef MACROMacroPresetListModel_H
#define MACROMacroPresetListModel_H

#include <QAbstractListModel>
#include <QSharedPointer>
#include "macropresetdata.h"

class MacroPresetListModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit MacroPresetListModel(QObject *parent = 0);

	void add(QSharedPointer<MacroPresetData> presetData);
	void removeAt(int index);
	void deleteAt(int index);
	void copy(int index, QSharedPointer<MacroPresetData> presetData);
	QSharedPointer<MacroPresetData> find(QString title);

	void update(MacroPresetData *data);
	QSharedPointer<MacroPresetData> at(int index) { return m_model.at(index); }
	void resetModel(void);
	void clear(void);
	int count(void) { return m_model.size(); }

protected:
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QHash<int, QByteArray> roleNames() const;

signals:

public slots:

private:
	QVector<QSharedPointer<MacroPresetData> > m_model;
};
#endif // MACROMacroPresetListModel_H
