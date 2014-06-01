#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>
#include <QString>
#include "macropresetlistmodel.h"
#include "gamescreenlistmodel.h"
#include "dropboxdata.h"

#ifndef Q_MOC_RUN
#include "Singleton.h"
#endif

class SettingManager : public QObject
{
	Q_OBJECT

public:
	SettingManager();

	void load(void);
	void save(void);

	GameScreenListModel* gameScreenModel(void) { return &m_gameScreenList; }
	MacroPresetListModel* macroPresetList(void) { return &m_macroPresetList; }
	DropBoxData* dropBox(void) { return &m_dropBox; }

	void addDefaultMacroPreset(void);

	void deleteMacroPreset(QSharedPointer<MacroPresetData> presetData);

private:
	void __loadDropBox(void);
	void __loadMacroPresetList(void);
	void __loadMacroPresetListOld(void);
	void __loadGameScreen(void);
	void __loadMacroPreset(QString l3Path, QSharedPointer<MacroPresetData> presetData);

	void __saveDropBox(void);
	void __saveMacroPresetList(void);
	void __saveGameScreen(void);
	void __saveMacroPreset(QString l3Path, QSharedPointer<MacroPresetData> presetData);

	void __makeDefaultDropBoxData(void);

signals:
	void macroPresetListChanged(void);

private:
	GameScreenListModel m_gameScreenList;
	MacroPresetListModel m_macroPresetList;
	DropBoxData m_dropBox;
	QString m_initPath;
};

#define SETTING_MANAGER CSingleton<SettingManager>::instance()

#endif // SETTINGMANAGER_H
