#include "client.h"
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>

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

bool Client::existe(int idToCheck)
{
    QSqlQuery query;
    query.prepare("SELECT ID FROM CLIENTS WHERE ID = ?");
    query.addBindValue(idToCheck);
    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}

// =============================================
// CRUD
// =============================================

bool Client::ajouter()
{
    if (existe(id)) {
        QMessageBox::critical(nullptr, "Erreur de doublon", "L'ID " + QString::number(id) + " existe deja dans la base de donnees Oracle !");
        return false;
    }

    // On utilise un formatage de chaine exact pour eviter tout bug de QODBC (parametre non lie).
    QString queryStr = QString("INSERT INTO CLIENTS (ID, NOM, PRENOM, EMAIL, TEL, ADRESSE, DATE_INSCRIPTION) "
                               "VALUES (%1, '%2', '%3', '%4', '%5', '%6', TO_DATE('%7', 'DD/MM/YYYY'))")
                          .arg(id)
                          .arg(QString(nom).replace("'", "''"))
                          .arg(QString(prenom).replace("'", "''"))
                          .arg(QString(email).replace("'", "''"))
                          .arg(QString(telephone).replace("'", "''"))
                          .arg(QString(adresse).replace("'", "''"))
                          .arg(dateInscription);

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        // IMPORTANT : Affiche l'erreur Oracle exacte !
        QMessageBox::critical(nullptr, "Erreur Oracle Cachee", "L'insertion a echoue car Oracle a refuse la requete. Voici l'erreur exacte :\n\n" + query.lastError().text());
        return false;
    }
    return true;
}

bool Client::supprimer(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENTS WHERE ID = ?");
    query.addBindValue(id);
    if (!query.exec()) {
        qDebug() << "Erreur supprimer:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Client::modifier(int id, QString nom, QString prenom,
                      QString email, QString telephone,
                      QString adresse, QString dateInscription)
{
    QSqlQuery query;
    query.prepare("UPDATE CLIENTS SET "
                  "NOM = ?, PRENOM = ?, EMAIL = ?, "
                  "TEL = ?, ADRESSE = ?, "
                  "DATE_INSCRIPTION = TO_DATE(?, 'DD/MM/YYYY') "
                  "WHERE ID = ?");
    query.addBindValue(nom);
    query.addBindValue(prenom);
    query.addBindValue(email);
    query.addBindValue(telephone);
    query.addBindValue(adresse);
    query.addBindValue(dateInscription);
    query.addBindValue(id);

    if (!query.exec()) {
        qDebug() << "Erreur modifier:" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlQueryModel* Client::afficher(QString critere)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    QString queryStr = "SELECT ID, NOM, PRENOM, EMAIL, TEL, ADRESSE, DATE_INSCRIPTION FROM CLIENTS ORDER BY " + critere;
    model->setQuery(queryStr);
    if (model->lastError().isValid()) {
        qDebug() << "Erreur afficher:" << model->lastError().text();
    }
    return model;
}
