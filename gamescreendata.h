#ifndef GAMESCREENDATA_H
#define GAMESCREENDATA_H

#include <QObject>
#include <QColor>

class GameScreenData : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(int pixelX READ pixelX WRITE setPixelX NOTIFY pixelXChanged)
	Q_PROPERTY(int pixelY READ pixelY WRITE setPixelY NOTIFY pixelYChanged)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
	GameScreenData(void);
	explicit GameScreenData(const QString &name, int pixelX, int pixelY, const QColor &color);

	const QString &name(void) { return m_name; }
	void setName(QString name) { m_name = name; emit nameChanged(); }

	int pixelX(void) { return m_pixelX; }
	void setPixelX(int pixelX) { m_pixelX = pixelX; emit pixelXChanged(); }

	int pixelY(void) { return m_pixelY; }
	void setPixelY(int pixelY) { m_pixelY = pixelY; emit pixelYChanged(); }

	const QColor& color(void) { return m_color; }
	void setColor(QColor color) { m_color = color; emit colorChanged(); }

	bool equals(GameScreenData *rhs);
	bool equalsWithoutColor(GameScreenData *rhs);

signals:
	void nameChanged(void);
	void pixelXChanged(void);
	void pixelYChanged(void);
	void colorChanged(void);

public slots:

private:
	QString m_name;
	int m_pixelX;
	int m_pixelY;
	QColor m_color;
};

#endif // GAMESCREENDATA_H
