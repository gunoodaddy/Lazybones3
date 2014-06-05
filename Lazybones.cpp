#include "Lazybones.h"
#include "settingmanager.h"
#include <QQmlEngine>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include "screencaptureprovider.h"

bool Lazybones::s_forceStopFlag = false;
InputEventManager* Lazybones::s_inputEventManager = NULL;
GameScreenManager* Lazybones::s_gameScreenManager = NULL;
QPoint Lazybones::s_registeredLastMousePos;

Lazybones::Lazybones(QObject *parent) :
	QObject(parent), m_currentMacroPresetIndex(0)
{
	connect(SETTING_MANAGER, SIGNAL(macroPresetListChanged()), this, SLOT(onMacroPresetListChanged()));

	connect(s_inputEventManager, SIGNAL(keyEvent(QKeyEvent *)), this, SLOT(onKeyEvent(QKeyEvent *)));
	connect(s_inputEventManager, SIGNAL(mouseEvent(QMouseEvent *)), this, SLOT(onMouseEvent(QMouseEvent *)));

	inputEventManager()->startFiltering(InputEventManager::KeyboardHook);	// for global key hooking
}

void Lazybones::initialize(void)
{
	MacroPresetData::initialize();
	InputAction::initialize();
	MacroAction::initialize();

	if(s_inputEventManager == NULL)
		s_inputEventManager = new InputEventManager();

	if(s_gameScreenManager == NULL)
		s_gameScreenManager = new GameScreenManager();

	SETTING_MANAGER->load();
}

void Lazybones::setCurrentMacroPresetIndex(int presetIndex)
{
	m_currentMacroPresetIndex = presetIndex;

	if(presetIndex < 0 || presetIndex >= SETTING_MANAGER->macroPresetList()->count())
	{
		m_currentMacroPresetData.clear();
	}
	else
	{
		m_currentMacroPresetData = SETTING_MANAGER->macroPresetList()->at(presetIndex);
	}

	emit currentMacroPresetChanged();
	emit currentMacroPresetIndexChanged();
}

void Lazybones::onMacroPresetListChanged(void)
{
	emit macroPresetListChanged();
}

void Lazybones::onKeyEvent(QKeyEvent *event)
{
	MacroPresetData *macroPreset = currentMacroPreset();

	//qDebug() << "KeyEvent" << event->key() << preset->recording();
	if(event->key() == Qt::Key_F10 || event->key() == Qt::Key_F12 || event->key() == Qt::Key_Pause)
	{
		s_forceStopFlag = true;

		if(macroPreset)
			macroPreset->setRunning(false);

		qDebug() << "EMERGENCY STOP";
	}
}

void Lazybones::onMouseEvent(QMouseEvent * /*event*/)
{
}

QColor Lazybones::pixelColor(int pixelX, int pixelY)
{
	return getPixelColor(pixelX, pixelY);
}

QPoint Lazybones::toRealPos(QImage *image, int imageX, int imageY)
{
	QScreen *screen = QGuiApplication::primaryScreen();
	qreal ratioH = image->size().width() / screen->size().width();
	qreal ratioV = image->size().height() / screen->size().height();

	return QPoint(imageX / ratioH, imageY / ratioV);
}

QColor Lazybones::getPixelColor(int pixelX, int pixelY)
{
	QImage image;
	QScreen *screen = QGuiApplication::primaryScreen();

	QPixmap screenShotPixmap = screen->grabWindow(0);
	image = screenShotPixmap.toImage();
	ScreenCaptureProvider::setLastPixmap(screenShotPixmap);

	return getPixelColor(screen, &image, pixelX, pixelY);
}

QColor Lazybones::getPixelColor(QScreen *screen, QImage *image, int pixelX, int pixelY)
{
	qreal ratioH = image->size().width() / screen->size().width();
	qreal ratioV = image->size().height() / screen->size().height();

	QColor clr = image->pixel(pixelX * ratioH, pixelY * ratioV);
	return clr;
}
