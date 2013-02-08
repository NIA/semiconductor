# -------------------------------------------------
# Project created by QtCreator 2010-12-13T21:09:32
# -------------------------------------------------
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = semiconductor
TEMPLATE = app
OBJECTS_DIR = ./build
DESTDIR = ./bin
SOURCES += main.cpp \
    widget.cpp \
    model.cpp
HEADERS += widget.h \
    model.h \
    includes.h
FORMS += widget.ui
unix {
    INCLUDEPATH += /usr/include/qwt/
    release:LIBS += /usr/lib/libqwt.so.6
}
win32 {
    greaterThan(QT_MAJOR_VERSION, 4) {
        INCLUDEPATH +=  C:\\Qwt-6.1.0-rc3\\include
        CONFIG(debug, debug|release) {
            LIBS += C:\\Qwt-6.1.0-rc3\\lib\\qwtd.dll
        } else {
            LIBS += C:\\Qwt-6.1.0-rc3\\lib\\qwt.dll
        }
    } else {
        INCLUDEPATH +=  C:\\Qwt-6.0.1\\include
        CONFIG(debug, debug|release) {
            LIBS += C:/Qwt-6.0.1/lib/qwtd.dll
        } else {
            LIBS += C:/Qwt-6.0.1/lib/qwt.dll
        }
    }
    RC_FILE     += semiconductor.rc
    OTHER_FILES += semiconductor.rc
}
TRANSLATIONS += semiconductor_ru.ts
