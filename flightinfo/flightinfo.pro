TEMPLATE = app
TARGET = flightinfo
SOURCES = flightinfo.cpp
FORMS += form.ui
RESOURCES = flightinfo.qrc
QT += network

symbian { 
    HEADERS += sym_iap_util.h
    LIBS += -lesock  -lconnmon
    TARGET.CAPABILITY = NetworkServices
}
