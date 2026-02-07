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
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QLocale>
#include <QColor>
#include <QFont>
#include <QDialog>
#include <QFrame>
#include <algorithm>

#if QT_VERSION >= 0x050000
#include <QtPrintSupport/QPrinter>
#else
#include <QPrinter>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    statsClientTotal(nullptr),
    statsEnCours(nullptr),
    statsTerminees(nullptr),
    statsTable(nullptr)
{
    setWindowTitle("Smart Menuiserie - Dashboard");
    resize(1600, 900);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #f7fafc;
        }
    )");

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
}

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

    btnListeClients = new QPushButton("👥  Gestion Clients");
    btnListeClients->setStyleSheet(buttonStyle);
    btnListeClients->setCursor(Qt::PointingHandCursor);
    btnListeClients->setMinimumHeight(50);
    sidebarLayout->addWidget(btnListeClients);

    btnStatistiques = new QPushButton("📊  Statistiques");
    btnStatistiques->setStyleSheet(buttonStyle);
    btnStatistiques->setCursor(Qt::PointingHandCursor);
    btnStatistiques->setMinimumHeight(50);
    sidebarLayout->addWidget(btnStatistiques);

    sidebarLayout->addStretch();

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

    connect(btnListeClients, &QPushButton::clicked, this, &MainWindow::showListeClients);
    connect(btnStatistiques, &QPushButton::clicked, this, &MainWindow::showStatistiques);
    connect(btnDeconnexion, &QPushButton::clicked, this, &MainWindow::onDeconnexion);

    centralWidget()->layout()->addWidget(sidebar);
}

void MainWindow::createListeClientsPage()
{
    pageListeClients = new QWidget();
    pageListeClients->setStyleSheet("background-color: #f7fafc;");

    QHBoxLayout *mainLayout = new QHBoxLayout(pageListeClients);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QWidget *leftPanel = new QWidget();
    leftPanel->setFixedWidth(450);
    leftPanel->setStyleSheet("QWidget { background-color: white; border-radius: 15px; }");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(30, 30, 30, 30);
    leftLayout->setSpacing(8);

    QLabel *formTitle = new QLabel("➕ Ajouter Client");
    formTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #1e40af; background: transparent;");
    leftLayout->addWidget(formTitle);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e2e8f0;");
    line->setFixedHeight(2);
    leftLayout->addWidget(line);

    leftLayout->addSpacing(10);

    QString inputStyle = R"(
        QLineEdit, QDateEdit {
            padding: 12px 15px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 14px;
            background-color: #f9fafb;
            height: 42px;
        }
        QLineEdit:focus, QDateEdit:focus {
            border: 2px solid #1e40af;
            background-color: white;
        }
    )";

    QString labelStyle = "font-size: 13px; font-weight: 600; color: #374151; background: transparent; padding: 0px; margin: 0px;";

    // ID Client
    QLabel *lblId = new QLabel("ID Client *");
    lblId->setStyleSheet(labelStyle);
    lblId->setFixedHeight(20);
    leftLayout->addWidget(lblId);

    idEdit = new QLineEdit();
    idEdit->setStyleSheet(inputStyle);
    idEdit->setPlaceholderText("Ex: 123");
    idEdit->setFixedHeight(42);
    leftLayout->addWidget(idEdit);

    // Nom
    QLabel *lblNom = new QLabel("Nom *");
    lblNom->setStyleSheet(labelStyle);
    lblNom->setFixedHeight(20);
    leftLayout->addWidget(lblNom);

    nomEdit = new QLineEdit();
    nomEdit->setStyleSheet(inputStyle);
    nomEdit->setPlaceholderText("Ex: Ben Ali");
    nomEdit->setFixedHeight(42);
    leftLayout->addWidget(nomEdit);

    // Prénom
    QLabel *lblPrenom = new QLabel("Prénom *");
    lblPrenom->setStyleSheet(labelStyle);
    lblPrenom->setFixedHeight(20);
    leftLayout->addWidget(lblPrenom);

    prenomEdit = new QLineEdit();
    prenomEdit->setStyleSheet(inputStyle);
    prenomEdit->setPlaceholderText("Ex: Mohamed");
    prenomEdit->setFixedHeight(42);
    leftLayout->addWidget(prenomEdit);

    // Email
    QLabel *lblEmail = new QLabel("Email *");
    lblEmail->setStyleSheet(labelStyle);
    lblEmail->setFixedHeight(20);
    leftLayout->addWidget(lblEmail);

    emailEdit = new QLineEdit();
    emailEdit->setStyleSheet(inputStyle);
    emailEdit->setPlaceholderText("email@exemple.com");
    emailEdit->setFixedHeight(42);
    leftLayout->addWidget(emailEdit);

    // Téléphone
    QLabel *lblTel = new QLabel("Téléphone *");
    lblTel->setStyleSheet(labelStyle);
    lblTel->setFixedHeight(20);
    leftLayout->addWidget(lblTel);

    telEdit = new QLineEdit();
    telEdit->setStyleSheet(inputStyle);
    telEdit->setPlaceholderText("99123456");
    telEdit->setFixedHeight(42);
    leftLayout->addWidget(telEdit);

    // Adresse
    QLabel *lblAdresse = new QLabel("Adresse *");
    lblAdresse->setStyleSheet(labelStyle);
    lblAdresse->setFixedHeight(20);
    leftLayout->addWidget(lblAdresse);

    adresseEdit = new QLineEdit();
    adresseEdit->setStyleSheet(inputStyle);
    adresseEdit->setPlaceholderText("Tunis, Ariana");
    adresseEdit->setFixedHeight(42);
    leftLayout->addWidget(adresseEdit);

    // Date inscription
    QLabel *lblDate = new QLabel("Date inscription *");
    lblDate->setStyleSheet(labelStyle);
    lblDate->setFixedHeight(20);
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
    lblStatut->setStyleSheet(labelStyle);
    lblStatut->setFixedHeight(20);
    leftLayout->addWidget(lblStatut);

    QWidget *statutWidget = new QWidget();
    QHBoxLayout *statutLayout = new QHBoxLayout(statutWidget);
    statutLayout->setContentsMargins(0, 0, 0, 0);
    statutLayout->setSpacing(15);

    statutGroup = new QButtonGroup(this);

    radioEnCours = new QRadioButton("En Cours");
    radioEnCours->setStyleSheet(R"(
        QRadioButton {
            font-size: 13px;
            color: #374151;
            background: transparent;
            padding: 5px;
        }
        QRadioButton::indicator { width: 18px; height: 18px; }
        QRadioButton::indicator:checked {
            background-color: #f59e0b;
            border: 2px solid #f59e0b;
            border-radius: 9px;
        }
        QRadioButton::indicator:unchecked {
            background-color: white;
            border: 2px solid #d1d5db;
            border-radius: 9px;
        }
    )");
    radioEnCours->setChecked(true);

    radioTermine = new QRadioButton("Terminé");
    radioTermine->setStyleSheet(R"(
        QRadioButton {
            font-size: 13px;
            color: #374151;
            background: transparent;
            padding: 5px;
        }
        QRadioButton::indicator { width: 18px; height: 18px; }
        QRadioButton::indicator:checked {
            background-color: #10b981;
            border: 2px solid #10b981;
            border-radius: 9px;
        }
        QRadioButton::indicator:unchecked {
            background-color: white;
            border: 2px solid #d1d5db;
            border-radius: 9px;
        }
    )");

    radioEnAttente = new QRadioButton("Attente");
    radioEnAttente->setStyleSheet(R"(
        QRadioButton {
            font-size: 13px;
            color: #374151;
            background: transparent;
            padding: 5px;
        }
        QRadioButton::indicator { width: 18px; height: 18px; }
        QRadioButton::indicator:checked {
            background-color: #3b82f6;
            border: 2px solid #3b82f6;
            border-radius: 9px;
        }
        QRadioButton::indicator:unchecked {
            background-color: white;
            border: 2px solid #d1d5db;
            border-radius: 9px;
        }
    )");

    statutGroup->addButton(radioEnCours, 0);
    statutGroup->addButton(radioTermine, 1);
    statutGroup->addButton(radioEnAttente, 2);

    statutLayout->addWidget(radioEnCours);
    statutLayout->addWidget(radioTermine);
    statutLayout->addWidget(radioEnAttente);
    statutLayout->addStretch();

    statutWidget->setFixedHeight(35);
    leftLayout->addWidget(statutWidget);

    leftLayout->addSpacing(15);

    // Boutons
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *btnSubmit = new QPushButton("✓ Ajouter");
    btnSubmit->setFixedHeight(45);
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
        QPushButton:hover { background-color: #059669; }
    )");
    connect(btnSubmit, &QPushButton::clicked, this, &MainWindow::onAjouterClientSubmit);

    QPushButton *btnClear = new QPushButton("✗ Effacer");
    btnClear->setFixedHeight(45);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet(R"(
        QPushButton {
            background-color: #6b7280;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #4b5563; }
    )");
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onAnnulerAjout);

    btnLayout->addWidget(btnSubmit);
    btnLayout->addWidget(btnClear);
    leftLayout->addLayout(btnLayout);

    leftLayout->addStretch();

    // Partie droite avec le tableau
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    QLabel *listTitle = new QLabel("👥 Liste des Clients");
    listTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #1a202c; background: transparent;");
    rightLayout->addWidget(listTitle);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("🔍 Rechercher par ID...");
    searchEdit->setFixedWidth(180);
    searchEdit->setStyleSheet(R"(
        QLineEdit {
            padding: 8px 12px;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            font-size: 13px;
        }
        QLineEdit:focus { border: 2px solid #1e40af; }
    )");
    toolbarLayout->addWidget(searchEdit);

    QPushButton *btnSearch = new QPushButton("Rechercher");
    btnSearch->setFixedHeight(35);
    btnSearch->setCursor(Qt::PointingHandCursor);
    btnSearch->setStyleSheet(R"(
        QPushButton {
            background-color: #3b82f6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 0 15px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #2563eb; }
    )");
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchClient);
    toolbarLayout->addWidget(btnSearch);

    toolbarLayout->addSpacing(15);

    sortComboBox = new QComboBox();
    sortComboBox->addItem("📊 Trier: ID");
    sortComboBox->addItem("📊 Trier: Date");
    sortComboBox->addItem("📊 Trier: Statut");
    sortComboBox->setFixedWidth(150);
    sortComboBox->setCursor(Qt::PointingHandCursor);
    sortComboBox->setStyleSheet("QComboBox { padding: 8px 12px; border: 2px solid #e2e8f0; border-radius: 8px; font-size: 13px; }");
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortClients);
    toolbarLayout->addWidget(sortComboBox);

    toolbarLayout->addStretch();

    QPushButton *btnExportPDF = new QPushButton("📄 PDF");
    btnExportPDF->setFixedHeight(35);
    btnExportPDF->setCursor(Qt::PointingHandCursor);
    btnExportPDF->setStyleSheet(R"(
        QPushButton {
            background-color: #dc2626;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 0 15px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #b91c1c; }
    )");
    connect(btnExportPDF, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    toolbarLayout->addWidget(btnExportPDF);

    QPushButton *btnExportExcel = new QPushButton("📊 Excel");
    btnExportExcel->setFixedHeight(35);
    btnExportExcel->setCursor(Qt::PointingHandCursor);
    btnExportExcel->setStyleSheet(R"(
        QPushButton {
            background-color: #059669;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 0 15px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #047857; }
    )");
    connect(btnExportExcel, &QPushButton::clicked, this, &MainWindow::onExportExcel);
    toolbarLayout->addWidget(btnExportExcel);

    rightLayout->addLayout(toolbarLayout);

    tableClients = new QTableWidget();
    tableClients->setColumnCount(9);
    tableClients->setHorizontalHeaderLabels({
        "ID", "Nom", "Prénom", "Email", "Téléphone", "Adresse", "Date", "Statut", "Actions"
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
            padding: 10px;
            border: none;
            font-weight: bold;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 8px;
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



void MainWindow::createStatistiquesPage()
{
    pageStatistiques = new QWidget();
    pageStatistiques->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *layout = new QVBoxLayout(pageStatistiques);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(30);

    QHBoxLayout *titleLayout = new QHBoxLayout();

    QLabel *titleLabel = new QLabel("📊 Statistiques - Tableau de Bord");
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #1a202c; background: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    QPushButton *btnGraphique = new QPushButton("📊 Clients par Date");
    btnGraphique->setFixedSize(200, 45);
    btnGraphique->setCursor(Qt::PointingHandCursor);
    btnGraphique->setStyleSheet(R"(
        QPushButton {
            background-color: #8b5cf6;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #7c3aed; }
    )");
    connect(btnGraphique, &QPushButton::clicked, this, &MainWindow::onShowGraphique);
    titleLayout->addWidget(btnGraphique);

    layout->addLayout(titleLayout);

    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    QWidget *card1 = new QWidget();
    card1->setStyleSheet("QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2); border-radius: 15px; }");
    card1->setMinimumHeight(150);

    QVBoxLayout *card1Layout = new QVBoxLayout(card1);
    QLabel *card1Title = new QLabel("👥 Total Clients");
    card1Title->setStyleSheet("font-size: 18px; color: white; background: transparent; font-weight: bold;");
    statsClientTotal = new QLabel(QString::number(tableClients->rowCount()));
    statsClientTotal->setStyleSheet("font-size: 48px; color: white; background: transparent; font-weight: bold;");
    card1Layout->addWidget(card1Title);
    card1Layout->addWidget(statsClientTotal);
    card1Layout->addStretch();

    QWidget *card2 = new QWidget();
    card2->setStyleSheet("QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f093fb, stop:1 #f5576c); border-radius: 15px; }");
    card2->setMinimumHeight(150);

    QVBoxLayout *card2Layout = new QVBoxLayout(card2);
    QLabel *card2Title = new QLabel("📦 En Cours");
    card2Title->setStyleSheet("font-size: 18px; color: white; background: transparent; font-weight: bold;");

    int enCours = 0;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 7) && tableClients->item(i, 7)->text().toLower().contains("cours")) {
            enCours++;
        }
    }

    statsEnCours = new QLabel(QString::number(enCours));
    statsEnCours->setStyleSheet("font-size: 48px; color: white; background: transparent; font-weight: bold;");
    card2Layout->addWidget(card2Title);
    card2Layout->addWidget(statsEnCours);
    card2Layout->addStretch();

    QWidget *card3 = new QWidget();
    card3->setStyleSheet("QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4facfe, stop:1 #00f2fe); border-radius: 15px; }");
    card3->setMinimumHeight(150);

    QVBoxLayout *card3Layout = new QVBoxLayout(card3);
    QLabel *card3Title = new QLabel("✅ Terminées");
    card3Title->setStyleSheet("font-size: 18px; color: white; background: transparent; font-weight: bold;");

    int terminees = 0;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 7) &&
            (tableClients->item(i, 7)->text().toLower().contains("terminé") ||
             tableClients->item(i, 7)->text().toLower().contains("livré"))) {
            terminees++;
        }
    }

    statsTerminees = new QLabel(QString::number(terminees));
    statsTerminees->setStyleSheet("font-size: 48px; color: white; background: transparent; font-weight: bold;");
    card3Layout->addWidget(card3Title);
    card3Layout->addWidget(statsTerminees);
    card3Layout->addStretch();

    cardsLayout->addWidget(card1);
    cardsLayout->addWidget(card2);
    cardsLayout->addWidget(card3);
    layout->addLayout(cardsLayout);

    QWidget *graphCard = new QWidget();
    graphCard->setStyleSheet("QWidget { background-color: white; border-radius: 15px; }");

    QVBoxLayout *graphLayout = new QVBoxLayout(graphCard);
    graphLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *graphTitle = new QLabel("📈 Répartition des Statuts");
    graphTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #1a202c; background: transparent;");
    graphLayout->addWidget(graphTitle);

    statsTable = new QTableWidget();
    statsTable->setColumnCount(3);
    statsTable->setHorizontalHeaderLabels({"Statut", "Nombre", "Pourcentage"});
    statsTable->setStyleSheet(R"(
        QTableWidget {
            background-color: transparent;
            border: none;
            gridline-color: #e2e8f0;
        }
        QHeaderView::section {
            background-color: #1e40af;
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
        }
        QTableWidget::item { padding: 8px; }
    )");

    QMap<QString, int> statutCounts;
    int total = tableClients->rowCount();

    for (int i = 0; i < total; ++i) {
        if (tableClients->item(i, 7)) {
            QString statut = tableClients->item(i, 7)->text();
            statutCounts[statut]++;
        }
    }

    statsTable->setRowCount(statutCounts.size());
    int row = 0;
    for (auto it = statutCounts.begin(); it != statutCounts.end(); ++it) {
        statsTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        double percentage = (total > 0) ? (it.value() * 100.0 / total) : 0;
        statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(percentage, 'f', 1) + "%"));
        row++;
    }

    statsTable->horizontalHeader()->setStretchLastSection(true);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setMaximumHeight(300);

    graphLayout->addWidget(statsTable);
    layout->addWidget(graphCard);

    QLabel *infoLabel = new QLabel(R"(
        <p style='color: #718096; font-size: 14px;'>
        💡 <b>Note:</b> Statistiques en temps réel<br>
        📅 <b>Période:</b> Toutes les données<br>
        🔄 <b>Mise à jour:</b> Automatique
        </p>
    )");
    infoLabel->setStyleSheet("background: transparent;");
    layout->addWidget(infoLabel);
    layout->addStretch();
}

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

void MainWindow::showListeClients()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::showListeCommandes()
{
    // Fonction supprimée - Liste Commandes n'existe plus
}

void MainWindow::showStatistiques()
{
    updateStatistiques();
    stackedWidget->setCurrentIndex(1);
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

void MainWindow::onAjouterClientSubmit()
{
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

    QString statut;
    if (radioEnCours->isChecked()) {
        statut = "En Cours";
    } else if (radioTermine->isChecked()) {
        statut = "Terminé";
    } else if (radioEnAttente->isChecked()) {
        statut = "En Attente";
    }

    int row = tableClients->rowCount();
    tableClients->insertRow(row);

    tableClients->setItem(row, 0, new QTableWidgetItem(idEdit->text().trimmed()));
    tableClients->setItem(row, 1, new QTableWidgetItem(nomEdit->text().trimmed()));
    tableClients->setItem(row, 2, new QTableWidgetItem(prenomEdit->text().trimmed()));
    tableClients->setItem(row, 3, new QTableWidgetItem(emailEdit->text().trimmed()));
    tableClients->setItem(row, 4, new QTableWidgetItem(telEdit->text().trimmed()));
    tableClients->setItem(row, 5, new QTableWidgetItem(adresseEdit->text().trimmed()));
    tableClients->setItem(row, 6, new QTableWidgetItem(dateEdit->date().toString("dd/MM/yyyy")));
    tableClients->setItem(row, 7, new QTableWidgetItem(statut));
    tableClients->setCellWidget(row, 8, createActionButtons(row));

    QMessageBox::information(this, "Succès", "✅ Client ajouté avec succès !");
    onAnnulerAjout();
    tableClients->scrollToBottom();
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
    radioEnCours->setChecked(true);
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

    QString statut = tableClients->item(row, 7)->text();
    if (statut.contains("Cours")) {
        radioEnCours->setChecked(true);
    } else if (statut.contains("Terminé") || statut.contains("Livré")) {
        radioTermine->setChecked(true);
    } else {
        radioEnAttente->setChecked(true);
    }

    tableClients->removeRow(row);
    idEdit->setFocus();
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

void MainWindow::onSearchClient()
{
    QString searchId = searchEdit->text().trimmed();
    if (searchId.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un ID.");
        return;
    }

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

void MainWindow::onSortClients()
{
    QString sortBy = sortComboBox->currentText();

    if (sortBy == "📊 Trier: Date") {
        QList<QPair<QDate, int>> dateList;

        for (int i = 0; i < tableClients->rowCount(); ++i) {
            if (tableClients->item(i, 6)) {
                QString dateStr = tableClients->item(i, 6)->text();
                QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
                dateList.append(qMakePair(date, i));
            }
        }

        std::sort(dateList.begin(), dateList.end(),
                  [](const QPair<QDate, int> &a, const QPair<QDate, int> &b) {
                      return a.first < b.first;
                  });

        QList<QList<QTableWidgetItem*>> rows;

        for (int i = 0; i < tableClients->rowCount(); ++i) {
            QList<QTableWidgetItem*> row;
            for (int col = 0; col < 8; ++col) {
                QTableWidgetItem *item = tableClients->item(i, col);
                row.append(item ? item->clone() : nullptr);
            }
            rows.append(row);
        }

        tableClients->setRowCount(0);

        for (int i = 0; i < dateList.size(); ++i) {
            int originalRow = dateList[i].second;
            tableClients->insertRow(i);

            for (int col = 0; col < 8; ++col) {
                if (rows[originalRow][col]) {
                    tableClients->setItem(i, col, rows[originalRow][col]->clone());
                }
            }

            tableClients->setCellWidget(i, 8, createActionButtons(i));
        }

    } else {
        int column = 0;

        if (sortBy == "📊 Trier: ID") column = 0;
        else if (sortBy == "📊 Trier: Statut") column = 7;

        tableClients->sortItems(column, Qt::AscendingOrder);

        for (int row = 0; row < tableClients->rowCount(); ++row) {
            tableClients->setCellWidget(row, 8, createActionButtons(row));
        }
    }
}

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
    html += "<th>Adresse</th><th>Date</th><th>Statut</th></tr>";

    for (int row = 0; row < tableClients->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < 8; ++col) {
            html += "<td>" + tableClients->item(row, col)->text() + "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF", "✅ Export PDF réussi !");
}

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
    out << "ID,Nom,Prénom,Email,Téléphone,Adresse,Date,Statut\n";

    for (int row = 0; row < tableClients->rowCount(); ++row) {
        for (int col = 0; col < 8; ++col) {
            out << tableClients->item(row, col)->text();
            if (col < 7) out << ",";
        }
        out << "\n";
    }

    file.close();
    QMessageBox::information(this, "Export Excel", "✅ Export Excel (CSV) réussi !");
}

void MainWindow::refreshClientTable()
{
}

void MainWindow::updateStatistiques()
{
    if (!statsClientTotal || !statsEnCours || !statsTerminees || !statsTable) {
        return;
    }

    statsClientTotal->setText(QString::number(tableClients->rowCount()));

    int enCours = 0;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 7) &&
            tableClients->item(i, 7)->text().toLower().contains("cours")) {
            enCours++;
        }
    }
    statsEnCours->setText(QString::number(enCours));

    int terminees = 0;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 7) &&
            (tableClients->item(i, 7)->text().toLower().contains("terminé") ||
             tableClients->item(i, 7)->text().toLower().contains("livré"))) {
            terminees++;
        }
    }
    statsTerminees->setText(QString::number(terminees));

    QMap<QString, int> statutCounts;
    int total = tableClients->rowCount();

    for (int i = 0; i < total; ++i) {
        if (tableClients->item(i, 7)) {
            QString statut = tableClients->item(i, 7)->text();
            statutCounts[statut]++;
        }
    }

    statsTable->setRowCount(statutCounts.size());
    int row = 0;
    for (auto it = statutCounts.begin(); it != statutCounts.end(); ++it) {
        statsTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        double percentage = (total > 0) ? (it.value() * 100.0 / total) : 0;
        statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(percentage, 'f', 1) + "%"));
        row++;
    }
}

void MainWindow::loadCommandesFromClients()
{
    // Fonction supprimée - Liste Commandes n'existe plus
}

void MainWindow::onFilterCommandes()
{
    // Fonction supprimée - Liste Commandes n'existe plus
}

void MainWindow::onRefreshCommandes()
{
    // Fonction supprimée - Liste Commandes n'existe plus
}

void MainWindow::onShowGraphique()
{
    // Collecte des données par mois/année
    QMap<QString, int> clientsParMois;

    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 6)) {
            QString dateStr = tableClients->item(i, 6)->text();
            QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");

            if (date.isValid()) {
                // Format: "Mois Année" (ex: "Janvier 2026")
                QLocale locale(QLocale::French);
                QString moisAnnee = QString("%1 %2")
                                        .arg(locale.monthName(date.month(), QLocale::LongFormat))
                                        .arg(date.year());

                clientsParMois[moisAnnee]++;
            }
        }
    }

    QDialog *graphDialog = new QDialog(this);
    graphDialog->setWindowTitle("📊 Clients par Date d'Inscription");
    graphDialog->resize(1000, 600);
    graphDialog->setStyleSheet("background-color: #f7fafc;");

    QVBoxLayout *dialogLayout = new QVBoxLayout(graphDialog);
    dialogLayout->setContentsMargins(30, 30, 30, 30);
    dialogLayout->setSpacing(20);

    QLabel *graphTitle = new QLabel("📈 Nombre de Clients Inscrits par Mois");
    graphTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: #1a202c; background: transparent;");
    graphTitle->setAlignment(Qt::AlignCenter);
    dialogLayout->addWidget(graphTitle);

    QWidget *graphWidget = new QWidget();
    graphWidget->setStyleSheet("QWidget { background-color: white; border-radius: 15px; }");
    graphWidget->setMinimumHeight(400);

    QVBoxLayout *graphLayout = new QVBoxLayout(graphWidget);
    graphLayout->setContentsMargins(40, 40, 40, 40);
    graphLayout->setSpacing(15);

    // Trouver le maximum pour l'échelle
    int maxClients = 0;
    for (auto count : clientsParMois.values()) {
        if (count > maxClients) maxClients = count;
    }

    // Créer les barres pour chaque mois
    QList<QString> moisOrdered = clientsParMois.keys();

    for (const QString &mois : moisOrdered) {
        int nbClients = clientsParMois[mois];

        QWidget *barContainer = new QWidget();
        QHBoxLayout *barLayout = new QHBoxLayout(barContainer);
        barLayout->setContentsMargins(0, 0, 0, 0);
        barLayout->setSpacing(10);

        QLabel *moisLabel = new QLabel(mois);
        moisLabel->setFixedWidth(150);
        moisLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #374151; background: transparent;");
        barLayout->addWidget(moisLabel);

        QWidget *bar = new QWidget();
        int barWidth = maxClients > 0 ? (nbClients * 500 / maxClients) : 0;
        bar->setFixedSize(barWidth, 35);

        // Dégradé de couleurs selon le nombre
        QString barColor;
        if (nbClients >= 5) {
            barColor = "#10b981";  // Vert pour beaucoup de clients
        } else if (nbClients >= 3) {
            barColor = "#3b82f6";  // Bleu pour moyen
        } else if (nbClients >= 1) {
            barColor = "#f59e0b";  // Orange pour peu
        } else {
            barColor = "#94a3b8";  // Gris pour aucun
        }

        bar->setStyleSheet(QString("QWidget { background: %1; border-radius: 8px; }").arg(barColor));
        barLayout->addWidget(bar);

        QLabel *countLabel = new QLabel(QString::number(nbClients) + " client" + (nbClients > 1 ? "s" : ""));
        countLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #1e40af; background: transparent;");
        barLayout->addWidget(countLabel);

        barLayout->addStretch();
        graphLayout->addWidget(barContainer);
    }

    graphLayout->addStretch();

    // Ajouter des statistiques en bas
    QLabel *statsLabel = new QLabel(QString(
                                        "📊 <b>Total:</b> %1 client(s) | "
                                        "📅 <b>Périodes:</b> %2 mois différent(s)"
                                        ).arg(tableClients->rowCount()).arg(clientsParMois.size()));
    statsLabel->setStyleSheet("font-size: 14px; color: #64748b; background: transparent; padding: 10px;");
    statsLabel->setAlignment(Qt::AlignCenter);
    graphLayout->addWidget(statsLabel);

    dialogLayout->addWidget(graphWidget);

    QPushButton *btnClose = new QPushButton("✖ Fermer");
    btnClose->setFixedSize(120, 40);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(R"(
        QPushButton {
            background-color: #6b7280;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #4b5563; }
    )");
    connect(btnClose, &QPushButton::clicked, graphDialog, &QDialog::accept);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnClose);
    dialogLayout->addLayout(btnLayout);

    graphDialog->exec();
}

MainWindow::~MainWindow()
{
}
