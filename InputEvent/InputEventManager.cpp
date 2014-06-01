#include "InputEventManager.h"
#include "windoweventworker.h"
#include "maceventworker.h"

InputEventManager::InputEventManager(void) : QObject(0)
{
#if defined(Q_OS_MAC)
	m_eventWorker = new MacEventManager(this);
#elif defined(Q_OS_WIN32)
	m_eventWorker = new WindowEventManager(this);
#endif
}

InputEventManager::~InputEventManager(void)
{

}

void InputEventManager::startFiltering(int hookBits)
{
	m_eventWorker->startFiltering(hookBits);
}

void InputEventManager::stopFiltering(int hookBits)
{
	m_eventWorker->stopFiltering(hookBits);
}
