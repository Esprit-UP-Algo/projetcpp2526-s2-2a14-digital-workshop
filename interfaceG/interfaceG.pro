QT += core gui sql printsupport network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = Atelier_Connexion
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11

SOURCES += \
    Chatwidget.cpp \
    chatbot.cpp \
    client.cpp \
    emailsender.cpp \
    main.cpp \
    mainwindow.cpp \
    connection.cpp

HEADERS += \
    Chatwidget.h \
    chatbot.h \
    client.h \
    emailsender.h \
    mainwindow.h \
    connection.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
