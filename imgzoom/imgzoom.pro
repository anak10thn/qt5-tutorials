TEMPLATE = app
SOURCES = imgzoom.cpp
RESOURCES = imgzoom.qrc
QT += widgets
symbian {  TARGET.EPOCHEAPSIZE = 0x20000 0x2000000 }
