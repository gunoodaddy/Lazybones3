#ifndef ABSTRASCTINPUTEVENTWORKER_H
#define ABSTRASCTINPUTEVENTWORKER_H

#include <QtGlobal>
#include <QRect>
#include <qnamespace.h>

class AbstractInputEventWorker
{
public:
	virtual void startFiltering(int hookBits) = 0;
	virtual void stopFiltering(int hookBits) = 0;

	virtual void doWindowGeometry(const QString &title, const QRect &geometry) = 0;
	virtual void doMouseLeftClick(int x, int y) = 0;
	virtual void doMouseLeftPress(int x, int y) = 0;
	virtual void doMouseLeftRelease(int x, int y) = 0;
	virtual void doMouseRightClick(int x, int y) = 0;
	virtual void doMouseRightPress(int x, int y) = 0;
	virtual void doMouseRightRelease(int x, int y) = 0;
	virtual void doMouseMove(int x, int y) = 0;
    virtual void doKeyboardInput(Qt::Key keyCode) = 0;
    virtual void doKeyboardPress(Qt::Key keyCode) = 0;
    virtual void doKeyboardRelease(Qt::Key keyCode) = 0;
};

#endif // ABSTRASCTINPUTEVENTWORKER_H
