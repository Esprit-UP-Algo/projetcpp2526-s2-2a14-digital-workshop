#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QDate>

class QLineEdit;
class QComboBox;
class QTableWidget;
class QPushButton;
class QListWidget;
class QLabel;
class QDateEdit;
class QSpinBox;

class UserWindow : public QMainWindow
{
    Q_OBJECT

public:
    UserWindow(QWidget *parent = nullptr);

private slots:
    // Fonctions pour la page Utilisateurs
    void addUser();
    void deleteUser();
    void clearFields();
    void modifyUser();
    void searchUser();
    void changePage(int index);
    void sortUsersByDate();
    void sortUsersByName();

    // Fonctions pour la page Matériel
    void addMaterial();
    void deleteMaterial();
    void modifyMaterial();
    void clearMaterialFields();
    void searchMaterial();
    void exportMaterials();
    void addExampleMaterials();

private:
    void setupUsersPage();
    void setupMaterialPage();
    void setupClientsPage();
    void setupOrdersPage();
    void updateUsersTable();
    void updateMaterialsTable();
    QPushButton* createStyledButton(const QString& text, const QString& color = "#3498db");

    // Navigation
    QListWidget *navList;
    QStackedWidget *pagesWidget;

    // Page Utilisateurs (FONCTIONNELLE)
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QLineEdit *idEdit;
    QLineEdit *searchEdit;
    QComboBox *roleBox;
    QComboBox *statusBox;
    QDateEdit *creationDateEdit;
    QTableWidget *usersTable;
    QPushButton *modifyBtn;

    // Page Matériel
    QWidget *materialPage;
    QLineEdit *materialSearchEdit;
    QLineEdit *materialIdEdit;
    QLineEdit *materialNameEdit;
    QLineEdit *materialSupplierEdit;
    QComboBox *materialTypeBox;
    QComboBox *materialStatusBox;
    QSpinBox *materialQuantityEdit;
    QDateEdit *materialDateEdit;
    QTableWidget *materialTable;
    QPushButton *materialModifyBtn;
    QLabel *totalMaterialsLabel;

    // Pages vides (à compléter par vos collègues)
    QWidget *clientsPage;
    QWidget *ordersPage;

    struct User {
        int id;
        QString name;
        QString email;
        QString role;
        QString status;
        QDate creationDate;
    };

    struct Material {
        int id;
        QString name;
        QString type;
        int quantity;
        QString supplier;
        QDate acquisitionDate;
        QString status;
    };

    QList<User> usersList;
    QList<Material> materialsList;
    int nextId;
    int nextMaterialId;
};

#endif
