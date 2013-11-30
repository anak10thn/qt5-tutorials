TEMPLATE = app
TARGET   = gsuggest
SOURCES  = gsuggest.cpp
FORMS    = search.ui

QT += network widgets webkitwidgets

SOURCES += ../dragmove/dragmovecharm.cpp
HEADERS += ../dragmove/dragmovecharm.h
INCLUDEPATH += ../dragmove
