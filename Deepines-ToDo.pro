QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 link_pkgconfig


PKGCONFIG += dtkwidget

TARGET = DeepinesToDo
TEMPLATE = app
VERSION = 1.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += \
    Task.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h \
    Task.h

FORMS += \
    MainWindow.ui \
    Task.ui

RESOURCES += \
    imgs.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
