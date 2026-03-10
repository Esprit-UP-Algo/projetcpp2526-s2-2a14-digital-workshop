#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Récupérer l'instance unique du Singleton
    Connection* conn = Connection::getInstance();

    // Tenter la connexion
    if (!conn->createconnect()) {
        QMessageBox::critical(nullptr, "Erreur de connexion",
                              "Impossible de se connecter a la base de donnees !\n"
                              "Verifiez Oracle XE.");
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Style
    app.setStyle("Fusion");

    // Application
    MainWindow window;
    window.show();

    return app.exec();
}
