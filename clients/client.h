#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>

class Client
{
public:
    Client();
    Client(int id, QString nom, QString prenom, QString email, QString tel,
           QString adresse, QString dateInscription, QString statut,
           int idUtilisateur = 0);
    QString lastError;

    int getId() const { return id; }
    void setId(int newId) { id = newId; }

    int getIdUtilisateur() const { return idUtilisateur; }
    void setIdUtilisateur(int val) { idUtilisateur = val; }

    bool ajouter();
    bool modifier(int idEdit, QString nouveauNom, QString nouveauPrenom,
                  QString nouvelEmail, QString nouveauTel,
                  QString nouvelleAdresse, QString nouvelleDate,
                  QString nouveauStatut);
    bool supprimer(int id);
    QSqlQueryModel* afficher();

    static QString validerTout(QString idStr, QString nom, QString prenom,
                               QString email, QString tel, QString adresse);

private:
    int id;
    QString nom;
    QString prenom;
    QString email;
    QString tel;
    QString adresse;
    QString dateInscription;
    QString statut;
    int idUtilisateur;
};

#endif // CLIENT_H
