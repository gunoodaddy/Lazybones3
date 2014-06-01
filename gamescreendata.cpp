#include "gamescreendata.h"
#include <QQmlEngine>

GameScreenData::GameScreenData(void) : QObject(0), m_pixelX(0), m_pixelY(0)
{

}

GameScreenData::GameScreenData(const QString &name, int pixelX, int pixelY, const QColor &color) : QObject(0)
  , m_name(name), m_pixelX(pixelX), m_pixelY(pixelY), m_color(color)
{
}

bool GameScreenData::equals(GameScreenData *rhs)
{
	if(!equalsWithoutColor(rhs))
		return false;

	if(rhs->color() != m_color)
		return false;

	return true;
}

bool GameScreenData::equalsWithoutColor(GameScreenData *rhs)
{
	if(rhs->name() != m_name)
		return false;
	if(rhs->pixelX() != m_pixelX)
		return false;
	if(rhs->pixelY() != m_pixelY)
		return false;

	return true;
}
