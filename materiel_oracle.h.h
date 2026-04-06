#ifndef MATERIEL_ORACLE_H
#define MATERIEL_ORACLE_H
#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>
#include <QDateEdit>
#include <QStatusBar>
#include <QDate>
#include <QTimer>

class MaterielOracle : public QMainWindow
{
    Q_OBJECT

public:
    explicit MaterielOracle(QWidget *parent = nullptr);
    ~MaterielOracle();

private slots:
    void ajouterMateriel();
    void modifierMateriel();
    void supprimerMateriel();
    void rechercherMateriel();
    void actualiserTable();
    void viderChamps();
    void onTableSelectionChanged();
    void exporterCSV();
    void importerCSV();
    void afficherStatistiques();

private:
    void setupUI();
    void setupConnections();
    void updateStatusBar();
    bool initDatabase();
    void loadData();
    void afficherMessage(const QString& message, bool erreur = false);
    bool verifierConnexion();
    void creerTableSiExiste();
    void insererDonneesExemple();

    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *topLayout;
    QHBoxLayout *contentLayout;
    QVBoxLayout *formLayout;
    QVBoxLayout *tableLayout;

    // Formulaire
    QLineEdit *idEdit;
    QLineEdit *nomEdit;
    QComboBox *typeBox;
    QLineEdit *quantiteEdit;
    QLineEdit *seuilEdit;
    QDateEdit *dateEdit;
    QLineEdit *fournisseurEdit;
    QComboBox *statutBox;

    // Boutons
    QPushButton *ajouterBtn;
    QPushButton *modifierBtn;
    QPushButton *supprimerBtn;
    QPushButton *actualiserBtn;
    QPushButton *viderBtn;
    QPushButton *rechercherBtn;
    QPushButton *exporterBtn;
    QPushButton *importerBtn;
    QPushButton *statistiquesBtn;

    // Recherche
    QLineEdit *searchEdit;

    // Tableau
    QTableView *tableView;
    QSqlTableModel *model;

    // Status
    QStatusBar *statusBar;
    QLabel *statusLabel;
    QLabel *connexionLabel;

    // Database
    bool dbConnected;
};

#endif // MATERIEL_ORACLE_H
