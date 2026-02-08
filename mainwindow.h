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

    QList<Material> materialsList;
    int nextId;
};

#endif
