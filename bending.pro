# -------------------------------------------------
# Project created by QtCreator 2010-12-13T21:09:32
# -------------------------------------------------
TARGET = bending
TEMPLATE = app
unix:INCLUDEPATH += /usr/local/qwt-6.0.0-rc5/include/
win32:INCLUDEPATH += C:\Qwt-6.0.0-rc5\include
OBJECTS_DIR = ./build
DESTDIR = ./bin
SOURCES += main.cpp \
    widget.cpp \
    model.cpp
HEADERS += widget.h \
    model.h \
    includes.h
FORMS += widget.ui
unix:release:LIBS += /usr/local/qwt-6.0.0-rc5/lib/libqwt.so.6
win32:release:LIBS += qwt.dll
