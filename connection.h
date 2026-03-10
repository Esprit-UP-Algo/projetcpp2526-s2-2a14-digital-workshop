#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class Connection
{
private:
    // Constructeur privé (Singleton)
    Connection() {}

    // Instance unique statique
    static Connection* instance;

    QSqlDatabase db;

public:

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;


    static Connection* getInstance()
    {
        if (instance == nullptr) {
            instance = new Connection();
        }
        return instance;
    }


    bool createconnect()
    {
        db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName("Driver={Oracle in XE};DBQ=XE;Uid=tayssir;Pwd=tesstess;");

        if (db.open()) {
            qDebug() << "Connexion Singleton reussie !";
            return true;
        } else {
            qDebug() << "Connexion echouee :" << db.lastError().text();
            return false;
        }
    }

    // Fermer la connexion
    void closeConnection()
    {
        if (db.isOpen()) {
            db.close();
            qDebug() << "Connexion fermee.";
        }
    }

    // Vérifier si connecté
    bool isConnected()
    {
        return db.isOpen();
    }
};

#endif // CONNECTION_H
