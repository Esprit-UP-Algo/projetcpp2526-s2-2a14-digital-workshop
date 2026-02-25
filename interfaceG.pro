QT       += core gui sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Atelier_Connexion
TEMPLATE = app

EFINES += QT_DEPRECATED_WARNINGS


CONFIG += c++11

SOURCES += \
    client.cpp \
        main.cpp \
        mainwindow.cpp \
    connection.cpp

HEADERS += \
    client.h \
        mainwindow.h \
    connection.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
