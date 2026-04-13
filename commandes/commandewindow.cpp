#include "commandewindow.h"
#include "ui_commandewindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QTextStream>
#include <QPieSeries>
#include <QChart>
#include <QChartView>
#include <QVBoxLayout>
#include "twiliomanager.h"

#include <QStandardItemModel>
#include <QStandardItem>


CommandeWindow::CommandeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CommandeWindow)
{
    ui->setupUi(this);

    //lors du l'execution


    remplir_comboBox_id_client();
    remplir_comboBox_id_utilisateur();

    ui->tableView_commande->setModel(co.afficher());



}

CommandeWindow::~CommandeWindow()
{
    delete ui;
}


void CommandeWindow::remplir_comboBox_id_client()
{
    ui->combo_client_commande->clear();
    QList<QString> liste = co.ListeClient();
    for (int i = 0; i < liste.length(); i++)
    {
        ui->combo_client_commande->addItem(liste[i]);
    }
}

void CommandeWindow::remplir_comboBox_id_utilisateur()
{
    ui->combo_utilisateur_commande->clear();
    QList<QString> liste = co.ListeUtilisateur();
    for (int i = 0; i < liste.length(); i++)
    {
        ui->combo_utilisateur_commande->addItem(liste[i]);
    }
}

void CommandeWindow::on_bt_ajouter_commande_clicked()
{
    //récuperation d'apres ui !
    QString nomClient=ui->combo_client_commande->currentText();
    QString nomUtilisateur=ui->combo_utilisateur_commande->currentText();
    int prix=ui->prix_totale_commande->text().toInt();
    QString mode_paiement=ui->combo_mode_paiement_commande->currentText();
    QString status=ui->combo_status_commande->currentText();

    int idClient=co.chercherIdClientDapreFullName(nomClient);

    int idUtilisateur=co.chercherIdUtilisateurDapreFullName(nomUtilisateur);
    QDate date_commande=QDate(QDate::currentDate());

    //controle de saisie !
    if(prix<0)
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                              QObject::tr("Le prix doit etre positif"), QMessageBox::Cancel);
        return;
    }



// appel du fonction du ajouter d'apres le classe

    commande c(idClient,idUtilisateur,date_commande, status,
              mode_paiement,
               prix);

    bool test = c.ajouter();


//affichage du resultat ,

        if(test)
    {
        QMessageBox::information(nullptr, QObject::tr(""),
                                 QObject::tr("Ajout avec succées"), QMessageBox::Cancel);
        ui->tableView_commande->setModel(co.afficher());

        ui->prix_totale_commande->clear();

        QFile file("C:/Users/amins/Desktop/projet/sans_titre1 (3)/sans_titre1/ajout.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "[NEW COMMANDE]\n"
                   << "Client       : " << nomClient << "\n"
                   << "Utilisateur  : " << nomUtilisateur << "\n"
                   << "Prix         : " << prix << "\n"
                   << "Paiement     : " << mode_paiement << "\n"
                   << "Status       : " << status << "\n"
                   << "Date         : " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n"
                   << "-----------------------------\n";
            file.close();
        }


        QSqlQuery query;
        query.prepare("SELECT NOM,PRENOM,TELEPHONE FROM CLIENT WHERE ID_CLIENT=:id");
        query.bindValue(":id",QString::number(idClient));
        if(query.exec() && query.next())
        {
            QString nom=query.value(0).toString();
            QString prenom=query.value(1).toString();
            QString telephone = query.value(2).toString();

            TwilioManager sms;
            QString message = "Merci Mr/Mme"+prenom+" "+nom+" pour Votre Confiance , votre commande a été effectuer avec succes num_telephone:"+telephone;
           // sms.sendSMS(message);

        }

    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                              QObject::tr("Ajout échoué"), QMessageBox::Cancel);
    }
}


void CommandeWindow::on_tableView_commande_clicked(const QModelIndex &index)
{

        //etape1: numero du ligne ! clicked !
        QAbstractItemModel* model = ui->tableView_commande->model();

        int row = index.row();
        //etape2:  table du format matrice , recuperation du donne
        QString id_commande= model->data(model->index(row, 0)).toString();
        int id_client = model->data(model->index(row, 1)).toInt();
        int id_utilisateur = model->data(model->index(row, 2)).toInt();
        QString statut = model->data(model->index(row, 4)).toString();
        QString  mode_paiement= model->data(model->index(row, 5)).toString();
        QString montant_totale = model->data(model->index(row, 6)).toString();

        //etape3: insertion dans ui  ( lineEdit , comboBox)

        ui->combo_client_commande->setCurrentText(co.chercherFullNameDapresIdClient(id_client));

        ui->combo_utilisateur_commande->setCurrentText(co.chercherFullNameDapresIdUtilisateur(id_utilisateur));

        ui->combo_status_commande->setCurrentText(statut);
        ui->prix_totale_commande->setText(montant_totale);

        ui->id_commande->setText(id_commande);


}


void CommandeWindow::on_bt_modifier_commande_clicked()
{
        //récuperation d'apres ui !
        QString nomClient=ui->combo_client_commande->currentText();
        QString nomUtilisateur=ui->combo_utilisateur_commande->currentText();
        int prix=ui->prix_totale_commande->text().toInt();
        QString mode_paiement=ui->combo_mode_paiement_commande->currentText();
        QString status=ui->combo_status_commande->currentText();

        int idClient=co.chercherIdClientDapreFullName(nomClient);
        int idUtilisateur=co.chercherIdUtilisateurDapreFullName(nomUtilisateur);
        QDate date_commande=QDate(QDate::currentDate());

        int id_commande=ui->id_commande->text().toInt();

        //controle de saisie !
        if(prix<0)
        {
            QMessageBox::critical(nullptr, QObject::tr(""),
                                  QObject::tr("Le prix doit etre positif"), QMessageBox::Cancel);
            return;
        }



        // appel du fonction du ajouter d'apres le classe

        commande c(id_commande,idClient,idUtilisateur,date_commande, status,
                   mode_paiement,
                   prix);

        bool test = c.modifier();


        //affichage du resultat ,

            if(test)
        {
            QMessageBox::information(nullptr, QObject::tr(""),
                                     QObject::tr("Modification avec succées"), QMessageBox::Cancel);
            ui->tableView_commande->setModel(co.afficher());

            ui->prix_totale_commande->clear();
            ui->id_commande->clear();



            QFile file("C:/Users/amins/Desktop/projet/sans_titre1 (3)/sans_titre1/modification.txt");
            if (file.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream stream(&file);
                stream << "[UPDATE COMMANDE]\n"
                       << "Id Commande       : " << id_commande << "\n"
                       << "Client       : " << nomClient << "\n"
                       << "Utilisateur  : " << nomUtilisateur << "\n"
                       << "Prix         : " << prix << "\n"
                       << "Paiement     : " << mode_paiement << "\n"
                       << "Status       : " << status << "\n"
                       << "Date         : " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n"
                       << "-----------------------------\n";
                file.close();
            }


        }
        else
        {
            QMessageBox::critical(nullptr, QObject::tr(""),
                                  QObject::tr("Modification échoué"), QMessageBox::Cancel);
        }


}

void CommandeWindow::on_bt_effacer_commande_clicked()
{
    int id_commande=ui->id_commande->text().toInt();

    if(id_commande==0)
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                              QObject::tr("Tu dois choisir une ligne d'apres le tableau pour supprimer "), QMessageBox::Cancel);
        return;
    }
    bool test = co.supprimer(id_commande);

    if(test)
    {
        QMessageBox::information(nullptr, QObject::tr(""),
                                 QObject::tr("Suppresion avec succées"), QMessageBox::Cancel);
        ui->tableView_commande->setModel(co.afficher());

        ui->prix_totale_commande->clear();
        ui->id_commande->clear();




        QFile file("C:/Users/amins/Desktop/projet/sans_titre1 (3)/sans_titre1/supression.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "[DELETED COMMANDE]\n"
                   << "Id Commande       : " << id_commande << "\n"
                   << "Date         : " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n"
                   << "-----------------------------\n";
            file.close();
        }

    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr(""),
                              QObject::tr("Suppresion échoué"), QMessageBox::Cancel);
    }
}


void CommandeWindow::on_lineEditSearch_textChanged(const QString &text)
{
    QString column=ui->column->currentText();
    ui->tableView_commande->setModel(co.recherche(column,text));
}



void CommandeWindow::on_btnSearch_clicked()
{
    QString column=ui->column->currentText();
    QString ordre=ui->ordre->currentText();
    ui->tableView_commande->setModel(co.tri(column,ordre));

}





void CommandeWindow::on_pdf_clicked()
{
    QString strStream;
    QTextStream out(&strStream);

    const int rowCount = ui->tableView_commande->model()->rowCount();
    const int columnCount = ui->tableView_commande->model()->columnCount();

    out << "<html>\n"
           "<head>\n"
           "<meta Content=\"Text/html; charset=Windows-1251\">\n"
           "<title>%1</title>\n"
           "<style>\n"
           "table {\n"
           "    width: 100%;\n"
           "    border-collapse: collapse;\n"
           "}\n"
           "th, td {\n"
           "    padding: 8px;\n"
           "    text-align: left;\n"
           "    border-bottom: 1px solid #ddd;\n"
           "}\n"
           "tr:nth-child(even) {\n"
           "    background-color: #f2f2f2;\n"
           "}\n"
           "</style>\n"
           "</head>\n"
           "<body bgcolor=#ffffff link=#5000A0>\n"
           "<center> <H1>Liste des Commandes</H1></center><br/><br/>\n"
           "<img src=\"path/to/your/image.jpg\" alt=\"Description of image\" style=\"max-width: 100%; height: auto;\">\n"
           "<table>\n";

    // headers
    out << "<thead><tr bgcolor=#f0f0f0> <th>Numero</th>";
    for (int column = 0; column < columnCount; column++)
    {
        if (!ui->tableView_commande->isColumnHidden(column))
        {
            out << QString("<th>%1</th>").arg(ui->tableView_commande->model()->headerData(column, Qt::Horizontal).toString());
        }
    }
    out << "</tr></thead>\n";

    // data table
    for (int row = 0; row < rowCount; row++)
    {
        out << "<tr> <td>" << row + 1 << "</td>";
        for (int column = 0; column < columnCount; column++)
        {
            if (!ui->tableView_commande->isColumnHidden(column))
            {
                QString data = ui->tableView_commande->model()->data(ui->tableView_commande->model()->index(row, column)).toString().simplified();
                out << QString("<td>%1</td>").arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }


    QString fileName = QFileDialog::getSaveFileName((QWidget *)0, "Sauvegarder en PDF", QString(), "*.pdf");
    if (QFileInfo(fileName).suffix().isEmpty())
    {
        fileName.append(".pdf");
    }

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(strStream);//out
    doc.print(&printer);//fileName
}


void CommandeWindow::on_stat_clicked()
{
    QMap<QString, int> stats = co.statistiquesParModePaiement();

    QPieSeries *series = new QPieSeries();
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        series->append(it.key(), it.value());
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Statistiques des Clients par Sexe");

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QDialog *chartDialog = new QDialog(this);
    chartDialog->setWindowTitle("Graphique des Statistiques");
    chartDialog->setFixedSize(480, 240);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(chartView);
    chartDialog->setLayout(layout);

    chartDialog->exec();

}


void CommandeWindow::on_btnRefresh_clicked()
{
    ui->tableView_commande->setModel(co.afficher());


}


void CommandeWindow::on_Historique_clicked()
{


        QString selectedOption = ui->combo_historique->currentText();
        QString filePath;
        QString modelHeader;
        if (selectedOption == "Ajout") {
            filePath = "C:/Users/amins/Desktop/projet/sans_titre1 (3)/sans_titre1/ajout.txt";
            modelHeader = "AJOUT";
        } else if (selectedOption == "Modifier") {
            filePath = "C:/Users/amins/Desktop/projet/sans_titre1 (3)/sans_titre1/modification.txt";
            modelHeader = "MODIFICATION";
        } else if (selectedOption == "Supprimer") {
            filePath = "C:/Users/amins/Desktop/projet/sans_titre1 (3)/sans_titre1/supression.txt";
            modelHeader = "SUPPRESSION";
        }

        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            QString fileContent = stream.readAll();
            file.close();

            if (fileContent.isEmpty()) {
                QMessageBox::information(this, "Empty", "The file is empty.");
            } else {
                QStringList rows = fileContent.split("\n");
                QStandardItemModel *model = new QStandardItemModel(this);
                model->setColumnCount(1);

                for (const QString &row : rows) {
                    QStandardItem *item = new QStandardItem(row);
                    model->appendRow(item);
                }

                model->setHeaderData(0, Qt::Horizontal, QObject::tr(modelHeader.toUtf8().constData()));

                ui->tableView_commande->setModel(model);
                ui->tableView_commande->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
                ui->tableView_commande->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            }
        } else {
            QMessageBox::warning(this, "Error", "Failed to open the file.");
        }


}

