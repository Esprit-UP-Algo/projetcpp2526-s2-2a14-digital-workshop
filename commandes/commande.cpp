#include "commande.h"




commande::commande()
{
    id_commande = 1;
    id_utilisateur_commande = 1;
    id_client_commande = 1;
    montant_total_commande = 0;
    date_commande = QDate::currentDate();
    Status_commande = "";
    methode_paiement_commande = "";
}
commande::commande(int id_commande,
                   int id_client_commande,
                   int id_utilisateur_commande,
                   QDate date_commande,
                   QString status_commande,
                   QString methode_paiement_commande,
                   int montant_total_commande)
{
    this->id_commande = id_commande;
    this->id_client_commande = id_client_commande;
    this->id_utilisateur_commande = id_utilisateur_commande;
    this->date_commande = date_commande;
    this->Status_commande = status_commande;
    this->methode_paiement_commande = methode_paiement_commande;
    this->montant_total_commande = montant_total_commande;
}
commande::commande(int id_client_commande,
                   int id_utilisateur_commande,
                   QDate date_commande,
                   QString status_commande,
                   QString methode_paiement_commande,
                   int montant_total_commande)
{
    this->id_client_commande = id_client_commande;
    this->id_utilisateur_commande = id_utilisateur_commande;
    this->date_commande = date_commande;
    this->Status_commande = status_commande;
    this->methode_paiement_commande = methode_paiement_commande;
    this->montant_total_commande = montant_total_commande;
}





    //CRUD
    bool commande::ajouter()
    {
        QSqlQuery query;
        query.prepare("INSERT INTO COMMANDES (ID_CLIENT,ID_UTILISATEUR,DATE_COMMANDE,STATUT,MODE_PAIEMENT,MONTANT_TOTAL)"
                      "VALUES (:ID_CLIENT,:ID_UTILISATEUR,:DATE_COMMANDE,:STATUT,:MODE_PAIEMENT,:MONTANT_TOTAL)");


        query.bindValue(":ID_CLIENT",id_client_commande);
        query.bindValue(":ID_UTILISATEUR",id_utilisateur_commande);
        query.bindValue(":DATE_COMMANDE",date_commande);
        query.bindValue(":STATUT",Status_commande);
        query.bindValue(":MODE_PAIEMENT",methode_paiement_commande);
        query.bindValue(":MONTANT_TOTAL",montant_total_commande);

        bool test=query.exec();
        if(test)
        {
            return true;
        }
        else
            return false;

    }
    bool commande::modifier()    {
        QSqlQuery query;
        query.prepare("UPDATE  COMMANDES SET ID_CLIENT=:ID_CLIENT,ID_UTILISATEUR=:ID_UTILISATEUR,DATE_COMMANDE=:DATE_COMMANDE,STATUT=:STATUT,MODE_PAIEMENT=:MODE_PAIEMENT,MONTANT_TOTAL=:MONTANT_TOTAL"
                      " WHERE ID_COMMANDE =:ID_CO");

        query.bindValue(":ID_CLIENT",id_client_commande);
        query.bindValue(":ID_UTILISATEUR",id_utilisateur_commande);
        query.bindValue(":DATE_COMMANDE",date_commande);
        query.bindValue(":STATUT",Status_commande);
        query.bindValue(":MODE_PAIEMENT",methode_paiement_commande);
        query.bindValue(":MONTANT_TOTAL",montant_total_commande);
        query.bindValue(":ID_CO",id_commande);


        bool test=query.exec();
        if(test)
        {
            return true;
        }
        else
            return false;
    }
    bool commande::supprimer(int id_commande)    {
        QSqlQuery query;
        query.prepare("DELETE FROM  COMMANDES WHERE ID_COMMANDE =:ID_CO");


        query.bindValue(":ID_CO",id_commande);


        bool test=query.exec();
        if(test)
        {
            return true;
        }
        else
            return false;
    }
    QSqlQueryModel*  commande::afficher()    {
        QSqlQueryModel* model = new QSqlQueryModel();
        model->setQuery("SELECT * FROM COMMANDES");
        return model;

    }

    //metier simple
    QSqlQueryModel*  commande::tri(QString column,QString ordre)    {
        QSqlQueryModel* model = new QSqlQueryModel();
        model->setQuery("SELECT * FROM COMMANDES ORDER BY "+column+ " "+ordre);
        return model;

    }
    QSqlQueryModel*  commande::recherche(QString column,QString text)    {
        QSqlQueryModel* model = new QSqlQueryModel();
        model->setQuery("SELECT * FROM COMMANDES WHERE "+column+ " LIKE "+ text);
        return model;
    }

    QMap<QString, int> commande::statistiquesParModePaiement() {
        QMap<QString, int> ModePaiementStats;

        QSqlQuery query;
        query.prepare("SELECT MODE_PAIEMENT, COUNT(*) as count FROM COMMANDES GROUP BY MODE_PAIEMENT");
        if (query.exec()) {
            while (query.next()) {
                QString statut = query.value(0).toString();
                int count = query.value(1).toInt();//2
                ModePaiementStats[statut] = count;
            }
        }

        return ModePaiementStats;
    }





    int commande::chercherIdClientDapreFullName(QString client_FullName)
    {
        int id_client = 0;
        QSqlQuery query;

        // Specify the table name in the SELECT statement
        query.prepare("SELECT ID_CLIENT FROM CLIENT WHERE prenom || ' ' || nom = :client");
        query.bindValue(":client", client_FullName);

        if (query.exec() && query.next())
        {
            id_client = query.value(0).toInt();
        }

        return id_client;
    }


    QString commande::chercherFullNameDapresIdClient(int id_client)
    {
        QString fullName = "";
        QSqlQuery query;

        // Specify the table name in the SELECT statement
        query.prepare("SELECT prenom || ' ' || nom FROM CLIENT WHERE  ID_CLIENT= :client");
        query.bindValue(":client", id_client);

        if (query.exec() && query.next())
        {
            fullName = query.value(0).toString();
        }

        return fullName;
    }


    QString commande::chercherFullNameDapresIdUtilisateur(int id_utilisateur)
    {
        QString fullName = "";
        QSqlQuery query;

        // Specify the table name in the SELECT statement
        query.prepare("SELECT prenom || ' ' || nom FROM UTILISATEUR WHERE  ID_UTILISATEUR= :utilisateur");
        query.bindValue(":utilisateur", id_utilisateur);

        if (query.exec() && query.next())
        {
            fullName = query.value(0).toString();
        }

        return fullName;
    }

    int commande::chercherIdUtilisateurDapreFullName(QString utilisateur_FullName)
    {
        int id_cutilisateur = 0;
        QSqlQuery query;

        // Specify the table name in the SELECT statement
        query.prepare("SELECT ID_UTILISATEUR FROM UTILISATEUR WHERE prenom || ' ' || nom = :utilisateur");
        query.bindValue(":utilisateur", utilisateur_FullName);

        if (query.exec() && query.next())
        {
            id_cutilisateur = query.value(0).toInt();
        }

        return id_cutilisateur;
    }

    QList<QString> commande::ListeUtilisateur()
    {
        QList<QString> list;
        QSqlQuery query;
        query.prepare("SELECT prenom || ' ' || nom FROM Utilisateur");
        if(query.exec())
        {
            while(query.next())
            {
                QString nomUtilisateur=query.value(0).toString();
                list.append(nomUtilisateur);

            }
        }
        return list;


    }


    QList<QString> commande::ListeClient()
    {
        QList<QString> list;
        QSqlQuery query;
        query.prepare("SELECT prenom || ' ' || nom FROM CLIENT");
        if(query.exec())
        {
            while(query.next())
            {
                QString nomClient=query.value(0).toString();//id_livreur=1;
                list.append(nomClient);

            }
        }
        return list;


    }


