TEMPLATE = app
TARGET = weatherinfo
SOURCES = weatherinfo.cpp
RESOURCES = weatherinfo.qrc
QT += network svg

symbian { 
    HEADERS += sym_iap_util.h
    LIBS += -lesock -lconnmon
    TARGET.CAPABILITY = NetworkServices
}
