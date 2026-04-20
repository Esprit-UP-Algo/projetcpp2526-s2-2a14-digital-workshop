#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQueryModel>
#include <QRegularExpression>

class Client
{
private:
    int     id;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString adresse;
    QString dateInscription;

public:
    Client() : id(0) {}

    Client(int id, QString nom, QString prenom, QString email,
           QString telephone, QString adresse,
           QString dateInscription)
        : id(id), nom(nom), prenom(prenom), email(email),
        telephone(telephone), adresse(adresse),
        dateInscription(dateInscription) {}

    // ===== CRUD =====
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id, QString nom, QString prenom,
                  QString email, QString telephone,
                  QString adresse, QString dateInscription);
    QSqlQueryModel* afficher();

    // ===== VALIDATIONS =====
    static bool validerID       (const QString &id,      QString &erreur);
    static bool validerNom      (const QString &nom,     QString &erreur);
    static bool validerPrenom   (const QString &prenom,  QString &erreur);
    static bool validerEmail    (const QString &email,   QString &erreur);
    static bool validerTelephone(const QString &tel,     QString &erreur);
    static bool validerAdresse  (const QString &adresse, QString &erreur);

    // Retourne "" si tout OK, sinon le message d'erreur
    static QString validerTout(const QString &id,
                               const QString &nom,
                               const QString &prenom,
                               const QString &email,
                               const QString &telephone,
                               const QString &adresse);
};

#endif // CLIENT_H
