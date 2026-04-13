#-------------------------------------------------
# GestionCollaborative - Qt Project File
#-------------------------------------------------

QT += core gui widgets sql network printsupport charts
CONFIG += c++17

TARGET   = GestionCollaborative
TEMPLATE = app

# ---- Sources ----
SOURCES += \
    main.cpp \
    connection/connection.cpp \
    utulisateur/loginwindow.cpp \
    utulisateur/userwindow.cpp \
    matriel/matriele.cpp \
    matriel/qrcodegen.cpp \
    clients/client.cpp \
    clients/clientwindow.cpp \
    clients/chatbot.cpp \
    clients/Chatwidget.cpp \
    clients/emailsender.cpp \
    commandes/commande.cpp \
    commandes/commandewindow.cpp \
    commandes/twiliomanager.cpp

# ---- Headers ----
HEADERS += \
    connection/connection.h \
    utulisateur/loginwindow.h \
    utulisateur/userwindow.h \
    matriel/matriele.h \
    matriel/qrcodegen.hpp \
    clients/client.h \
    clients/clientwindow.h \
    clients/chatbot.h \
    clients/Chatwidget.h \
    clients/emailsender.h \
    commandes/commande.h \
    commandes/commandewindow.h \
    commandes/twiliomanager.h


# ---- Formulaires UI ----
FORMS += \
    userwindow.ui \
    matriel/matriele.ui \
    commandes/commandewindow.ui

# ---- Répertoires d'inclusion ----
INCLUDEPATH += \
    connection \
    utulisateur \
    matriel \
    clients
