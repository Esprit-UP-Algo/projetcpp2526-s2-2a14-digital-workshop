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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showListeClients();
    void showListeCommandes();
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
    void onFilterCommandes();
    void onRefreshCommandes();
    void onShowGraphique();

private:
    void createSidebar();
    void createListeClientsPage();
    void createListeCommandesPage();
    void createStatistiquesPage();
    QWidget* createActionButtons(int row);
    void refreshClientTable();
    void updateStatistiques();
    void loadCommandesFromClients();

    QStackedWidget *stackedWidget;

    QWidget *pageListeClients;
    QWidget *pageStatistiques;

    QLineEdit *idEdit;
    QLineEdit *nomEdit;
    QLineEdit *prenomEdit;
    QLineEdit *emailEdit;
    QLineEdit *telEdit;
    QLineEdit *adresseEdit;
    QDateEdit *dateEdit;
    QLineEdit *commandeEdit;

    QButtonGroup *statutGroup;
    QRadioButton *radioEnCours;
    QRadioButton *radioTermine;
    QRadioButton *radioEnAttente;

    QTableWidget *tableClients;
    QTableWidget *tableCommandes;

    QLineEdit *searchEdit;
    QComboBox *sortComboBox;
    QComboBox *filterStatutComboBox;

    QLabel *statsClientTotal;
    QLabel *statsEnCours;
    QLabel *statsTerminees;
    QTableWidget *statsTable;

    QPushButton *btnListeClients;
    QPushButton *btnListeCommandes;
    QPushButton *btnStatistiques;
    QPushButton *btnDeconnexion;
};

#endif // MAINWINDOW_H
