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
		QPixmap screenShotPixmap = screen->grabWindow(0, screen->availableGeometry().x(), screen->availableGeometry().y(), screen->availableGeometry().width(), screen->availableGeometry().height());
		//QPixmap copied = screenShotPixmap.copy(screen->availableGeometry());
		//qDebug() << "SSSSSSSSS " << copied << screen->availableGeometry();
		setLastPixmap(screenShotPixmap);

		return screenShotPixmap;
	}

	return QPixmap();
}
