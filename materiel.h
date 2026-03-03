#ifndef MATERIEL_H
#define MATERIEL_H

#include <QWidget>
#include <QDate>
#include <QList>

class QLineEdit;
class QComboBox;
class QTableWidget;
class QPushButton;
class QDateEdit;
class QSpinBox;
class QLabel;

struct Material {
    int id;
    QString name;
    QString type;
    int quantity;
    int minThreshold;
    QString supplier;
    QDate acquisitionDate;
    QString status;
};

class Materiel : public QWidget
{
    Q_OBJECT

public:
    explicit Materiel(QWidget *parent = nullptr);

    void loadMaterialsFromDatabase();
    void saveMaterialToDatabase(const Material &material);
    void deleteMaterialFromDatabase(int id);
    void updateMaterialInDatabase(const Material &material);

private slots:
    void addMaterial();
    void deleteMaterial();
    void modifyMaterial();
    void clearMaterialFields();
    void searchMaterial();
    void exportMaterials();
    void addExampleMaterials();
    void sortMaterialsByQuantity();
    void sortMaterialsByType();
    void sortMaterialsByName();
    void sortMaterialsByStatus();
    void checkMaterialAlerts();

private:
    void setupUI();
    void updateMaterialsTable();
    QPushButton* createStyledButton(const QString& text, const QString& color = "#3498db");

    QLineEdit *materialSearchEdit;
    QLineEdit *materialIdEdit;
    QLineEdit *materialNameEdit;
    QLineEdit *materialSupplierEdit;
    QComboBox *materialTypeBox;
    QComboBox *materialStatusBox;
    QSpinBox *materialQuantityEdit;
    QSpinBox *materialMinThresholdEdit;
    QDateEdit *materialDateEdit;
    QTableWidget *materialTable;
    QPushButton *materialModifyBtn;
    QLabel *totalMaterialsLabel;
    QLabel *alertMaterialsLabel;
    QLabel *ruptureMaterialsLabel;

    QList<Material> materialsList;
    int nextMaterialId;
    bool databaseEnabled;
};

#endif // MATERIEL_H
