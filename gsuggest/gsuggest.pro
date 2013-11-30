TEMPLATE = app
TARGET   = gsuggest
SOURCES  = gsuggest.cpp
FORMS    = search.ui

QT += network

SOURCES += ../dragmove/dragmovecharm.cpp
HEADERS += ../dragmove/dragmovecharm.h
INCLUDEPATH += ../dragmove
