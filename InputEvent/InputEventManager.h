#ifndef ABSTRACTEVENTMANAGER_H
#define ABSTRACTEVENTMANAGER_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QVector>
#include "AbstractInputEventWorker.h"

class InputEventManager : public QObject
{
	Q_OBJECT
public:
	enum Type {
		KeyboardHook = 0x1,
		MouseHook = 0x2,
		AllHook = 0xFF,
	};

	InputEventManager(void);
	virtual ~InputEventManager(void);

	AbstractInputEventWorker *worker(void) { return m_eventWorker; }

	void startFiltering(int hookBits);
	void stopFiltering(int hookBits);

signals:
	void keyEvent(QKeyEvent *keyEvent);
	void mouseEvent(QMouseEvent *mouseEvent);

private:
	AbstractInputEventWorker *m_eventWorker;
};

#endif // ABSTRACTEVENTMANAGER_H
