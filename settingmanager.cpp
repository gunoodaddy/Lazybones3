#include "settingmanager.h"
#include <QSettings>
#include <QApplication>
#include <QQmlEngine>
#include <QDir>
#include <QStack>
#include "Lazybones.h"
#include "Logger.h"

const static QString Preset_GROUP_NAME = "PresetGroups";

SettingManager::SettingManager()
{
	m_initPath = qApp->applicationDirPath() + QDir::separator() + "setting.ini";

	QQmlEngine::setObjectOwnership(&m_macroPresetList, QQmlEngine::CppOwnership);
	QQmlEngine::setObjectOwnership(&m_dropBox, QQmlEngine::CppOwnership);
}

void SettingManager::addDefaultMacroPreset(void)
{
	QSharedPointer<MacroPresetData> presetData;

	presetData = QSharedPointer<MacroPresetData>(new MacroPresetData());
	presetData->setTitle("Empty_Macro");
	presetData->setHangUpSeconds(45);

	m_macroPresetList.add(presetData);
}

void SettingManager::__makeDefaultDropBoxData(void)
{
	m_dropBox.setDropBoxPath(m_dropBox.defaultDropBoxPath());
	m_dropBox.setScreenShotIntervalSec(60);
	m_dropBox.setScreenShotFileRotateCount(10);
}

void SettingManager::load(void)
{
//	// to migration
//	__loadMacroPresetListOld();
//	__loadDropBox();
//	__loadGameScreen();
//	save();
//	return;

	__loadDropBox();
	__loadGameScreen();
	__loadMacroPresetList();
}


void SettingManager::save(void)
{
	QFile::remove(m_initPath);

	__saveDropBox();
	__saveGameScreen();
	__saveMacroPresetList();
}

void SettingManager::__loadDropBox(void)
{
	QSettings settings(m_initPath, QSettings::IniFormat );

	settings.beginGroup("DropBox");
	if(settings.contains("dropBoxPath"))
	{
		m_dropBox.setDropBoxPath(settings.value("dropBoxPath").toString());
		m_dropBox.setScreenShotIntervalSec((settings.value("screenShotInterval").toInt()));
		m_dropBox.setScreenShotFileRotateCount(settings.value("screenShotFileRotate").toInt());
	}
	else
	{
		__makeDefaultDropBoxData();
	}
	settings.endGroup();
}

void SettingManager::__saveDropBox(void)
{
	QSettings settings(m_initPath, QSettings::IniFormat );

	settings.beginGroup("DropBox");
	settings.setValue("dropBoxPath", m_dropBox.dropBoxPath());
	settings.setValue("screenShotInterval", m_dropBox.screenShotIntervalSec());
	settings.setValue("screenShotFileRotate", m_dropBox.screenShotFileRotateCount());
	settings.endGroup();
}

void SettingManager::__loadGameScreen(void)
{
	m_gameScreenList.clear();

	QSettings settings(m_initPath, QSettings::IniFormat );

	int dataCount = settings.beginReadArray("GameScreen");

	for(int i = 0; i < dataCount; i++)
	{
		GameScreenData *data = new GameScreenData();

		settings.setArrayIndex(i);
		settings.beginGroup("ScreenData");
		data->setName(settings.value("name").toString());
		data->setPixelX(settings.value("pixelX").toInt());
		data->setPixelY(settings.value("pixelY").toInt());
		data->setColor(settings.value("color").toString());
		settings.endGroup();

		Lazybones::gameScreenManager()->addScreenData(data);
		delete data;
	}
	settings.endArray();
}

void SettingManager::__saveGameScreen(void)
{
	QSettings settings(m_initPath, QSettings::IniFormat );
	settings.beginWriteArray("GameScreen");

	for(int i = 0; i < m_gameScreenList.count(); i++)
	{
		QSharedPointer<GameScreenData> data = m_gameScreenList.at(i);
		settings.setArrayIndex(i);
		settings.beginGroup("ScreenData");
		settings.setValue("name", data->name());
		settings.setValue("pixelX", data->pixelX());
		settings.setValue("pixelY", data->pixelY());
		settings.setValue("color", data->color());
		settings.endGroup();
	}
	settings.endArray();
}

void SettingManager::__loadMacroPresetList(void)
{
	m_macroPresetList.clear();

	QDir dir(qApp->applicationDirPath());
	QFileInfoList list = dir.entryInfoList();

	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);
		if(fileInfo.completeSuffix() == "h3")
		{
			QSettings settings(fileInfo.absoluteFilePath(), QSettings::IniFormat );
			QSharedPointer<MacroPresetData> presetData = QSharedPointer<MacroPresetData>(new MacroPresetData());

			presetData->setTitle(fileInfo.baseName());
			presetData->setWindowTitle(settings.value("windowTitle").toString());
			presetData->setWindowRect(settings.value("windowRect").toRect());
			presetData->setHangUpSeconds(settings.value("hangupSeconds").toInt());

			QString l3FilePath = fileInfo.absolutePath() + QDir::separator() + fileInfo.baseName() + ".l3";
			__loadMacroPreset(l3FilePath, presetData);

			m_macroPresetList.add(presetData);
		}
	}

	if(m_macroPresetList.count() <= 0)
		addDefaultMacroPreset();
}

void SettingManager::__saveMacroPresetList(void)
{
	for(int i = 0; i < m_macroPresetList.count(); i++)
	{
		QSharedPointer<MacroPresetData> presetData = m_macroPresetList.at(i);

		QString h3FilePath = qApp->applicationDirPath() + QDir::separator() + presetData->title() + ".h3";
		QString l3FilePath = qApp->applicationDirPath()+ QDir::separator() + presetData->title() + ".l3";

		QSettings settings(h3FilePath, QSettings::IniFormat );

		settings.setValue("windowTitle", presetData->windowTitle());
		settings.setValue("windowRect", presetData->windowRect());
		settings.setValue("hangupSeconds", presetData->hangUpSeconds());

		__saveMacroPreset(l3FilePath, presetData);
	}
}


void SettingManager::__loadMacroPreset(QString l3Path, QSharedPointer<MacroPresetData> presetData)
{
	QFile file(l3Path);
	file.open(QFile::ReadOnly);

	QTextStream in(&file);
	presetData->read(in);
	file.close();
}

void SettingManager::__saveMacroPreset(QString l3Path, QSharedPointer<MacroPresetData> presetData)
{
	QFile file(l3Path);
	file.open(QFile::ReadWrite | QFile::Truncate);

	QTextStream out(&file);
	presetData->write(out);
	file.close();
}

void SettingManager::deleteMacroPreset(QSharedPointer<MacroPresetData> presetData)
{
	QString h3FilePath = qApp->applicationDirPath() + QDir::separator() + presetData->title() + ".h3";
	QString l3FilePath = qApp->applicationDirPath()+ QDir::separator() + presetData->title() + ".l3";
	QFile::remove(h3FilePath);
	QFile::remove(l3FilePath);
}


//==================================================================================================
// For old compatability
//==================================================================================================

void SettingManager::__loadMacroPresetListOld(void)
{
	m_macroPresetList.clear();

	QSettings settings(m_initPath, QSettings::IniFormat );

	int presetCount = settings.beginReadArray("MacroPreset");

	for(int i = 0; i < presetCount; i++)
	{
		QSharedPointer<MacroPresetData> presetData = QSharedPointer<MacroPresetData>(new MacroPresetData());

		settings.setArrayIndex(i);
		presetData->setTitle(settings.value("title").toString());
		presetData->setWindowTitle(settings.value("windowTitle").toString());
		presetData->setWindowRect(settings.value("windowRect").toRect());
		presetData->setHangUpSeconds(settings.value("hangupSeconds").toInt());

		int actionCount = settings.beginReadArray("MacroActionList");

		for(int j = 0; j < actionCount; j++)
		{
			MacroAction* macroAction = new MacroAction();
			InputAction *inputAction = macroAction->inputAction();
			settings.setArrayIndex(j);
			settings.beginGroup("MacroAction");
			macroAction->setActionType(settings.value("macroActionType").toInt());
			macroAction->setConditionType(settings.value("macroConditionType").toInt());
			macroAction->setComment(settings.value("comment").toString());
			macroAction->setFilePath(settings.value("filePath").toString());
			macroAction->setSleepMsec(settings.value("sleepMsec").toInt());
			macroAction->setMacroIndex(settings.value("gotoIndex").toInt());
			macroAction->setLoopIndex(settings.value("loopIndex").toInt());
			macroAction->setConditionScreen(settings.value("conditionScreen").toString());
			inputAction->setActionType(settings.value("actionType").toInt());
			inputAction->setKeyCode(settings.value("keyCode").toInt());
			inputAction->setMouseX(settings.value("mouseX").toInt());
			inputAction->setMouseY(settings.value("mouseY").toInt());
			inputAction->setDelayedMsec(settings.value("delayed").toInt());

			int intpuActionCount = settings.beginReadArray("ActionList");

			for(int k = 0; k < intpuActionCount; k++)
			{
				InputAction* inputAction = new InputAction();
				settings.setArrayIndex(k);
				settings.beginGroup("Action");
				inputAction->setActionType(settings.value("actionType").toInt());
				inputAction->setKeyCode(settings.value("keyCode").toInt());
				inputAction->setMouseX(settings.value("mouseX").toInt());
				inputAction->setMouseY(settings.value("mouseY").toInt());
				inputAction->setDelayedMsec(settings.value("delayed").toInt());
				settings.endGroup();

				macroAction->addInputActionChunk(inputAction);

				delete inputAction;
			}
			settings.endArray();

			settings.endGroup();

			presetData->addAction(macroAction);

			delete macroAction;
		}

		settings.endArray();

		m_macroPresetList.add(presetData);
	}
	settings.endArray();

	if(m_macroPresetList.count() <= 0)
		addDefaultMacroPreset();
}


