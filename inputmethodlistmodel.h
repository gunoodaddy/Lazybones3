#ifndef INPUTMETHODLISTMODEL_H
#define INPUTMETHODLISTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QSharedPointer>
#include "inputaction.h"

class InputMethodListModel : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	Q_PROPERTY(qint64 currentTimeMsec READ currentTimeMsec NOTIFY currentTimeMsecChanged)
	Q_PROPERTY(qint64 totalTimeMsec READ totalTimeMsec NOTIFY totalTimeMsecChanged)
public:
	explicit InputMethodListModel(void);

	void add(QSharedPointer<InputAction> action);
	void remove(int index);
	QSharedPointer<InputAction> find(InputAction *rawPointer);
	QSharedPointer<InputAction> at(int index) { return m_inputMethods.at(index); }
	Q_INVOKABLE void clear(void);

	qint64 currentTimeMsec(void);
	qint64 totalTimeMsec(void);
	int currentIndex(void) { return m_currentActionIndex; }
	void setCurrentIndex(int index);
	Q_INVOKABLE int count(void) { return m_inputMethods.size(); }
	void stop(void);
	void start(int fromIndex = -1);
	void doNextAction(void);
	bool running(void) { return m_running; }
	void setRepeat(bool repeat) { m_repeat = repeat; }

protected:
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QHash<int, QByteArray> roleNames() const;

signals:
	void currentIndexChanged(void);
	void currentTimeMsecChanged(void);
	void countChanged(void);
	void totalTimeMsecChanged(void);

public slots:

private:
	QVector<QSharedPointer<InputAction> > m_inputMethods;
	int m_currentActionIndex;

	bool m_running;
	bool m_repeat;

	qint64 m_currentTimeMsec;
};

#endif // INPUTMETHODLISTMODEL_H
