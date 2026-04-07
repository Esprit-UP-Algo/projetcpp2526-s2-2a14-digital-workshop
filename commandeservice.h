#ifndef COMMANDESERVICE_H
#define COMMANDESERVICE_H

#include <QString>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "emailjsservice.h"

class CommandeService
{
public:
    CommandeService();
    ~CommandeService();

    bool connectToDatabase(const QString &host, int port,
                           const QString &dbName,
                           const QString &user, const QString &password,
                           const QString &odbcDriver = "Oracle dans OraDB21Home1");

    bool addCommande(int id, const QString &clientName, const QString &product,
                     int quantity, double price, const QDate &orderDate);

    bool updateCommande(int id, const QString &clientName, const QString &product,
                        int quantity, double price, const QDate &orderDate);

    bool deleteCommande(int id);

    QSqlQueryModel *getAllCommandes(const QString &filter = QString());

    QString lastError() const { return m_lastError; }

    // Access the email service to configure API credentials
    EmailJSService *emailService() { return &m_emailService; }

private:
    QSqlDatabase   m_db;
    QString        m_lastError;
    EmailJSService m_emailService;
};

#endif // COMMANDESERVICE_H
