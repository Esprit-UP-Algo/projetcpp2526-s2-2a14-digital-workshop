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
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>

const QString Matriele::APP_VERSION = "2.0.0";
const QString Matriele::APP_NAME = "Gestion Matériel";
const int Matriele::AUTO_SAVE_INTERVAL = 300000;

Matriele::Matriele(QWidget *parent) : QMainWindow(parent),
    nextId(1), blinkState(false), dataModified(false) {

    setWindowTitle(QString("%1 v%2").arg(APP_NAME).arg(APP_VERSION));
    setMinimumSize(1200, 700);

    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #f5f6fa;");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    pagesWidget = new QStackedWidget;

    setupMaterialPage();
    setupStatusBar();
    setupShortcuts();

    mainLayout->addWidget(pagesWidget, 1);

    loadData();

    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &Matriele::saveData);
    autoSaveTimer->start(AUTO_SAVE_INTERVAL);

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &Matriele::updateBlinkingState);
    blinkTimer->start(500);

    updateStatusBar();
}

Matriele::~Matriele() {
    qDeleteAll(fieldAnimations);
}

void Matriele::setupMaterialPage() {
    QWidget *materialPage = new QWidget;
    materialPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(materialPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    QFrame *headerFrame = new QFrame;
    headerFrame->setObjectName("headerFrame");
    headerFrame->setStyleSheet("#headerFrame { background-color: white; border-radius: 15px; padding: 20px; }");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);

    QLabel *titleLabel = new QLabel("📦 GESTION MATÉRIELLE");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);

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
    saveBtn = createStyledButton("🔄 Actualiser", "#27ae60");

    actionButtonsLayout->addWidget(exportBtn);
    actionButtonsLayout->addWidget(statsBtn);
    actionButtonsLayout->addWidget(qrBtn);
    actionButtonsLayout->addWidget(sortBtn);
    actionButtonsLayout->addWidget(saveBtn);

    headerLayout->addLayout(actionButtonsLayout);
    pageLayout->addWidget(headerFrame);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(25);

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
    typeBox->addItems({"Bois", "Outil", "Consommable"});
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
    statusBox->addItems({"Disponible", "Rupture"});
    statusBox->setStyleSheet(inputStyle);

    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(15);
    inputForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    inputForm->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto makeLabel = [](const QString &text) {
        QLabel *l = new QLabel(text);
        l->setStyleSheet("font-weight: bold; color: #34495e; padding: 0px; min-height: 15px;");
        return l;
    };

    inputForm->addRow(makeLabel("🔑 ID:"), idEdit);
    inputForm->addRow(makeLabel("🏷️ Nom *:"), nameEdit);
    inputForm->addRow(makeLabel("📋 Type:"), typeBox);
    inputForm->addRow(makeLabel("🔢 Quantité *:"), quantityEdit);
    inputForm->addRow(makeLabel("⚠️ Seuil *:"), thresholdEdit);
    inputForm->addRow(makeLabel("📅 Date d'ajout:"), dateEdit);
    inputForm->addRow(makeLabel("🏢 Fournisseur *:"), supplierEdit);
    inputForm->addRow(makeLabel("📊 Statut:"), statusBox);

    formLayout->addLayout(inputForm);
    formLayout->addStretch();

    QLabel *infoNote = new QLabel("⚠️ Les champs avec * sont obligatoires");
    infoNote->setStyleSheet("color: #e74c3c; font-size: 11px; padding-top: 15px; font-weight: bold;");
    formLayout->addWidget(infoNote);

    contentLayout->addWidget(formFrame);

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
        "QTableWidget::item { padding: 12px; border-bottom: 1px solid #e9ecef; color: #2c3e50; }"
        "QTableWidget::item:selected { background-color: #e3f2fd; }"
        "QHeaderView::section { background-color: #2c3e50; color: white; padding: 12px; border: none; font-weight: bold; }");
    materialTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    materialTable->verticalHeader()->setVisible(false);
    materialTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    materialTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    materialTable->setAlternatingRowColors(true);

    tableFrameLayout->addWidget(materialTable);
    contentLayout->addWidget(tableFrame, 2);

    pageLayout->addLayout(contentLayout);

    connect(addBtn, &QPushButton::clicked, [this]() { if (validateMaterialFields()) addMaterial(); });
    connect(deleteBtn, &QPushButton::clicked, this, &Matriele::deleteMaterial);
    connect(modifyBtn, &QPushButton::clicked, [this]() { if (validateMaterialFields()) modifyMaterial(); });
    connect(exportBtn, &QPushButton::clicked, this, &Matriele::exportToPdf);
    connect(statsBtn, &QPushButton::clicked, this, &Matriele::showStatistics);
    connect(qrBtn, &QPushButton::clicked, this, &Matriele::generateQRCode);
    connect(saveBtn, &QPushButton::clicked, this, &Matriele::saveData);
    connect(sortBtn, &QPushButton::clicked, this, &Matriele::sortMaterial);

    connect(materialTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = materialTable->currentRow();
        if (row >= 0) {
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

void Matriele::addMaterial() {
    Material newMaterial;
    newMaterial.name      = nameEdit->text().trimmed();
    newMaterial.type      = typeBox->currentText();
    newMaterial.quantity  = quantityEdit->text().toInt();
    newMaterial.threshold = thresholdEdit->text().toInt();
    newMaterial.addedDate = dateEdit->date();
    newMaterial.supplier  = supplierEdit->text().trimmed();
    newMaterial.status    = (newMaterial.quantity <= newMaterial.threshold) ? "Rupture" : statusBox->currentText();

    QSqlQuery qMax(Connection::getInstance()->getDatabase());
    qMax.exec("SELECT NVL(MAX(ID_MATERIEL),0) FROM MATERIEL");
    if (qMax.next()) newMaterial.id = qMax.value(0).toInt() + 1;
    else newMaterial.id = nextId;

    QSqlQuery query(Connection::getInstance()->getDatabase());
    query.prepare("INSERT INTO MATERIEL (ID_MATERIEL, NOM, TYPE, QUANTITE, SEUIL, DATE_AJOUT, FOURNISSEUR, STATUT) "
                  "VALUES (:id, :nom, :type, :quantite, :seuil, TO_DATE(:date,'YYYY-MM-DD'), :fournisseur, :statut)");
    query.bindValue(":id",          newMaterial.id);
    query.bindValue(":nom",         newMaterial.name);
    query.bindValue(":type",        newMaterial.type);
    query.bindValue(":quantite",    newMaterial.quantity);
    query.bindValue(":seuil",       newMaterial.threshold);
    query.bindValue(":date",        newMaterial.addedDate.toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", newMaterial.supplier);
    query.bindValue(":statut",      newMaterial.status);

    if (!query.exec()) {
        showNotification("Erreur ajout Oracle:\n" + query.lastError().text(), true);
        return;
    }

    loadData();
    clearMaterialFields();
    showNotification("✓ Matériel ajouté avec succès dans Oracle !", false);
}

void Matriele::modifyMaterial() {
    int currentId = idEdit->text().toInt();
    if (currentId <= 0) {
        showNotification("Sélectionnez un matériel à modifier.", true);
        return;
    }

    QString newStatus = statusBox->currentText();
    int qty   = quantityEdit->text().toInt();
    int seuil = thresholdEdit->text().toInt();
    if (qty <= seuil) newStatus = "Rupture";

    QSqlQuery query(Connection::getInstance()->getDatabase());
    query.prepare("UPDATE MATERIEL SET NOM=:nom, TYPE=:type, QUANTITE=:quantite, "
                  "SEUIL=:seuil, DATE_AJOUT=TO_DATE(:date,'YYYY-MM-DD'), FOURNISSEUR=:fournisseur, STATUT=:statut "
                  "WHERE ID_MATERIEL=:id");
    query.bindValue(":nom",         nameEdit->text().trimmed());
    query.bindValue(":type",        typeBox->currentText());
    query.bindValue(":quantite",    qty);
    query.bindValue(":seuil",       seuil);
    query.bindValue(":date",        dateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", supplierEdit->text().trimmed());
    query.bindValue(":statut",      newStatus);
    query.bindValue(":id",          currentId);

    if (!query.exec()) {
        showNotification("Erreur modification Oracle:\n" + query.lastError().text(), true);
        return;
    }

    loadData();
    showNotification("✓ Matériel modifié avec succès !", false);
}

void Matriele::deleteMaterial() {
    int row = materialTable->currentRow();
    if (row < 0) { showNotification("Sélectionnez un matériel à supprimer.", true); return; }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmation", "Voulez-vous supprimer ce matériel ?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int id = materialTable->item(row, 0)->text().toInt();

        QSqlQuery query(Connection::getInstance()->getDatabase());
        query.prepare("DELETE FROM MATERIEL WHERE ID_MATERIEL = :id");
        query.bindValue(":id", id);

        if (!query.exec()) {
            showNotification("Erreur suppression Oracle:\n" + query.lastError().text(), true);
            return;
        }

        loadData();
        clearMaterialFields();
        showNotification("✓ Matériel supprimé !", false);
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

void Matriele::updateMaterialTable() {
    materialTable->setRowCount(0);
    for (const Material &material : materialsList) {
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
    idEdit->setText(QString::number(nextId));
}

void Matriele::updateStatusBar() {
    int total      = materialsList.size();
    int lowStock   = 0;
    int outOfStock = 0;

    for (const Material &m : materialsList) {
        if (m.quantity <= m.threshold) lowStock++;
        if (m.status == "Rupture") outOfStock++;
    }

    statusLabel->setText(QString("✅ Prêt - %1 matériel(s) en stock").arg(total));
    statsLabel->setText(QString("⚠️ Stock critique: %1 | ❌ Rupture: %2").arg(lowStock).arg(outOfStock));
}

void Matriele::updateBlinkingState() {
    blinkState = !blinkState;
    if (!materialTable) return;

    for (int i = 0; i < materialTable->rowCount(); ++i) {
        if (!materialTable->item(i, 3) || !materialTable->item(i, 4)) continue;

        int qty    = materialTable->item(i, 3)->text().toInt();
        int thresh = materialTable->item(i, 4)->text().toInt();

        if (qty <= thresh) {
            QColor bgColor = blinkState ? QColor(255, 220, 220) : QColor(255, 235, 235);
            for (int col = 0; col < materialTable->columnCount(); ++col)
                if (materialTable->item(i, col))
                    materialTable->item(i, col)->setBackground(bgColor);
        } else {
            for (int col = 0; col < materialTable->columnCount(); ++col)
                if (materialTable->item(i, col))
                    materialTable->item(i, col)->setBackground((i % 2 == 0) ? Qt::white : QColor(250, 250, 252));
        }
    }
}

void Matriele::exportToPdf() {
    showNotification("📄 Export PDF - Fonctionnalité à implémenter", false);
}

void Matriele::exportToCsv() {
    showNotification("📊 Export CSV - Fonctionnalité à implémenter", false);
}

void Matriele::showStatistics() {
    if (materialsList.isEmpty()) {
        showNotification("Aucune donnée disponible", true);
        return;
    }

    int totalMaterials = materialsList.size();
    int lowStock = 0, available = 0, outOfStock = 0, totalQuantity = 0;
    QMap<QString, int> typeCount;

    for (const Material &m : materialsList) {
        if (m.quantity <= m.threshold) lowStock++;
        if (m.status == "Disponible") available++;
        if (m.status == "Rupture") outOfStock++;
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
    for (auto it = typeCount.begin(); it != typeCount.end(); ++it)
        stats += "• " + it.key() + " : " + QString::number(it.value()) + "\n";

    QMessageBox::information(this, "Statistiques", stats);
}

void Matriele::generateQRCode() {
    int row = materialTable->currentRow();
    if (row < 0) {
        showNotification("Sélectionnez un matériel pour générer son Code QR.", true);
        return;
    }

    int id          = materialTable->item(row, 0)->text().toInt();
    QString name     = materialTable->item(row, 1)->text();
    QString type     = materialTable->item(row, 2)->text();
    QString quantity = materialTable->item(row, 3)->text();
    QString supplier = materialTable->item(row, 6)->text();

    QString qrData = QString("ID: %1\nNom: %2\nType: %3\nQuantité: %4\nFournisseur: %5")
                         .arg(id).arg(name).arg(type).arg(quantity).arg(supplier);

    using namespace qrcodegen;
    QrCode qr = QrCode::encodeText(qrData.toUtf8().constData(), QrCode::Ecc::MEDIUM);

    int scale = 5, margin = 2;
    int size    = qr.getSize();
    int imgSize = (size + 2 * margin) * scale;

    QImage img(imgSize, imgSize, QImage::Format_RGB32);
    img.fill(Qt::white);

    QPainter painter(&img);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);

    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++)
            if (qr.getModule(x, y))
                painter.drawRect((x + margin) * scale, (y + margin) * scale, scale, scale);

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

void Matriele::importFromFile() {
    showNotification("📂 Import - Fonctionnalité à implémenter", false);
}

void Matriele::saveData() {
    loadData();
    showNotification("✓ Données actualisées depuis Oracle", false);
}

void Matriele::loadData() {
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen()) {
        showNotification("❌ Base de données non connectée !", true);
        return;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT ID_MATERIEL, NOM, TYPE, QUANTITE, SEUIL, DATE_AJOUT, FOURNISSEUR, STATUT "
                    "FROM MATERIEL ORDER BY ID_MATERIEL")) {
        showNotification("Erreur chargement Oracle:\n" + query.lastError().text(), true);
        return;
    }

    materialsList.clear();
    while (query.next()) {
        Material m;
        m.id        = query.value(0).toInt();
        m.name      = query.value(1).toString();
        m.type      = query.value(2).toString();
        m.quantity  = query.value(3).toInt();
        m.threshold = query.value(4).toInt();
        m.addedDate = query.value(5).toDate();
        m.supplier  = query.value(6).toString();
        m.status    = query.value(7).toString();
        materialsList.append(m);
    }

    nextId = materialsList.isEmpty() ? 1 : materialsList.last().id + 1;
    idEdit->setText(QString::number(nextId));
    updateMaterialTable();
}
