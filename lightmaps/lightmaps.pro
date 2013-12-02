TEMPLATE = app
SOURCES = lightmaps.cpp
QT += network widgets

symbian {
    HEADERS += sym_iap_util.h
    LIBS += -lesock  -lconnmon
    TARGET.CAPABILITY = NetworkServices
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
}
