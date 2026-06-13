QT       += core gui widgets dbus

CONFIG += c++17 link_pkgconfig
PKGCONFIG += dtk6widget

TARGET = ToDo
TEMPLATE = app
VERSION = 1.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += \
    src/Task.cpp \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/MainWindow.h \
    src/Task.h

FORMS += \
    src/MainWindow.ui \
    src/Task.ui

RESOURCES += \
    resources/imgs.qrc

target.path = /usr/bin

desktop.path = /usr/share/applications
desktop.files = resources/deepines-todo.desktop

icon.path = /usr/share/icons/hicolor/256x256/apps
icon.files = resources/Logo-Racoon.png
icon.extra = mv $(DESTDIR)$(icon.path)/Logo-Racoon.png $(DESTDIR)$(icon.path)/deepines-todo.png

doc.path = /usr/share/doc/ToDo
doc.files = README.md LICENSE

INSTALLS += target desktop icon doc
