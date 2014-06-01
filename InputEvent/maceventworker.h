#ifndef MACEVENTMANAGER_H
#define MACEVENTMANAGER_H

#include <QSharedPointer>
#include <QAbstractNativeEventFilter>
#include "AbstractInputEventWorker.h"

#ifdef Q_OS_MAC

class InputEventManager;
class MacEventFilter;

class MacEventManager : public AbstractInputEventWorker
{
public:
	explicit MacEventManager(InputEventManager *manager);
	virtual ~MacEventManager(void);

	virtual void startFiltering(int hookBits);
	virtual void stopFiltering(int hookBits);

	virtual void doWindowGeometry(const QString &title, const QRect &geometry);
	virtual void doMouseLeftClick(int x, int y);
	virtual void doMouseLeftPress(int x, int y);
	virtual void doMouseLeftRelease(int x, int y);
	virtual void doMouseRightClick(int x, int y);
	virtual void doMouseRightPress(int x, int y);
	virtual void doMouseRightRelease(int x, int y);
	virtual void doMouseMove(int x, int y);
    virtual void doKeyboardPress(Qt::Key keyCode);
    virtual void doKeyboardRelease(Qt::Key keyCode);
    virtual void doKeyboardInput(Qt::Key keyCode);

private:
	friend class MacEventFilter;
	InputEventManager *m_manager;
	QSharedPointer<QAbstractNativeEventFilter> m_filter;
	void * m_monitorId;
	int m_hookBitsEnabled;
};

#endif
#endif // MACEVENTMANAGER_H
