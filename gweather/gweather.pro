TEMPLATE = app
SOURCES = gweather.cpp
QT += network \
    webkitwidgets \
    core
RESOURCES = gweather.qrc
win32:CONFIG += console
FORMS += form.ui
