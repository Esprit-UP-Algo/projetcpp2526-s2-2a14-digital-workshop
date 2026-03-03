#include "materiel.h"
#include "connection.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateEdit>
#include <QMessageBox>
#include <QFrame>
#include <QLabel>
#include <QDate>
#include <QFileDialog>
#include <QTextStream>
#include <algorithm>
#include <QSpinBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Materiel::Materiel(QWidget *parent)
    : QWidget(parent), nextMaterialId(1), databaseEnabled(false)
{
    setupUI();

    if (Connection::getInstance()->isOpen()) {
        databaseEnabled = true;
        loadMaterialsFromDatabase();

        if (!materialsList.isEmpty()) {
            nextMaterialId = materialsList.last().id + 1;
        }
    } else {
        databaseEnabled = false;
        addExampleMaterials();
    }

    materialIdEdit->setText(QString::number(nextMaterialId));
    updateMaterialsTable();
}

QPushButton* Materiel::createStyledButton(const QString& text, const QString& color)
{
    QPushButton *button = new QPushButton(text);
    QString style = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 15px;"
        "    font-weight: bold;"
        "    font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "}"
    ).arg(color).arg(color).arg(color); 

    button->setStyleSheet(style);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void Materiel::setupUI()
{
    setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(this);
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

    // Recherche
    materialSearchEdit = new QLineEdit;
    materialSearchEdit->setPlaceholderText("🔍 Rechercher...");
    materialSearchEdit->setStyleSheet(
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

    QPushButton *materialSearchBtn = createStyledButton("Chercher", "#3498db");
    QPushButton *materialDeleteBtn = createStyledButton("Supprimer", "#e74c3c");

    headerLayout->addWidget(materialSearchEdit);
    headerLayout->addWidget(materialSearchBtn);

    // Boutons de tri
    QPushButton *sortMatNameBtn = createStyledButton("📝 Tri Nom", "#16a085");
    QPushButton *sortMatTypeBtn = createStyledButton("🏷️ Tri Type", "#e67e22");
    QPushButton *sortMatQtyBtn = createStyledButton("🔢 Tri Quantité", "#8e44ad");
    QPushButton *sortMatStatusBtn = createStyledButton("📊 Tri Statut", "#9b59b6");

    headerLayout->addWidget(sortMatNameBtn);
    headerLayout->addWidget(sortMatTypeBtn);
    headerLayout->addWidget(sortMatQtyBtn);
    headerLayout->addWidget(sortMatStatusBtn);
    headerLayout->addWidget(materialDeleteBtn);

    pageLayout->addWidget(header);

    // === CONTENU (2 colonnes) ===
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

    QLabel *formTitle = new QLabel("NOUVEAU MATÉRIEL");
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
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 4px solid transparent;"
        "    border-right: 4px solid transparent;"
        "    border-top: 5px solid #2c3e50;"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "}";

    QString spinBoxStyle =
        "QSpinBox {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "    border: none;"
        "    background-color: #e9ecef;"
        "    width: 20px;"
        "}";

    QString dateEditStyle =
        "QDateEdit {"
        "    color: black;"
        "    background-color: white;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
        "QDateEdit::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}";

    // Inputs
    materialIdEdit = new QLineEdit;
    materialIdEdit->setReadOnly(true);
    materialIdEdit->setText(QString::number(nextMaterialId));
    materialIdEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #e9ecef;"
        "    color: #6c757d;"
        "    border: 1px solid #ced4da;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "}"
    );

    materialNameEdit = new QLineEdit;
    materialNameEdit->setPlaceholderText("Nom du matériel");
    materialNameEdit->setStyleSheet(lineEditStyle);

    materialTypeBox = new QComboBox;
    materialTypeBox->addItems({"Bois", "Outil", "Consommable", "Électronique", "Mécanique", "Bureau", "Sécurité"});
    materialTypeBox->setStyleSheet(comboBoxStyle);

    materialQuantityEdit = new QSpinBox;
    materialQuantityEdit->setMinimum(0);
    materialQuantityEdit->setMaximum(9999);
    materialQuantityEdit->setValue(1);
    materialQuantityEdit->setStyleSheet(spinBoxStyle);

    materialMinThresholdEdit = new QSpinBox;
    materialMinThresholdEdit->setMinimum(0);
    materialMinThresholdEdit->setMaximum(9999);
    materialMinThresholdEdit->setValue(5);
    materialMinThresholdEdit->setStyleSheet(spinBoxStyle);

    materialDateEdit = new QDateEdit;
    materialDateEdit->setDate(QDate::currentDate());
    materialDateEdit->setCalendarPopup(true);
    materialDateEdit->setDisplayFormat("dd/MM/yyyy");
    materialDateEdit->setStyleSheet(dateEditStyle);

    materialSupplierEdit = new QLineEdit;
    materialSupplierEdit->setPlaceholderText("Nom du fournisseur");
    materialSupplierEdit->setStyleSheet(lineEditStyle);

    materialStatusBox = new QComboBox;
    materialStatusBox->addItems({"Disponible", "Rupture", "En maintenance", "En prêt", "Hors service"});
    materialStatusBox->setStyleSheet(comboBoxStyle);

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

    auto createLabel = [&](const QString &text) {
        QLabel *l = new QLabel(text);
        l->setStyleSheet(labelStyle);
        return l;
    };

    inputForm->addRow(createLabel("ID Matériel:"), materialIdEdit);
    inputForm->addRow(createLabel("Nom:"), materialNameEdit);
    inputForm->addRow(createLabel("Type:"), materialTypeBox);
    inputForm->addRow(createLabel("Quantité:"), materialQuantityEdit);
    inputForm->addRow(createLabel("Seuil min:"), materialMinThresholdEdit);
    inputForm->addRow(createLabel("Date d'ajout:"), materialDateEdit);
    inputForm->addRow(createLabel("Fournisseur:"), materialSupplierEdit);
    inputForm->addRow(createLabel("Statut:"), materialStatusBox);

    int fieldHeight = 38;
    materialIdEdit->setFixedHeight(fieldHeight);
    materialNameEdit->setFixedHeight(fieldHeight);
    materialTypeBox->setFixedHeight(fieldHeight);
    materialQuantityEdit->setFixedHeight(fieldHeight);
    materialMinThresholdEdit->setFixedHeight(fieldHeight);
    materialDateEdit->setFixedHeight(fieldHeight);
    materialSupplierEdit->setFixedHeight(fieldHeight);
    materialStatusBox->setFixedHeight(fieldHeight);

    inputForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    formLayout->addLayout(inputForm);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
    formLayout->addItem(verticalSpacer);

    QHBoxLayout *formButtons = new QHBoxLayout;
    formButtons->setSpacing(10);
    formButtons->setContentsMargins(0, 10, 0, 0);

    QPushButton *materialAddBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    materialModifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *materialClearBtn = createStyledButton("🔄 Effacer", "#95a5a6");
    QPushButton *materialExportBtn = createStyledButton("📥 Exporter", "#9b59b6");

    materialAddBtn->setFixedHeight(fieldHeight);
    materialModifyBtn->setFixedHeight(fieldHeight);
    materialClearBtn->setFixedHeight(fieldHeight);
    materialExportBtn->setFixedHeight(fieldHeight);

    formButtons->addWidget(materialAddBtn);
    formButtons->addWidget(materialModifyBtn);
    formButtons->addWidget(materialClearBtn);
    formButtons->addWidget(materialExportBtn);

    formLayout->addLayout(formButtons);

    // Tableau de bord matériel
    QFrame *dashboardFrame = new QFrame;
    dashboardFrame->setStyleSheet("background-color: #f1f2f6; border-radius: 8px; margin-top: 15px;");
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardFrame);
    
    QLabel *dashTitle = new QLabel("TABLEAU DE BORD");
    dashTitle->setStyleSheet("font-weight: bold; color: #2c3e50; font-size: 14px;");
    dashboardLayout->addWidget(dashTitle);

    totalMaterialsLabel = new QLabel("Total articles: 0");
    alertMaterialsLabel = new QLabel("Articles en alerte: 0");
    ruptureMaterialsLabel = new QLabel("Articles en rupture: 0");
    
    QString dashLabelStyle = "color: #34495e; font-size: 13px;";
    totalMaterialsLabel->setStyleSheet(dashLabelStyle);
    alertMaterialsLabel->setStyleSheet("color: #e67e22; font-size: 13px; font-weight: bold;");
    ruptureMaterialsLabel->setStyleSheet("color: #e74c3c; font-size: 13px; font-weight: bold;");
    
    dashboardLayout->addWidget(totalMaterialsLabel);
    dashboardLayout->addWidget(alertMaterialsLabel);
    dashboardLayout->addWidget(ruptureMaterialsLabel);
    
    formLayout->addWidget(dashboardFrame);

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

    QLabel *tableTitle = new QLabel("INVENTAIRE");
    tableTitle->setStyleSheet("color: #2c3e50; font-size: 16px; font-weight: bold; padding: 20px 20px 10px 20px;");
    tableFrameLayout->addWidget(tableTitle);

    materialTable = new QTableWidget(0, 8);
    QStringList headers = {"ID", "NOM", "TYPE", "QUANTITÉ", "SEUIL", "FOURNISSEUR", "DATE", "STATUT"};
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
    connect(materialAddBtn, &QPushButton::clicked, this, &Materiel::addMaterial);
    connect(materialDeleteBtn, &QPushButton::clicked, this, &Materiel::deleteMaterial);
    connect(materialModifyBtn, &QPushButton::clicked, this, &Materiel::modifyMaterial);
    connect(materialClearBtn, &QPushButton::clicked, this, &Materiel::clearMaterialFields);
    connect(materialSearchBtn, &QPushButton::clicked, this, &Materiel::searchMaterial);
    connect(materialExportBtn, &QPushButton::clicked, this, &Materiel::exportMaterials);

    connect(sortMatNameBtn, &QPushButton::clicked, this, &Materiel::sortMaterialsByName);
    connect(sortMatTypeBtn, &QPushButton::clicked, this, &Materiel::sortMaterialsByType);
    connect(sortMatQtyBtn, &QPushButton::clicked, this, &Materiel::sortMaterialsByQuantity);
    connect(sortMatStatusBtn, &QPushButton::clicked, this, &Materiel::sortMaterialsByStatus);

    connect(materialTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = materialTable->currentRow();
        if (row >= 0) {
            materialIdEdit->setText(materialTable->item(row, 0)->text());
            materialNameEdit->setText(materialTable->item(row, 1)->text());
            
            QString type = materialTable->item(row, 2)->text();
            int typeIndex = materialTypeBox->findText(type);
            if (typeIndex >= 0) materialTypeBox->setCurrentIndex(typeIndex);
            
            materialQuantityEdit->setValue(materialTable->item(row, 3)->text().toInt());
            materialMinThresholdEdit->setValue(materialTable->item(row, 4)->text().toInt());
            materialSupplierEdit->setText(materialTable->item(row, 5)->text());
            materialDateEdit->setDate(QDate::fromString(materialTable->item(row, 6)->text(), "dd/MM/yyyy"));
            
            QString status = materialTable->item(row, 7)->text();
            int statusIndex = materialStatusBox->findText(status);
            if (statusIndex >= 0) materialStatusBox->setCurrentIndex(statusIndex);
        }
    });
}

void Materiel::addMaterial()
{
    if (materialNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir le nom du matériel.");
        return;
    }

    Material newMat;
    newMat.id = nextMaterialId++;
    newMat.name = materialNameEdit->text();
    newMat.type = materialTypeBox->currentText();
    newMat.quantity = materialQuantityEdit->value();
    newMat.minThreshold = materialMinThresholdEdit->value();
    newMat.supplier = materialSupplierEdit->text();
    newMat.acquisitionDate = materialDateEdit->date();
    
    // Auto-update status based on quantity
    if (newMat.quantity == 0) {
        newMat.status = "Rupture";
        materialStatusBox->setCurrentText("Rupture");
    } else {
        newMat.status = materialStatusBox->currentText();
    }

    materialsList.append(newMat);

    if (databaseEnabled) {
        saveMaterialToDatabase(newMat);
    }

    updateMaterialsTable();
    clearMaterialFields();

    QMessageBox::information(this, "Succès", "Matériel ajouté avec succès !");
}

void Materiel::deleteMaterial()
{
    int row = materialTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Information", "Sélectionnez un matériel à supprimer.");
        return;
    }

    int id = materialTable->item(row, 0)->text().toInt();

    for (int i = 0; i < materialsList.size(); ++i) {
        if (materialsList[i].id == id) {
            if (databaseEnabled) {
                deleteMaterialFromDatabase(id);
            }

            materialsList.removeAt(i);
            updateMaterialsTable();
            clearMaterialFields();
            QMessageBox::information(this, "Succès", "Matériel supprimé avec succès !");
            return;
        }
    }
}

void Materiel::modifyMaterial()
{
    int currentId = materialIdEdit->text().toInt();
    if (currentId <= 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un matériel à modifier.");
        return;
    }

    for (Material &mat : materialsList) {
        if (mat.id == currentId) {
            mat.name = materialNameEdit->text();
            mat.type = materialTypeBox->currentText();
            mat.quantity = materialQuantityEdit->value();
            mat.minThreshold = materialMinThresholdEdit->value();
            mat.supplier = materialSupplierEdit->text();
            mat.acquisitionDate = materialDateEdit->date();
            
            // Auto-update status
            if (mat.quantity == 0) {
                mat.status = "Rupture";
            } else {
                mat.status = materialStatusBox->currentText();
            }

            if (databaseEnabled) {
                updateMaterialInDatabase(mat);
            }
            break;
        }
    }

    updateMaterialsTable();
    QMessageBox::information(this, "Succès", "Matériel modifié avec succès !");
}

void Materiel::clearMaterialFields()
{
    materialNameEdit->clear();
    materialTypeBox->setCurrentIndex(0);
    materialQuantityEdit->setValue(1);
    materialMinThresholdEdit->setValue(5);
    materialSupplierEdit->clear();
    materialDateEdit->setDate(QDate::currentDate());
    materialStatusBox->setCurrentIndex(0);
    materialTable->clearSelection();

    materialIdEdit->setText(QString::number(nextMaterialId));
}

void Materiel::searchMaterial()
{
    QString searchText = materialSearchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        updateMaterialsTable();
        return;
    }

    materialTable->setRowCount(0);

    for (const Material &mat : materialsList) {
        if (mat.name.contains(searchText, Qt::CaseInsensitive) ||
            mat.supplier.contains(searchText, Qt::CaseInsensitive) ||
            mat.type.contains(searchText, Qt::CaseInsensitive)) {

            int row = materialTable->rowCount();
            materialTable->insertRow(row);

            materialTable->setItem(row, 0, new QTableWidgetItem(QString::number(mat.id)));
            materialTable->setItem(row, 1, new QTableWidgetItem(mat.name));
            materialTable->setItem(row, 2, new QTableWidgetItem(mat.type));
            
            QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(mat.quantity));
            if (mat.quantity == 0) qtyItem->setForeground(QBrush(QColor("#e74c3c")));
            else if (mat.quantity <= mat.minThreshold) qtyItem->setForeground(QBrush(QColor("#e67e22")));
            materialTable->setItem(row, 3, qtyItem);
            
            materialTable->setItem(row, 4, new QTableWidgetItem(QString::number(mat.minThreshold)));
            materialTable->setItem(row, 5, new QTableWidgetItem(mat.supplier));
            materialTable->setItem(row, 6, new QTableWidgetItem(mat.acquisitionDate.toString("dd/MM/yyyy")));
            
            QTableWidgetItem *statusItem = new QTableWidgetItem(mat.status);
            if (mat.status == "Rupture") statusItem->setForeground(QBrush(QColor("#e74c3c")));
            materialTable->setItem(row, 7, statusItem);
        }
    }
}

void Materiel::updateMaterialsTable()
{
    materialTable->setRowCount(0);

    for (const Material &mat : materialsList) {
        int row = materialTable->rowCount();
        materialTable->insertRow(row);

        materialTable->setItem(row, 0, new QTableWidgetItem(QString::number(mat.id)));
        materialTable->setItem(row, 1, new QTableWidgetItem(mat.name));
        materialTable->setItem(row, 2, new QTableWidgetItem(mat.type));
        
        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(mat.quantity));
        if (mat.quantity == 0) {
            qtyItem->setForeground(QBrush(QColor("#e74c3c"))); // Red
        } else if (mat.quantity <= mat.minThreshold) {
            qtyItem->setForeground(QBrush(QColor("#e67e22"))); // Orange
        }
        materialTable->setItem(row, 3, qtyItem);
        
        materialTable->setItem(row, 4, new QTableWidgetItem(QString::number(mat.minThreshold)));
        materialTable->setItem(row, 5, new QTableWidgetItem(mat.supplier));
        materialTable->setItem(row, 6, new QTableWidgetItem(mat.acquisitionDate.toString("dd/MM/yyyy")));
        
        QTableWidgetItem *statusItem = new QTableWidgetItem(mat.status);
        if (mat.status == "Rupture") {
            statusItem->setForeground(QBrush(QColor("#e74c3c")));
        }
        materialTable->setItem(row, 7, statusItem);
    }
    
    checkMaterialAlerts();
}

void Materiel::checkMaterialAlerts()
{
    int total = materialsList.size();
    int alerts = 0;
    int ruptures = 0;
    
    for (const Material &mat : materialsList) {
        if (mat.quantity == 0) {
            ruptures++;
        } else if (mat.quantity <= mat.minThreshold) {
            alerts++;
        }
    }
    
    totalMaterialsLabel->setText("Total articles: " + QString::number(total));
    alertMaterialsLabel->setText("Articles en alerte: " + QString::number(alerts));
    ruptureMaterialsLabel->setText("Articles en rupture: " + QString::number(ruptures));
}

void Materiel::sortMaterialsByName()
{
    std::sort(materialsList.begin(), materialsList.end(), [](const Material &a, const Material &b) {
        return a.name < b.name;
    });
    updateMaterialsTable();
}

void Materiel::sortMaterialsByType()
{
    std::sort(materialsList.begin(), materialsList.end(), [](const Material &a, const Material &b) {
        return a.type < b.type;
    });
    updateMaterialsTable();
}

void Materiel::sortMaterialsByQuantity()
{
    std::sort(materialsList.begin(), materialsList.end(), [](const Material &a, const Material &b) {
        return a.quantity < b.quantity;
    });
    updateMaterialsTable();
}

void Materiel::sortMaterialsByStatus()
{
    std::sort(materialsList.begin(), materialsList.end(), [](const Material &a, const Material &b) {
        return a.status < b.status;
    });
    updateMaterialsTable();
}

void Materiel::exportMaterials()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter le matériel", "", "Fichiers CSV (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier pour l'export.");
        return;
    }

    QTextStream out(&file);
    out << "ID;Nom;Type;Quantité;Seuil Min;Fournisseur;Date;Statut\n";

    for (const Material &mat : materialsList) {
        out << mat.id << ";"
            << mat.name << ";"
            << mat.type << ";"
            << mat.quantity << ";"
            << mat.minThreshold << ";"
            << mat.supplier << ";"
            << mat.acquisitionDate.toString("dd/MM/yyyy") << ";"
            << mat.status << "\n";
    }

    file.close();
    QMessageBox::information(this, "Succès", "Matériel exporté avec succès !");
}

void Materiel::addExampleMaterials()
{
    Material m1 = {1, "Planches de Chêne", "Bois", 50, 10, "BricoBois", QDate(2024, 1, 15), "Disponible"};
    Material m2 = {2, "Scie Circulaire", "Outil", 2, 1, "OutilPro", QDate(2023, 11, 20), "Disponible"};
    Material m3 = {3, "Vis Bois 4x40", "Consommable", 5, 500, "FixFast", QDate(2024, 5, 2), "Rupture"};
    Material m4 = {4, "Colle à bois", "Consommable", 8, 10, "ChemPro", QDate(2024, 4, 10), "Disponible"};

    materialsList << m1 << m2 << m3 << m4;
    nextMaterialId = 5;
}

void Materiel::loadMaterialsFromDatabase()
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    QSqlQuery query(db);

    if (query.exec("SELECT ID, NOM, TYPE, QUANTITE, SEUIL_MIN, DATE_AJOUT, FOURNISSEUR, STATUT FROM MATERIELS")) {
        materialsList.clear();
        while (query.next()) {
            Material material;
            material.id = query.value(0).toInt();
            material.name = query.value(1).toString();
            material.type = query.value(2).toString();
            material.quantity = query.value(3).toInt();
            material.minThreshold = query.value(4).toInt();
            material.acquisitionDate = QDate::fromString(query.value(5).toString(), "yyyy-MM-dd");
            material.supplier = query.value(6).toString();
            material.status = query.value(7).toString();
            materialsList.append(material);
        }
        updateMaterialsTable();
        qDebug() << "Matériels chargés depuis Oracle:" << materialsList.size();
    } else {
        qDebug() << "Erreur lors du chargement des matériels:" << query.lastError().text();
    }
}

void Materiel::saveMaterialToDatabase(const Material &material)
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    QSqlQuery query(db);
    query.prepare("INSERT INTO MATERIELS (ID, NOM, TYPE, QUANTITE, SEUIL_MIN, DATE_AJOUT, FOURNISSEUR, STATUT) "
                  "VALUES (:id, :nom, :type, :quantite, :seuil, :date, :fournisseur, :statut)");

    query.bindValue(":id", material.id);
    query.bindValue(":nom", material.name);
    query.bindValue(":type", material.type);
    query.bindValue(":quantite", material.quantity);
    query.bindValue(":seuil", material.minThreshold);
    query.bindValue(":date", material.acquisitionDate.toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", material.supplier);
    query.bindValue(":statut", material.status);

    if (!query.exec()) {
        qDebug() << "Erreur lors de l'insertion du matériel:" << query.lastError().text();
    } else {
        qDebug() << "Matériel ajouté en base de données, ID:" << material.id;
    }
}

void Materiel::deleteMaterialFromDatabase(int id)
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM MATERIELS WHERE ID = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la suppression du matériel:" << query.lastError().text();
    } else {
        qDebug() << "Matériel supprimé de la base de données, ID:" << id;
    }
}

void Materiel::updateMaterialInDatabase(const Material &material)
{
    if (!databaseEnabled) return;

    QSqlDatabase db = Connection::getInstance()->getDatabase();
    QSqlQuery query(db);
    query.prepare("UPDATE MATERIELS SET NOM = :nom, TYPE = :type, QUANTITE = :quantite, "
                  "SEUIL_MIN = :seuil, DATE_AJOUT = :date, FOURNISSEUR = :fournisseur, STATUT = :statut WHERE ID = :id");

    query.bindValue(":id", material.id);
    query.bindValue(":nom", material.name);
    query.bindValue(":type", material.type);
    query.bindValue(":quantite", material.quantity);
    query.bindValue(":seuil", material.minThreshold);
    query.bindValue(":date", material.acquisitionDate.toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", material.supplier);
    query.bindValue(":statut", material.status);

    if (!query.exec()) {
        qDebug() << "Erreur lors de la mise à jour du matériel:" << query.lastError().text();
    } else {
        qDebug() << "Matériel mis à jour en base de données, ID:" << material.id;
    }
}
