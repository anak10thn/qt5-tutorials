TEMPLATE = app
TARGET = cymonster
SOURCES = cymonster.cpp
RESOURCES = cymonster.qrc
QT += network widgets


INCLUDEPATH = $${PWD}/v8/include
LIBS += $${PWD}/v8/libv8.a
TARGETDEPS += $${PWD}/v8/libv8.a

win32: LIBS += -lwinmm -lws2_32
