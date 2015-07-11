#-------------------------------------------------
#
# Project created by QtCreator 2015-06-07T07:05:03
#
#-------------------------------------------------

QT       -= gui

TARGET = staticlibtest
TEMPLATE = lib
android {
    TEMPLATE = app  # for Android this is an "app"
}
windows {
	DEFINES += WINVER=0x0601 \
		_WIN32_WINNT=0x0601 \
}

CONFIG += staticlib
QMAKE_CXXFLAGS += -std=c++11

SOURCES += staticlibtest.cpp \
    ../../../asd/asd/src/address.cpp \
    ../../../asd/asd/src/exception.cpp \
    ../../../asd/asd/src/lock.cpp \
    ../../../asd/asd/src/semaphore.cpp \
    ../../../asd/asd/src/socket.cpp \
    ../../../asd/asd/src/string.cpp \
    ../../../asd/asd/src/test.cpp \
    ../../../asd/asd/src/threadpool.cpp \


HEADERS += staticlibtest.h \
    ../../../asd/asd/src/stdafx.h \
    ../../../asd/asd/include/address.h \
    ../../../asd/asd/include/asdbase.h \
    ../../../asd/asd/include/exception.h \
    ../../../asd/asd/include/lock.h \
    ../../../asd/asd/include/objpool.h \
    ../../../asd/asd/include/semaphore.h \
    ../../../asd/asd/include/socket.h \
    ../../../asd/asd/include/string.h \
    ../../../asd/asd/include/tempbuffer.h \
    ../../../asd/asd/include/test.h \
    ../../../asd/asd/include/threadpool.h \
    ../../../asd/asd/include/tlsmanager.h \

unix {
    target.path = /usr/lib
    INSTALLS += target
}
