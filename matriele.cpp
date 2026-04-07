#include "matriele.h"
#include "qrcodegen.hpp"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QShortcut>
#include <QPainter>
#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QDateTime>

const QString Matriele::APP_VERSION = "2.0.0";
const QString Matriele::APP_NAME = "Gestion Matériel";
const int Matriele::AUTO_SAVE_INTERVAL = 300000;

// ==================== CONSTRUCTEUR ====================

Matriele::Matriele(QWidget *parent) : QMainWindow(parent),
    nextId(1), blinkState(false), dataModified(false)
{
    setWindowTitle(QString("%1 v%2").arg(APP_NAME).arg(APP_VERSION));
    setMinimumSize(1200, 700);

    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #f5f6fa;");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QFrame *sidebar = new QFrame;
    sidebar->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "stop:0 #2c3e50, stop:1 #34495e); border: none; }"
        "QLabel { color: white; font-size: 16px; font-weight: bold; }"
        "QListWidget { background: transparent; border: none; color: #ecf0f1; font-size: 14px; }"
        "QListWidget::item { padding: 15px 20px; border-bottom: 1px solid rgba(255,255,255,0.1); }"
        "QListWidget::item:selected { background-color: #3498db; border-left: 4px solid #2980b9; }"
        "QListWidget::item:hover { background-color: rgba(52, 152, 219, 0.3); }");
    sidebar->setFixedWidth(260);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    QLabel *appTitle = new QLabel("🏢 " + APP_NAME);
    appTitle->setAlignment(Qt::AlignCenter);
    appTitle->setStyleSheet("font-size: 18px; padding: 25px;");
    sidebarLayout->addWidget(appTitle);

    navList = new QListWidget;
    navList->addItem("🛠️  Matériel");
    navList->setCurrentRow(0);
    connect(navList, &QListWidget::currentRowChanged, this, &Matriele::changePage);

    sidebarLayout->addWidget(navList);
    sidebarLayout->addStretch();

    QLabel *footer = new QLabel(QString("v%1 - © 2024").arg(APP_VERSION));
    footer->setStyleSheet("color: #7f8c8d; font-size: 10px; padding: 15px;");
    footer->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(footer);

    pagesWidget = new QStackedWidget;

    setupMaterialPage();
    setupStatusBar();
    setupShortcuts();

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pagesWidget, 1);

    // Charger les données ou créer des données de test
    if (materialsList.isEmpty()) {
        // Données de test avec stocks critiques
        Material mat1 = {1, "Poutre en chêne", "Bois", 3, 10, QDate::currentDate(), "BoisCorp", "Stock critique"};
        Material mat2 = {2, "Marteau perforateur", "Outil", 15, 5, QDate(2024, 1, 15), "TechTools", "Disponible"};
        Material mat3 = {3, "Vis à bois 5x60", "Consommable", 2, 10, QDate(2024, 2, 20), "FixPro", "Stock critique"};
        Material mat4 = {4, "Perceuse électrique", "Électrique", 8, 3, QDate(2024, 3, 10), "Bosch", "Disponible"};
        Material mat5 = {5, "Câble HDMI", "Électronique", 0, 5, QDate(2024, 3, 15), "CablePro", "Rupture"};
        materialsList << mat1 << mat2 << mat3 << mat4 << mat5;
        nextId = 6;
    }

    updateMaterialTable();

    // ========== TIMERS POUR CLIGNOTEMENT ROUGE ==========
    // Timer pour la sauvegarde auto
    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &Matriele::saveData);
    autoSaveTimer->start(AUTO_SAVE_INTERVAL);

    // Timer pour le clignotement (toutes les 300ms pour un effet visible)
    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &Matriele::updateBlinkingState);
    blinkTimer->start(300);

    // Timer pour vérifier le stock critique (toutes les 3 secondes)
    lowStockCheckTimer = new QTimer(this);
    connect(lowStockCheckTimer, &QTimer::timeout, this, &Matriele::checkLowStock);
    lowStockCheckTimer->start(3000);

    // Vérification initiale des stocks
    checkLowStock();

    // Forcer le premier clignotement après 100ms
    QTimer::singleShot(100, this, &Matriele::updateBlinkingState);
    // ===================================================

    updateStatusBar();
}

Matriele::~Matriele() {
    qDeleteAll(fieldAnimations);
}

// ==================== SETUP UI ====================

void Matriele::setupMaterialPage() {
    QWidget *materialPage = new QWidget;
    materialPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(materialPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    // Header
    QFrame *headerFrame = new QFrame;
    headerFrame->setObjectName("headerFrame");
    headerFrame->setStyleSheet("#headerFrame { background-color: white; border-radius: 15px; padding: 20px; }");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);

    QLabel *titleLabel = new QLabel("📦 GESTION MATÉRIELLE");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);

    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->setSpacing(15);

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 Rechercher par nom, type ou fournisseur...");
    searchEdit->setStyleSheet(
        "QLineEdit { background-color: #f8f9fa; border: 2px solid #e9ecef; border-radius: 10px;"
        "padding: 12px 15px; font-size: 14px; color: #2c3e50; }"
        "QLineEdit:focus { border-color: #3498db; }");
    searchEdit->setMinimumWidth(300);
    connect(searchEdit, &QLineEdit::textChanged, this, &Matriele::searchMaterial);

    sortComboBox = new QComboBox;
    sortComboBox->addItems({"📊 Trier par ID (croissant)", "📊 Trier par ID (décroissant)",
                            "📈 Trier par quantité (croissant)", "📈 Trier par quantité (décroissant)",
                            "🏷️ Trier par type", "🔤 Trier par nom"});
    sortComboBox->setStyleSheet(
        "QComboBox { background-color: #f8f9fa; border: 2px solid #e9ecef; border-radius: 10px;"
        "padding: 10px 15px; font-size: 13px; min-width: 220px; color: #2c3e50; }"
        "QComboBox:hover { border-color: #3498db; }");

    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(sortComboBox);
    searchLayout->addStretch();
    headerLayout->addLayout(searchLayout);

    // Action buttons
    QHBoxLayout *actionButtonsLayout = new QHBoxLayout;
    actionButtonsLayout->setSpacing(12);

    QPushButton *addBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    modifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *deleteBtn = createStyledButton("🗑️ Supprimer", "#e74c3c");

    actionButtonsLayout->addWidget(addBtn);
    actionButtonsLayout->addWidget(modifyBtn);
    actionButtonsLayout->addWidget(deleteBtn);
    actionButtonsLayout->addStretch();

    exportBtn = createStyledButton("📄 Exporter PDF", "#9b59b6");
    statsBtn = createStyledButton("📈 Statistiques", "#1abc9c");
    qrBtn = createStyledButton("📱 Code QR", "#34495e");
    QPushButton *sortBtn = createStyledButton("🔄 Trier", "#f39c12");
    saveBtn = createStyledButton("💾 Sauvegarder", "#27ae60");
    QPushButton *testBlinkBtn = createStyledButton("🔴 Test Clignotement", "#e74c3c");

    actionButtonsLayout->addWidget(exportBtn);
    actionButtonsLayout->addWidget(statsBtn);
    actionButtonsLayout->addWidget(qrBtn);
    actionButtonsLayout->addWidget(sortBtn);
    actionButtonsLayout->addWidget(saveBtn);
    actionButtonsLayout->addWidget(testBlinkBtn);

    headerLayout->addLayout(actionButtonsLayout);
    pageLayout->addWidget(headerFrame);

    // Main content
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(25);

    // Form
    QFrame *formFrame = new QFrame;
    formFrame->setObjectName("formFrame");
    formFrame->setStyleSheet("#formFrame { background-color: white; border-radius: 15px; padding: 25px; }");
    formFrame->setFixedWidth(450);

    QVBoxLayout *formLayout = new QVBoxLayout(formFrame);

    QLabel *formTitle = new QLabel("📝 Formulaire matériel");
    formTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #3498db; padding-bottom: 15px;");
    formLayout->addWidget(formTitle);

    QString inputStyle =
        "QLineEdit, QComboBox, QDateEdit { background-color: #f8f9fa; border: 2px solid #e9ecef;"
        "border-radius: 8px; padding: 10px 12px; font-size: 13px; color: #2c3e50; min-height: 25px; }"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border-color: #3498db; }"
        "QLineEdit:read-only { background-color: #e9ecef; color: #6c757d; }";

    idEdit = new QLineEdit;
    idEdit->setReadOnly(true);
    idEdit->setStyleSheet(inputStyle);

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Ex: Poutre en chêne");
    nameEdit->setStyleSheet(inputStyle);
    connect(nameEdit, &QLineEdit::textChanged, this, &Matriele::animateField);

    typeBox = new QComboBox;
    typeBox->addItems({"Bois", "Outil", "Consommable", "Électrique", "Électronique", "Métal", "Plastique"});
    typeBox->setStyleSheet(inputStyle);

    quantityEdit = new QLineEdit;
    quantityEdit->setPlaceholderText("Ex: 50");
    quantityEdit->setStyleSheet(inputStyle);
    quantityEdit->setValidator(new QIntValidator(0, 999999, this));
    connect(quantityEdit, &QLineEdit::textChanged, this, &Matriele::animateField);

    thresholdEdit = new QLineEdit;
    thresholdEdit->setPlaceholderText("Ex: 10");
    thresholdEdit->setStyleSheet(inputStyle);
    thresholdEdit->setValidator(new QIntValidator(0, 999999, this));
    connect(thresholdEdit, &QLineEdit::textChanged, this, &Matriele::animateField);

    dateEdit = new QDateEdit;
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setStyleSheet(inputStyle);

    supplierEdit = new QLineEdit;
    supplierEdit->setPlaceholderText("Ex: BoisCorp");
    supplierEdit->setStyleSheet(inputStyle);
    connect(supplierEdit, &QLineEdit::textChanged, this, &Matriele::animateField);

    statusBox = new QComboBox;
    statusBox->addItems({"Disponible", "Rupture", "Stock critique"});
    statusBox->setStyleSheet(inputStyle);

    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(15);
    inputForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    inputForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    QLabel *idLabel = new QLabel("🔑 ID:");
    idLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *nameLabel = new QLabel("🏷️ Nom *:");
    nameLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *typeLabel = new QLabel("📋 Type:");
    typeLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *quantityLabel = new QLabel("🔢 Quantité *:");
    quantityLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *thresholdLabel = new QLabel("⚠️ Seuil *:");
    thresholdLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *dateLabel = new QLabel("📅 Date d'ajout:");
    dateLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *supplierLabel = new QLabel("🏢 Fournisseur *:");
    supplierLabel->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
    QLabel *statusLabelForm = new QLabel("📊 Statut:");
    statusLabelForm->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");

    inputForm->addRow(idLabel, idEdit);
    inputForm->addRow(nameLabel, nameEdit);
    inputForm->addRow(typeLabel, typeBox);
    inputForm->addRow(quantityLabel, quantityEdit);
    inputForm->addRow(thresholdLabel, thresholdEdit);
    inputForm->addRow(dateLabel, dateEdit);
    inputForm->addRow(supplierLabel, supplierEdit);
    inputForm->addRow(statusLabelForm, statusBox);

    formLayout->addLayout(inputForm);
    formLayout->addStretch();

    QLabel *infoNote = new QLabel("⚠️ Les champs avec * sont obligatoires");
    infoNote->setStyleSheet("color: #e74c3c; font-size: 11px; padding-top: 15px; font-weight: bold;");
    formLayout->addWidget(infoNote);

    contentLayout->addWidget(formFrame);

    // Table
    QFrame *tableFrame = new QFrame;
    tableFrame->setObjectName("tableFrame");
    tableFrame->setStyleSheet("#tableFrame { background-color: white; border-radius: 15px; padding: 5px; }");

    QVBoxLayout *tableFrameLayout = new QVBoxLayout(tableFrame);

    QLabel *tableTitle = new QLabel("📋 Inventaire du matériel");
    tableTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 15px;");
    tableFrameLayout->addWidget(tableTitle);

    materialTable = new QTableWidget(0, 8);
    materialTable->setHorizontalHeaderLabels({"ID", "NOM", "TYPE", "QUANTITÉ", "SEUIL", "DATE", "FOURNISSEUR", "STATUT"});
    materialTable->setStyleSheet(
        "QTableWidget { background-color: white; border: none; font-size: 13px; }"
        "QTableWidget::item { padding: 12px; border-bottom: 1px solid #e9ecef; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; }"
        "QHeaderView::section { background-color: #2c3e50; color: white; padding: 12px; border: none; font-weight: bold; }");
    materialTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    materialTable->verticalHeader()->setVisible(false);
    materialTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    materialTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    materialTable->setAlternatingRowColors(false);  // Désactivé pour notre clignotement

    tableFrameLayout->addWidget(materialTable);
    contentLayout->addWidget(tableFrame, 2);

    pageLayout->addLayout(contentLayout);

    // Connections
    connect(addBtn, &QPushButton::clicked, [this]() { if (validateMaterialFields()) addMaterial(); });
    connect(deleteBtn, &QPushButton::clicked, this, &Matriele::deleteMaterial);
    connect(modifyBtn, &QPushButton::clicked, [this]() { if (validateMaterialFields()) modifyMaterial(); });
    connect(exportBtn, &QPushButton::clicked, this, &Matriele::exportToPdf);
    connect(statsBtn, &QPushButton::clicked, this, &Matriele::showStatistics);
    connect(qrBtn, &QPushButton::clicked, this, &Matriele::generateQRCode);
    connect(saveBtn, &QPushButton::clicked, this, &Matriele::saveData);
    connect(sortBtn, &QPushButton::clicked, this, &Matriele::sortMaterial);
    connect(testBlinkBtn, &QPushButton::clicked, this, &Matriele::testBlinking);

    connect(materialTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = materialTable->currentRow();
        if (row >= 0 && materialTable->item(row, 0)) {
            idEdit->setText(materialTable->item(row, 0)->text());
            nameEdit->setText(materialTable->item(row, 1)->text());
            typeBox->setCurrentText(materialTable->item(row, 2)->text());
            quantityEdit->setText(materialTable->item(row, 3)->text());
            thresholdEdit->setText(materialTable->item(row, 4)->text());
            dateEdit->setDate(QDate::fromString(materialTable->item(row, 5)->text(), "dd/MM/yyyy"));
            supplierEdit->setText(materialTable->item(row, 6)->text());
            statusBox->setCurrentText(materialTable->item(row, 7)->text());
        }
    });

    pagesWidget->addWidget(materialPage);
}

void Matriele::setupStatusBar() {
    m_statusBar = new QStatusBar;
    setStatusBar(m_statusBar);

    statusLabel = new QLabel("✅ Prêt");
    statsLabel = new QLabel;

    m_statusBar->addWidget(statusLabel);
    m_statusBar->addPermanentWidget(statsLabel);

    m_statusBar->setStyleSheet("QStatusBar { background-color: #2c3e50; color: white; padding: 5px 10px; }"
                               "QStatusBar QLabel { color: white; }");
}

void Matriele::setupShortcuts() {
    QShortcut *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &Matriele::saveData);

    QShortcut *quitShortcut = new QShortcut(QKeySequence::Quit, this);
    connect(quitShortcut, &QShortcut::activated, this, &QWidget::close);
}

QPushButton* Matriele::createStyledButton(const QString& text, const QString& color) {
    QPushButton *button = new QPushButton(text);
    button->setStyleSheet(QString(
                              "QPushButton { background-color: %1; color: white; border: none; border-radius: 10px;"
                              "padding: 10px 18px; font-size: 13px; font-weight: bold; min-width: 100px; }"
                              "QPushButton:hover { background-color: %1; opacity: 0.9; }"
                              "QPushButton:pressed { background-color: %1; padding: 12px 18px 8px 18px; }").arg(color));
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(40);
    return button;
}

// ==================== VALIDATION ====================

bool Matriele::isValidName(const QString& name) {
    QString trimmedName = name.trimmed();
    if (trimmedName.length() < 2 || trimmedName.length() > 50) return false;

    for (int i = 0; i < trimmedName.length(); ++i) {
        QChar c = trimmedName[i];
        bool isLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        bool isSpecial = (c == ' ' || c == '-' || c == '\'');
        if (!isLetter && !isSpecial) return false;
        if (c == ' ' && i > 0 && trimmedName[i-1] == ' ') return false;
        if ((c == ' ' || c == '-' || c == '\'') && (i == 0 || i == trimmedName.length() - 1)) return false;
    }
    return true;
}

bool Matriele::validateMaterialFields() {
    bool valid = true;
    QString errorMessage;

    if (nameEdit->text().trimmed().isEmpty()) {
        errorMessage += "• Le nom du matériel est requis\n";
        valid = false;
    } else if (!isValidName(nameEdit->text())) {
        errorMessage += "• Le nom doit contenir uniquement des lettres (A-Z, a-z)\n";
        valid = false;
    }

    if (quantityEdit->text().trimmed().isEmpty()) {
        errorMessage += "• La quantité est requise\n";
        valid = false;
    } else {
        bool ok;
        int value = quantityEdit->text().toInt(&ok);
        if (!ok || value < 0) {
            errorMessage += "• La quantité doit être un nombre positif\n";
            valid = false;
        }
    }

    if (thresholdEdit->text().trimmed().isEmpty()) {
        errorMessage += "• Le seuil minimum est requis\n";
        valid = false;
    } else {
        bool ok;
        int value = thresholdEdit->text().toInt(&ok);
        if (!ok || value < 0) {
            errorMessage += "• Le seuil doit être un nombre positif\n";
            valid = false;
        }
    }

    if (supplierEdit->text().trimmed().isEmpty()) {
        errorMessage += "• Le fournisseur est requis\n";
        valid = false;
    }

    if (!valid && !errorMessage.isEmpty()) {
        showNotification("Erreurs de validation :\n" + errorMessage, true);
    }

    return valid;
}

// ==================== ANIMATIONS ====================

void Matriele::animateField() {
    QLineEdit* field = qobject_cast<QLineEdit*>(sender());
    if (!field) return;

    QString originalStyle = field->styleSheet();
    field->setStyleSheet(originalStyle + "border: 2px solid #3498db; background-color: #e8f0fe;");
    QTimer::singleShot(300, [field, originalStyle]() {
        if (field) field->setStyleSheet(originalStyle);
    });
}

void Matriele::animateButton() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    button->setStyleSheet(button->styleSheet() + "padding: 12px 18px 8px 18px;");
    QTimer::singleShot(100, [button]() {
        if (button) button->setStyleSheet(button->styleSheet().replace("padding: 12px 18px 8px 18px;", "padding: 10px 18px;"));
    });
}

void Matriele::showNotification(const QString& message, bool isError) {
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(isError ? "❌ Erreur" : "✅ Succès");
    msgBox->setText(message);
    msgBox->setIcon(isError ? QMessageBox::Warning : QMessageBox::Information);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    msgBox->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(300);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();

    QTimer::singleShot(3000, msgBox, &QMessageBox::accept);
    msgBox->exec();
}

// ==================== GESTION DES MATÉRIAUX ====================

void Matriele::addMaterial() {
    Material newMaterial;
    newMaterial.id = nextId++;
    newMaterial.name = nameEdit->text().trimmed();
    newMaterial.type = typeBox->currentText();
    newMaterial.quantity = quantityEdit->text().toInt();
    newMaterial.threshold = thresholdEdit->text().toInt();
    newMaterial.addedDate = dateEdit->date();
    newMaterial.supplier = supplierEdit->text().trimmed();

    // Déterminer le statut
    if (newMaterial.quantity <= 0) {
        newMaterial.status = "Rupture";
    } else if (newMaterial.quantity <= newMaterial.threshold) {
        newMaterial.status = "Stock critique";
    } else {
        newMaterial.status = "Disponible";
    }

    materialsList.append(newMaterial);
    dataModified = true;
    updateMaterialTable();
    clearMaterialFields();
    checkLowStock();

    showNotification("✓ Matériel ajouté avec succès !", false);
}

void Matriele::modifyMaterial() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel à modifier.", true);
        return;
    }

    int currentId = idEdit->text().toInt();

    for (int i = 0; i < materialsList.size(); i++) {
        if (materialsList[i].id == currentId) {
            materialsList[i].name = nameEdit->text().trimmed();
            materialsList[i].type = typeBox->currentText();
            materialsList[i].quantity = quantityEdit->text().toInt();
            materialsList[i].threshold = thresholdEdit->text().toInt();
            materialsList[i].addedDate = dateEdit->date();
            materialsList[i].supplier = supplierEdit->text().trimmed();

            if (materialsList[i].quantity <= 0) {
                materialsList[i].status = "Rupture";
            } else if (materialsList[i].quantity <= materialsList[i].threshold) {
                materialsList[i].status = "Stock critique";
            } else {
                materialsList[i].status = "Disponible";
            }
            break;
        }
    }

    dataModified = true;
    updateMaterialTable();
    clearMaterialFields();
    checkLowStock();

    showNotification("✓ Matériel modifié avec succès !", false);
}

void Matriele::deleteMaterial() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel à supprimer.", true);
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmation", "Voulez-vous vraiment supprimer ce matériel ?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int id = materialTable->item(row, 0)->text().toInt();

        for (int i = 0; i < materialsList.size(); i++) {
            if (materialsList[i].id == id) {
                materialsList.removeAt(i);
                break;
            }
        }

        dataModified = true;
        updateMaterialTable();
        clearMaterialFields();
        checkLowStock();

        showNotification("✓ Matériel supprimé avec succès !", false);
    }
}

void Matriele::searchMaterial() {
    QString searchText = searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        updateMaterialTable();
        return;
    }

    materialTable->setRowCount(0);
    for (const Material &material : materialsList) {
        if (material.name.contains(searchText, Qt::CaseInsensitive) ||
            material.type.contains(searchText, Qt::CaseInsensitive) ||
            material.supplier.contains(searchText, Qt::CaseInsensitive)) {
            int row = materialTable->rowCount();
            materialTable->insertRow(row);
            materialTable->setItem(row, 0, new QTableWidgetItem(QString::number(material.id)));
            materialTable->setItem(row, 1, new QTableWidgetItem(material.name));
            materialTable->setItem(row, 2, new QTableWidgetItem(material.type));
            materialTable->setItem(row, 3, new QTableWidgetItem(QString::number(material.quantity)));
            materialTable->setItem(row, 4, new QTableWidgetItem(QString::number(material.threshold)));
            materialTable->setItem(row, 5, new QTableWidgetItem(material.addedDate.toString("dd/MM/yyyy")));
            materialTable->setItem(row, 6, new QTableWidgetItem(material.supplier));
            materialTable->setItem(row, 7, new QTableWidgetItem(material.status));
        }
    }
}

void Matriele::sortMaterial() {
    QString sortCriteria = sortComboBox->currentText();

    if (sortCriteria.contains("ID (croissant)")) {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) { return a.id < b.id; });
    } else if (sortCriteria.contains("ID (décroissant)")) {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) { return a.id > b.id; });
    } else if (sortCriteria.contains("quantité (croissant)")) {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) { return a.quantity < b.quantity; });
    } else if (sortCriteria.contains("quantité (décroissant)")) {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) { return a.quantity > b.quantity; });
    } else if (sortCriteria.contains("type")) {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) { return a.type < b.type; });
    } else if (sortCriteria.contains("nom")) {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) { return a.name < b.name; });
    }

    updateMaterialTable();
    showNotification("✓ Liste triée", false);
}

// ==================== AFFICHAGE DU TABLEAU ====================

void Matriele::updateMaterialTable() {
    materialTable->setRowCount(0);

    for (const Material &material : materialsList) {
        int row = materialTable->rowCount();
        materialTable->insertRow(row);

        materialTable->setItem(row, 0, new QTableWidgetItem(QString::number(material.id)));
        materialTable->setItem(row, 1, new QTableWidgetItem(material.name));
        materialTable->setItem(row, 2, new QTableWidgetItem(material.type));

        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(material.quantity));
        if (material.quantity <= material.threshold) {
            qtyItem->setForeground(QBrush(QColor(231, 76, 60)));
            QFont font = qtyItem->font();
            font.setBold(true);
            qtyItem->setFont(font);
        }
        materialTable->setItem(row, 3, qtyItem);

        materialTable->setItem(row, 4, new QTableWidgetItem(QString::number(material.threshold)));
        materialTable->setItem(row, 5, new QTableWidgetItem(material.addedDate.toString("dd/MM/yyyy")));
        materialTable->setItem(row, 6, new QTableWidgetItem(material.supplier));

        QTableWidgetItem *statusItem = new QTableWidgetItem(material.status);
        if (material.quantity <= material.threshold) {
            statusItem->setForeground(QBrush(QColor(231, 76, 60)));
            QFont font = statusItem->font();
            font.setBold(true);
            statusItem->setFont(font);
        }
        materialTable->setItem(row, 7, statusItem);
    }

    updateBlinkingState();
    updateStatusBar();
}

void Matriele::clearMaterialFields() {
    nameEdit->clear();
    quantityEdit->clear();
    thresholdEdit->clear();
    supplierEdit->clear();
    typeBox->setCurrentIndex(0);
    statusBox->setCurrentIndex(0);
    dateEdit->setDate(QDate::currentDate());
    materialTable->clearSelection();
    idEdit->clear();
}

void Matriele::updateStatusBar() {
    int total = materialsList.size();
    int lowStock = 0;
    int outOfStock = 0;

    for (const Material &m : materialsList) {
        if (m.quantity <= m.threshold) lowStock++;
        if (m.quantity == 0) outOfStock++;
    }

    statsLabel->setText(QString("📊 Total: %1 | ⚠️ Stock critique: %2 | 🔴 Rupture: %3")
                            .arg(total).arg(lowStock).arg(outOfStock));
}

// ==================== CLIGNOTEMENT ROUGE (FONCTION PRINCIPALE) ====================

void Matriele::checkLowStock()
{
    lowStockIds.clear();
    int lowStockCount = 0;
    int outOfStockCount = 0;

    qDebug() << "=== Vérification des stocks ===";
    qDebug() << "Nombre de matériels:" << materialsList.size();

    for (const Material &m : materialsList) {
        if (m.quantity <= m.threshold) {
            lowStockIds.insert(m.id);
            lowStockCount++;
            if (m.quantity == 0) {
                outOfStockCount++;
            }
            qDebug() << "⚠️ Stock critique:" << m.name << "(Qté:" << m.quantity << "/Seuil:" << m.threshold << ")";
        }
    }

    qDebug() << "Total stocks critiques:" << lowStockCount;

    if (!lowStockIds.isEmpty()) {
        QString alertMsg;
        if (outOfStockCount > 0) {
            alertMsg = QString("🔴 ALERTE : %1 matériel(s) critique (%2 en rupture)!")
                           .arg(lowStockCount).arg(outOfStockCount);
            statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold; background-color: #2c3e50;");
        } else {
            alertMsg = QString("⚠️ ATTENTION : %1 matériel(s) en stock critique !").arg(lowStockCount);
            statusLabel->setStyleSheet("color: #f39c12; font-weight: bold; background-color: #2c3e50;");
        }
        statusLabel->setText(alertMsg);
    } else {
        statusLabel->setText("✅ Stock normal");
        statusLabel->setStyleSheet("color: #2ecc71; font-weight: normal; background-color: #2c3e50;");
    }

    updateMaterialTable();
}

void Matriele::updateBlinkingState()
{
    if (!materialTable) {
        qDebug() << "ERROR: materialTable est NULL!";
        return;
    }

    blinkState = !blinkState;

    int rowCount = materialTable->rowCount();

    for (int row = 0; row < rowCount; ++row) {
        if (!materialTable->item(row, 0)) continue;

        int id = materialTable->item(row, 0)->text().toInt();
        bool isLowStock = lowStockIds.contains(id);

        if (isLowStock) {
            // Couleur rouge clignotante
            QColor bgColor;
            if (blinkState) {
                bgColor = QColor(231, 76, 60, 220);  // Rouge vif
            } else {
                bgColor = QColor(192, 57, 43, 200);  // Rouge foncé
            }

            // Appliquer à toute la ligne
            for (int col = 0; col < materialTable->columnCount(); ++col) {
                QTableWidgetItem *item = materialTable->item(row, col);
                if (item) {
                    item->setBackground(QBrush(bgColor));
                    item->setForeground(QBrush(Qt::white));
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                }
            }

            // Texte spécial pour le statut
            if (materialTable->item(row, 7)) {
                int qty = materialTable->item(row, 3)->text().toInt();
                if (qty == 0) {
                    materialTable->item(row, 7)->setText("🔴 RUPTURE");
                } else if (blinkState) {
                    materialTable->item(row, 7)->setText("⚠️ CRITIQUE !");
                } else {
                    materialTable->item(row, 7)->setText("⚠️ Stock faible");
                }
            }

            // Icône devant la quantité
            if (materialTable->item(row, 3)) {
                QString qtyText = materialTable->item(row, 3)->text();
                qtyText.remove("⚠️ ");
                qtyText.remove("🔴 ");
                if (blinkState) {
                    materialTable->item(row, 3)->setText("⚠️ " + qtyText);
                } else {
                    materialTable->item(row, 3)->setText(qtyText);
                }
            }
        } else {
            // Style normal
            QColor bgColor = (row % 2 == 0) ? Qt::white : QColor(248, 249, 250);
            for (int col = 0; col < materialTable->columnCount(); ++col) {
                QTableWidgetItem *item = materialTable->item(row, col);
                if (item) {
                    item->setBackground(QBrush(bgColor));
                    item->setForeground(QBrush(QColor(44, 62, 80)));
                    QFont font = item->font();
                    font.setBold(false);
                    item->setFont(font);
                }
            }

            // Restaurer le statut original
            if (materialTable->item(row, 7)) {
                for (const Material &m : materialsList) {
                    if (m.id == id) {
                        materialTable->item(row, 7)->setText(m.status);
                        break;
                    }
                }
            }

            // Restaurer la quantité
            if (materialTable->item(row, 3)) {
                QString qtyText = materialTable->item(row, 3)->text();
                qtyText.remove("⚠️ ");
                qtyText.remove("🔴 ");
                materialTable->item(row, 3)->setText(qtyText);
            }
        }
    }

    materialTable->viewport()->update();
}

void Matriele::testBlinking()
{
    qDebug() << "=== TEST MANUEL DU CLIGNOTEMENT ===";
    qDebug() << "blinkState actuel:" << blinkState;
    qDebug() << "lowStockIds:" << lowStockIds.values();
    updateBlinkingState();
}

// ==================== EXPORTATION PDF ====================

void Matriele::exportToPdf()
{
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée à exporter !", true);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter le rapport en PDF",
                                                    QDir::homePath() + "/rapport_materiel_berus.pdf",
                                                    "PDF Files (*.pdf)");

    if (fileName.isEmpty())
        return;

    QString html = generateHtmlReport();

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument document;
    document.setHtml(html);
    document.print(&printer);

    showNotification(QString("✓ Rapport PDF exporté avec succès !\n\nFichier : %1").arg(fileName), false);
}

QString Matriele::generateHtmlReport()
{
    int totalMaterials = materialsList.size();
    int totalQuantity = 0;
    int lowStock = 0;
    int outOfStock = 0;
    int available = 0;

    QMap<QString, int> typeCount;

    for (const Material &m : materialsList) {
        totalQuantity += m.quantity;
        if (m.quantity <= m.threshold) lowStock++;
        if (m.quantity == 0) outOfStock++;
        if (m.status == "Disponible") available++;
        typeCount[m.type] = typeCount.value(m.type, 0) + 1;
    }

    QString html;

    html += "<!DOCTYPE html>";
    html += "<html><head><meta charset='UTF-8'>";
    html += "<title>Rapport Matériel - BERUS</title>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; }";
    html += "h1 { color: #2c3e50; border-bottom: 3px solid #3498db; }";
    html += ".stats { background: #ecf0f1; padding: 15px; border-radius: 10px; margin: 20px 0; }";
    html += "table { width: 100%; border-collapse: collapse; }";
    html += "th { background: #2c3e50; color: white; padding: 10px; }";
    html += "td { padding: 8px; border-bottom: 1px solid #ddd; }";
    html += ".critical { background-color: #f8d7da; color: #721c24; }";
    html += ".footer { margin-top: 30px; text-align: center; font-size: 10px; color: #95a5a6; }";
    html += "</style></head><body>";

    html += "<h1>📦 BERUS - RAPPORT D'INVENTAIRE</h1>";
    html += "<p>Date: " + QDate::currentDate().toString("dd/MM/yyyy") + " à " + QTime::currentTime().toString("HH:mm:ss") + "</p>";

    html += "<div class='stats'>";
    html += "<h3>Statistiques</h3>";
    html += "<p>Total matériels: " + QString::number(totalMaterials) + "</p>";
    html += "<p>Unités en stock: " + QString::number(totalQuantity) + "</p>";
    html += "<p>Stock critique: " + QString::number(lowStock) + "</p>";
    html += "<p>En rupture: " + QString::number(outOfStock) + "</p>";
    html += "</div>";

    html += "<table>";
    html += "<tr><th>ID</th><th>Nom</th><th>Type</th><th>Quantité</th><th>Seuil</th><th>Statut</th><th>Fournisseur</th></tr>";

    for (const Material &m : materialsList) {
        QString rowClass = (m.quantity <= m.threshold) ? "class='critical'" : "";
        html += "<tr " + rowClass + ">";
        html += "<td>" + QString::number(m.id) + "</td>";
        html += "<td>" + m.name + "</td>";
        html += "<td>" + m.type + "</td>";
        html += "<td>" + QString::number(m.quantity) + "</td>";
        html += "<td>" + QString::number(m.threshold) + "</td>";
        html += "<td>" + m.status + "</td>";
        html += "<td>" + m.supplier + "</td>";
        html += "</tr>";
    }

    html += "</table>";
    html += "<div class='footer'>BERUS - Système de Gestion de Matériel</div>";
    html += "</body></html>";

    return html;
}

void Matriele::exportToCsv()
{
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée à exporter !", true);
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Exporter en CSV",
                                                    QDir::homePath() + "/materiel_berus.csv",
                                                    "CSV Files (*.csv)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        showNotification("Impossible de créer le fichier CSV !", true);
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    stream << "ID;NOM;TYPE;QUANTITE;SEUIL;STATUT;FOURNISSEUR;DATE\n";

    for (const Material &m : materialsList) {
        stream << m.id << ";" << m.name << ";" << m.type << ";"
               << m.quantity << ";" << m.threshold << ";" << m.status << ";"
               << m.supplier << ";" << m.addedDate.toString("dd/MM/yyyy") << "\n";
    }

    file.close();
    showNotification("✓ Export CSV réussi !", false);
}

// ==================== STATISTIQUES ET QR CODE ====================

void Matriele::showStatistics() {
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée disponible", true);
        return;
    }

    int totalMaterials = materialsList.size();
    int lowStock = 0;
    int available = 0;
    int outOfStock = 0;
    int totalQuantity = 0;
    QMap<QString, int> typeCount;

    for (const Material &m : materialsList) {
        if (m.quantity <= m.threshold) lowStock++;
        if (m.status == "Disponible") available++;
        if (m.quantity == 0) outOfStock++;
        totalQuantity += m.quantity;
        typeCount[m.type] = typeCount.value(m.type, 0) + 1;
    }

    QString stats = "📊 STATISTIQUES DU MATÉRIEL\n\n";
    stats += "═══════════════════════════════\n\n";
    stats += "📦 Vue d'ensemble :\n";
    stats += "• Total matériels : " + QString::number(totalMaterials) + "\n";
    stats += "• Quantité totale : " + QString::number(totalQuantity) + "\n";
    stats += "• Stock disponible : " + QString::number(available) + "\n";
    stats += "• Stock critique : " + QString::number(lowStock) + "\n";
    stats += "• En rupture : " + QString::number(outOfStock) + "\n\n";

    stats += "🏷️ Par type :\n";
    for (auto it = typeCount.begin(); it != typeCount.end(); ++it) {
        stats += "• " + it.key() + " : " + QString::number(it.value()) + "\n";
    }

    if (lowStock > 0) {
        stats += "\n⚠️ ATTENTION : " + QString::number(lowStock) + " matériel(s) en stock critique !";
    }

    QMessageBox::information(this, "Statistiques", stats);
}

void Matriele::generateQRCode() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel pour générer son Code QR.", true);
        return;
    }

    int id = materialTable->item(row, 0)->text().toInt();
    QString name = materialTable->item(row, 1)->text();
    QString type = materialTable->item(row, 2)->text();
    QString quantity = materialTable->item(row, 3)->text();
    QString supplier = materialTable->item(row, 6)->text();
    QString status = materialTable->item(row, 7)->text();

    QString qrData = QString("BERUS - Gestion Matériel\n"
                             "ID: %1\nNom: %2\nType: %3\n"
                             "Quantité: %4\nFournisseur: %5\nStatut: %6")
                         .arg(id).arg(name).arg(type).arg(quantity).arg(supplier).arg(status);

    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(qrData.toUtf8().constData(), QrCode::Ecc::MEDIUM);

    int scale = 5;
    int margin = 2;
    int size = qr.getSize();
    int imgSize = (size + 2 * margin) * scale;

    QImage img(imgSize, imgSize, QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (qr.getModule(x, y)) {
                painter.drawRect((x + margin) * scale, (y + margin) * scale, scale, scale);
            }
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Code QR : " + name);
    dialog.setMinimumSize(400, 500);

    QVBoxLayout layout(&dialog);
    QLabel label;
    label.setPixmap(QPixmap::fromImage(img).scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label.setAlignment(Qt::AlignCenter);
    layout.addWidget(&label);

    QLabel infoLabel;
    infoLabel.setText(qrData);
    infoLabel.setAlignment(Qt::AlignCenter);
    infoLabel.setStyleSheet("font-family: monospace; padding: 10px; background-color: #f8f9fa; border-radius: 5px;");
    layout.addWidget(&infoLabel);

    QPushButton* closeBtn = createStyledButton("Fermer", "#3498db");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout.addWidget(closeBtn);

    dialog.exec();
}

// ==================== AUTRES FONCTIONS ====================

void Matriele::importFromFile() {
    showNotification("📂 Import - Fonctionnalité à implémenter", false);
}

void Matriele::saveData() {
    showNotification("ℹ️ Données sauvegardées", false);
}

void Matriele::loadData() {
    updateMaterialTable();
}

void Matriele::changePage(int index) {
    pagesWidget->setCurrentIndex(index);
    if (index == 1) {
        showStatistics();
        QTimer::singleShot(100, [this]() { navList->setCurrentRow(0); });
    }
}
