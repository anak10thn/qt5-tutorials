TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build
UI_DIR = build

# Input
HEADERS += twitter.h twitterview.h mainwindow.h
SOURCES += main.cpp twitter.cpp twitterview.cpp mainwindow.cpp

FORMS += mainwindow.ui

RESOURCES += twitterview.qrc

QT += webkitwidgets network xmlpatterns
