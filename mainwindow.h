#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QDateEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include "client.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showListeClients();
    void showStatistiques();
    void onDeconnexion();
    void onAjouterClientSubmit();
    void onAnnulerAjout();
    void onEditClient(int row);
    void onDeleteClient(int row);
    void onSearchClient();
    void onSortClients();
    void onExportPDF();
    void onExportExcel();
    void onShowGraphique();

private:
    void createSidebar();
    void createListeClientsPage();
    void createStatistiquesPage();
    QWidget* createActionButtons(int row);
    void refreshClientTable();
    void updateStatistiques();

    // Stacked pages
    QStackedWidget *stackedWidget;
    QWidget *pageListeClients;
    QWidget *pageStatistiques;

    // Formulaire
    QLineEdit *idEdit;
    QLineEdit *nomEdit;
    QLineEdit *prenomEdit;
    QLineEdit *emailEdit;
    QLineEdit *telEdit;
    QLineEdit *adresseEdit;
    QDateEdit *dateEdit;
    QButtonGroup *statutGroup;
    QRadioButton *radioEnCours;
    QRadioButton *radioTermine;
    QRadioButton *radioEnAttente;

    // Table
    QTableWidget *tableClients;
    QLineEdit *searchEdit;
    QComboBox *sortComboBox;

    // Stats
    QLabel *statsClientTotal;
    QLabel *statsEnCours;
    QLabel *statsTerminees;
    QTableWidget *statsTable;

    // Sidebar boutons
    QPushButton *btnListeClients;
    QPushButton *btnStatistiques;
    QPushButton *btnDeconnexion;

    // Modèle Client (comme "Etudiant Etmp" dans le cours de la prof)
    Client Ctmp;

    // Variables pour gérer le mode Ajout / Modification
    int editingId;
    bool isEditing;
};

#endif // MAINWINDOW_H
