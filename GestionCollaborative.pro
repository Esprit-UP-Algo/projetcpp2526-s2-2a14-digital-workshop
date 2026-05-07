#-------------------------------------------------
# GestionCollaborative - Qt Project File
#-------------------------------------------------

QT += core gui widgets sql network printsupport charts serialport
CONFIG += c++17

TARGET   = GestionCollaborative
TEMPLATE = app

# ---- Sources ----
SOURCES += \
    commande/arduino_commande.cpp \
    main.cpp \
    connection/connection.cpp \
    utulisateur/loginwindow.cpp \
    utulisateur/userwindow.cpp \
    matriel/matriele.cpp \
    matriel/qrcodegen.cpp \
    client/Chatwidget.cpp \
    client/chatbot.cpp \
    client/client.cpp \
    client/emailsender.cpp \
    client/mainwindow.cpp \
    client/arduino.cpp \
    commande/commande.cpp \
    commande/commandewindow.cpp \
    commande/twiliomanager.cpp \
    ardouino/ardouino.cpp

# ---- Headers ----
HEADERS += \
    commande/arduino_commande.h \
    connection/connection.h \
    utulisateur/loginwindow.h \
    utulisateur/userwindow.h \
    matriel/matriele.h \
    matriel/qrcodegen.hpp \
    client/Chatwidget.h \
    client/chatbot.h \
    client/client.h \
    client/emailsender.h \
    client/mainwindow.h \
    client/arduino.h \
    commande/commande.h \
    commande/commandewindow.h \
    commande/twiliomanager.h \
    ardouino/ardouino.h

# ---- Formulaires UI ----
FORMS += \
    userwindow.ui \
    matriel/matriele.ui \
    commande/commandewindow.ui

# ---- Répertoires d'inclusion ----
INCLUDEPATH += \
    connection \
    utulisateur \
    matriel \
    client \
    commande \
    ardouino
