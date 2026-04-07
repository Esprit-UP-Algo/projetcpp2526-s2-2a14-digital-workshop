#include "connection.h"

Connection::Connection()
{
}

bool Connection::createconnect()
{
    bool test = false;
    db = QSqlDatabase::addDatabase("QODBC"); // QOCDB is the standard ODBC driver for Qt
    db.setDatabaseName("ORCL"); // User provided DSN ORCL
    db.setUserName("ahmed"); // User provided id ahmed
    db.setPassword("123456789"); // User provided mdp 123456789

    if (db.open()) {
        test = true;
    }
    return test;
}

void Connection::closeConnection()
{
    if(db.isOpen()) {
        db.close();
    }
}
