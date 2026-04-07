#include "commandeservice.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

CommandeService::CommandeService()
{
    m_db = QSqlDatabase::addDatabase("QODBC", "commande_conn");
}

CommandeService::~CommandeService()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase("commande_conn");
}

bool CommandeService::connectToDatabase(const QString &host, int port,
                                        const QString &dbName,
                                        const QString &user, const QString &password,
                                        const QString &odbcDriver)
{
    // ODBC connection string for Oracle — requires Oracle ODBC driver installed
    // (comes with Oracle Instant Client or full Oracle Client)
    QString dsn = QString(
        "DRIVER={%1};"
        "DBQ=%2:%3/%4;"
        "UID=%5;"
        "PWD=%6;"
    ).arg(odbcDriver).arg(host).arg(port).arg(dbName).arg(user).arg(password);

    m_db.setDatabaseName(dsn);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    return true;
}

bool CommandeService::addCommande(int id, const QString &clientName, const QString &product,
                                  int quantity, double price, const QDate &orderDate)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO commande (id, client_name, product, quantity, price, order_date) "
              "VALUES (:id, :client_name, :product, :quantity, :price, :order_date)");
    q.bindValue(":id",          id);
    q.bindValue(":client_name", clientName);
    q.bindValue(":product",     product);
    q.bindValue(":quantity",    quantity);
    q.bindValue(":price",       price);
    q.bindValue(":order_date",  orderDate);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }

    // Notify admin via EmailJS after successful insert
    m_emailService.sendCommandeEmail(clientName, product, quantity);

    return true;
}

bool CommandeService::updateCommande(int id, const QString &clientName, const QString &product,
                                     int quantity, double price, const QDate &orderDate)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE commande SET client_name=:client_name, product=:product, "
              "quantity=:quantity, price=:price, order_date=:order_date WHERE id=:id");
    q.bindValue(":client_name", clientName);
    q.bindValue(":product",     product);
    q.bindValue(":quantity",    quantity);
    q.bindValue(":price",       price);
    q.bindValue(":order_date",  orderDate);
    q.bindValue(":id",          id);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        m_lastError = "No record found with id " + QString::number(id);
        return false;
    }
    return true;
}

bool CommandeService::deleteCommande(int id)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM commande WHERE id=:id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        m_lastError = "No record found with id " + QString::number(id);
        return false;
    }
    return true;
}

QSqlQueryModel *CommandeService::getAllCommandes(const QString &filter)
{
    auto *model = new QSqlQueryModel();

    QString sql = "SELECT id, client_name, product, quantity, price, "
                  "CAST(order_date AS VARCHAR2(20)) AS order_date "
                  "FROM commande";

    if (!filter.isEmpty()) {
        QString f = filter;
        f.replace("'", "''"); // basic SQL injection guard
        sql += " WHERE LOWER(client_name) LIKE LOWER('%" + f + "%')"
               " OR LOWER(product) LIKE LOWER('%" + f + "%')"
               " OR TO_CHAR(id) LIKE '%" + f + "%'";
    }
    sql += " ORDER BY id";

    model->setQuery(sql, m_db);

    // Wait for the query to finish (needed for some ODBC drivers)
    while (model->canFetchMore())
        model->fetchMore();

    if (model->lastError().isValid()) {
        m_lastError = model->lastError().text();
        delete model;
        return nullptr;
    }

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Client");
    model->setHeaderData(2, Qt::Horizontal, "Produit");
    model->setHeaderData(3, Qt::Horizontal, "Quantité");
    model->setHeaderData(4, Qt::Horizontal, "Prix");
    model->setHeaderData(5, Qt::Horizontal, "Date");
    return model;
}
