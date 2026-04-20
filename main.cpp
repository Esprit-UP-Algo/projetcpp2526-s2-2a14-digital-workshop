#include <QApplication>
#include <QMessageBox>
#include "matriele.h"
#include "connection.h"
#include <QSqlDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "Drivers disponibles:" << QSqlDatabase::drivers();

    Connection c;
    bool test = c.createconnect();

    if(test)
    {
        QMessageBox::information(nullptr, "Connexion Oracle",
                                 "Connexion à la base de données Oracle réussie.\nBienvenue !");
    }
    else
    {
        QMessageBox::critical(nullptr, "Connexion Oracle",
                              "Échec de connexion à la base de données.\nVeuillez vérifier vos identifiants réseau.");
    }

    Matriele window;
    window.show();

    return a.exec();
}
