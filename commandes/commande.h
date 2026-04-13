#ifndef COMMANDE_H
#define COMMANDE_H
#include <QDate>
#include <QString>
#include <QSqlQueryModel>
#include <QSqlQuery>
class commande
{
public:
    commande();
    commande(int id_commande,
             int id_client_commande,
             int id_utilisateur_commande,
             QDate date_commande,
             QString status_commande,
             QString methode_paiement_commande,
             int montant_total_commande);
    commande(int id_client_commande,
             int id_utilisateur_commande,
             QDate date_commande,
             QString status_commande,
             QString methode_paiement_commande,
             int montant_total_commande);

    // ===== GETTERS =====
    int getid_commande()
    {
        return id_commande;
    }

    int getid_utilisateur_commande()
    {
        return id_utilisateur_commande;
    }

    int getid_client_commande()
    {
        return id_client_commande;
    }

    int getmontant_total_commande()
    {
        return montant_total_commande;
    }

    QDate getdate_commande()
    {
        return date_commande;
    }

    QString getstatus_commande()
    {
        return Status_commande;
    }

    QString getmethode_paiement_commande()
    {
        return methode_paiement_commande;
    }

    // ===== SETTERS =====
    void setId_commande(int id)
    {
        this->id_commande = id;
    }

    void setid_utilisateur_commande(int id)
    {
        this->id_utilisateur_commande = id;
    }

    void setid_client_commande(int id)
    {
        this->id_client_commande = id;
    }

    void setmontant_total_commande(int montant)
    {
        this->montant_total_commande = montant;
    }

    void setdate_commande(QDate date)
    {
        this->date_commande = date;
    }

    void setstatus_commande(QString status)
    {
        this->Status_commande = status;
    }

    void setmethode_paiement_commande(QString methode)
    {
        this->methode_paiement_commande = methode;
    }


    //CRUD
    bool ajouter();
    bool modifier();
    bool supprimer(int id_commande);
    QSqlQueryModel*  afficher();

    //metier simple
    QSqlQueryModel*  tri(QString column,QString ordre);
    QSqlQueryModel*  recherche(QString column,QString text);
    QMap<QString, int> statistiquesParModePaiement();


    //extra functions
    QList<QString> ListeUtilisateur();
    QList<QString> ListeClient();

    int chercherIdClientDapreFullName(QString client_FullName);
    int chercherIdUtilisateurDapreFullName(QString utilisateur_FullName);

    QString chercherFullNameDapresIdClient(int id_client);
    QString chercherFullNameDapresIdUtilisateur(int id_utilisateur);





    private:
    int id_commande, id_utilisateur_commande, id_client_commande, montant_total_commande;
    QDate date_commande;
    QString Status_commande, methode_paiement_commande;
};
#endif // COMMANDE_H
