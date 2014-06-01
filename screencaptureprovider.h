#ifndef SCREENCAPTUREPROVIDER_H
#define SCREENCAPTUREPROVIDER_H

#include <QQuickImageProvider>

class ScreenCaptureProvider : public QQuickImageProvider
{
public:
	ScreenCaptureProvider(void);
	virtual ~ScreenCaptureProvider(void);

	virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize);
	static const QPixmap * lastPixmap(void) { return s_lastScreenShoPixmap; }
	static void setLastPixmap(QPixmap &pixmap)
	{
		if(s_lastScreenShoPixmap)
			delete s_lastScreenShoPixmap;
		s_lastScreenShoPixmap = new QPixmap(pixmap);
	}

private:
	static QPixmap * s_lastScreenShoPixmap;
};


#endif // SCREENCAPTUREPROVIDER_H
