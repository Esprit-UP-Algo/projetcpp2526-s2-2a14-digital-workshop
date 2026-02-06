#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QDateEdit>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showListeClients();
    void showListeCommandes();
    void showAjouterClient();
    void onDeconnexion();
    void onAjouterClientSubmit();
    void onAnnulerAjout();
    void onEditClient(int row);
    void onDeleteClient(int row);
    void onSearchClient();
    void onSortClients();
    void onExportPDF();
    void onExportExcel();

private:
    void createSidebar();
    void createListeClientsPage();
    void createListeCommandesPage();
    void createAjouterClientPage();
    QWidget* createActionButtons(int row);
    void refreshClientTable();

    // Widgets principaux
    QStackedWidget *stackedWidget;

    // Pages
    QWidget *pageListeClients;
    QWidget *pageListeCommandes;
    QWidget *pageAjouterClient;

    // Formulaire ajout client
    QLineEdit *idEdit;
    QLineEdit *nomEdit;
    QLineEdit *prenomEdit;
    QLineEdit *emailEdit;
    QLineEdit *telEdit;
    QLineEdit *adresseEdit;
    QDateEdit *dateEdit;  // Calendrier
    QLineEdit *commandeEdit;
    QLineEdit *statutEdit;

    // Tableau clients
    QTableWidget *tableClients;

    // Recherche et tri
    QLineEdit *searchEdit;
    QComboBox *sortComboBox;

    // Boutons sidebar
    QPushButton *btnListeClients;
    QPushButton *btnListeCommandes;
    QPushButton *btnAjouterClient;
    QPushButton *btnDeconnexion;
};

#endif // MAINWINDOW_H
