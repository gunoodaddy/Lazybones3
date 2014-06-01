DEFINES += CUTELOGGER_LIBRARY

INCLUDEPATH += $$PWD/include/

SOURCES += $$PWD/src/Logger.cpp \
		   $$PWD/src/AbstractAppender.cpp \
		   $$PWD/src/AbstractStringAppender.cpp \
		   $$PWD/src/ConsoleAppender.cpp \
		   $$PWD/src/FileAppender.cpp

HEADERS += $$PWD/include/Logger.h \
		   $$PWD/include/CuteLogger_global.h \
		   $$PWD/include/AbstractAppender.h \
		   $$PWD/include/AbstractStringAppender.h \
		   $$PWD/include/ConsoleAppender.h \
		   $$PWD/include/FileAppender.h

win32 {
	SOURCES += $$PWD/src/OutputDebugAppender.cpp
	HEADERS += $$PWD/include/OutputDebugAppender.h
}

