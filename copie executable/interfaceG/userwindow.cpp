#include "userwindow.h"

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
#include <QSpinBox>
#include <QGroupBox>
#include <QDate>
#include <algorithm>
#include <QFileDialog>
#include <QTextStream>

UserWindow::UserWindow(QWidget *parent)
    : QMainWindow(parent), nextId(1), nextMaterialId(1)
{
    QWidget *centralWidget = new QWidget;
    centralWidget->setStyleSheet("background-color: #f5f6fa;");
    setCentralWidget(centralWidget);

    // Layout principal
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // === SIDEBAR GAUCHE (Navigation) ===
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

    // Titre de l'application
    QLabel *appTitle = new QLabel("🏢 GESTION");
    appTitle->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(appTitle);

    // Navigation
    navList = new QListWidget;
    navList->addItem("👥 Utilisateurs");
    navList->addItem("🛠️ Matériel");
    navList->addItem("👨‍💼 Clients");
    navList->addItem("📦 Commandes");
    navList->setCurrentRow(0);

    connect(navList, &QListWidget::currentRowChanged, this, &UserWindow::changePage);

    sidebarLayout->addWidget(navList);
    sidebarLayout->addStretch();

    // Footer
    QLabel *footer = new QLabel("v1.0 - En développement");
    footer->setStyleSheet("color: #7f8c8d; font-size: 11px; padding: 10px;");
    footer->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(footer);

    // === CONTENU PRINCIPAL ===
    pagesWidget = new QStackedWidget;

    // Créer les 4 pages
    setupUsersPage();
    setupMaterialPage();
    setupClientsPage();
    setupOrdersPage();

    // === ASSEMBLAGE ===
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pagesWidget, 1);

    setWindowTitle("Système de Gestion - Version collaborative");
    resize(1200, 700);

    // Ajouter quelques utilisateurs d'exemple avec dates de création
    User user1 = {1, "Admin Principal", "admin@entreprise.com", "Admin", "Actif", QDate(2024, 1, 15)};
    User user2 = {2, "Manager RH", "rh@entreprise.com", "Responsable", "Actif", QDate(2024, 3, 10)};
    User user3 = {3, "Technicien", "tech@entreprise.com", "Ouvrier", "Actif", QDate(2024, 2, 20)};
    User user4 = {4, "Analyste SI", "si@entreprise.com", "Technicien", "Actif", QDate(2024, 5, 5)};
    User user5 = {5, "Assistante", "assist@entreprise.com", "Ouvrier", "Inactif", QDate(2024, 4, 1)};
    usersList << user1 << user2 << user3 << user4 << user5;
    nextId = 6;
    updateUsersTable();
}

// ==================== PAGE UTILISATEURS (FONCTIONNELLE) ====================

void UserWindow::setupUsersPage()
{
    QWidget *usersPage = new QWidget;
    usersPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(usersPage);
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

    QLabel *pageTitle = new QLabel("👥 GESTION DES UTILISATEURS");
    pageTitle->setStyleSheet("color: #2c3e50; font-size: 22px; font-weight: bold;");

    headerLayout->addWidget(pageTitle);
    headerLayout->addStretch();

    // Recherche
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

    QPushButton *searchBtn = createStyledButton("Chercher", "#3498db");
    QPushButton *deleteBtn = createStyledButton("Supprimer", "#e74c3c");

    // Boutons de tri
    QPushButton *sortByDateBtn = createStyledButton("📅 Trier par date", "#9b59b6");
    QPushButton *sortByNameBtn = createStyledButton("👤 Trier par nom", "#1abc9c");

    headerLayout->addWidget(searchEdit);
    headerLayout->addWidget(searchBtn);
    headerLayout->addWidget(sortByDateBtn);
    headerLayout->addWidget(sortByNameBtn);
    headerLayout->addWidget(deleteBtn);

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

    QLabel *formTitle = new QLabel("Ajouter/Modifier un utilisateur");
    formTitle->setStyleSheet("color: #3498db; font-size: 16px; font-weight: bold; margin-bottom: 15px;");
    formLayout->addWidget(formTitle);

    // Style commun pour tous les champs de saisie
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

    // Inputs
    idEdit = new QLineEdit;
    idEdit->setReadOnly(true);
    idEdit->setText(QString::number(nextId));
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
    nameEdit->setPlaceholderText("Nom complet");
    nameEdit->setStyleSheet(lineEditStyle);

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText("email@entreprise.com");
    emailEdit->setStyleSheet(lineEditStyle);

    passwordEdit = new QLineEdit;
    passwordEdit->setPlaceholderText("Mot de passe");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(lineEditStyle);

    roleBox = new QComboBox;
    roleBox->addItems({"Admin", "Responsable", "Ouvrier", "Technicien"});
    roleBox->setStyleSheet(comboBoxStyle);

    statusBox = new QComboBox;
    statusBox->addItems({"Actif", "Inactif"});
    statusBox->setStyleSheet(comboBoxStyle);

    // Champ pour la date de création (caché mais utilisé pour le tri)
    creationDateEdit = new QDateEdit;
    creationDateEdit->setDate(QDate::currentDate());
    creationDateEdit->setVisible(false);

    // Formulaire avec espacement et alignement
    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(12);  // Espacement vertical entre les lignes
    inputForm->setContentsMargins(0, 0, 0, 0);

    // Style pour les labels
    QString labelStyle =
        "QLabel {"
        "    color: #2c3e50;"  // Bleu foncé au lieu de noir pur
        "    font-weight: 600;"  // Semi-gras
        "    font-size: 13px;"
        "    padding: 2px 0;"  // Petit padding vertical
        "}";

    // Créer les labels séparément pour mieux les contrôler
    QLabel *idLabel = new QLabel("ID:");
    idLabel->setStyleSheet(labelStyle);

    QLabel *nameLabel = new QLabel("Nom:");
    nameLabel->setStyleSheet(labelStyle);

    QLabel *emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet(labelStyle);

    QLabel *passwordLabel = new QLabel("Mot de passe:");
    passwordLabel->setStyleSheet(labelStyle);

    QLabel *roleLabel = new QLabel("Rôle:");
    roleLabel->setStyleSheet(labelStyle);

    QLabel *statusLabel = new QLabel("Statut:");
    statusLabel->setStyleSheet(labelStyle);

    // Ajouter les champs au formulaire
    inputForm->addRow(idLabel, idEdit);
    inputForm->addRow(nameLabel, nameEdit);
    inputForm->addRow(emailLabel, emailEdit);
    inputForm->addRow(passwordLabel, passwordEdit);
    inputForm->addRow(roleLabel, roleBox);
    inputForm->addRow(statusLabel, statusBox);

    // Configurer la taille des champs
    int fieldHeight = 38;
    idEdit->setFixedHeight(fieldHeight);
    nameEdit->setFixedHeight(fieldHeight);
    emailEdit->setFixedHeight(fieldHeight);
    passwordEdit->setFixedHeight(fieldHeight);
    roleBox->setFixedHeight(fieldHeight);
    statusBox->setFixedHeight(fieldHeight);

    // Aligner les labels à droite
    inputForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Fixer la largeur des labels pour l'alignement
    int labelWidth = 120;
    idLabel->setFixedWidth(labelWidth);
    nameLabel->setFixedWidth(labelWidth);
    emailLabel->setFixedWidth(labelWidth);
    passwordLabel->setFixedWidth(labelWidth);
    roleLabel->setFixedWidth(labelWidth);
    statusLabel->setFixedWidth(labelWidth);

    formLayout->addLayout(inputForm);

    // Espace avant les boutons
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
    formLayout->addItem(verticalSpacer);

    // Boutons avec espacement
    QHBoxLayout *formButtons = new QHBoxLayout;
    formButtons->setSpacing(10);
    formButtons->setContentsMargins(0, 10, 0, 0);

    QPushButton *addBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    modifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *clearBtn = createStyledButton("🔄 Effacer", "#95a5a6");

    // Taille fixe pour les boutons
    int buttonHeight = 38;
    addBtn->setFixedHeight(buttonHeight);
    modifyBtn->setFixedHeight(buttonHeight);
    clearBtn->setFixedHeight(buttonHeight);

    // Largeur minimale pour les boutons
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

    QLabel *tableTitle = new QLabel("Liste des utilisateurs");
    tableTitle->setStyleSheet("color: #2c3e50; font-size: 16px; font-weight: bold; padding: 20px 20px 10px 20px;");
    tableFrameLayout->addWidget(tableTitle);

    // Tableau avec colonne pour la date de création
    usersTable = new QTableWidget(0, 6);
    QStringList headers = {"ID", "NOM", "EMAIL", "RÔLE", "STATUT", "DATE CRÉATION"};
    usersTable->setHorizontalHeaderLabels(headers);

    usersTable->setStyleSheet(
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

    usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    usersTable->verticalHeader()->setVisible(false);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableFrameLayout->addWidget(usersTable, 1);
    contentLayout->addWidget(tableFrame, 2);

    pageLayout->addLayout(contentLayout, 1);

    // Connexions des signaux
    connect(addBtn, &QPushButton::clicked, this, &UserWindow::addUser);
    connect(deleteBtn, &QPushButton::clicked, this, &UserWindow::deleteUser);
    connect(modifyBtn, &QPushButton::clicked, this, &UserWindow::modifyUser);
    connect(clearBtn, &QPushButton::clicked, this, &UserWindow::clearFields);
    connect(searchBtn, &QPushButton::clicked, this, &UserWindow::searchUser);
    connect(sortByDateBtn, &QPushButton::clicked, this, &UserWindow::sortUsersByDate);
    connect(sortByNameBtn, &QPushButton::clicked, this, &UserWindow::sortUsersByName);

    connect(usersTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = usersTable->currentRow();
        if (row >= 0) {
            idEdit->setText(usersTable->item(row, 0)->text());
            nameEdit->setText(usersTable->item(row, 1)->text());
            emailEdit->setText(usersTable->item(row, 2)->text());
            passwordEdit->clear();

            // Mettre à jour les comboboxes
            QString role = usersTable->item(row, 3)->text();
            int roleIndex = roleBox->findText(role);
            if (roleIndex >= 0) roleBox->setCurrentIndex(roleIndex);

            QString status = usersTable->item(row, 4)->text();
            int statusIndex = statusBox->findText(status);
            if (statusIndex >= 0) statusBox->setCurrentIndex(statusIndex);
        }
    });

    pagesWidget->addWidget(usersPage);
}

// ==================== PAGE MATÉRIEL ====================

void UserWindow::setupMaterialPage()
{
    materialPage = new QWidget;
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

    // Style commun pour tous les champs de saisie
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

    // Inputs pour le matériel
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
    materialTypeBox->addItems({"Électronique", "Mécanique", "Informatique", "Bureau", "Outillage", "Sécurité", "Transport"});
    materialTypeBox->setStyleSheet(comboBoxStyle);

    materialQuantityEdit = new QSpinBox;
    materialQuantityEdit->setMinimum(1);
    materialQuantityEdit->setMaximum(9999);
    materialQuantityEdit->setValue(1);
    materialQuantityEdit->setStyleSheet(spinBoxStyle);

    materialSupplierEdit = new QLineEdit;
    materialSupplierEdit->setPlaceholderText("Nom du fournisseur");
    materialSupplierEdit->setStyleSheet(lineEditStyle);

    materialDateEdit = new QDateEdit;
    materialDateEdit->setDate(QDate::currentDate());
    materialDateEdit->setCalendarPopup(true);
    materialDateEdit->setDisplayFormat("dd/MM/yyyy");
    materialDateEdit->setStyleSheet(dateEditStyle);

    materialStatusBox = new QComboBox;
    materialStatusBox->addItems({"Disponible", "En maintenance", "En prêt", "Hors service", "Réservé"});
    materialStatusBox->setStyleSheet(comboBoxStyle);

    // Formulaire avec espacement et alignement
    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(12);  // Espacement vertical entre les lignes
    inputForm->setContentsMargins(0, 0, 0, 0);

    // Style pour les labels
    QString labelStyle =
        "QLabel {"
        "    color: #2c3e50;"  // Bleu foncé
        "    font-weight: 600;"  // Semi-gras
        "    font-size: 13px;"
        "    padding: 2px 0;"
        "}";

    // Créer les labels
    QLabel *idLabel = new QLabel("ID:");
    idLabel->setStyleSheet(labelStyle);

    QLabel *nameLabel = new QLabel("Nom:");
    nameLabel->setStyleSheet(labelStyle);

    QLabel *typeLabel = new QLabel("Type:");
    typeLabel->setStyleSheet(labelStyle);

    QLabel *quantityLabel = new QLabel("Quantité:");
    quantityLabel->setStyleSheet(labelStyle);

    QLabel *supplierLabel = new QLabel("Fournisseur:");
    supplierLabel->setStyleSheet(labelStyle);

    QLabel *dateLabel = new QLabel("Date:");
    dateLabel->setStyleSheet(labelStyle);

    QLabel *statusLabel = new QLabel("Statut:");
    statusLabel->setStyleSheet(labelStyle);

    // Ajouter les champs au formulaire
    inputForm->addRow(idLabel, materialIdEdit);
    inputForm->addRow(nameLabel, materialNameEdit);
    inputForm->addRow(typeLabel, materialTypeBox);
    inputForm->addRow(quantityLabel, materialQuantityEdit);
    inputForm->addRow(supplierLabel, materialSupplierEdit);
    inputForm->addRow(dateLabel, materialDateEdit);
    inputForm->addRow(statusLabel, materialStatusBox);

    // Configurer la taille des champs
    int fieldHeight = 38;
    materialIdEdit->setFixedHeight(fieldHeight);
    materialNameEdit->setFixedHeight(fieldHeight);
    materialTypeBox->setFixedHeight(fieldHeight);
    materialQuantityEdit->setFixedHeight(fieldHeight);
    materialSupplierEdit->setFixedHeight(fieldHeight);
    materialDateEdit->setFixedHeight(fieldHeight);
    materialStatusBox->setFixedHeight(fieldHeight);

    // Aligner les labels à droite
    inputForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Fixer la largeur des labels pour l'alignement
    int labelWidth = 120;
    idLabel->setFixedWidth(labelWidth);
    nameLabel->setFixedWidth(labelWidth);
    typeLabel->setFixedWidth(labelWidth);
    quantityLabel->setFixedWidth(labelWidth);
    supplierLabel->setFixedWidth(labelWidth);
    dateLabel->setFixedWidth(labelWidth);
    statusLabel->setFixedWidth(labelWidth);

    formLayout->addLayout(inputForm);

    // Espace avant les boutons
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);
    formLayout->addItem(verticalSpacer);

    // Boutons avec espacement
    QHBoxLayout *formButtons = new QHBoxLayout;
    formButtons->setSpacing(10);
    formButtons->setContentsMargins(0, 10, 0, 0);

    QPushButton *materialAddBtn = createStyledButton("➕ Ajouter", "#2ecc71");
    materialModifyBtn = createStyledButton("✏️ Modifier", "#3498db");
    QPushButton *materialClearBtn = createStyledButton("🔄 Effacer", "#95a5a6");

    // Taille fixe pour les boutons
    int buttonHeight = 38;
    materialAddBtn->setFixedHeight(buttonHeight);
    materialModifyBtn->setFixedHeight(buttonHeight);
    materialClearBtn->setFixedHeight(buttonHeight);

    // Largeur minimale pour les boutons
    materialAddBtn->setMinimumWidth(120);
    materialModifyBtn->setMinimumWidth(120);
    materialClearBtn->setMinimumWidth(120);

    formButtons->addWidget(materialAddBtn);
    formButtons->addWidget(materialModifyBtn);
    formButtons->addWidget(materialClearBtn);

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

    QLabel *tableTitle = new QLabel("TABLE DES MATÉRIAUX");
    tableTitle->setStyleSheet("color: #2c3e50; font-size: 16px; font-weight: bold; padding: 20px 20px 10px 20px;");
    tableFrameLayout->addWidget(tableTitle);

    // Tableau
    materialTable = new QTableWidget(0, 6);
    QStringList headers = {"ID", "NOM", "TYPE", "QUANTITÉ", "STATUT", "FOURNISSEUR"};
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

    // === FOOTER - TOTAL MATÉRIELS ===
    QFrame *footerFrame = new QFrame;
    footerFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 10px;"
        "    padding: 15px 25px;"
        "}"
        );

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);

    totalMaterialsLabel = new QLabel("Total matériels : 0");
    totalMaterialsLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: bold;");

    footerLayout->addWidget(totalMaterialsLabel);
    footerLayout->addStretch();

    // Bouton d'export
    QPushButton *exportBtn = createStyledButton("📊 Exporter", "#9b59b6");
    exportBtn->setFixedHeight(35);
    footerLayout->addWidget(exportBtn);

    pageLayout->addWidget(footerFrame);

    // Ajouter quelques matériels d'exemple
    addExampleMaterials();

    // Connexions des signaux
    connect(materialAddBtn, &QPushButton::clicked, this, &UserWindow::addMaterial);
    connect(materialDeleteBtn, &QPushButton::clicked, this, &UserWindow::deleteMaterial);
    connect(materialModifyBtn, &QPushButton::clicked, this, &UserWindow::modifyMaterial);
    connect(materialClearBtn, &QPushButton::clicked, this, &UserWindow::clearMaterialFields);
    connect(materialSearchBtn, &QPushButton::clicked, this, &UserWindow::searchMaterial);
    connect(exportBtn, &QPushButton::clicked, this, &UserWindow::exportMaterials);

    connect(materialTable, &QTableWidget::itemSelectionChanged, [this]() {
        int row = materialTable->currentRow();
        if (row >= 0) {
            materialIdEdit->setText(materialTable->item(row, 0)->text());
            materialNameEdit->setText(materialTable->item(row, 1)->text());

            QString type = materialTable->item(row, 2)->text();
            int typeIndex = materialTypeBox->findText(type);
            if (typeIndex >= 0) materialTypeBox->setCurrentIndex(typeIndex);

            materialQuantityEdit->setValue(materialTable->item(row, 3)->text().toInt());

            QString status = materialTable->item(row, 4)->text();
            int statusIndex = materialStatusBox->findText(status);
            if (statusIndex >= 0) materialStatusBox->setCurrentIndex(statusIndex);

            materialSupplierEdit->setText(materialTable->item(row, 5)->text());
        }
    });

    pagesWidget->addWidget(materialPage);
}

// ==================== FONCTIONS DE TRI POUR UTILISATEURS ====================

void UserWindow::sortUsersByDate()
{
    // Trier la liste par date de création (du plus ancien au plus récent)
    std::sort(usersList.begin(), usersList.end(), [](const User &a, const User &b) {
        return a.creationDate < b.creationDate;
    });

    updateUsersTable();
    QMessageBox::information(this, "Tri effectué", "Liste triée par date de création (du plus ancien au plus récent)");
}

void UserWindow::sortUsersByName()
{
    // Trier la liste par nom, et pour les mêmes noms, trier par ID
    std::sort(usersList.begin(), usersList.end(), [](const User &a, const User &b) {
        if (a.name == b.name) {
            return a.id < b.id;  // Si mêmes noms, trier par ID croissant
        }
        return a.name < b.name;  // Trier par nom alphabétique
    });

    updateUsersTable();
    QMessageBox::information(this, "Tri effectué", "Liste triée par nom (et par ID pour les mêmes noms)");
}

// ==================== PAGE CLIENTS (VIDE - À COMPLÉTER) ====================

void UserWindow::setupClientsPage()
{
    clientsPage = new QWidget;
    clientsPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(clientsPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);

    // Page vide avec un message
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
        "Utilisez la structure de la page Utilisateurs comme exemple."
        );
    messageLabel->setStyleSheet("color: #7f8c8d; font-size: 14px; text-align: center; line-height: 1.5;");
    messageLabel->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(iconLabel);
    emptyLayout->addWidget(titleLabel);
    emptyLayout->addWidget(messageLabel);

    pageLayout->addWidget(emptyFrame, 1);
    pagesWidget->addWidget(clientsPage);
}

// ==================== PAGE COMMANDES (VIDE - À COMPLÉTER) ====================

void UserWindow::setupOrdersPage()
{
    ordersPage = new QWidget;
    ordersPage->setStyleSheet("background-color: #f5f6fa;");

    QVBoxLayout *pageLayout = new QVBoxLayout(ordersPage);
    pageLayout->setContentsMargins(30, 30, 30, 30);

    // Page vide avec un message
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
        "Utilisez la structure de la page Utilisateurs comme exemple."
        );
    messageLabel->setStyleSheet("color: #7f8c8d; font-size: 14px; text-align: center; line-height: 1.5;");
    messageLabel->setAlignment(Qt::AlignCenter);

    emptyLayout->addWidget(iconLabel);
    emptyLayout->addWidget(titleLabel);
    emptyLayout->addWidget(messageLabel);

    pageLayout->addWidget(emptyFrame, 1);
    pagesWidget->addWidget(ordersPage);
}

// ==================== FONCTIONS DE NAVIGATION ====================

void UserWindow::changePage(int index)
{
    pagesWidget->setCurrentIndex(index);
}

// ==================== FONCTION POUR CRÉER DES BOUTONS ====================

QPushButton* UserWindow::createStyledButton(const QString& text, const QString& color) {
    QPushButton *button = new QPushButton(text);
    button->setStyleSheet(QString(
                              "QPushButton {"
                              "    background-color: %1;"
                              "    color: white;"
                              "    border: none;"
                              "    border-radius: 6px;"
                              "    padding: 8px 16px;"
                              "    font-size: 12px;"
                              "    font-weight: 500;"
                              "}"
                              "QPushButton:hover {"
                              "    opacity: 0.9;"
                              "}"
                              ).arg(color));

    button->setCursor(Qt::PointingHandCursor);
    return button;
}

// ==================== FONCTIONS DE LA PAGE UTILISATEURS ====================

void UserWindow::addUser()
{
    if (nameEdit->text().isEmpty() || emailEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir le nom et l'email.");
        return;
    }

    User newUser;
    newUser.id = nextId++;
    newUser.name = nameEdit->text();
    newUser.email = emailEdit->text();
    newUser.role = roleBox->currentText();
    newUser.status = statusBox->currentText();
    newUser.creationDate = QDate::currentDate();  // Date actuelle

    usersList.append(newUser);
    updateUsersTable();
    clearFields();

    QMessageBox::information(this, "Succès", "Utilisateur ajouté avec succès !");
}

void UserWindow::modifyUser()
{
    int currentId = idEdit->text().toInt();
    if (currentId <= 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un utilisateur à modifier.");
        return;
    }

    for (User &user : usersList) {
        if (user.id == currentId) {
            user.name = nameEdit->text();
            user.email = emailEdit->text();
            user.role = roleBox->currentText();
            user.status = statusBox->currentText();
            break;
        }
    }

    updateUsersTable();
    QMessageBox::information(this, "Succès", "Utilisateur modifié avec succès !");
}

void UserWindow::deleteUser()
{
    int row = usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Information", "Sélectionnez un utilisateur à supprimer.");
        return;
    }

    int id = usersTable->item(row, 0)->text().toInt();

    for (int i = 0; i < usersList.size(); ++i) {
        if (usersList[i].id == id) {
            usersList.removeAt(i);
            updateUsersTable();
            clearFields();
            QMessageBox::information(this, "Succès", "Utilisateur supprimé avec succès !");
            return;
        }
    }
}

void UserWindow::searchUser()
{
    QString searchText = searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        updateUsersTable();
        return;
    }

    usersTable->setRowCount(0);

    for (const User &user : usersList) {
        if (user.name.contains(searchText, Qt::CaseInsensitive) ||
            user.email.contains(searchText, Qt::CaseInsensitive)) {

            int row = usersTable->rowCount();
            usersTable->insertRow(row);

            usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user.id)));
            usersTable->setItem(row, 1, new QTableWidgetItem(user.name));
            usersTable->setItem(row, 2, new QTableWidgetItem(user.email));
            usersTable->setItem(row, 3, new QTableWidgetItem(user.role));
            usersTable->setItem(row, 4, new QTableWidgetItem(user.status));
            usersTable->setItem(row, 5, new QTableWidgetItem(user.creationDate.toString("dd/MM/yyyy")));
        }
    }
}

void UserWindow::clearFields()
{
    nameEdit->clear();
    emailEdit->clear();
    passwordEdit->clear();
    roleBox->setCurrentIndex(0);
    statusBox->setCurrentIndex(0);
    usersTable->clearSelection();

    // Réinitialiser l'ID
    idEdit->setText(QString::number(nextId));
}

void UserWindow::updateUsersTable()
{
    usersTable->setRowCount(0);

    for (const User &user : usersList) {
        int row = usersTable->rowCount();
        usersTable->insertRow(row);

        usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user.id)));
        usersTable->setItem(row, 1, new QTableWidgetItem(user.name));
        usersTable->setItem(row, 2, new QTableWidgetItem(user.email));
        usersTable->setItem(row, 3, new QTableWidgetItem(user.role));
        usersTable->setItem(row, 4, new QTableWidgetItem(user.status));
        usersTable->setItem(row, 5, new QTableWidgetItem(user.creationDate.toString("dd/MM/yyyy")));
    }
}

// ==================== FONCTIONS MATÉRIEL (COMPLÈTES) ====================

void UserWindow::addMaterial()
{
    if (materialNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez remplir le nom du matériel.");
        return;
    }

    Material newMaterial;
    newMaterial.id = nextMaterialId++;
    newMaterial.name = materialNameEdit->text();
    newMaterial.type = materialTypeBox->currentText();
    newMaterial.quantity = materialQuantityEdit->value();
    newMaterial.supplier = materialSupplierEdit->text();
    newMaterial.acquisitionDate = materialDateEdit->date();
    newMaterial.status = materialStatusBox->currentText();

    materialsList.append(newMaterial);
    updateMaterialsTable();
    clearMaterialFields();

    QMessageBox::information(this, "Succès", "Matériel ajouté avec succès !");
}

void UserWindow::deleteMaterial()
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
            updateMaterialsTable();
            clearMaterialFields();
            QMessageBox::information(this, "Succès", "Matériel supprimé avec succès !");
            return;
        }
    }
}

void UserWindow::modifyMaterial()
{
    int currentId = materialIdEdit->text().toInt();
    if (currentId <= 0) {
        QMessageBox::warning(this, "Erreur", "Sélectionnez un matériel à modifier.");
        return;
    }

    for (Material &material : materialsList) {
        if (material.id == currentId) {
            material.name = materialNameEdit->text();
            material.type = materialTypeBox->currentText();
            material.quantity = materialQuantityEdit->value();
            material.supplier = materialSupplierEdit->text();
            material.acquisitionDate = materialDateEdit->date();
            material.status = materialStatusBox->currentText();
            break;
        }
    }

    updateMaterialsTable();
    QMessageBox::information(this, "Succès", "Matériel modifié avec succès !");
}

void UserWindow::clearMaterialFields()
{
    materialNameEdit->clear();
    materialSupplierEdit->clear();
    materialTypeBox->setCurrentIndex(0);
    materialStatusBox->setCurrentIndex(0);
    materialQuantityEdit->setValue(1);
    materialDateEdit->setDate(QDate::currentDate());
    materialTable->clearSelection();

    // Réinitialiser l'ID
    materialIdEdit->setText(QString::number(nextMaterialId));
}

void UserWindow::searchMaterial()
{
    QString searchText = materialSearchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        updateMaterialsTable();
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
            materialTable->setItem(row, 4, new QTableWidgetItem(material.status));
            materialTable->setItem(row, 5, new QTableWidgetItem(material.supplier));
        }
    }
}

void UserWindow::exportMaterials()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les matériels", "", "Fichiers CSV (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier pour l'export.");
        return;
    }

    QTextStream out(&file);
    out << "ID;Nom;Type;Quantité;Statut;Fournisseur;Date d'acquisition\n";

    for (const Material &material : materialsList) {
        out << material.id << ";"
            << material.name << ";"
            << material.type << ";"
            << material.quantity << ";"
            << material.status << ";"
            << material.supplier << ";"
            << material.acquisitionDate.toString("dd/MM/yyyy") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Succès", "Matériels exportés avec succès !");
}

void UserWindow::addExampleMaterials()
{
    Material mat1 = {1, "Ordinateur portable", "Informatique", 5, "Dell", QDate(2024, 1, 15), "Disponible"};
    Material mat2 = {2, "Tournevis électrique", "Outillage", 10, "Bosch", QDate(2024, 2, 20), "Disponible"};
    Material mat3 = {3, "Imprimante laser", "Bureau", 3, "HP", QDate(2024, 3, 10), "En maintenance"};
    Material mat4 = {4, "Multimètre numérique", "Électronique", 8, "Fluke", QDate(2024, 4, 5), "Disponible"};
    Material mat5 = {5, "Casque de sécurité", "Sécurité", 20, "3M", QDate(2024, 5, 12), "Réservé"};

    materialsList << mat1 << mat2 << mat3 << mat4 << mat5;
    nextMaterialId = 6;
    updateMaterialsTable();
}

void UserWindow::updateMaterialsTable()
{
    materialTable->setRowCount(0);

    for (const Material &material : materialsList) {
        int row = materialTable->rowCount();
        materialTable->insertRow(row);

        materialTable->setItem(row, 0, new QTableWidgetItem(QString::number(material.id)));
        materialTable->setItem(row, 1, new QTableWidgetItem(material.name));
        materialTable->setItem(row, 2, new QTableWidgetItem(material.type));
        materialTable->setItem(row, 3, new QTableWidgetItem(QString::number(material.quantity)));
        materialTable->setItem(row, 4, new QTableWidgetItem(material.status));
        materialTable->setItem(row, 5, new QTableWidgetItem(material.supplier));
    }

    // Mettre à jour le label du total
    totalMaterialsLabel->setText("Total matériels : " + QString::number(materialsList.size()));
}
