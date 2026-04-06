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
#include <QSet>
#include <QColor>
#include <QBrush>
#include <QDebug>

// ========== INCLUDES POUR PDF ==========
#include <QPrinter>
#include <QTextDocument>
#include <QFileDialog>
#include <QDateTime>
#include <QPageLayout>
#include <QPageSize>
#include <QTextStream>
// =======================================

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
    void changePage(int index);
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
    void updateBlinkingState();      // Pour le clignotement rouge
    void checkLowStock();            // Vérifie les stocks critiques
    void testBlinking();             // Pour tester le clignotement
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

    // ========== METHODES POUR PDF ==========
    QString generateHtmlReport();
    // =======================================

    // Material members
    QList<Material> materialsList;
    QTableWidget* materialTable;
    QLineEdit *idEdit, *nameEdit, *quantityEdit, *thresholdEdit, *supplierEdit;
    QComboBox *typeBox, *statusBox;
    QDateEdit *dateEdit;
    QLineEdit *searchEdit;
    QComboBox *sortComboBox;
    QPushButton *modifyBtn, *exportBtn, *statsBtn, *qrBtn, *importBtn, *saveBtn;

    // Timers pour animations
    QTimer *blinkTimer;           // Timer pour clignotement (300ms)
    QTimer *autoSaveTimer;        // Timer pour sauvegarde auto
    QTimer *lowStockCheckTimer;   // Timer pour vérifier stock critique (3 secondes)

    bool blinkState;              // État du clignotement (true/false)
    QSet<int> lowStockIds;        // Liste des IDs en stock critique

    int nextId;
    bool dataModified;

    QStackedWidget* pagesWidget;
    QListWidget* navList;
    QStatusBar* m_statusBar;
    QLabel* statusLabel;
    QLabel* statsLabel;

    QMap<QLineEdit*, QPropertyAnimation*> fieldAnimations;

    static const QString APP_VERSION;
    static const QString APP_NAME;
    static const int AUTO_SAVE_INTERVAL;
};

#endif // MATRIELE_H
