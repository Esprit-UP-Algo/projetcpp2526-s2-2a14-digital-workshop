#include "mainwindow.h"

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>
#include <QDateEdit>
#include <QIntValidator>
#include <QDate>
#include <QSpacerItem>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), nextId(1)
{
    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #f5f6fa;");
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // === SIDEBAR GAUCHE ===
    QFrame *sidebar = new QFrame;
    sidebar->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                                 stop:0 #2c3e50, stop:1 #34495e);"
        "    border: none;"
        "}"
        "QLabel {"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 20px;"
        "}"
        "QListWidget {"
        "    background: transparent;"
        "    border: none;"
        "    color: #ecf0f1;"
        "    font-size: 14px;"
        "}"
        "QListWidget::item {"
        "    padding: 15px 20px;"
        "    border-bottom: 1px solid rgba(255,255,255,0.1);"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #3498db;"
        "    border-left: 4px solid #2980b9;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: rgba(52, 152, 219, 0.3);"
        "}"
        );
    sidebar->setFixedWidth(250);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    QLabel *appTitle = new QLabel("🏢 GESTION");
    appTitle->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(appTitle);

    navList = new QListWidget;
    navList->addItem("👥 Utilisateurs");
    navList->addItem("🛠️ Matériel");
    navList->addItem("👨‍💼 Clients");
    navList->addItem("📦 Commandes");
    navList->setCurrentRow(1);

    connect(navList, &QListWidget::currentRowChanged, this, &MainWindow::changePage);

    sidebarLayout->addWidget(navList);
    sidebarLayout->addStretch();

    QLabel *footer = new QLabel("v1.0 - Gestion Matériel");
    footer->setStyleSheet("color: #7f8c8d; font-size: 11px; padding: 10px;");
    footer->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(footer);

    // === CONTENU PRINCIPAL ===
    pagesWidget = new QStackedWidget;

    setupUsersPage();
    setupMaterialPage();
    setupClientsPage();
    setupOrdersPage();

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pagesWidget, 1);

    setWindowTitle("Système de Gestion Matériel");
    resize(1200, 700);

    // Ajouter des exemples
    Material mat1 = {1, "Poutre en chêne", "Bois", 50, 10, QDate::currentDate(), "BoisCorp", "Disponible"};
    Material mat2 = {2, "Marteau perforateur", "Outil", 15, 5, QDate(2024, 1, 15), "TechTools", "Disponible"};
    Material mat3 = {3, "Vis à bois 5x60", "Consommable", 500, 100, QDate(2024, 2, 20), "FixPro", "Disponible"};
    materialsList << mat1 << mat2 << mat3;
    nextId = 4;
    updateMaterialTable();
}

void MainWindow::setupMaterialPage()
{
    QWidget *materialPage = new QWidget;
    materialPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(materialPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    // === EN-TÊTE ===
    QFrame *header = new QFrame;
    header->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "    padding: 20px;"
        "}"
        );

    QHBoxLayout *headerLayout = new QHBoxLayout(header);

    QLabel *pageTitle = new QLabel("🛠️ GESTION DU MATÉRIEL");
    pageTitle->setStyleSheet("color: #2c3e50; font-size: 22px; font-weight: bold;");

    headerLayout->addWidget(pageTitle);
    headerLayout->addStretch();

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("🔍 Rechercher...");
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #dfe6e9;"
        "    border-radius: 6px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "    min-width: 200px;"
        "    color: black;"
        "}"
        );

    sortComboBox = new QComboBox;
    sortComboBox->addItems({"Trier par ID (croissant)", "Trier par ID (décroissant)",
                            "Trier par quantité (croissant)", "Trier par quantité (décroissant)",
                            "Trier par type", "Trier par nom"});
    sortComboBox->setStyleSheet(
        "QComboBox {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #dfe6e9;"
        "    border-radius: 6px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "    min-width: 220px;"
        "    color: black;"
        "}"
        );

    QPushButton *searchBtn = createStyledButton("Chercher", "#3498db");
    QPushButton *deleteBtn = createStyledButton("Supprimer", "#e74c3c");
    exportBtn = createStyledButton("📊 Exporter PDF", "#9b59b6");
    statsBtn = createStyledButton("📈 Statistiques", "#1abc9c");
    sortBtn = createStyledButton("📊 Trier", "#f39c12");

    headerLayout->addWidget(searchEdit);
    headerLayout->addWidget(searchBtn);
    headerLayout->addWidget(sortComboBox);
    headerLayout->addWidget(sortBtn);
    headerLayout->addWidget(deleteBtn);
    headerLayout->addWidget(exportBtn);
    headerLayout->addWidget(statsBtn);

    pageLayout->addWidget(header);

    // === CONTENU ===
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(20);

    // Colonne gauche - Formulaire
    QFrame *formFrame = new QFrame;
    formFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "    padding: 25px;"
        "}"
        );

    QVBoxLayout *formLayout = new QVBoxLayout(formFrame);

    QLabel *formTitle = new QLabel("Ajouter/Modifier du matériel");
    formTitle->setStyleSheet("color: #3498db; font-size: 16px; font-weight: bold; margin-bottom: 15px;");
    formLayout->addWidget(formTitle);

    QString lineEditStyle =
        "QLineEdit {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #3498db;"
        "    outline: none;"
        "}";

    QString comboBoxStyle =
        "QComboBox {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "}";

    idEdit = new QLineEdit;
    idEdit->setReadOnly(true);
    idEdit->setText("1");
    idEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #e9ecef;"
        "    color: #6c757d;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
        );

    nameEdit = new QLineEdit;
    nameEdit->setPlaceholderText("Nom du matériel");
    nameEdit->setStyleSheet(lineEditStyle);

    typeBox = new QComboBox;
    typeBox->addItems({"Bois", "Outil", "Consommable", "Équipement", "Métal"});
    typeBox->setStyleSheet(comboBoxStyle);

    quantityEdit = new QLineEdit;
    quantityEdit->setPlaceholderText("Quantité disponible");
    quantityEdit->setValidator(new QIntValidator(0, 99999, this));
    quantityEdit->setStyleSheet(lineEditStyle);

    thresholdEdit = new QLineEdit;
    thresholdEdit->setPlaceholderText("Seuil minimum");
    thresholdEdit->setValidator(new QIntValidator(0, 99999, this));
    thresholdEdit->setStyleSheet(lineEditStyle);

    dateEdit = new QDateEdit;
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setStyleSheet(
        "QDateEdit {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
        );

    supplierEdit = new QLineEdit;
    supplierEdit->setPlaceholderText("Fournisseur");
    supplierEdit->setStyleSheet(lineEditStyle);

    statusBox = new QComboBox;
    statusBox->addItems({"Disponible", "Rupture", "En commande", "Hors service"});
    statusBox->setStyleSheet(comboBoxStyle);

    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(12);
    inputForm->setContentsMargins(0, 0, 0, 0);

    QString labelStyle =
        "QLabel {"
        "    color: #2c3e50;"
        "    font-weight: 600;"
        "    font-size: 13px;"
        "    padding: 2px 0;"
        "}";

    QLabel *idLabel = new QLabel("ID:");
    idLabel->setStyleSheet(labelStyle);
    QLabel *nameLabel = new QLabel("Nom:");
    nameLabel->setStyleSheet(labelStyle);
    QLabel *typeLabel = new QLabel("Type:");
    typeLabel->setStyleSheet(labelStyle);
    QLabel *quantityLabel = new QLabel("Quantité:");
    quantityLabel->setStyleSheet(labelStyle);
    QLabel *thresholdLabel = new QLabel("Seuil min:");
    thresholdLabel->setStyleSheet(labelStyle);
    QLabel *dateLabel = new QLabel("Date ajout:");
    dateLabel->setStyleSheet(labelStyle);
    QLabel *supplierLabel = new QLabel("Fournisseur:");
    supplierLabel->setStyleSheet(labelStyle);
    QLabel *statusLabel = new QLabel("Statut:");
    statusLabel->setStyleSheet(labelStyle);

    inputForm->addRow(idLabel, idEdit);
    inputForm->addRow(nameLabel, nameEdit);
    inputForm->addRow(typeLabel, typeBox);
    inputForm->addRow(quantityLabel, quantityEdit);
    inputForm->addRow(thresholdLabel, thresholdEdit);
    inputForm->addRow(dateLabel, dateEdit);
    inputForm->addRow(supplierLabel, supplierEdit);
    inputForm->addRow(statusLabel, statusBox);

    int fieldHeight = 38;
    int labelWidth = 100;

    idEdit->setFixedHeight(fieldHeight);
    nameEdit->setFixedHeight(fieldHeight);
    typeBox->setFixedHeight(fieldHeight);
    quantityEdit->setFixedHeight(fieldHeight);
    thresholdEdit->setFixedHeight(fieldHeight);
    dateEdit->setFixedHeight(fieldHeight);
    supplierEdit->setFixedHeight(fieldHeight);
    statusBox->setFixedHeight(fieldHeight);

    idLabel->setFixedWidth(labelWidth);
    nameLabel->setFixedWidth(labelWidth);
    typeLabel->setFixedWidth(labelWidth);
    quantityLabel->setFixedWidth(labelWidth);
    thresholdLabel->setFixedWidth(labelWidth);
    dateLabel->setFixedWidth(labelWidth);
    supplierLabel->setFixedWidth(labelWidth);
    statusLabel->setFixedWidth(labelWidth);

    formLayout->addLayout(inputForm);

    QHBoxLayout *formButtons = new QHBoxLayout;
    formButtons->setSpacing(10);
    formButtons->setContentsMargins(0, 20, 0, 0);

    QPushButton *addBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    modifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *clearBtn = createStyledButton("🔄 Effacer", "#95a5a6");

    int buttonHeight = 38;
    addBtn->setFixedHeight(buttonHeight);
    modifyBtn->setFixedHeight(buttonHeight);
    clearBtn->setFixedHeight(buttonHeight);
    addBtn->setMinimumWidth(120);
    modifyBtn->setMinimumWidth(120);
    clearBtn->setMinimumWidth(120);

    formButtons->addWidget(addBtn);
    formButtons->addWidget(modifyBtn);
    formButtons->addWidget(clearBtn);

    formLayout->addLayout(formButtons);
    formLayout->addStretch();

    contentLayout->addWidget(formFrame, 1);

    // Colonne droite - Tableau
    QFrame *tableFrame = new QFrame;
    tableFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "}"
        );

    QVBoxLayout *tableFrameLayout = new QVBoxLayout(tableFrame);

    QLabel *tableTitle = new QLabel("Inventaire du matériel");
    tableTitle->setStyleSheet("color: #2c3e50; font-size: 16px; font-weight: bold; padding: 20px 20px 10px 20px;");
    tableFrameLayout->addWidget(tableTitle);

    materialTable = new QTableWidget(0, 8);
    QStringList headers = {"ID", "NOM", "TYPE", "QUANTITÉ", "SEUIL", "DATE", "FOURNISSEUR", "STATUT"};
    materialTable->setHorizontalHeaderLabels(headers);

    materialTable->setStyleSheet(
        "QTableWidget {"
        "    background-color: white;"
        "    border: none;"
        "    font-size: 13px;"
        "}"
        "QTableWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid #f1f2f6;"
        "    color: #2c3e50;"
        "}"
        "QHeaderView::section {"
        "    background-color: #2c3e50;"
        "    color: white;"
        "    padding: 12px;"
        "    border: none;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "}"
        );

    materialTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    materialTable->verticalHeader()->setVisible(false);
    materialTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    materialTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableFrameLayout->addWidget(materialTable, 1);
    contentLayout->addWidget(tableFrame, 2);

    pageLayout->addLayout(contentLayout, 1);

    // Connexions
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addMaterial);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::deleteMaterial);
    connect(modifyBtn, &QPushButton::clicked, this, &MainWindow::modifyMaterial);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearFields);
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::searchMaterial);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportToPdf);
    connect(statsBtn, &QPushButton::clicked, this, &MainWindow::showStatistics);
    connect(sortBtn, &QPushButton::clicked, this, &MainWindow::sortMaterial);

    connect(materialTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = materialTable->currentRow();
        if (row >= 0) {
            idEdit->setText(materialTable->item(row, 0)->text());
            nameEdit->setText(materialTable->item(row, 1)->text());

            QString type = materialTable->item(row, 2)->text();
            int typeIndex = typeBox->findText(type);
            if (typeIndex >= 0) typeBox->setCurrentIndex(typeIndex);

            quantityEdit->setText(materialTable->item(row, 3)->text());
            thresholdEdit->setText(materialTable->item(row, 4)->text());

            QDate date = QDate::fromString(materialTable->item(row, 5)->text(), "dd/MM/yyyy");
            dateEdit->setDate(date.isValid() ? date : QDate::currentDate());

            supplierEdit->setText(materialTable->item(row, 6)->text());

            QString status = materialTable->item(row, 7)->text();
            int statusIndex = statusBox->findText(status);
            if (statusIndex >= 0) statusBox->setCurrentIndex(statusIndex);
        }
    });

    pagesWidget->addWidget(materialPage);
}

void MainWindow::addMaterial()
{
    if (nameEdit->text().isEmpty() || quantityEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir le nom et la quantité.");
        return;
    }

    Material newMaterial;
    newMaterial.id = nextId++;
    newMaterial.name = nameEdit->text();
    newMaterial.type = typeBox->currentText();
    newMaterial.quantity = quantityEdit->text().toInt();
    newMaterial.threshold = thresholdEdit->text().toInt();
    newMaterial.addedDate = dateEdit->date();
    newMaterial.supplier = supplierEdit->text();
    newMaterial.status = statusBox->currentText();

    if (newMaterial.quantity <= newMaterial.threshold && newMaterial.status != "Rupture") {
        newMaterial.status = "Rupture";
        statusBox->setCurrentText("Rupture");
    }

    materialsList.append(newMaterial);
    updateMaterialTable();
    clearFields();

    QMessageBox::information(this, "Succès", "Matériel ajouté avec succès !");
}

void MainWindow::modifyMaterial()
{
    int currentId = idEdit->text().toInt();
    if (currentId <= 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un matériel à modifier.");
        return;
    }

    for (Material &material : materialsList) {
        if (material.id == currentId) {
            material.name = nameEdit->text();
            material.type = typeBox->currentText();
            material.quantity = quantityEdit->text().toInt();
            material.threshold = thresholdEdit->text().toInt();
            material.addedDate = dateEdit->date();
            material.supplier = supplierEdit->text();
            material.status = statusBox->currentText();

            if (material.quantity <= material.threshold && material.status != "Rupture") {
                material.status = "Rupture";
            }
            break;
        }
    }

    updateMaterialTable();
    QMessageBox::information(this, "Succès", "Matériel modifié avec succès !");
}

void MainWindow::deleteMaterial()
{
    int row = materialTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Information", "Sélectionnez un matériel à supprimer.");
        return;
    }

    int id = materialTable->item(row, 0)->text().toInt();

    for (int i = 0; i < materialsList.size(); ++i) {
        if (materialsList[i].id == id) {
            materialsList.removeAt(i);
            updateMaterialTable();
            clearFields();
            QMessageBox::information(this, "Succès", "Matériel supprimé avec succès !");
            return;
        }
    }
}

void MainWindow::searchMaterial()
{
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

void MainWindow::sortMaterial()
{
    QString sortCriteria = sortComboBox->currentText();

    if (sortCriteria == "Trier par ID (croissant)") {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) {
                      return a.id < b.id;
                  });
    }
    else if (sortCriteria == "Trier par ID (décroissant)") {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) {
                      return a.id > b.id;
                  });
    }
    else if (sortCriteria == "Trier par quantité (croissant)") {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) {
                      return a.quantity < b.quantity;
                  });
    }
    else if (sortCriteria == "Trier par quantité (décroissant)") {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) {
                      return a.quantity > b.quantity;
                  });
    }
    else if (sortCriteria == "Trier par type") {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) {
                      return a.type < b.type;
                  });
    }
    else if (sortCriteria == "Trier par nom") {
        std::sort(materialsList.begin(), materialsList.end(),
                  [](const Material &a, const Material &b) {
                      return a.name < b.name;
                  });
    }

    updateMaterialTable();

    // CORRECTION : Message simple sans concaténation problématique
    QString message = "Liste triée avec succès !\nCritère : " + sortCriteria;
    QMessageBox::information(this, "Tri effectué", message);
}

void MainWindow::updateMaterialTable()
{
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
}

void MainWindow::clearFields()
{
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

void MainWindow::exportToPdf()
{
    QMessageBox::information(this, "Export PDF", "Fonction d'export PDF à implémenter");
}

void MainWindow::showStatistics()
{
    QMessageBox::information(this, "Statistiques", "Fonction de statistiques à implémenter");
}

void MainWindow::setupUsersPage()
{
    usersPage = new QWidget;
    usersPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(usersPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *emptyFrame = new QFrame;
    emptyFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "    padding: 40px;"
        "}"
        );

    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyFrame);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QLabel *iconLabel = new QLabel("👥");
    iconLabel->setStyleSheet("font-size: 60px; margin-bottom: 20px;");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("GESTION DES UTILISATEURS");
    titleLabel->setStyleSheet("color: #2c3e50; font-size: 24px; font-weight: bold; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *messageLabel = new QLabel(
        "Cette page est maintenant vide.\n\n"
        "➤ La fonctionnalité a été déplacée vers la page Matériel\n"
        "➤ L'interface des utilisateurs est disponible comme exemple\n"
        "➤ Vous pouvez réimplémenter cette page si nécessaire\n\n"
        "Exemple de code disponible dans les fichiers sources."
        );
    messageLabel->setStyleSheet("color: #7f8c8d; font-size: 14px; text-align: center; line-height: 1.5;");
    messageLabel->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(iconLabel);
    emptyLayout->addWidget(titleLabel);
    emptyLayout->addWidget(messageLabel);

    pageLayout->addWidget(emptyFrame, 1);
    pagesWidget->addWidget(usersPage);
}

void MainWindow::setupClientsPage()
{
    clientsPage = new QWidget;
    clientsPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(clientsPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *emptyFrame = new QFrame;
    emptyFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "    padding: 40px;"
        "}"
        );

    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyFrame);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QLabel *iconLabel = new QLabel("👨‍💼");
    iconLabel->setStyleSheet("font-size: 60px; margin-bottom: 20px;");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("GESTION DES CLIENTS");
    titleLabel->setStyleSheet("color: #2c3e50; font-size: 24px; font-weight: bold; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *messageLabel = new QLabel(
        "Cette page est actuellement vide.\n\n"
        "➤ À implémenter par : [Nom du collègue]\n"
        "➤ Fonctionnalités attendues :\n"
        "   • Gestion de la base clients\n"
        "   • Suivi des contacts\n"
        "   • Historique des achats\n"
        "   • Segmentation clients\n\n"
        "Utilisez la structure de la page Matériel comme exemple."
        );
    messageLabel->setStyleSheet("color: #7f8c8d; font-size: 14px; text-align: center; line-height: 1.5;");
    messageLabel->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(iconLabel);
    emptyLayout->addWidget(titleLabel);
    emptyLayout->addWidget(messageLabel);

    pageLayout->addWidget(emptyFrame, 1);
    pagesWidget->addWidget(clientsPage);
}

void MainWindow::setupOrdersPage()
{
    ordersPage = new QWidget;
    ordersPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(ordersPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *emptyFrame = new QFrame;
    emptyFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "    padding: 40px;"
        "}"
        );

    QVBoxLayout *emptyLayout = new QVBoxLayout(emptyFrame);
    emptyLayout->setAlignment(Qt::AlignCenter);

    QLabel *iconLabel = new QLabel("📦");
    iconLabel->setStyleSheet("font-size: 60px; margin-bottom: 20px;");
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("GESTION DES COMMANDES");
    titleLabel->setStyleSheet("color: #2c3e50; font-size: 24px; font-weight: bold; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *messageLabel = new QLabel(
        "Cette page est actuellement vide.\n\n"
        "➤ À implémenter par : [Nom du collègue]\n"
        "➤ Fonctionnalités attendues :\n"
        "   • Création de commandes\n"
        "   • Suivi des livraisons\n"
        "   • Facturation\n"
        "   • Statistiques de vente\n\n"
        "Utilisez la structure de la page Matériel comme exemple."
        );
    messageLabel->setStyleSheet("color: #7f8c8d; font-size: 14px; text-align: center; line-height: 1.5;");
    messageLabel->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(iconLabel);
    emptyLayout->addWidget(titleLabel);
    emptyLayout->addWidget(messageLabel);

    pageLayout->addWidget(emptyFrame, 1);
    pagesWidget->addWidget(ordersPage);
}

void MainWindow::changePage(int index)
{
    pagesWidget->setCurrentIndex(index);
}

QPushButton* MainWindow::createStyledButton(const QString& text, const QString& color) {
    QPushButton *button = new QPushButton(text);
    button->setStyleSheet(QString(
                              "QPushButton {"
                              "    background-color: %1;"
                              "    color: white;"
                              "    border: none;"
                              "    border-radius: 6px;"
                              "    padding: 8px 16px;"
                              "    font-size: 13px;"
                              "    font-weight: 500;"
                              "}"
                              "QPushButton:hover {"
                              "    opacity: 0.9;"
                              "}"
                              ).arg(color));

    button->setCursor(Qt::PointingHandCursor);
    return button;
}
