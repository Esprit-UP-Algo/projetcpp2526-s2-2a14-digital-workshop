#include "client.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>

// ===== AJOUTER =====
bool Client::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO CLIENTS (ID, NOM, PRENOM, EMAIL, "
                  "TELEPHONE, ADRESSE, DATE_INSCRIPTION, STATUT) "
                  "VALUES (:id, :nom, :prenom, :email, "
                  ":telephone, :adresse, :date, :statut)");

    query.bindValue(":id",        id);
    query.bindValue(":nom",       nom);
    query.bindValue(":prenom",    prenom);
    query.bindValue(":email",     email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse",   adresse);
    query.bindValue(":date",      dateInscription);
    query.bindValue(":statut",    statut);

    if (!query.exec()) {
        qDebug() << "Erreur ajouter:" << query.lastError().text();
        return false;
    }
    return true;
}

// ===== SUPPRIMER =====
bool Client::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENTS WHERE ID = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur supprimer:" << query.lastError().text();
        return false;
    }
    return true;
}

// ===== MODIFIER =====
bool Client::modifier(int id, QString nom, QString prenom, QString email,
                      QString telephone, QString adresse,
                      QString dateInscription, QString statut)
{
    QSqlQuery query;
    query.prepare("UPDATE CLIENTS SET "
                  "NOM = :nom, PRENOM = :prenom, EMAIL = :email, "
                  "TELEPHONE = :telephone, ADRESSE = :adresse, "
                  "DATE_INSCRIPTION = :date, STATUT = :statut "
                  "WHERE ID = :id");

    query.bindValue(":id",        id);
    query.bindValue(":nom",       nom);
    query.bindValue(":prenom",    prenom);
    query.bindValue(":email",     email);
    query.bindValue(":telephone", telephone);
    query.bindValue(":adresse",   adresse);
    query.bindValue(":date",      dateInscription);
    query.bindValue(":statut",    statut);

    if (!query.exec()) {
        qDebug() << "Erreur modifier:" << query.lastError().text();
        return false;
    }
    return true;
}

// ===== AFFICHER TOUT =====
QSqlQueryModel* Client::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID, NOM, PRENOM, EMAIL, TELEPHONE, "
                    "ADRESSE, DATE_INSCRIPTION, STATUT "
                    "FROM CLIENTS ORDER BY ID");

    if (model->lastError().isValid()) {
        qDebug() << "Erreur afficher:" << model->lastError().text();
    }
    return model;
}
