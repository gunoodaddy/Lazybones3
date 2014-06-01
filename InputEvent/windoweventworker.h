#ifndef WINDOWEVENTMANAGER_H
#define WINDOWEVENTMANAGER_H

#include "AbstractInputEventWorker.h"

#ifdef Q_OS_WIN32
#include <Windows.h>

class InputEventManager;

class WindowEventManager : public AbstractInputEventWorker
{
public:
	explicit WindowEventManager(InputEventManager *manager);
	virtual ~WindowEventManager(void);

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
	virtual void doKeyboardInput(Qt::Key keyCode);
    virtual void doKeyboardPress(Qt::Key keyCode);
    virtual void doKeyboardRelease(Qt::Key keyCode);

private:
	static LRESULT CALLBACK processKeyboard( int code, WPARAM event, LPARAM kb );
	static LRESULT CALLBACK processMouse( int code, WPARAM event, LPARAM kb );

private:
	HHOOK m_hookMouse;
	HHOOK m_hookKeyboard;

	InputEventManager *m_manager;
	int m_hookBitsEnabled;
};

#endif
#endif // WINDOWEVENTMANAGER_H
