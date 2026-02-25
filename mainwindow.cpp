#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>
#include <QFileDialog>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QLocale>
#include <QDialog>
#include <QFrame>
#include <QSqlRecord>
#include <algorithm>

#if QT_VERSION >= 0x050000
#include <QtPrintSupport/QPrinter>
#else
#include <QPrinter>
#endif

// =============================================
// CONSTRUCTEUR
// =============================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    statsClientTotal(nullptr),
    statsEnCours(nullptr),
    statsTerminees(nullptr),
    statsTable(nullptr),
    editingId(-1),
    isEditing(false)
{
    setWindowTitle("Smart Menuiserie - Dashboard");
    resize(1600, 900);
    setStyleSheet("QMainWindow { background-color: #f7fafc; }");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    createSidebar();

    stackedWidget = new QStackedWidget();
    createListeClientsPage();
    createStatistiquesPage();

    stackedWidget->addWidget(pageListeClients);
    stackedWidget->addWidget(pageStatistiques);
    stackedWidget->setCurrentIndex(0);

    mainLayout->addWidget(stackedWidget);

    // Charger les données depuis Oracle au démarrage
    refreshClientTable();
}

MainWindow::~MainWindow() {}

// =============================================
// SIDEBAR
// =============================================
void MainWindow::createSidebar()
{
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1e3a8a, stop:1 #1e40af);
        }
    )");

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(5);
    sidebarLayout->setContentsMargins(15, 30, 15, 30);

    QLabel *logoLabel = new QLabel("Smart Menuiserie");
    logoLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white; padding: 20px 10px; background: transparent;");
    logoLabel->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(logoLabel);
    sidebarLayout->addSpacing(30);

    QString activeStyle = R"(
        QPushButton {
            background-color: rgba(255,255,255,0.15); color: white;
            border: none; border-radius: 10px; padding: 15px 20px;
            font-size: 15px; text-align: left; font-weight: 500;
        }
        QPushButton:hover { background-color: rgba(255,255,255,0.25); }
    )";

    QString disabledStyle = R"(
        QPushButton {
            background-color: rgba(255,255,255,0.03);
            color: rgba(255,255,255,0.3); border: none;
            border-radius: 10px; padding: 15px 20px;
            font-size: 15px; text-align: left;
        }
    )";

    QString submenuStyle = R"(
        QPushButton {
            background-color: rgba(255,255,255,0.08);
            color: rgba(255,255,255,0.9); border: none;
            border-radius: 8px; padding: 12px 20px 12px 45px;
            font-size: 14px; text-align: left;
        }
        QPushButton:hover { background-color: rgba(255,255,255,0.18); }
    )";

    QPushButton *btnUtilisateurs = new QPushButton("  Utilisateurs");
    btnUtilisateurs->setStyleSheet(disabledStyle);
    btnUtilisateurs->setMinimumHeight(50);
    btnUtilisateurs->setEnabled(false);
    sidebarLayout->addWidget(btnUtilisateurs);

    QPushButton *btnMateriel = new QPushButton("  Materiel");
    btnMateriel->setStyleSheet(disabledStyle);
    btnMateriel->setMinimumHeight(50);
    btnMateriel->setEnabled(false);
    sidebarLayout->addWidget(btnMateriel);

    btnListeClients = new QPushButton("  Clients");
    btnListeClients->setStyleSheet(activeStyle);
    btnListeClients->setCursor(Qt::PointingHandCursor);
    btnListeClients->setMinimumHeight(50);
    sidebarLayout->addWidget(btnListeClients);

    btnStatistiques = new QPushButton("     Statistiques");
    btnStatistiques->setStyleSheet(submenuStyle);
    btnStatistiques->setCursor(Qt::PointingHandCursor);
    btnStatistiques->setMinimumHeight(42);
    btnStatistiques->setVisible(false);
    sidebarLayout->addWidget(btnStatistiques);

    QPushButton *btnCommandes = new QPushButton("  Commandes");
    btnCommandes->setStyleSheet(disabledStyle);
    btnCommandes->setMinimumHeight(50);
    btnCommandes->setEnabled(false);
    sidebarLayout->addWidget(btnCommandes);

    sidebarLayout->addStretch();

    btnDeconnexion = new QPushButton("  Deconnexion");
    btnDeconnexion->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(239,68,68,0.8); color: white;
            border: none; border-radius: 10px; padding: 15px 20px;
            font-size: 15px; text-align: left;
        }
        QPushButton:hover { background-color: rgba(220,38,38,0.9); }
    )");
    btnDeconnexion->setCursor(Qt::PointingHandCursor);
    btnDeconnexion->setMinimumHeight(50);
    sidebarLayout->addWidget(btnDeconnexion);

    connect(btnListeClients, &QPushButton::clicked, this, [this]() {
        showListeClients();
        btnStatistiques->setVisible(true);
    });
    connect(btnStatistiques, &QPushButton::clicked, this, &MainWindow::showStatistiques);
    connect(btnDeconnexion,  &QPushButton::clicked, this, &MainWindow::onDeconnexion);

    centralWidget()->layout()->addWidget(sidebar);
}

// =============================================
// PAGE LISTE CLIENTS
// =============================================
void MainWindow::createListeClientsPage()
{
    pageListeClients = new QWidget();
    pageListeClients->setStyleSheet("background-color: #f7fafc;");

    QHBoxLayout *mainLayout = new QHBoxLayout(pageListeClients);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // ---- PANNEAU GAUCHE (formulaire) ----
    QWidget *leftPanel = new QWidget();
    leftPanel->setFixedWidth(450);
    leftPanel->setStyleSheet("QWidget { background-color: white; border-radius: 15px; }");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(30, 30, 30, 30);
    leftLayout->setSpacing(8);

    QLabel *formTitle = new QLabel("Ajouter / Modifier Client");
    formTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e40af; background: transparent;");
    leftLayout->addWidget(formTitle);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e2e8f0;");
    line->setFixedHeight(2);
    leftLayout->addWidget(line);
    leftLayout->addSpacing(10);

    QString inputStyle = R"(
        QLineEdit, QDateEdit {
            padding: 12px 15px; border: 2px solid #e2e8f0;
            border-radius: 8px; font-size: 14px;
            background-color: #f9fafb; height: 42px;
        }
        QLineEdit:focus, QDateEdit:focus {
            border: 2px solid #1e40af; background-color: white;
        }
    )";
    QString labelStyle = "font-size: 13px; font-weight: 600; color: #374151; background: transparent;";

    // ID
    QLabel *lblId = new QLabel("ID Client *");
    lblId->setStyleSheet(labelStyle); lblId->setFixedHeight(20);
    leftLayout->addWidget(lblId);
    idEdit = new QLineEdit();
    idEdit->setStyleSheet(inputStyle); idEdit->setPlaceholderText("Ex: 123");
    idEdit->setFixedHeight(42); leftLayout->addWidget(idEdit);

    // Nom
    QLabel *lblNom = new QLabel("Nom *");
    lblNom->setStyleSheet(labelStyle); lblNom->setFixedHeight(20);
    leftLayout->addWidget(lblNom);
    nomEdit = new QLineEdit();
    nomEdit->setStyleSheet(inputStyle); nomEdit->setPlaceholderText("Ex: Ben Ali");
    nomEdit->setFixedHeight(42); leftLayout->addWidget(nomEdit);

    // Prenom
    QLabel *lblPrenom = new QLabel("Prenom *");
    lblPrenom->setStyleSheet(labelStyle); lblPrenom->setFixedHeight(20);
    leftLayout->addWidget(lblPrenom);
    prenomEdit = new QLineEdit();
    prenomEdit->setStyleSheet(inputStyle); prenomEdit->setPlaceholderText("Ex: Mohamed");
    prenomEdit->setFixedHeight(42); leftLayout->addWidget(prenomEdit);

    // Email
    QLabel *lblEmail = new QLabel("Email *");
    lblEmail->setStyleSheet(labelStyle); lblEmail->setFixedHeight(20);
    leftLayout->addWidget(lblEmail);
    emailEdit = new QLineEdit();
    emailEdit->setStyleSheet(inputStyle); emailEdit->setPlaceholderText("email@exemple.com");
    emailEdit->setFixedHeight(42); leftLayout->addWidget(emailEdit);

    // Telephone
    QLabel *lblTel = new QLabel("Telephone *");
    lblTel->setStyleSheet(labelStyle); lblTel->setFixedHeight(20);
    leftLayout->addWidget(lblTel);
    telEdit = new QLineEdit();
    telEdit->setStyleSheet(inputStyle); telEdit->setPlaceholderText("99123456");
    telEdit->setFixedHeight(42); leftLayout->addWidget(telEdit);

    // Adresse
    QLabel *lblAdresse = new QLabel("Adresse *");
    lblAdresse->setStyleSheet(labelStyle); lblAdresse->setFixedHeight(20);
    leftLayout->addWidget(lblAdresse);
    adresseEdit = new QLineEdit();
    adresseEdit->setStyleSheet(inputStyle); adresseEdit->setPlaceholderText("Tunis, Ariana");
    adresseEdit->setFixedHeight(42); leftLayout->addWidget(adresseEdit);

    // Date
    QLabel *lblDate = new QLabel("Date inscription *");
    lblDate->setStyleSheet(labelStyle); lblDate->setFixedHeight(20);
    leftLayout->addWidget(lblDate);
    dateEdit = new QDateEdit();
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    dateEdit->setStyleSheet(inputStyle);
    dateEdit->setFixedHeight(42);
    leftLayout->addWidget(dateEdit);

    // Statut
    QLabel *lblStatut = new QLabel("Statut *");
    lblStatut->setStyleSheet(labelStyle); lblStatut->setFixedHeight(20);
    leftLayout->addWidget(lblStatut);

    QWidget *statutWidget = new QWidget();
    QHBoxLayout *statutLayout = new QHBoxLayout(statutWidget);
    statutLayout->setContentsMargins(0, 0, 0, 0);
    statutLayout->setSpacing(15);

    statutGroup    = new QButtonGroup(this);
    radioEnCours   = new QRadioButton("En Cours");
    radioTermine   = new QRadioButton("Termine");
    radioEnAttente = new QRadioButton("Attente");
    radioEnCours->setChecked(true);

    QString radioBase = "QRadioButton { font-size: 13px; color: #374151; background: transparent; padding: 5px; } QRadioButton::indicator { width: 18px; height: 18px; } QRadioButton::indicator:unchecked { background-color: white; border: 2px solid #d1d5db; border-radius: 9px; }";
    radioEnCours->setStyleSheet(radioBase   + "QRadioButton::indicator:checked { background-color: #f59e0b; border: 2px solid #f59e0b; border-radius: 9px; }");
    radioTermine->setStyleSheet(radioBase   + "QRadioButton::indicator:checked { background-color: #10b981; border: 2px solid #10b981; border-radius: 9px; }");
    radioEnAttente->setStyleSheet(radioBase + "QRadioButton::indicator:checked { background-color: #3b82f6; border: 2px solid #3b82f6; border-radius: 9px; }");

    statutGroup->addButton(radioEnCours,   0);
    statutGroup->addButton(radioTermine,   1);
    statutGroup->addButton(radioEnAttente, 2);

    statutLayout->addWidget(radioEnCours);
    statutLayout->addWidget(radioTermine);
    statutLayout->addWidget(radioEnAttente);
    statutLayout->addStretch();
    statutWidget->setFixedHeight(35);
    leftLayout->addWidget(statutWidget);
    leftLayout->addSpacing(15);

    // Boutons Enregistrer / Effacer
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *btnSubmit = new QPushButton("Enregistrer");
    btnSubmit->setFixedHeight(45);
    btnSubmit->setCursor(Qt::PointingHandCursor);
    btnSubmit->setStyleSheet(R"(
        QPushButton { background-color: #10b981; color: white; border: none; border-radius: 8px; font-size: 15px; font-weight: bold; }
        QPushButton:hover { background-color: #059669; }
    )");
    connect(btnSubmit, &QPushButton::clicked, this, &MainWindow::onAjouterClientSubmit);

    QPushButton *btnClear = new QPushButton("Effacer");
    btnClear->setFixedHeight(45);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet(R"(
        QPushButton { background-color: #6b7280; color: white; border: none; border-radius: 8px; font-size: 15px; font-weight: bold; }
        QPushButton:hover { background-color: #4b5563; }
    )");
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onAnnulerAjout);

    btnLayout->addWidget(btnSubmit);
    btnLayout->addWidget(btnClear);
    leftLayout->addLayout(btnLayout);
    leftLayout->addStretch();

    // ---- PANNEAU DROIT (tableau) ----
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    QLabel *listTitle = new QLabel("Liste des Clients");
    listTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #1a202c; background: transparent;");
    rightLayout->addWidget(listTitle);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Rechercher par ID...");
    searchEdit->setFixedWidth(180);
    searchEdit->setStyleSheet(R"(
        QLineEdit { padding: 8px 12px; border: 2px solid #e2e8f0; border-radius: 8px; font-size: 13px; }
        QLineEdit:focus { border: 2px solid #1e40af; }
    )");
    toolbarLayout->addWidget(searchEdit);

    QPushButton *btnSearch = new QPushButton("Rechercher");
    btnSearch->setFixedHeight(35);
    btnSearch->setCursor(Qt::PointingHandCursor);
    btnSearch->setStyleSheet(R"(
        QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 8px; padding: 0 15px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background-color: #2563eb; }
    )");
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchClient);
    toolbarLayout->addWidget(btnSearch);

    toolbarLayout->addSpacing(15);

    sortComboBox = new QComboBox();
    sortComboBox->addItem("Trier: ID");
    sortComboBox->addItem("Trier: Date");
    sortComboBox->addItem("Trier: Statut");
    sortComboBox->setFixedWidth(150);
    sortComboBox->setCursor(Qt::PointingHandCursor);
    sortComboBox->setStyleSheet("QComboBox { padding: 8px 12px; border: 2px solid #e2e8f0; border-radius: 8px; font-size: 13px; }");
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortClients);
    toolbarLayout->addWidget(sortComboBox);

    toolbarLayout->addStretch();

    QPushButton *btnExportPDF = new QPushButton("PDF");
    btnExportPDF->setFixedHeight(35);
    btnExportPDF->setCursor(Qt::PointingHandCursor);
    btnExportPDF->setStyleSheet(R"(
        QPushButton { background-color: #dc2626; color: white; border: none; border-radius: 8px; padding: 0 15px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background-color: #b91c1c; }
    )");
    connect(btnExportPDF, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    toolbarLayout->addWidget(btnExportPDF);

    QPushButton *btnExportExcel = new QPushButton("Excel");
    btnExportExcel->setFixedHeight(35);
    btnExportExcel->setCursor(Qt::PointingHandCursor);
    btnExportExcel->setStyleSheet(R"(
        QPushButton { background-color: #059669; color: white; border: none; border-radius: 8px; padding: 0 15px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background-color: #047857; }
    )");
    connect(btnExportExcel, &QPushButton::clicked, this, &MainWindow::onExportExcel);
    toolbarLayout->addWidget(btnExportExcel);

    rightLayout->addLayout(toolbarLayout);

    tableClients = new QTableWidget();
    tableClients->setColumnCount(9);
    tableClients->setHorizontalHeaderLabels({
        "ID", "Nom", "Prenom", "Email", "Telephone", "Adresse", "Date", "Statut", "Actions"
    });
    tableClients->setStyleSheet(R"(
        QTableWidget { background-color: white; border: none; border-radius: 15px; gridline-color: #e2e8f0; }
        QHeaderView::section { background-color: #1e40af; color: white; padding: 10px; border: none; font-weight: bold; font-size: 13px; }
        QTableWidget::item { padding: 8px; border-bottom: 1px solid #e2e8f0; }
        QTableWidget::item:selected { background-color: #dbeafe; color: #1e40af; }
    )");
    tableClients->horizontalHeader()->setStretchLastSection(false);
    tableClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableClients->setAlternatingRowColors(true);
    tableClients->setColumnWidth(0, 70);
    tableClients->setColumnWidth(1, 120);
    tableClients->setColumnWidth(2, 120);
    tableClients->setColumnWidth(3, 200);
    tableClients->setColumnWidth(4, 130);
    tableClients->setColumnWidth(5, 180);
    tableClients->setColumnWidth(6, 120);
    tableClients->setColumnWidth(7, 110);
    tableClients->setColumnWidth(8, 220);

    rightLayout->addWidget(tableClients);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
}

// =============================================
// CHARGER DONNÉES DEPUIS ORACLE
// =============================================
void MainWindow::refreshClientTable()
{
    QSqlQueryModel *model = Ctmp.afficher();

    tableClients->setRowCount(0);

    for (int i = 0; i < model->rowCount(); i++) {
        tableClients->insertRow(i);
        for (int col = 0; col < 8; col++) {
            tableClients->setItem(i, col,
                                  new QTableWidgetItem(
                                      model->record(i).value(col).toString()
                                      ));
        }
        tableClients->setCellWidget(i, 8, createActionButtons(i));
    }

    delete model;
}

// =============================================
// BOUTONS MODIFIER / SUPPRIMER PAR LIGNE
// =============================================
QWidget* MainWindow::createActionButtons(int row)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(8);

    QPushButton *btnEdit = new QPushButton("Modifier");
    btnEdit->setMinimumWidth(100);
    btnEdit->setMinimumHeight(35);
    btnEdit->setStyleSheet(R"(
        QPushButton { background-color: #f59e0b; color: white; border-radius: 6px; padding: 8px 15px; font-weight: bold; font-size: 13px; }
        QPushButton:hover { background-color: #d97706; }
    )");
    btnEdit->setCursor(Qt::PointingHandCursor);

    QPushButton *btnDelete = new QPushButton("Supprimer");
    btnDelete->setMinimumWidth(110);
    btnDelete->setMinimumHeight(35);
    btnDelete->setStyleSheet(R"(
        QPushButton { background-color: #ef4444; color: white; border-radius: 6px; padding: 8px 15px; font-weight: bold; font-size: 13px; }
        QPushButton:hover { background-color: #dc2626; }
    )");
    btnDelete->setCursor(Qt::PointingHandCursor);

    connect(btnEdit,   &QPushButton::clicked, this, [this, row]() { onEditClient(row); });
    connect(btnDelete, &QPushButton::clicked, this, [this, row]() { onDeleteClient(row); });

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    return widget;
}

// =============================================
// AJOUTER ou MODIFIER (même bouton Enregistrer)
// =============================================
void MainWindow::onAjouterClientSubmit()
{
    // --- Validations ---
    if (idEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "L'ID est obligatoire !");
        idEdit->setFocus(); return;
    }
    if (nomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "Le Nom est obligatoire !");
        nomEdit->setFocus(); return;
    }
    if (prenomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "Le Prenom est obligatoire !");
        prenomEdit->setFocus(); return;
    }
    if (emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "L'Email est obligatoire !");
        emailEdit->setFocus(); return;
    }
    if (telEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "Le Telephone est obligatoire !");
        telEdit->setFocus(); return;
    }
    if (adresseEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ manquant", "L'Adresse est obligatoire !");
        adresseEdit->setFocus(); return;
    }

    // --- Récupérer le statut ---
    QString statut = radioEnCours->isChecked()  ? "En Cours"  :
                         radioTermine->isChecked()   ? "Termine"   : "En Attente";

    // --- Récupérer les champs ---
    int     id      = idEdit->text().trimmed().toInt();
    QString nom     = nomEdit->text().trimmed();
    QString prenom  = prenomEdit->text().trimmed();
    QString email   = emailEdit->text().trimmed();
    QString tel     = telEdit->text().trimmed();
    QString adresse = adresseEdit->text().trimmed();
    QString date    = dateEdit->date().toString("dd/MM/yyyy");

    bool ok = false;

    if (isEditing) {
        // ===== MODE MODIFICATION =====
        // Appel de la méthode modifier() du MODELE (comme dans le cours de la prof)
        ok = Ctmp.modifier(editingId, nom, prenom, email, tel, adresse, date, statut);
        if (ok) {
            QMessageBox::information(this, "Succes", "Client modifie avec succes !");
        } else {
            QMessageBox::critical(this, "Erreur", "Modification echouee !");
        }
        isEditing = false;
        editingId = -1;
    } else {
        // ===== MODE AJOUT =====
        // Instancier un objet Client avec les données du formulaire
        // puis appeler ajouter() du MODELE (comme dans le cours de la prof)
        Client C(id, nom, prenom, email, tel, adresse, date, statut);
        ok = C.ajouter();
        if (ok) {
            QMessageBox::information(this, "Succes", "Client ajoute avec succes !");
        } else {
            QMessageBox::critical(this, "Erreur", "Ajout echoue ! (ID deja existant ?)");
        }
    }

    if (ok) {
        onAnnulerAjout();
        refreshClientTable(); // Recharger depuis Oracle
    }
}

// =============================================
// EFFACER LE FORMULAIRE
// =============================================
void MainWindow::onAnnulerAjout()
{
    idEdit->clear();
    nomEdit->clear();
    prenomEdit->clear();
    emailEdit->clear();
    telEdit->clear();
    adresseEdit->clear();
    dateEdit->setDate(QDate::currentDate());
    radioEnCours->setChecked(true);
    isEditing = false;
    editingId = -1;
    idEdit->setReadOnly(false);
}

// =============================================
// CHARGER UN CLIENT DANS LE FORMULAIRE POUR MODIFIER
// =============================================
void MainWindow::onEditClient(int row)
{
    // Récupérer l'ID de la ligne cliquée
    editingId = tableClients->item(row, 0)->text().toInt();
    isEditing = true;

    // Remplir le formulaire avec les données existantes
    idEdit->setText(tableClients->item(row, 0)->text());
    nomEdit->setText(tableClients->item(row, 1)->text());
    prenomEdit->setText(tableClients->item(row, 2)->text());
    emailEdit->setText(tableClients->item(row, 3)->text());
    telEdit->setText(tableClients->item(row, 4)->text());
    adresseEdit->setText(tableClients->item(row, 5)->text());
    dateEdit->setDate(QDate::fromString(tableClients->item(row, 6)->text(), "dd/MM/yyyy"));

    QString statut = tableClients->item(row, 7)->text();
    if (statut.contains("Cours"))    radioEnCours->setChecked(true);
    else if (statut.contains("min")) radioTermine->setChecked(true);
    else                             radioEnAttente->setChecked(true);

    // Bloquer la modification de l'ID
    idEdit->setReadOnly(true);
    idEdit->setFocus();
}

// =============================================
// SUPPRIMER UN CLIENT
// =============================================
void MainWindow::onDeleteClient(int row)
{
    int id      = tableClients->item(row, 0)->text().toInt();
    QString nom = tableClients->item(row, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmation",
        QString("Supprimer le client %1 (ID: %2) ?").arg(nom).arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Appel de la méthode supprimer() du MODELE
        bool ok = Ctmp.supprimer(id);
        if (ok) {
            QMessageBox::information(this, "Succes", "Client supprime !");
            refreshClientTable();
        } else {
            QMessageBox::critical(this, "Erreur", "Suppression echouee !");
        }
    }
}

// =============================================
// RECHERCHER (dans le tableau déjà chargé)
// =============================================
void MainWindow::onSearchClient()
{
    QString searchText = searchEdit->text().trimmed();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un ID.");
        return;
    }

    // Recherche dans le QTableWidget (déjà chargé depuis Oracle)
    bool found = false;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 0) &&
            tableClients->item(i, 0)->text() == searchText) {
            tableClients->selectRow(i);
            tableClients->scrollToItem(tableClients->item(i, 0));
            found = true;
            break;
        }
    }

    if (!found) {
        QMessageBox::information(this, "Recherche",
                                 "Aucun client trouve avec l'ID: " + searchText);
    }
}

// =============================================
// TRI
// =============================================
void MainWindow::onSortClients()
{
    QString sortBy = sortComboBox->currentText();
    int column = 0;
    if      (sortBy.contains("ID"))     column = 0;
    else if (sortBy.contains("Date"))   column = 6;
    else if (sortBy.contains("Statut")) column = 7;

    tableClients->sortItems(column, Qt::AscendingOrder);

    for (int row = 0; row < tableClients->rowCount(); ++row) {
        tableClients->setCellWidget(row, 8, createActionButtons(row));
    }
}

// =============================================
// EXPORT PDF
// =============================================
void MainWindow::onExportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en PDF", "", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);

    QString html = "<h1 style='color:#1e40af'>Liste des Clients - Smart Menuiserie</h1>";
    html += "<table border='1' cellspacing='0' cellpadding='5' width='100%'>";
    html += "<tr style='background-color:#1e40af; color:white;'>"
            "<th>ID</th><th>Nom</th><th>Prenom</th><th>Email</th>"
            "<th>Telephone</th><th>Adresse</th><th>Date</th><th>Statut</th></tr>";

    for (int row = 0; row < tableClients->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < 8; ++col)
            html += "<td>" + tableClients->item(row, col)->text() + "</td>";
        html += "</tr>";
    }
    html += "</table>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF", "Export PDF reussi !");
}

// =============================================
// EXPORT EXCEL (CSV)
// =============================================
void MainWindow::onExportExcel()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter en Excel", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier !"); return;
    }

    QTextStream out(&file);
    out << "ID,Nom,Prenom,Email,Telephone,Adresse,Date,Statut\n";
    for (int row = 0; row < tableClients->rowCount(); ++row) {
        for (int col = 0; col < 8; ++col) {
            out << tableClients->item(row, col)->text();
            if (col < 7) out << ",";
        }
        out << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Excel", "Export CSV reussi !");
}

// =============================================
// NAVIGATION
// =============================================
void MainWindow::showListeClients()  { stackedWidget->setCurrentIndex(0); }

void MainWindow::showStatistiques()
{
    updateStatistiques();
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onDeconnexion()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Deconnexion", "Voulez-vous vraiment vous deconnecter ?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) qApp->quit();
}

// =============================================
// PAGE STATISTIQUES
// =============================================
void MainWindow::createStatistiquesPage()
{
    pageStatistiques = new QWidget();
    pageStatistiques->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *layout = new QVBoxLayout(pageStatistiques);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(30);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Statistiques - Tableau de Bord");
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #1a202c; background: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    QPushButton *btnGraphique = new QPushButton("Clients par Date");
    btnGraphique->setFixedSize(200, 45);
    btnGraphique->setCursor(Qt::PointingHandCursor);
    btnGraphique->setStyleSheet(R"(
        QPushButton { background-color: #8b5cf6; color: white; border: none; border-radius: 8px; font-size: 15px; font-weight: bold; }
        QPushButton:hover { background-color: #7c3aed; }
    )");
    connect(btnGraphique, &QPushButton::clicked, this, &MainWindow::onShowGraphique);
    titleLayout->addWidget(btnGraphique);
    layout->addLayout(titleLayout);

    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    // Card 1 - Total
    QWidget *card1 = new QWidget();
    card1->setStyleSheet("QWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #667eea,stop:1 #764ba2); border-radius: 15px; }");
    card1->setMinimumHeight(150);
    QVBoxLayout *c1l = new QVBoxLayout(card1);
    QLabel *c1t = new QLabel("Total Clients");
    c1t->setStyleSheet("font-size: 18px; color: white; background: transparent; font-weight: bold;");
    statsClientTotal = new QLabel("0");
    statsClientTotal->setStyleSheet("font-size: 48px; color: white; background: transparent; font-weight: bold;");
    c1l->addWidget(c1t); c1l->addWidget(statsClientTotal); c1l->addStretch();

    // Card 2 - En Cours
    QWidget *card2 = new QWidget();
    card2->setStyleSheet("QWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #f093fb,stop:1 #f5576c); border-radius: 15px; }");
    card2->setMinimumHeight(150);
    QVBoxLayout *c2l = new QVBoxLayout(card2);
    QLabel *c2t = new QLabel("En Cours");
    c2t->setStyleSheet("font-size: 18px; color: white; background: transparent; font-weight: bold;");
    statsEnCours = new QLabel("0");
    statsEnCours->setStyleSheet("font-size: 48px; color: white; background: transparent; font-weight: bold;");
    c2l->addWidget(c2t); c2l->addWidget(statsEnCours); c2l->addStretch();

    // Card 3 - Terminees
    QWidget *card3 = new QWidget();
    card3->setStyleSheet("QWidget { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4facfe,stop:1 #00f2fe); border-radius: 15px; }");
    card3->setMinimumHeight(150);
    QVBoxLayout *c3l = new QVBoxLayout(card3);
    QLabel *c3t = new QLabel("Terminees");
    c3t->setStyleSheet("font-size: 18px; color: white; background: transparent; font-weight: bold;");
    statsTerminees = new QLabel("0");
    statsTerminees->setStyleSheet("font-size: 48px; color: white; background: transparent; font-weight: bold;");
    c3l->addWidget(c3t); c3l->addWidget(statsTerminees); c3l->addStretch();

    cardsLayout->addWidget(card1);
    cardsLayout->addWidget(card2);
    cardsLayout->addWidget(card3);
    layout->addLayout(cardsLayout);

    // Tableau repartition
    QWidget *graphCard = new QWidget();
    graphCard->setStyleSheet("QWidget { background-color: white; border-radius: 15px; }");
    QVBoxLayout *graphLayout = new QVBoxLayout(graphCard);
    graphLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *graphTitle = new QLabel("Repartition des Statuts");
    graphTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #1a202c; background: transparent;");
    graphLayout->addWidget(graphTitle);

    statsTable = new QTableWidget();
    statsTable->setColumnCount(3);
    statsTable->setHorizontalHeaderLabels({"Statut", "Nombre", "Pourcentage"});
    statsTable->setStyleSheet(R"(
        QTableWidget { background-color: transparent; border: none; gridline-color: #e2e8f0; }
        QHeaderView::section { background-color: #1e40af; color: white; padding: 10px; border: none; font-weight: bold; }
        QTableWidget::item { padding: 8px; }
    )");
    statsTable->horizontalHeader()->setStretchLastSection(true);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setMaximumHeight(300);
    graphLayout->addWidget(statsTable);
    layout->addWidget(graphCard);
    layout->addStretch();
}

// =============================================
// METTRE À JOUR LES STATISTIQUES
// =============================================
void MainWindow::updateStatistiques()
{
    if (!statsClientTotal || !statsEnCours || !statsTerminees || !statsTable) return;

    int total = tableClients->rowCount();
    statsClientTotal->setText(QString::number(total));

    int enCours = 0, terminees = 0;
    QMap<QString, int> statutCounts;

    for (int i = 0; i < total; ++i) {
        if (!tableClients->item(i, 7)) continue;
        QString statut = tableClients->item(i, 7)->text();
        statutCounts[statut]++;
        if (statut.toLower().contains("cours"))  enCours++;
        if (statut.toLower().contains("min"))     terminees++;
    }

    statsEnCours->setText(QString::number(enCours));
    statsTerminees->setText(QString::number(terminees));

    statsTable->setRowCount(statutCounts.size());
    int row = 0;
    for (auto it = statutCounts.begin(); it != statutCounts.end(); ++it) {
        statsTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        double pct = total > 0 ? (it.value() * 100.0 / total) : 0;
        statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(pct, 'f', 1) + "%"));
        row++;
    }
}

// =============================================
// GRAPHIQUE PAR MOIS
// =============================================
void MainWindow::onShowGraphique()
{
    QMap<QString, int> clientsParMois;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (!tableClients->item(i, 6)) continue;
        QDate date = QDate::fromString(tableClients->item(i, 6)->text(), "dd/MM/yyyy");
        if (date.isValid()) {
            QLocale locale(QLocale::French);
            QString moisAnnee = locale.monthName(date.month(), QLocale::LongFormat)
                                + " " + QString::number(date.year());
            clientsParMois[moisAnnee]++;
        }
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Clients par Date d'Inscription");
    dlg->resize(1000, 600);
    dlg->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *dlgLayout = new QVBoxLayout(dlg);
    dlgLayout->setContentsMargins(30, 30, 30, 30);
    dlgLayout->setSpacing(20);

    QLabel *title = new QLabel("Nombre de Clients Inscrits par Mois");
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #1a202c; background: transparent;");
    title->setAlignment(Qt::AlignCenter);
    dlgLayout->addWidget(title);

    QWidget *graphWidget = new QWidget();
    graphWidget->setStyleSheet("QWidget { background-color: white; border-radius: 15px; }");
    graphWidget->setMinimumHeight(400);

    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->setContentsMargins(40, 40, 40, 40);
    graphLayout->setSpacing(15);

    int maxVal = 0;
    for (auto v : clientsParMois.values()) maxVal = qMax(maxVal, v);

    for (auto it = clientsParMois.begin(); it != clientsParMois.end(); ++it) {
        QWidget *barContainer = new QWidget();
        QHBoxLayout *barLayout = new QHBoxLayout(barContainer);
        barLayout->setContentsMargins(0, 0, 0, 0);
        barLayout->setSpacing(10);

        QLabel *moisLabel = new QLabel(it.key());
        moisLabel->setFixedWidth(150);
        moisLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151; background: transparent;");
        barLayout->addWidget(moisLabel);

        QWidget *bar = new QWidget();
        int barWidth = maxVal > 0 ? (it.value() * 500 / maxVal) : 0;
        bar->setFixedSize(barWidth, 35);
        QString color = it.value() >= 5 ? "#10b981" : it.value() >= 3 ? "#3b82f6" : "#f59e0b";
        bar->setStyleSheet(QString("QWidget { background: %1; border-radius: 8px; }").arg(color));
        barLayout->addWidget(bar);

        QLabel *countLabel = new QLabel(QString::number(it.value()) + " client(s)");
        countLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #1e40af; background: transparent;");
        barLayout->addWidget(countLabel);
        barLayout->addStretch();

        graphLayout->addWidget(barContainer);
    }
    graphLayout->addStretch();
    dlgLayout->addWidget(graphWidget);

    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setFixedSize(120, 40);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(R"(
        QPushButton { background-color: #6b7280; color: white; border: none; border-radius: 8px; font-size: 14px; font-weight: bold; }
        QPushButton:hover { background-color: #4b5563; }
    )");
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::accept);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    dlgLayout->addLayout(btnLayout);

    dlg->exec();
}
