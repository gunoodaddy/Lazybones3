TEMPLATE = app

QT += qml quick widgets

include(InputEvent/InputEvent.pri)
include(CuteLogger/CuteLogger.pri)

macx {
	LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
}

SOURCES += main.cpp \
	inputmethodlistmodel.cpp \
	Lazybones.cpp \
	inputaction.cpp \
	screencaptureprovider.cpp \
	settingmanager.cpp \
	dropboxdata.cpp \
	gamescreenmanager.cpp \
	gamescreenlistmodel.cpp \
	gamescreendata.cpp \
	macropresetdata.cpp \
	macroaction.cpp \
	macroactionlistmodel.cpp \
	macropresetlistmodel.cpp \
	simplestringlistmodel.cpp \
	dropboxcommandreader.cpp \
	commanddata.cpp

HEADERS += inputmethodlistmodel.h \
	Lazybones.h \
	inputaction.h \
	screencaptureprovider.h \
	settingmanager.h \
	dropboxdata.h \
	gamescreenmanager.h \
	gamescreenlistmodel.h \
	gamescreendata.h \
	macropresetdata.h \
	macroaction.h \
	macroactionlistmodel.h \
	macropresetlistmodel.h \
	simplestringlistmodel.h \
	dropboxcommandreader.h \
	commanddata.h \
    Defines.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
