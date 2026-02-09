#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void addMaterial();
    void deleteMaterial();
    void clearFields();
    void modifyMaterial();
    void searchMaterial();
    void changePage(int index);
    void exportToPdf();
    void showStatistics();
    void sortMaterial();

private:
    void setupUsersPage();
    void setupMaterialPage();
    void setupClientsPage();
    void setupOrdersPage();
    void updateMaterialTable();
    QPushButton* createStyledButton(const QString& text, const QString& color = "#3498db");

    QListWidget *navList;
    QStackedWidget *pagesWidget;

    QLineEdit *idEdit;
    QLineEdit *nameEdit;
    QLineEdit *quantityEdit;
    QLineEdit *thresholdEdit;
    QLineEdit *supplierEdit;
    QLineEdit *searchEdit;
    QComboBox *typeBox;
    QComboBox *statusBox;
    QComboBox *sortComboBox;
    QDateEdit *dateEdit;
    QTableWidget *materialTable;
    QPushButton *modifyBtn;
    QPushButton *exportBtn;
    QPushButton *statsBtn;
    QPushButton *sortBtn;

    QWidget *usersPage;
    QWidget *clientsPage;
    QWidget *ordersPage;

    struct Material {
        int id;
        QString name;
        QString type;
        int quantity;
        int threshold;
        QDate addedDate;
        QString supplier;
        QString status;
    };

    struct User {
        int id;
        QString name;
        QString email;
        QString role;
        QString status;
        QDate creationDate;
    };

    QList<Material> materialsList;
    QList<User> usersList;
    int nextId;
    int nextUserId;

    // User Management UI members
    QLineEdit *userNameEdit;
    QLineEdit *userEmailEdit;
    QLineEdit *userPasswordEdit;
    QLineEdit *userIdEdit;
    QLineEdit *userSearchEdit;
    QComboBox *userRoleBox;
    QComboBox *userStatusBox;
    QDateEdit *userCreationDateEdit;
    QTableWidget *usersTable;
    QPushButton *userModifyBtn;

    void updateUsersTable();
    void addUser();
    void deleteUser();
    void modifyUser();
    void searchUser();
    void sortUsersByDate();
    void sortUsersByName();

#endif
