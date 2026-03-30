#ifndef MATRIELE_H
#define MATRIELE_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QStatusBar>
#include <QTimer>
#include <QMap>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QList>

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

class Matriele : public QMainWindow {
    Q_OBJECT

public:
    explicit Matriele(QWidget *parent = nullptr);
    ~Matriele();

private slots:
    void addMaterial();
    void modifyMaterial();
    void deleteMaterial();
    void searchMaterial();
    void sortMaterial();
    void clearMaterialFields();
    void exportToPdf();
    void exportToCsv();
    void showStatistics();
    void generateQRCode();
    void updateBlinkingState();
    void importFromFile();
    void saveData();
    void loadData();
    void animateField();
    void animateButton();

private:
    void setupMaterialPage();
    void setupStatusBar();
    void setupShortcuts();
    void updateMaterialTable();
    void updateStatusBar();
    QPushButton* createStyledButton(const QString& text, const QString& color);
    bool validateMaterialFields();
    bool isValidName(const QString& name);
    void showNotification(const QString& message, bool isError = false);

    // Material members
    QList<Material> materialsList;
    QTableWidget* materialTable;
    QLineEdit *idEdit, *nameEdit, *quantityEdit, *thresholdEdit, *supplierEdit;
    QComboBox *typeBox, *statusBox;
    QDateEdit *dateEdit;
    QLineEdit *searchEdit;
    QComboBox *sortComboBox;
    QPushButton *modifyBtn, *exportBtn, *statsBtn, *qrBtn, *importBtn, *saveBtn;
    QTimer *blinkTimer;
    QTimer *autoSaveTimer;
    bool blinkState;
    int nextId;
    bool dataModified;

    // UI members
    QStackedWidget* pagesWidget;
    QStatusBar* m_statusBar;
    QLabel* statusLabel;
    QLabel* statsLabel;

    // Animation
    QMap<QLineEdit*, QPropertyAnimation*> fieldAnimations;

    // Constantes
    static const QString APP_VERSION;
    static const QString APP_NAME;
    static const int AUTO_SAVE_INTERVAL;
};

#endif // MATRIELE_H
