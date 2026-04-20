#include "connection.h"
#include <QDebug>

Connection::Connection() {}

bool Connection::createconnect() {
    db = QSqlDatabase::addDatabase("QODBC");

    // Tout dans la chaîne de connexion
    db.setDatabaseName(
        "DRIVER={Oracle in XE};"
        "DBQ=localhost:1521/xe;"
        "UID=ahmed;"
        "PWD=ahmed123;"
        );
    // Ne pas utiliser setUserName ni setPassword

    if (db.open()) {
        qDebug() << "Connexion réussie !";
        return true;
    }
    qDebug() << "Erreur:" << db.lastError().text();
    return false;
}

void Connection::closeConnection() {
    if(db.isOpen()) {
        db.close();
    }
}
