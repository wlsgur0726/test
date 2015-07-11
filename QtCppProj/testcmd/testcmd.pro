
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    ../../../asd/asd_test/main.cpp \
    ../../../asd/asd_test/test_exception.cpp \
    ../../../asd/asd_test/test_objpool.cpp \
    ../../../asd/asd_test/test_semaphore.cpp \
    ../../../asd/asd_test/test_string.cpp \
    ../../../asd/asd_test/test_template.cpp \
    ../../../asd/asd_test/test_tlsmanager.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/release/ -lstaticlibtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/debug/ -lstaticlibtest

INCLUDEPATH += $$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/debug \
	../../../gtest-1.7.0/include \

DEPENDPATH += $$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/debug

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/release/libstaticlibtest.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/debug/libstaticlibtest.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/release/staticlibtest.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-staticlibtest-Desktop_Qt_5_4_1_MinGW_32bit-Debug/debug/staticlibtest.lib
