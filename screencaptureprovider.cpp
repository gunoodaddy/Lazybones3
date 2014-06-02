#include "screencaptureprovider.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDebug>

QPixmap *ScreenCaptureProvider::s_lastScreenShoPixmap;

ScreenCaptureProvider::ScreenCaptureProvider() : QQuickImageProvider(QQmlImageProviderBase::Pixmap)
{

}

ScreenCaptureProvider::~ScreenCaptureProvider(void)
{
}


QPixmap ScreenCaptureProvider::requestPixmap(const QString &/*id*/, QSize * /*size*/, const QSize& /*requestedSize*/)
{
	QScreen *screen = QGuiApplication::primaryScreen();
	if (screen)
	{
		QPixmap screenShotPixmap = screen->grabWindow(0);
		setLastPixmap(screenShotPixmap);

		return screenShotPixmap;
	}

	return QPixmap();
}
