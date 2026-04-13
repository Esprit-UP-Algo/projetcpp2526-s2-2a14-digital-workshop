#include "client.h"
#include <QSqlDatabase>
#include <QDebug>
#include <QRegularExpression>

Client::Client()
{
    id = 0;
    nom = "";
    prenom = "";
    email = "";
    tel = "";
    adresse = "";
    dateInscription = "";
    statut = "";
    idUtilisateur = 0;
}

Client::Client(int id, QString nom, QString prenom, QString email, QString tel,
               QString adresse, QString dateInscription, QString statut,
               int idUtilisateur)
{
    this->id = id;
    this->nom = nom;
    this->prenom = prenom;
    this->email = email;
    this->tel = tel;
    this->adresse = adresse;
    this->dateInscription = dateInscription;
    this->statut = statut;
    this->idUtilisateur = idUtilisateur;
}

bool Client::ajouter()
{
    QSqlQuery query;

    // Vérifier si l'ID existe déjà
    query.prepare("SELECT COUNT(*) FROM CLIENTS WHERE ID = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        qDebug() << "ID deja existant:" << id;
        return false;
    }

    // Vérifier si l'email existe déjà
    query.prepare("SELECT COUNT(*) FROM CLIENTS WHERE EMAIL = :email");
    query.bindValue(":email", email);
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        qDebug() << "Email deja existant:" << email;
        lastError = "Cet email est déjà utilisé par un autre client !";
        return false;
    }

    // Vérifier si le téléphone existe déjà
    query.prepare("SELECT COUNT(*) FROM CLIENTS WHERE TEL = :tel");
    query.bindValue(":tel", tel);
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        qDebug() << "Tel deja existant:" << tel;
        lastError = "Ce téléphone est déjà utilisé par un autre client !";
        return false;
    }

    // INSERT
    query.prepare(
        "INSERT INTO CLIENTS (ID, NOM, PRENOM, EMAIL, TEL, ADRESSE, "
        "DATE_INSCRIPTION, STATUT) "
        "VALUES (:id, :nom, :prenom, :email, :tel, :adresse, "
        "TO_DATE(:dateInscription, 'DD/MM/YYYY'), :statut)"
        );
    query.bindValue(":id",              id);
    query.bindValue(":nom",             nom);
    query.bindValue(":prenom",          prenom);
    query.bindValue(":email",           email);
    query.bindValue(":tel",             tel);
    query.bindValue(":adresse",         adresse);
    query.bindValue(":dateInscription", dateInscription);
    query.bindValue(":statut",          statut);

    if (!query.exec()) {
        lastError = query.lastError().databaseText();
        qDebug() << "Erreur ajouter:" << lastError;
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
}

bool Client::modifier(int idEdit, QString nouveauNom, QString nouveauPrenom,
                      QString nouvelEmail, QString nouveauTel,
                      QString nouvelleAdresse, QString nouvelleDate,
                      QString nouveauStatut)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.prepare(
        "UPDATE CLIENTS SET "
        "NOM=:nom, PRENOM=:prenom, EMAIL=:email, TEL=:tel, "
        "ADRESSE=:adresse, DATE_INSCRIPTION=TO_DATE(:dateInscription,'DD/MM/YYYY'), "
        "STATUT=:statut "
        "WHERE ID=:id"
        );
    query.bindValue(":id",              idEdit);
    query.bindValue(":nom",             nouveauNom);
    query.bindValue(":prenom",          nouveauPrenom);
    query.bindValue(":email",           nouvelEmail);
    query.bindValue(":tel",             nouveauTel);
    query.bindValue(":adresse",         nouvelleAdresse);
    query.bindValue(":dateInscription", nouvelleDate);
    query.bindValue(":statut",          nouveauStatut);

    if (!query.exec()) {
        qDebug() << "Erreur modifier:" << query.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
}

bool Client::supprimer(int id)
{
    QSqlDatabase::database().transaction();
    QSqlQuery query;
    query.prepare("DELETE FROM CLIENTS WHERE ID = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qDebug() << "Erreur supprimer:" << query.lastError().text();
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
}

QSqlQueryModel* Client::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(
        "SELECT ID, NOM, PRENOM, EMAIL, TEL, ADRESSE, DATE_INSCRIPTION, STATUT "
        "FROM CLIENTS ORDER BY ID"
        );
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Nom"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Prenom"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Email"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Telephone"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Adresse"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Statut"));
    return model;
}

QString Client::validerTout(QString idStr, QString nom, QString prenom,
                            QString email, QString tel, QString adresse)
{
    QString erreurs = "";

    if (idStr.isEmpty() || idStr.toInt() <= 0)
        erreurs += "- L'ID doit etre un nombre entier positif.\n";

    QRegularExpression regexAlpha("^[a-zA-Z\\s]+$");
    if (nom.isEmpty() || !regexAlpha.match(nom).hasMatch())
        erreurs += "- Le nom ne doit contenir que des lettres.\n";

    if (prenom.isEmpty() || !regexAlpha.match(prenom).hasMatch())
        erreurs += "- Le prenom ne doit contenir que des lettres.\n";

    QRegularExpression regexEmail(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
    if (email.isEmpty() || !regexEmail.match(email).hasMatch())
        erreurs += "- L'email n'est pas valide.\n";

    QRegularExpression regexTel("^[0-9]{8}$");
    if (tel.isEmpty() || !regexTel.match(tel).hasMatch())
        erreurs += "- Le telephone doit contenir 8 chiffres.\n";

    if (adresse.isEmpty())
        erreurs += "- L'adresse ne peut pas etre vide.\n";

    return erreurs;
}
