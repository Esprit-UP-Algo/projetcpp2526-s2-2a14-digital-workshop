#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

class Client {
private:
    int id;
    QString nom;
    QString prenom;
    QString email;
    QString telephone;
    QString adresse;
    QString dateInscription;
    QString statut;

public:
    Client() : id(0) {}
    Client(int id, QString nom, QString prenom, QString email,
           QString telephone, QString adresse,
           QString dateInscription, QString statut)
        : id(id), nom(nom), prenom(prenom), email(email),
        telephone(telephone), adresse(adresse),
        dateInscription(dateInscription), statut(statut) {}

    // Getters
    int getId() { return id; }
    QString getNom() { return nom; }
    QString getPrenom() { return prenom; }
    QString getEmail() { return email; }
    QString getTelephone() { return telephone; }
    QString getAdresse() { return adresse; }
    QString getDateInscription() { return dateInscription; }
    QString getStatut() { return statut; }

    // Setters
    void setId(int i) { id = i; }
    void setNom(QString n) { nom = n; }
    void setPrenom(QString p) { prenom = p; }
    void setEmail(QString e) { email = e; }
    void setTelephone(QString t) { telephone = t; }
    void setAdresse(QString a) { adresse = a; }
    void setDateInscription(QString d) { dateInscription = d; }
    void setStatut(QString s) { statut = s; }

    // Méthodes CRUD - pas de rechercher() ici, la recherche se fait dans la Vue
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id, QString nom, QString prenom, QString email,
                  QString telephone, QString adresse,
                  QString dateInscription, QString statut);
    QSqlQueryModel* afficher();
};

#endif // CLIENT_H
