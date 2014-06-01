#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <qqml.h>

#include "Lazybones.h"
#include "inputaction.h"
#include "gamescreenlistmodel.h"
#include "gamescreenmanager.h"
#include "inputmethodlistmodel.h"
#include "screencaptureprovider.h"
#include "macropresetlistmodel.h"
#include "settingmanager.h"
#include "simplestringlistmodel.h"
#include "dropboxdata.h"
#include "ConsoleAppender.h"
#include "FileAppender.h"
#include "Logger.h"

// TODO
// 1. Disassemble all item function
// 2. Key press/release event split

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QQmlApplicationEngine engine;

	Lazybones::initialize();

	QString logPath = qApp->applicationDirPath() + QDir::separator() + "Log.txt";
	QFile::remove(logPath);
	FileAppender* fileAppender = new FileAppender(logPath);
	fileAppender->setFormat("%t{yyyy-MM-dd HH:mm:ss.zzz} %m\n");

	ConsoleAppender* consoleAppender = new ConsoleAppender();
	consoleAppender->setFormat("%t{yyyy-MM-dd HH:mm:ss.zzz} %m\n");

	Logger::registerAppender(fileAppender);
	Logger::registerAppender(consoleAppender);

	LOG_INFO() << "Starting Application..";

	qmlRegisterType<Lazybones>("gunoodaddy", 1, 0, "Lazybones");
	qmlRegisterType<InputAction>("gunoodaddy", 1, 0, "InputAction");
	qmlRegisterType<GameScreenData>("gunoodaddy", 1, 0, "GameScreenData");
	qmlRegisterType<MacroAction>("gunoodaddy", 1, 0, "MacroAction");
	qmlRegisterUncreatableType<SimpleStringListModel>("gunoodaddy", 1, 0, "SimpleStringListModel", "Can only be created by C++.");
	qmlRegisterUncreatableType<GameScreenManager>("gunoodaddy", 1, 0, "GameScreenManager", "Can only be created by C++.");
	qmlRegisterUncreatableType<GameScreenListModel>("gunoodaddy", 1, 0, "GameScreenListModel", "Can only be created by C++.");
	qmlRegisterUncreatableType<InputMethodListModel>("gunoodaddy", 1, 0, "InputMethodListModel", "Can only be created by C++.");
	qmlRegisterUncreatableType<MacroActionListModel>("gunoodaddy", 1, 0, "MacroActionListModel", "Can only be created by C++.");
	qmlRegisterUncreatableType<MacroPresetListModel>("gunoodaddy", 1, 0, "MacroPresetListModel", "Can only be created by C++.");
	qmlRegisterUncreatableType<MacroPresetData>("gunoodaddy", 1, 0, "MacroPresetData", "Can only be created by C++.");
	qmlRegisterUncreatableType<DropBoxData>("gunoodaddy", 1, 0, "DropBoxData", "Can only be created by C++.");

	engine.addImageProvider("capture", new ScreenCaptureProvider());

	engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

	SETTING_MANAGER->dropBox()->commandReader()->start();

	int ret = app.exec();

	SETTING_MANAGER->dropBox()->commandReader()->stop();

	return ret;
}
