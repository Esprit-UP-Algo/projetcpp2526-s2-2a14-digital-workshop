#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QFileDialog>
#include <QTextDocument>
#include <QPrinter>
#include <QFile>
#include <QTextStream>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Smart Menuiserie - Dashboard");
    resize(1400, 800);

    // ===== STYLE GLOBAL =====
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f7fafc;
        }
    )");

    // ===== WIDGET CENTRAL =====
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ===== CRÉER LA SIDEBAR =====
    createSidebar();

    // ===== STACKED WIDGET (pour changer les pages) =====
    stackedWidget = new QStackedWidget();

    // Créer les différentes pages
    createListeClientsPage();
    createListeCommandesPage();
    createAjouterClientPage();

    // Ajouter les pages au stackedWidget
    stackedWidget->addWidget(pageListeClients);      // Index 0
    stackedWidget->addWidget(pageListeCommandes);    // Index 1
    stackedWidget->addWidget(pageAjouterClient);     // Index 2

    // Afficher la liste des clients par défaut
    stackedWidget->setCurrentIndex(0);

    mainLayout->addWidget(stackedWidget);
}

// ===== CRÉER LA SIDEBAR =====
void MainWindow::createSidebar()
{
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 #1e3a8a,
                stop:1 #1e40af
            );
        }
    )");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(5);
    sidebarLayout->setContentsMargins(15, 30, 15, 30);

    // ===== LOGO / TITRE =====
    QLabel *logoLabel = new QLabel("🏢 Smart Menuiserie");
    logoLabel->setStyleSheet(R"(
        font-size: 20px;
        font-weight: bold;
        color: white;
        padding: 20px 10px;
        background: transparent;
    )");
    logoLabel->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(logoLabel);

    sidebarLayout->addSpacing(30);

    // ===== STYLE DES BOUTONS =====
    QString buttonStyle = R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.1);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 15px 20px;
            font-size: 15px;
            text-align: left;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.2);
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.3);
        }
    )";

    // ===== BOUTONS DE NAVIGATION =====
    btnListeClients = new QPushButton("👥  Liste des Clients");
    btnListeClients->setStyleSheet(buttonStyle);
    btnListeClients->setCursor(Qt::PointingHandCursor);
    btnListeClients->setMinimumHeight(50);
    sidebarLayout->addWidget(btnListeClients);

    btnListeCommandes = new QPushButton("📋  Liste des Commandes");
    btnListeCommandes->setStyleSheet(buttonStyle);
    btnListeCommandes->setCursor(Qt::PointingHandCursor);
    btnListeCommandes->setMinimumHeight(50);
    sidebarLayout->addWidget(btnListeCommandes);

    btnAjouterClient = new QPushButton("➕  Ajouter un Client");
    btnAjouterClient->setStyleSheet(buttonStyle);
    btnAjouterClient->setCursor(Qt::PointingHandCursor);
    btnAjouterClient->setMinimumHeight(50);
    sidebarLayout->addWidget(btnAjouterClient);

    sidebarLayout->addStretch();

    // ===== BOUTON DÉCONNEXION =====
    btnDeconnexion = new QPushButton("🚪  Déconnexion");
    btnDeconnexion->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(239, 68, 68, 0.8);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 15px 20px;
            font-size: 15px;
            text-align: left;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: rgba(220, 38, 38, 0.9);
        }
    )");
    btnDeconnexion->setCursor(Qt::PointingHandCursor);
    btnDeconnexion->setMinimumHeight(50);
    sidebarLayout->addWidget(btnDeconnexion);

    // ===== CONNEXIONS =====
    connect(btnListeClients, &QPushButton::clicked, this, &MainWindow::showListeClients);
    connect(btnListeCommandes, &QPushButton::clicked, this, &MainWindow::showListeCommandes);
    connect(btnAjouterClient, &QPushButton::clicked, this, &MainWindow::showAjouterClient);
    connect(btnDeconnexion, &QPushButton::clicked, this, &MainWindow::onDeconnexion);

    centralWidget()->layout()->addWidget(sidebar);
}

// ===== PAGE LISTE DES CLIENTS =====
void MainWindow::createListeClientsPage()
{
    pageListeClients = new QWidget();
    pageListeClients->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *layout = new QVBoxLayout(pageListeClients);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);

    // Titre
    QLabel *titleLabel = new QLabel("👥 Liste des Clients");
    titleLabel->setStyleSheet(R"(
        font-size: 32px;
        font-weight: bold;
        color: #1a202c;
        background: transparent;
    )");
    layout->addWidget(titleLabel);

    // ===== BARRE D'OUTILS (Recherche, Tri, Export) =====
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(15);

    // Recherche par ID
    QLabel *searchLabel = new QLabel("🔍 Rechercher par ID:");
    searchLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #2d3748; background: transparent;");
    toolbarLayout->addWidget(searchLabel);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Entrez l'ID du client...");
    searchEdit->setFixedWidth(200);
    searchEdit->setStyleSheet(R"(
        QLineEdit {
            padding: 10px 15px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border: 2px solid #1e40af;
        }
    )");
    toolbarLayout->addWidget(searchEdit);

    QPushButton *btnSearch = new QPushButton("Rechercher");
    btnSearch->setFixedHeight(40);
    btnSearch->setCursor(Qt::PointingHandCursor);
    btnSearch->setStyleSheet(R"(
        QPushButton {
            background-color: #3b82f6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 0 20px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #2563eb; }
    )");
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchClient);
    toolbarLayout->addWidget(btnSearch);

    toolbarLayout->addSpacing(30);

    // Tri
    QLabel *sortLabel = new QLabel("📊 Trier par:");
    sortLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #2d3748; background: transparent;");
    toolbarLayout->addWidget(sortLabel);

    sortComboBox = new QComboBox();
    sortComboBox->addItem("ID");
    sortComboBox->addItem("Date");
    sortComboBox->addItem("Commande");
    sortComboBox->addItem("Statut");
    sortComboBox->setFixedWidth(150);
    sortComboBox->setCursor(Qt::PointingHandCursor);
    sortComboBox->setStyleSheet(R"(
        QComboBox {
            padding: 10px 15px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            background-color: white;
            font-size: 14px;
        }
        QComboBox:hover {
            border: 2px solid #1e40af;
        }
        QComboBox::drop-down {
            border: none;
        }
    )");
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortClients);
    toolbarLayout->addWidget(sortComboBox);

    toolbarLayout->addStretch();

    // Boutons Export
    QPushButton *btnExportPDF = new QPushButton("📄 Export PDF");
    btnExportPDF->setFixedHeight(40);
    btnExportPDF->setCursor(Qt::PointingHandCursor);
    btnExportPDF->setStyleSheet(R"(
        QPushButton {
            background-color: #dc2626;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 0 20px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #b91c1c; }
    )");
    connect(btnExportPDF, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    toolbarLayout->addWidget(btnExportPDF);

    QPushButton *btnExportExcel = new QPushButton("📊 Export Excel");
    btnExportExcel->setFixedHeight(40);
    btnExportExcel->setCursor(Qt::PointingHandCursor);
    btnExportExcel->setStyleSheet(R"(
        QPushButton {
            background-color: #059669;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 0 20px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #047857; }
    )");
    connect(btnExportExcel, &QPushButton::clicked, this, &MainWindow::onExportExcel);
    toolbarLayout->addWidget(btnExportExcel);

    layout->addLayout(toolbarLayout);

    // Tableau
    tableClients = new QTableWidget();
    tableClients->setColumnCount(10);
    tableClients->setHorizontalHeaderLabels({
        "ID", "Nom", "Prénom", "Email", "Téléphone", "Adresse", "Date", "Commande", "Statut", "Actions"
    });

    tableClients->setStyleSheet(R"(
        QTableWidget {
            background-color: white;
            border: none;
            border-radius: 15px;
            gridline-color: #e2e8f0;
        }
        QHeaderView::section {
            background-color: #1e40af;
            color: white;
            padding: 12px;
            border: none;
            font-weight: bold;
            font-size: 14px;
        }
        QTableWidget::item {
            padding: 10px;
            border-bottom: 1px solid #e2e8f0;
        }
        QTableWidget::item:selected {
            background-color: #dbeafe;
            color: #1e40af;
        }
    )");

    tableClients->horizontalHeader()->setStretchLastSection(false);
    tableClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableClients->setAlternatingRowColors(true);

    // ===== AJUSTER LA LARGEUR DES COLONNES =====
    tableClients->setColumnWidth(0, 80);   // ID
    tableClients->setColumnWidth(1, 120);  // Nom
    tableClients->setColumnWidth(2, 120);  // Prénom
    tableClients->setColumnWidth(3, 200);  // Email (plus large)
    tableClients->setColumnWidth(4, 120);  // Téléphone
    tableClients->setColumnWidth(5, 200);  // Adresse (plus large)
    tableClients->setColumnWidth(6, 120);  // Date
    tableClients->setColumnWidth(7, 150);  // Commande
    tableClients->setColumnWidth(8, 100);  // Statut
    tableClients->setColumnWidth(9, 240);  // Actions (plus large pour 2 boutons)

    layout->addWidget(tableClients);
}

// ===== PAGE LISTE DES COMMANDES =====
void MainWindow::createListeCommandesPage()
{
    pageListeCommandes = new QWidget();
    pageListeCommandes->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *layout = new QVBoxLayout(pageListeCommandes);
    layout->setContentsMargins(40, 40, 40, 40);

    QLabel *titleLabel = new QLabel("📋 Liste des Commandes");
    titleLabel->setStyleSheet(R"(
        font-size: 32px;
        font-weight: bold;
        color: #1a202c;
        background: transparent;
    )");
    layout->addWidget(titleLabel);

    QLabel *infoLabel = new QLabel("Cette section affichera la liste des commandes.");
    infoLabel->setStyleSheet(R"(
        font-size: 16px;
        color: #718096;
        background: transparent;
        margin-top: 20px;
    )");
    layout->addWidget(infoLabel);

    layout->addStretch();
}

// ===== PAGE AJOUTER UN CLIENT =====
void MainWindow::createAjouterClientPage()
{
    pageAjouterClient = new QWidget();
    pageAjouterClient->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *layout = new QVBoxLayout(pageAjouterClient);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(25);

    // Titre
    QLabel *titleLabel = new QLabel("➕ Ajouter un Nouveau Client");
    titleLabel->setStyleSheet(R"(
        font-size: 32px;
        font-weight: bold;
        color: #1a202c;
        background: transparent;
    )");
    layout->addWidget(titleLabel);

    // Carte du formulaire
    QWidget *formCard = new QWidget();
    formCard->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-radius: 15px;
        }
    )");

    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(20);

    // Formulaire
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QString inputStyle = R"(
        QLineEdit, QDateEdit {
            padding: 12px 15px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 14px;
            background-color: #f7fafc;
        }
        QLineEdit:focus, QDateEdit:focus {
            border: 2px solid #1e40af;
            background-color: white;
        }
    )";

    idEdit = new QLineEdit();
    idEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("ID Client:"), idEdit);

    nomEdit = new QLineEdit();
    nomEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Nom:"), nomEdit);

    prenomEdit = new QLineEdit();
    prenomEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Prénom:"), prenomEdit);

    emailEdit = new QLineEdit();
    emailEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Email:"), emailEdit);

    telEdit = new QLineEdit();
    telEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Téléphone:"), telEdit);

    adresseEdit = new QLineEdit();
    adresseEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Adresse:"), adresseEdit);

    // ===== CALENDRIER POUR LA DATE =====
    dateEdit = new QDateEdit();
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);  // Affiche un calendrier
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    dateEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Date inscription:"), dateEdit);

    commandeEdit = new QLineEdit();
    commandeEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Commande:"), commandeEdit);

    statutEdit = new QLineEdit();
    statutEdit->setStyleSheet(inputStyle);
    formLayout->addRow(new QLabel("Statut:"), statutEdit);

    cardLayout->addLayout(formLayout);

    // Boutons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnSubmit = new QPushButton("✓ Ajouter le Client");
    btnSubmit->setFixedSize(180, 45);
    btnSubmit->setCursor(Qt::PointingHandCursor);
    btnSubmit->setStyleSheet(R"(
        QPushButton {
            background-color: #10b981;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #059669;
        }
    )");

    QPushButton *btnCancel = new QPushButton("✗ Annuler");
    btnCancel->setFixedSize(120, 45);
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet(R"(
        QPushButton {
            background-color: #6b7280;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4b5563;
        }
    )");

    btnLayout->addWidget(btnSubmit);
    btnLayout->addWidget(btnCancel);

    cardLayout->addSpacing(20);
    cardLayout->addLayout(btnLayout);

    layout->addWidget(formCard);
    layout->addStretch();

    // Connexions
    connect(btnSubmit, &QPushButton::clicked, this, &MainWindow::onAjouterClientSubmit);
    connect(btnCancel, &QPushButton::clicked, this, &MainWindow::onAnnulerAjout);
}

// ===== CRÉER LES BOUTONS D'ACTION =====
QWidget* MainWindow::createActionButtons(int row)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(8);

    QPushButton *btnEdit = new QPushButton("✏️ Modifier");
    btnEdit->setMinimumWidth(100);
    btnEdit->setMinimumHeight(35);
    btnEdit->setStyleSheet(R"(
        QPushButton {
            background-color: #f59e0b;
            color: white;
            border-radius: 6px;
            padding: 8px 15px;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton:hover { background-color: #d97706; }
    )");
    btnEdit->setCursor(Qt::PointingHandCursor);

    QPushButton *btnDelete = new QPushButton("🗑️ Supprimer");
    btnDelete->setMinimumWidth(110);
    btnDelete->setMinimumHeight(35);
    btnDelete->setStyleSheet(R"(
        QPushButton {
            background-color: #ef4444;
            color: white;
            border-radius: 6px;
            padding: 8px 15px;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton:hover { background-color: #dc2626; }
    )");
    btnDelete->setCursor(Qt::PointingHandCursor);

    connect(btnEdit, &QPushButton::clicked, this, [this, row]() { onEditClient(row); });
    connect(btnDelete, &QPushButton::clicked, this, [this, row]() { onDeleteClient(row); });

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);

    return widget;
}

// ===== NAVIGATION =====
void MainWindow::showListeClients()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::showListeCommandes()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::showAjouterClient()
{
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::onDeconnexion()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Déconnexion",
                                  "Voulez-vous vraiment vous déconnecter ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qApp->quit();
    }
}

// ===== AJOUTER UN CLIENT =====
void MainWindow::onAjouterClientSubmit()
{
    // ===== CONTRÔLE DE SAISIE =====
    if (idEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ ID Client est obligatoire !");
        idEdit->setFocus();
        return;
    }

    if (nomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Nom est obligatoire !");
        nomEdit->setFocus();
        return;
    }

    if (prenomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Prénom est obligatoire !");
        prenomEdit->setFocus();
        return;
    }

    if (emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Email est obligatoire !");
        emailEdit->setFocus();
        return;
    }

    if (telEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Téléphone est obligatoire !");
        telEdit->setFocus();
        return;
    }

    if (adresseEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Adresse est obligatoire !");
        adresseEdit->setFocus();
        return;
    }

    if (commandeEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Commande est obligatoire !");
        commandeEdit->setFocus();
        return;
    }

    if (statutEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "⚠️ Le champ Statut est obligatoire !");
        statutEdit->setFocus();
        return;
    }

    // Si tous les champs sont remplis, ajouter le client
    int row = tableClients->rowCount();
    tableClients->insertRow(row);

    tableClients->setItem(row, 0, new QTableWidgetItem(idEdit->text().trimmed()));
    tableClients->setItem(row, 1, new QTableWidgetItem(nomEdit->text().trimmed()));
    tableClients->setItem(row, 2, new QTableWidgetItem(prenomEdit->text().trimmed()));
    tableClients->setItem(row, 3, new QTableWidgetItem(emailEdit->text().trimmed()));
    tableClients->setItem(row, 4, new QTableWidgetItem(telEdit->text().trimmed()));
    tableClients->setItem(row, 5, new QTableWidgetItem(adresseEdit->text().trimmed()));
    tableClients->setItem(row, 6, new QTableWidgetItem(dateEdit->date().toString("dd/MM/yyyy")));
    tableClients->setItem(row, 7, new QTableWidgetItem(commandeEdit->text().trimmed()));
    tableClients->setItem(row, 8, new QTableWidgetItem(statutEdit->text().trimmed()));
    tableClients->setCellWidget(row, 9, createActionButtons(row));

    QMessageBox::information(this, "Succès", "✅ Client ajouté avec succès !");
    onAnnulerAjout();
    showListeClients();
}

void MainWindow::onAnnulerAjout()
{
    idEdit->clear();
    nomEdit->clear();
    prenomEdit->clear();
    emailEdit->clear();
    telEdit->clear();
    adresseEdit->clear();
    dateEdit->setDate(QDate::currentDate());
    commandeEdit->clear();
    statutEdit->clear();
}

void MainWindow::onEditClient(int row)
{
    idEdit->setText(tableClients->item(row, 0)->text());
    nomEdit->setText(tableClients->item(row, 1)->text());
    prenomEdit->setText(tableClients->item(row, 2)->text());
    emailEdit->setText(tableClients->item(row, 3)->text());
    telEdit->setText(tableClients->item(row, 4)->text());
    adresseEdit->setText(tableClients->item(row, 5)->text());
    dateEdit->setDate(QDate::fromString(tableClients->item(row, 6)->text(), "dd/MM/yyyy"));
    commandeEdit->setText(tableClients->item(row, 7)->text());
    statutEdit->setText(tableClients->item(row, 8)->text());

    tableClients->removeRow(row);
    showAjouterClient();
}

void MainWindow::onDeleteClient(int row)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation",
                                  "Voulez-vous vraiment supprimer ce client ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        tableClients->removeRow(row);
    }
}

// ===== RECHERCHE CLIENT PAR ID =====
void MainWindow::onSearchClient()
{
    QString searchId = searchEdit->text().trimmed();

    if (searchId.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un ID pour rechercher.");
        return;
    }

    // Parcourir le tableau pour trouver l'ID
    bool found = false;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 0)->text() == searchId) {
            tableClients->selectRow(i);
            tableClients->scrollToItem(tableClients->item(i, 0));
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::information(this, "Recherche", "Aucun client trouvé avec l'ID: " + searchId);
    }
}

// ===== TRI DES CLIENTS =====
void MainWindow::onSortClients()
{
    int column = 0;  // Par défaut, trier par ID

    QString sortBy = sortComboBox->currentText();
    if (sortBy == "ID") column = 0;
    else if (sortBy == "Date") column = 6;
    else if (sortBy == "Commande") column = 7;
    else if (sortBy == "Statut") column = 8;

    tableClients->sortItems(column, Qt::AscendingOrder);
}

// ===== EXPORT PDF =====
void MainWindow::onExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "PDF Files (*.pdf)");

    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    QString html = "<h1>Liste des Clients - Smart Menuiserie</h1>";
    html += "<table border='1' cellspacing='0' cellpadding='5' width='100%'>";
    html += "<tr style='background-color:#1e40af; color:white;'>";
    html += "<th>ID</th><th>Nom</th><th>Prénom</th><th>Email</th><th>Téléphone</th>";
    html += "<th>Adresse</th><th>Date</th><th>Commande</th><th>Statut</th></tr>";

    for (int row = 0; row < tableClients->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < 9; ++col) {
            html += "<td>" + tableClients->item(row, col)->text() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF", "✅ Export PDF réussi !");
}

// ===== EXPORT EXCEL (CSV) =====
void MainWindow::onExportExcel()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en Excel", "", "CSV Files (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier !");
        return;
    }

    QTextStream out(&file);

    // En-têtes
    out << "ID,Nom,Prénom,Email,Téléphone,Adresse,Date,Commande,Statut\n";

    // Données
    for (int row = 0; row < tableClients->rowCount(); ++row) {
        for (int col = 0; col < 9; ++col) {
            out << tableClients->item(row, col)->text();
            if (col < 8) out << ",";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Excel", "✅ Export Excel (CSV) réussi !");
}

void MainWindow::refreshClientTable()
{
    // Fonction pour rafraîchir le tableau si nécessaire
}

MainWindow::~MainWindow()
{
}
