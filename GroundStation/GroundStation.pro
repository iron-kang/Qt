QT       += core gui webenginewidgets network opengl charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GroundStation
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lglut -lGLU

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        glwidget.cpp \
        drone.cpp \
    joysticthread.cpp

HEADERS += \
        mainwindow.h \
        datatype.h \
        glwidget.h \
        drone.h \
    joysticthread.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    pic/bg-roll.png
