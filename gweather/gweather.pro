TEMPLATE = app
SOURCES = gweather.cpp
QT += network \
    webkit
RESOURCES = gweather.qrc
win32:CONFIG += console
FORMS += form.ui
