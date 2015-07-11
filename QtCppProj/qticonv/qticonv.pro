#-------------------------------------------------
#
# Project created by QtCreator 2015-06-08T00:35:34
#
#-------------------------------------------------

QT       -= core gui

TARGET = qticonv
TEMPLATE = lib
android {
    TEMPLATE = app

    QMAKE_CFLAGS += \
        -DANDROID \
        -DLIBDIR="/" \
        -DBUILDING_LIBICONV \
        -DIN_LIBRARY
}

DEFINES += QTICONV_LIBRARY

INCLUDEPATH += \
    ../../../libiconv-master/source \
    ../../../libiconv-master/source/include \
    ../../../libiconv-master/source/lib \
    ../../../libiconv-master/source/libcharset \
    ../../../libiconv-master/source/libcharset/include \
    ../../../libiconv-master/source/srclib

SOURCES += \
    ../../../libiconv-master/source/lib/iconv.c \
    ../../../libiconv-master/source/lib/relocatable.c \
    ../../../libiconv-master/source/libcharset/lib/localcharset.c

HEADERS += \
    ../../../libiconv-master/source/config.h \
    ../../../libiconv-master/source/include/iconv.h \
    ../../../libiconv-master/source/lib/localcharset.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
