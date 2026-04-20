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
    void showListeCommandes() {}
    void loadCommandesFromClients() {}
    void onFilterCommandes() {}
    void onRefreshCommandes() {}

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

    QButtonGroup *statutGroup;
    QRadioButton *radioEnCours;
    QRadioButton *radioTermine;
    QRadioButton *radioEnAttente;

    QTableWidget *tableClients;

    QLineEdit *searchEdit;
    QComboBox *sortComboBox;

    QLabel *statsClientTotal;
    QLabel *statsEnCours;
    QLabel *statsTerminees;
    QTableWidget *statsTable;

    QPushButton *btnListeClients;
    QPushButton *btnStatistiques;
    QPushButton *btnDeconnexion;
};

#endif // MAINWINDOW_H
