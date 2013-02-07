# -------------------------------------------------
# Project created by QtCreator 2010-12-13T21:09:32
# -------------------------------------------------
TARGET = semiconductor
TEMPLATE = app
unix:INCLUDEPATH += /usr/include/qwt/
win32:INCLUDEPATH += C:\\Qwt-6.0.1\\include
OBJECTS_DIR = ./build
DESTDIR = ./bin
SOURCES += main.cpp \
    widget.cpp \
    model.cpp
HEADERS += widget.h \
    model.h \
    includes.h
FORMS += widget.ui
win32 {
    RC_FILE     += semiconductor.rc
    OTHER_FILES += semiconductor.rc
}
unix:release:LIBS += /usr/lib/libqwt.so.6
win32:release:LIBS += C:\\Qwt-6.0.1\\lib\\qwt.dll
TRANSLATIONS += semiconductor_ru.ts
