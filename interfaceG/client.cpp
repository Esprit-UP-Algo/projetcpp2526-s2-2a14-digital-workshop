#include "client.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>

// =============================================
// VALIDATIONS
// =============================================

bool Client::validerID(const QString &id, QString &erreur)
{
    if (id.trimmed().isEmpty()) {
        erreur = "L'ID est obligatoire !";
        return false;
    }
    bool ok;
    int val = id.trimmed().toInt(&ok);
    if (!ok || val <= 0) {
        erreur = "L'ID doit etre un nombre entier positif !\nExemple : 123";
        return false;
    }
    return true;
}

bool Client::validerNom(const QString &nom, QString &erreur)
{
    if (nom.trimmed().isEmpty()) {
        erreur = "Le Nom est obligatoire !";
        return false;
    }
    if (nom.trimmed().length() < 2) {
        erreur = "Le Nom doit contenir au moins 2 caracteres !";
        return false;
    }
    if (nom.trimmed().length() > 50) {
        erreur = "Le Nom ne peut pas depasser 50 caracteres !";
        return false;
    }
    QRegularExpression regex("^[A-Za-z\\s\\-]+$");
    if (!regex.match(nom.trimmed()).hasMatch()) {
        erreur = "Le Nom ne doit contenir que des lettres !\n"
                 "Les chiffres et caracteres speciaux sont interdits.\n"
                 "Exemple valide : Ben Ali";
        return false;
    }
    return true;
}

bool Client::validerPrenom(const QString &prenom, QString &erreur)
{
    if (prenom.trimmed().isEmpty()) {
        erreur = "Le Prenom est obligatoire !";
        return false;
    }
    if (prenom.trimmed().length() < 2) {
        erreur = "Le Prenom doit contenir au moins 2 caracteres !";
        return false;
    }
    if (prenom.trimmed().length() > 50) {
        erreur = "Le Prenom ne peut pas depasser 50 caracteres !";
        return false;
    }
    QRegularExpression regex("^[A-Za-z\\s\\-]+$");
    if (!regex.match(prenom.trimmed()).hasMatch()) {
        erreur = "Le Prenom ne doit contenir que des lettres !\n"
                 "Les chiffres et caracteres speciaux sont interdits.\n"
                 "Exemple valide : Mohamed";
        return false;
    }
    return true;
}

bool Client::validerEmail(const QString &email, QString &erreur)
{
    if (email.trimmed().isEmpty()) {
        erreur = "L'Email est obligatoire !";
        return false;
    }
    QRegularExpression regex(
        "^[A-Za-z0-9._%+\\-]+@[A-Za-z0-9.\\-]+\\.[A-Za-z]{2,}$");
    if (!regex.match(email.trimmed()).hasMatch()) {
        erreur = "L'Email n'est pas valide !\n"
                 "Format attendu : exemple@domaine.com\n"
                 "Exemples : contact@gmail.com / ben.ali@yahoo.fr";
        return false;
    }
    return true;
}

bool Client::validerTelephone(const QString &tel, QString &erreur)
{
    if (tel.trimmed().isEmpty()) {
        erreur = "Le Telephone est obligatoire !";
        return false;
    }
    QRegularExpression regexChiffres("^[0-9]+$");
    if (!regexChiffres.match(tel.trimmed()).hasMatch()) {
        erreur = "Le Telephone ne doit contenir que des chiffres !\n"
                 "Exemple valide : 99123456";
        return false;
    }
    if (tel.trimmed().length() != 8) {
        erreur = "Le Telephone doit contenir exactement 8 chiffres !\n"
                 "Exemple valide : 99123456";
        return false;
    }
    QChar premier = tel.trimmed().at(0);
    if (premier != '2' && premier != '5' &&
        premier != '7' && premier != '9') {
        erreur = "Le numero doit commencer par 2, 5, 7 ou 9 !\n"
                 "Exemples : 99123456 / 55987654 / 20123456";
        return false;
    }
    return true;
}

bool Client::validerAdresse(const QString &adresse, QString &erreur)
{
    if (adresse.trimmed().isEmpty()) {
        erreur = "L'Adresse est obligatoire !";
        return false;
    }
    if (adresse.trimmed().length() < 5) {
        erreur = "L'Adresse doit contenir au moins 5 caracteres !\n"
                 "Exemple : Tunis, Ariana";
        return false;
    }
    return true;
}

QString Client::validerTout(const QString &id,
                            const QString &nom,
                            const QString &prenom,
                            const QString &email,
                            const QString &telephone,
                            const QString &adresse)
{
    QString erreur;
    if (!validerID(id, erreur))              return erreur;
    if (!validerNom(nom, erreur))            return erreur;
    if (!validerPrenom(prenom, erreur))      return erreur;
    if (!validerEmail(email, erreur))        return erreur;
    if (!validerTelephone(telephone, erreur))return erreur;
    if (!validerAdresse(adresse, erreur))    return erreur;
    return "";
}

// =============================================
// CRUD
// =============================================

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

bool Client::modifier(int id, QString nom, QString prenom,
                      QString email, QString telephone,
                      QString adresse, QString dateInscription,
                      QString statut)
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
