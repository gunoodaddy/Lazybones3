#include "maceventworker.h"
#ifdef Q_OS_MAC
#include <Foundation/Foundation.h>
#include <AppKit/NSAccessibility.h>
#include <ApplicationServices/ApplicationServices.h>
#include <QDebug>
#include <QApplication>
#include <QThread>
#include <QDesktopWidget>
#include "InputEventManager.h"
#import <AppKit/AppKit.h>

static QMap<Qt::Key, int> s_keyMap;

static int convertToMacKey(Qt::Key keyCode)
{
	QMap<Qt::Key, int>::iterator it = s_keyMap.find(keyCode);
	if(it != s_keyMap.end())
		return it.value();
	return keyCode;
}

static int convertToQtKey(int macKey)
{
	QMap<Qt::Key, int>::iterator it = s_keyMap.begin();
	for(; it != s_keyMap.end(); it++)
	{
		if(it.value() == macKey)
			return it.key();
	}
	return macKey;
}

class MacEventFilter : public QAbstractNativeEventFilter
{
public:
	MacEventFilter(MacEventManager *eventWorker) : m_eventWorker(eventWorker) { }
	bool nativeEventFilter(const QByteArray &eventType, void *message, long *result)
	{
		Q_UNUSED(eventType);
		Q_UNUSED(result);
		NSEvent * event = (NSEvent*)message;

		int type = [event type];
		static int g_currentModifierFlag = 0;
		unsigned int keyCode = 0;

		if(type == NSKeyDown || type == NSKeyUp)
			keyCode = [event keyCode];

		switch (type) {
		case NSMouseMoved:
		{
			if(!(m_eventWorker->m_hookBitsEnabled & InputEventManager::MouseHook))
				return false;

			NSPoint pt = [NSEvent mouseLocation];
			NSRect screenRect = [[NSScreen mainScreen] frame];
			NSInteger height = screenRect.size.height;
			QMouseEvent ev(QEvent::MouseMove, QPoint(pt.x, height - pt.y), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
			emit m_eventWorker->m_manager->mouseEvent(&ev);
			break;
		}
		case NSRightMouseDown:
		case NSLeftMouseDown:
		{
			if(!(m_eventWorker->m_hookBitsEnabled & InputEventManager::MouseHook))
				return false;

			NSPoint pt = [NSEvent mouseLocation];
			NSRect screenRect = [[NSScreen mainScreen] frame];
			NSInteger height = screenRect.size.height;
			QMouseEvent ev(QEvent::MouseButtonPress, QPoint(pt.x, height - pt.y), type == NSLeftMouseDown ? Qt::LeftButton : Qt::RightButton, Qt::NoButton, Qt::NoModifier);
			emit m_eventWorker->m_manager->mouseEvent(&ev);
			break;
		}
		case NSRightMouseUp:
		case NSLeftMouseUp:
		{
			if(!(m_eventWorker->m_hookBitsEnabled & InputEventManager::MouseHook))
				return false;

			NSPoint pt = [NSEvent mouseLocation];
			NSRect screenRect = [[NSScreen mainScreen] frame];
			NSInteger height = screenRect.size.height;
			QMouseEvent ev(QEvent::MouseButtonRelease, QPoint(pt.x, height - pt.y), type == NSLeftMouseUp ? Qt::LeftButton : Qt::RightButton, Qt::NoButton, Qt::NoModifier);
			emit m_eventWorker->m_manager->mouseEvent(&ev);
			break;
		}
		case NSFlagsChanged:
		{
			if(!(m_eventWorker->m_hookBitsEnabled & InputEventManager::KeyboardHook))
				return false;

			int currFlag = [event modifierFlags];
			if((currFlag & NSShiftKeyMask) && !(g_currentModifierFlag & NSShiftKeyMask))
			{
				type = NSKeyDown;
				keyCode = 56;
			}
			if(!(currFlag & NSShiftKeyMask) && (g_currentModifierFlag & NSShiftKeyMask))
			{
				type = NSKeyUp;
				keyCode = 56;
			}

			g_currentModifierFlag = [event modifierFlags];
		}
		case NSKeyDown:
		case NSKeyUp:
		{
			if(!(m_eventWorker->m_hookBitsEnabled & InputEventManager::KeyboardHook))
				return false;

			//qDebug() << "Mac native key code=" << keyCode;
			QKeyEvent ev(type == NSKeyDown ? QEvent::KeyPress : QEvent::KeyRelease, convertToQtKey(keyCode), Qt::NoModifier);
			emit m_eventWorker->m_manager->keyEvent(&ev);
			break;
		}
		default:
			return false;
		}
		return false;
	}
private:
	MacEventManager *m_eventWorker;
};

MacEventManager::MacEventManager(InputEventManager *manager) : m_manager(manager), m_hookBitsEnabled(0)
{
	qDebug() << "AXAPIEnabled : " << AXAPIEnabled();
	//	NSString *appPath = [[NSBundle mainBundle] bundlePath];
	//	AXError error = AXMakeProcessTrusted( (CFStringRef)CFBridgingRetain(appPath) );
	//	NSLog(@"%@", appPath);
	//	qDebug() << "error :" << error;
	m_filter = QSharedPointer<QAbstractNativeEventFilter>(new MacEventFilter(this));
	const int mask = NSMouseMovedMask |
			NSLeftMouseDownMask | NSLeftMouseUpMask |
			NSRightMouseDownMask | NSRightMouseUpMask |
			NSOtherMouseDownMask | NSOtherMouseUpMask |
			NSKeyDownMask | NSKeyUpMask ;

	m_monitorId = [NSEvent addGlobalMonitorForEventsMatchingMask:mask handler:^(NSEvent* event) {
			m_filter->nativeEventFilter("NSEvent", event, 0);
}];

	qApp->installNativeEventFilter(m_filter.data());

	if(s_keyMap.size() <= 0)
	{
		 //http://web.archive.org/web/20100501161453/http://www.classicteck.com/rbarticles/mackeyboard.php
		s_keyMap[Qt::Key_0] = 29;
		s_keyMap[Qt::Key_1] = 18;
		s_keyMap[Qt::Key_2] = 19;
		s_keyMap[Qt::Key_3] = 20;
		s_keyMap[Qt::Key_4] = 21;
		s_keyMap[Qt::Key_5] = 23;
		s_keyMap[Qt::Key_6] = 22;
		s_keyMap[Qt::Key_7] = 26;
		s_keyMap[Qt::Key_8] = 28;
		s_keyMap[Qt::Key_9] = 25;
		s_keyMap[Qt::Key_A] = 0;
		s_keyMap[Qt::Key_B] = 11;
		s_keyMap[Qt::Key_C] = 8;
		s_keyMap[Qt::Key_D] = 2;
		s_keyMap[Qt::Key_E] = 14;
		s_keyMap[Qt::Key_F] = 3;
		s_keyMap[Qt::Key_G] = 5;
		s_keyMap[Qt::Key_H] = 4;
		s_keyMap[Qt::Key_I] = 34;
		s_keyMap[Qt::Key_J] = 38;
		s_keyMap[Qt::Key_K] = 40;
		s_keyMap[Qt::Key_L] = 37;
		s_keyMap[Qt::Key_M] = 46;
		s_keyMap[Qt::Key_N] = 45;
		s_keyMap[Qt::Key_O] = 31;
		s_keyMap[Qt::Key_P] = 35;
		s_keyMap[Qt::Key_Q] = 12;
		s_keyMap[Qt::Key_R] = 15;
		s_keyMap[Qt::Key_S] = 1;
		s_keyMap[Qt::Key_T] = 17;
		s_keyMap[Qt::Key_U] = 32;
		s_keyMap[Qt::Key_V] = 9;
		s_keyMap[Qt::Key_W] = 13;
		s_keyMap[Qt::Key_X] = 7;
		s_keyMap[Qt::Key_Y] = 16;
		s_keyMap[Qt::Key_Z] = 6;
		s_keyMap[Qt::Key_F1] = 122;
		s_keyMap[Qt::Key_F2] = 120;
		s_keyMap[Qt::Key_F3] = 99;
		s_keyMap[Qt::Key_F4] = 118;
		s_keyMap[Qt::Key_F5] = 96;
		s_keyMap[Qt::Key_F6] = 97;
		s_keyMap[Qt::Key_F7] = 98;
		s_keyMap[Qt::Key_F8] = 100;
		s_keyMap[Qt::Key_F9] = 101;
		s_keyMap[Qt::Key_F10] = 109;
		s_keyMap[Qt::Key_F11] = 103;
		s_keyMap[Qt::Key_F12] = 111;
		s_keyMap[Qt::Key_Escape] = 53;
		s_keyMap[Qt::Key_Space] = 49;
		s_keyMap[Qt::Key_Option] = 58;
		s_keyMap[Qt::Key_Shift] = 56;
		s_keyMap[Qt::Key_Alt] = 58;
	}
}

MacEventManager::~MacEventManager(void)
{
	[NSEvent removeMonitor:(id)m_monitorId];
}

void MacEventManager::startFiltering(int hookBits)
{
	m_hookBitsEnabled = hookBits;
}

void MacEventManager::stopFiltering(int hookBits)
{
	m_hookBitsEnabled &= ~hookBits;
}

QString qt_mac_NSStringToQString(const NSString *nsstr)
{
	NSRange range;
	range.location = 0;
	range.length = [nsstr length];
	QString result(range.length, QChar(0));

	unichar *chars = new unichar[range.length];
	[nsstr getCharacters:chars range:range];
	result = QString::fromUtf16(chars, range.length);
	delete[] chars;
	return result;
}


void MacEventManager::doWindowGeometry(const QString &title, const QRect &geometry)
{
	qDebug() << "doWindowGeometry :" << title << geometry;

	@autoreleasepool {
		CGWindowListOption listOpt = kCGWindowListOptionOnScreenOnly;
		CFArrayRef windowList = CGWindowListCopyWindowInfo(listOpt, kCGNullWindowID);
		NSArray* arr = CFBridgingRelease(windowList);
		// Loop through the windows
		for (NSMutableDictionary* entry in arr)
		{
			NSString *applicationName = [entry objectForKey:(id)kCGWindowOwnerName];

			QString appName = qt_mac_NSStringToQString(applicationName);
			qDebug() << appName;
			if(appName != title)
				continue;

			// Get window PID
			pid_t pid = [[entry objectForKey:(id)kCGWindowOwnerPID] intValue];
			// Get AXUIElement using PID
			AXUIElementRef appRef = AXUIElementCreateApplication(pid);
			NSLog(@"Ref = %@",appRef);

			//			// Get the windows
			//			CFArrayRef windowList;
			//			AXUIElementCopyAttributeValue(appRef, kAXWindowsAttribute, (CFTypeRef *)&windowList);
			//			NSLog(@"WindowList = %@", windowList);
			//			if ((!windowList) || CFArrayGetCount(windowList)<1)
			//				continue;

			// get just the first window for now
			//			AXUIElementRef windowRef = (AXUIElementRef) CFArrayGetValueAtIndex( windowList, 0);
			//			CFTypeRef role;
			//			AXUIElementCopyAttributeValue(windowRef, kAXRoleAttribute, (CFTypeRef *)&role);
			AXUIElementRef windowRef;
			AXUIElementCopyAttributeValue(
						appRef, kAXMainWindowAttribute, (CFTypeRef *)&windowRef
						);

			if(windowRef == NULL)
				break;

			AXValueRef temp;
			CGSize windowSize;
			CGPoint windowPosition;

			windowPosition.x = geometry.x();
			windowPosition.y = geometry.y();
			windowSize.width = geometry.width();
			windowSize.height = geometry.height();

			temp = AXValueCreate(kAXValueCGPointType, &windowPosition);
			AXUIElementSetAttributeValue(windowRef, kAXPositionAttribute, temp);
			CFRelease(temp);

			temp = AXValueCreate(kAXValueCGSizeType, &windowSize);
			AXUIElementSetAttributeValue(windowRef, kAXSizeAttribute, temp);
			CFRelease(temp);
		}
	}
}

void MacEventManager::doMouseLeftClick(int x, int y)
{
	doMouseLeftPress(x, y);
	doMouseLeftRelease(x, y);
}

void MacEventManager::doMouseLeftPress(int x, int y)
{
	CGPoint pt;
	pt.x = x;
	pt.y = y;
	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, pt, kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doMouseLeftRelease(int x, int y)
{
	CGPoint pt;
	pt.x = x;
	pt.y = y;
	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, pt, kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doMouseRightClick(int x, int y)
{
	doMouseRightPress(x, y);
	doMouseRightRelease(x, y);
}

void MacEventManager::doMouseRightPress(int x, int y)
{
	CGPoint pt;
	pt.x = x;
	pt.y = y;
	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventRightMouseDown, pt, kCGMouseButtonRight);
	CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doMouseRightRelease(int x, int y)
{
	CGPoint pt;
	pt.x = x;
	pt.y = y;
	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventRightMouseUp, pt, kCGMouseButtonRight);
	CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doMouseMove(int x, int y)
{
	CGPoint pt;
	pt.x = x;
	pt.y = y;
	CGEventRef ev = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, pt, kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doKeyboardInput(Qt::Key keyCode)
{
	CGEventRef ev;
	ev = CGEventCreateKeyboardEvent(NULL, convertToMacKey(keyCode), true);
	CGEventPost (kCGHIDEventTap, ev);
	ev = CGEventCreateKeyboardEvent(NULL, convertToMacKey(keyCode), false);
    CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doKeyboardPress(Qt::Key keyCode)
{
    CGEventRef ev = CGEventCreateKeyboardEvent(NULL, convertToMacKey(keyCode), true);
    CGEventPost (kCGHIDEventTap, ev);
}

void MacEventManager::doKeyboardRelease(Qt::Key keyCode)
{
    CGEventRef ev = CGEventCreateKeyboardEvent(NULL, convertToMacKey(keyCode), false);
    CGEventPost (kCGHIDEventTap, ev);
}

#endif
