#include "windoweventworker.h"
#include "QtGlobal"
#ifdef Q_OS_WIN32
#include "InputEventManager.h"
#include <QKeyEvent>
#include <QDebug>
#include <Windows.h>
#include <TlHelp32.h>

static QMap<Qt::Key, int> s_keyMap;
static WindowEventManager *g_currEventWorker;	// TODO : WARNING.. THIS CODE SO DANGEROUS

static int convertToNativeKey(Qt::Key keyCode)
{
	QMap<Qt::Key, int>::iterator it = s_keyMap.find(keyCode);
	if(it != s_keyMap.end())
		return it.value();
	return keyCode;
}

static int convertToQtKey(int nativeKey)
{
	QMap<Qt::Key, int>::iterator it = s_keyMap.begin();
	for(; it != s_keyMap.end(); it++)
	{
		if(it.value() == nativeKey)
			return it.key();
	}
	return nativeKey;
}

static QEvent::Type convertToQtEventType(int messageType)
{
	if(messageType == WM_LBUTTONDOWN || messageType == WM_MBUTTONDOWN || messageType == WM_RBUTTONDOWN)
		return QEvent::MouseButtonPress;
	if(messageType == WM_LBUTTONUP || messageType == WM_MBUTTONUP || messageType == WM_RBUTTONUP)
		return QEvent::MouseButtonRelease;
	if(messageType == WM_MOUSEMOVE)
		return QEvent::MouseMove;
	return QEvent::None;
}

static Qt::MouseButton convertToQtMouseButton(int messageType)
{
	if(messageType == WM_LBUTTONDOWN || messageType == WM_LBUTTONUP)
		return Qt::LeftButton;
	if(messageType == WM_MBUTTONDOWN || messageType == WM_MBUTTONUP)
		return Qt::MiddleButton;
	if(messageType == WM_RBUTTONDOWN || messageType == WM_RBUTTONUP)
		return Qt::RightButton;
	return Qt::NoButton;
}

WindowEventManager::WindowEventManager(InputEventManager *manager) : m_hookMouse(NULL), m_hookKeyboard(NULL), m_manager(manager), m_hookBitsEnabled(0)
{
	if(s_keyMap.size() <= 0)
	{
		s_keyMap[Qt::Key_Pause] = VK_PAUSE;
		s_keyMap[Qt::Key_Escape] = VK_ESCAPE;
		s_keyMap[Qt::Key_Shift] = VK_LSHIFT;
		s_keyMap[Qt::Key_Return] = VK_RETURN;
		s_keyMap[Qt::Key_Enter] = VK_RETURN;
		s_keyMap[Qt::Key_Control] = VK_LCONTROL;
		s_keyMap[Qt::Key_Alt] = VK_LMENU;
		s_keyMap[Qt::Key_F1] = VK_F1;
		s_keyMap[Qt::Key_F2] = VK_F2;
		s_keyMap[Qt::Key_F3] = VK_F3;
		s_keyMap[Qt::Key_F4] = VK_F4;
		s_keyMap[Qt::Key_F5] = VK_F5;
		s_keyMap[Qt::Key_F6] = VK_F6;
		s_keyMap[Qt::Key_F7] = VK_F7;
		s_keyMap[Qt::Key_F8] = VK_F8;
		s_keyMap[Qt::Key_F9] = VK_F9;
		s_keyMap[Qt::Key_F10] = VK_F10;
		s_keyMap[Qt::Key_F11] = VK_F11;
	}

	g_currEventWorker = this;
}

WindowEventManager::~WindowEventManager(void)
{
	stopFiltering(InputEventManager::AllHook);

	g_currEventWorker = NULL;
}

LRESULT CALLBACK WindowEventManager::processKeyboard( int code, WPARAM wParam, LPARAM lParam )
{
	KBDLLHOOKSTRUCT* keyHook = (KBDLLHOOKSTRUCT*)lParam;

	if(g_currEventWorker && (g_currEventWorker->m_hookBitsEnabled & InputEventManager::KeyboardHook))
	{
		QEvent::Type type;
		if(wParam == WM_KEYDOWN)
		{
			type = QEvent::KeyPress;
		}
		else if(wParam == WM_KEYUP)
		{
			type = QEvent::KeyRelease;
		}

		QKeyEvent ev(type, convertToQtKey(keyHook->vkCode), Qt::NoModifier);

		ev.setAccepted(false);
		emit g_currEventWorker->m_manager->keyEvent(&ev);
		if(ev.isAccepted())
			return true;
	}

	return CallNextHookEx(NULL, code, wParam, lParam );
}


LRESULT CALLBACK WindowEventManager::processMouse( int code, WPARAM wParam, LPARAM lParam )
{
	MSLLHOOKSTRUCT* mouseHook = (MSLLHOOKSTRUCT*)lParam;

	if(g_currEventWorker && (g_currEventWorker->m_hookBitsEnabled & InputEventManager::MouseHook))
	{
		QMouseEvent ev(convertToQtEventType(wParam), QPoint(mouseHook->pt.x, mouseHook->pt.y), convertToQtMouseButton(wParam), convertToQtMouseButton(wParam), Qt::NoModifier);
		emit g_currEventWorker->m_manager->mouseEvent(&ev);
	}

	return CallNextHookEx(NULL, code, wParam, lParam );
}


void WindowEventManager::startFiltering(int hookBits)
{
	m_hookBitsEnabled |= hookBits;

	HINSTANCE appInstance = GetModuleHandle(NULL);//App Instance for call back
	if(m_hookBitsEnabled & InputEventManager::MouseHook)
	{
		UnhookWindowsHookEx(m_hookMouse);
		m_hookMouse = SetWindowsHookEx( WH_MOUSE_LL, processMouse, appInstance, 0 );
	}
	if(m_hookBitsEnabled & InputEventManager::KeyboardHook)
	{
		UnhookWindowsHookEx(m_hookKeyboard);
		m_hookKeyboard = SetWindowsHookEx( WH_KEYBOARD_LL, processKeyboard, appInstance, 0 );
	}
}

void WindowEventManager::stopFiltering(int hookBits)
{
	m_hookBitsEnabled &= ~hookBits;

	if(!(m_hookBitsEnabled & InputEventManager::MouseHook))
		UnhookWindowsHookEx(m_hookMouse);
	if(!(m_hookBitsEnabled & InputEventManager::KeyboardHook))
		UnhookWindowsHookEx(m_hookKeyboard);
}

static ULONG ProcIDFromWnd(HWND hwnd)
{
	ULONG idProc;
	GetWindowThreadProcessId( hwnd, &idProc );
	return idProc;
}

static HWND GetWinHandle(ULONG pid)
{
	HWND tempHwnd = FindWindow(NULL,NULL);

	while( tempHwnd != NULL )
	{
		if( GetParent(tempHwnd) == NULL )
			if( pid == ProcIDFromWnd(tempHwnd) )
				return tempHwnd;
		tempHwnd = GetWindow(tempHwnd, GW_HWNDNEXT);
	}
	return NULL;
}

void WindowEventManager::doWindowGeometry(const QString &title, const QRect &geometry)
{
	std::wstring wtitle = title.toStdWString();

	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	if (!snapshotHandle)
		return;

	Process32First(snapshotHandle, &processEntry);
	do
	{
		qDebug() << QString::fromStdWString(processEntry.szExeFile);
		if (!wtitle.compare(processEntry.szExeFile))
		{
			HWND hwnd = GetWinHandle(processEntry.th32ProcessID);
			if(hwnd)
			{
				::SetWindowPos(hwnd, HWND_TOP, geometry.x(), geometry.y(), geometry.width(), geometry.height(), 0);
			}
			break;
		}
	} while (Process32Next(snapshotHandle, &processEntry));
}

void WindowEventManager::doMouseLeftClick(int x, int y)
{
	::SetCursorPos(x, y);

	INPUT Input={0};
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1,&Input,sizeof(INPUT));

	::ZeroMemory(&Input,sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1,&Input,sizeof(INPUT));
}

void WindowEventManager::doMouseLeftPress(int x, int y)
{
	::SetCursorPos(x, y);

	INPUT Input={0};
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1,&Input,sizeof(INPUT));
}

void WindowEventManager::doMouseLeftRelease(int x, int y)
{
	::SetCursorPos(x, y);

	INPUT Input={0};
	::ZeroMemory(&Input,sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1,&Input,sizeof(INPUT));
}

void WindowEventManager::doMouseRightClick(int x, int y)
{
	::SetCursorPos(x, y);

	INPUT Input={0};
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	::SendInput(1,&Input,sizeof(INPUT));

	::ZeroMemory(&Input,sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	::SendInput(1,&Input,sizeof(INPUT));
}

void WindowEventManager::doMouseRightPress(int x, int y)
{
	::SetCursorPos(x, y);

	INPUT Input={0};
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	::SendInput(1,&Input,sizeof(INPUT));
}

void WindowEventManager::doMouseRightRelease(int x, int y)
{
	::SetCursorPos(x, y);

	INPUT Input={0};
	::ZeroMemory(&Input,sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	::SendInput(1,&Input,sizeof(INPUT));
}

void WindowEventManager::doMouseMove(int x, int y)
{
	::SetCursorPos(x, y);
}

void WindowEventManager::doKeyboardInput(Qt::Key keyCode)
{
	qint32 vkKey = convertToNativeKey(keyCode);

	//qDebug() << "doKeyboardInput" << keyCode << vkKey;

	INPUT input[2];
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = vkKey;
	input[0].ki.dwFlags = 0 ;
	input[0].ki.time = 0;
	input[0].ki.dwExtraInfo = 0;

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = vkKey;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;
	input[1].ki.time = 0;
	input[1].ki.dwExtraInfo = 0;

	::SendInput(2, input, sizeof(INPUT));
}

void WindowEventManager::doKeyboardPress(Qt::Key keyCode)
{
	qint32 vkKey = convertToNativeKey(keyCode);

	//qDebug() << "doKeyboardInput" << keyCode << vkKey;

	INPUT input[1];
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = vkKey;
	input[0].ki.dwFlags = 0 ;
	input[0].ki.time = 0;
	input[0].ki.dwExtraInfo = 0;

	::SendInput(1, input, sizeof(INPUT));
}

void WindowEventManager::doKeyboardRelease(Qt::Key keyCode)
{
	qint32 vkKey = convertToNativeKey(keyCode);

	//qDebug() << "doKeyboardInput" << keyCode << vkKey;

	INPUT input[1];
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = vkKey;
	input[0].ki.dwFlags = KEYEVENTF_KEYUP;
	input[0].ki.time = 0;
	input[0].ki.dwExtraInfo = 0;

	::SendInput(1, input, sizeof(INPUT));
}
#endif
