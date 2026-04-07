#include "materiel_oracle.h.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

MaterielOracle::MaterielOracle(QWidget *parent) : QMainWindow(parent), dbConnected(false)
{
    setupUI();
    setupConnections();

    dbConnected = QSqlDatabase::database().isOpen();
    if (dbConnected) {
        creerTableSiExiste();
        insererDonneesExemple();
        loadData();
        updateStatusBar();
        afficherMessage("Connexion à la base de données Oracle établie avec succès !", false);
    } else {
        afficherMessage("Impossible de se connecter à la base de données Oracle !\n"
                        "Vérifiez que :\n"
                        "1. Oracle est installé et en cours d'exécution\n"
                        "2. Le service ORCL est disponible\n"
                        "3. Les identifiants sont corrects", true);
    }
}

MaterielOracle::~MaterielOracle()
{
}

void MaterielOracle::setupUI()
{
    setWindowTitle("Gestion de Matériel - Oracle Database");
    setMinimumSize(1400, 800);

    centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #f0f2f5;");
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Header
    QFrame *headerFrame = new QFrame;
    headerFrame->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "stop:0 #3498db, stop:1 #2ecc71); border-radius: 15px; }");
    headerFrame->setFixedHeight(120);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(25, 20, 25, 20);

    QVBoxLayout *titleLayout = new QVBoxLayout;
    QLabel *titleLabel = new QLabel("📦 GESTION DE MATÉRIEL - ORACLE DATABASE");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");
    titleLayout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel(QString("Base de données connectée"));
    subtitleLabel->setStyleSheet("font-size: 12px; color: rgba(255,255,255,0.9);");
    titleLayout->addWidget(subtitleLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dddd dd MMMM yyyy"));
    dateLabel->setStyleSheet("font-size: 14px; color: white;");
    headerLayout->addWidget(dateLabel);

    mainLayout->addWidget(headerFrame);

    // Barre de recherche
    QFrame *searchFrame = new QFrame;
    searchFrame->setStyleSheet("QFrame { background-color: white; border-radius: 12px; }");
    searchFrame->setFixedHeight(70);

    QHBoxLayout *searchLayout = new QHBoxLayout(searchFrame);
    searchLayout->setContentsMargins(20, 10, 20, 10);
    searchLayout->setSpacing(15);

    QLabel *searchLabel = new QLabel("🔍 Rechercher :");
    searchLabel->setStyleSheet("font-weight: bold; color: #2c3e50; font-size: 13px;");

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Rechercher par nom, type ou fournisseur...");
    searchEdit->setStyleSheet(
        "QLineEdit { background-color: #f8f9fa; border: 2px solid #e9ecef; border-radius: 10px;"
        "padding: 12px 15px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #3498db; }");
    searchEdit->setMinimumWidth(400);

    rechercherBtn = new QPushButton("🔍 Rechercher");
    rechercherBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; border-radius: 8px;"
        "padding: 10px 20px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }");

    actualiserBtn = new QPushButton("🔄 Actualiser");
    actualiserBtn->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; border: none; border-radius: 8px;"
        "padding: 10px 20px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #229954; }");

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(rechercherBtn);
    searchLayout->addStretch();
    searchLayout->addWidget(actualiserBtn);

    mainLayout->addWidget(searchFrame);

    // Contenu principal
    contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(20);

    // Formulaire
    QFrame *formFrame = new QFrame;
    formFrame->setStyleSheet("QFrame { background-color: white; border-radius: 15px; }");
    formFrame->setFixedWidth(480);

    formLayout = new QVBoxLayout(formFrame);
    formLayout->setContentsMargins(25, 20, 25, 20);
    formLayout->setSpacing(15);

    QLabel *formTitle = new QLabel("📝 Formulaire matériel");
    formTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #3498db;");
    formLayout->addWidget(formTitle);

    QString inputStyle =
        "QLineEdit, QComboBox, QDateEdit { background-color: #ffffff; border: 2px solid #e0e0e0;"
        "border-radius: 8px; padding: 10px 12px; font-size: 13px; min-height: 20px; }"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border-color: #3498db; }"
        "QLineEdit:read-only { background-color: #f5f5f5; color: #7f8c8d; }";

    QFormLayout *inputForm = new QFormLayout;
    inputForm->setSpacing(12);
    inputForm->setLabelAlignment(Qt::AlignRight);

    idEdit = new QLineEdit;
    idEdit->setReadOnly(true);
    idEdit->setStyleSheet(inputStyle);

    nomEdit = new QLineEdit;
    nomEdit->setPlaceholderText("Ex: Poutre en chêne");
    nomEdit->setStyleSheet(inputStyle);

    typeBox = new QComboBox;
    typeBox->addItems({"Bois", "Outil", "Consommable", "Électrique", "Électronique", "Métal", "Plastique"});
    typeBox->setStyleSheet(inputStyle);

    quantiteEdit = new QLineEdit;
    quantiteEdit->setPlaceholderText("Ex: 50");
    quantiteEdit->setStyleSheet(inputStyle);
    quantiteEdit->setValidator(new QIntValidator(0, 999999, this));

    seuilEdit = new QLineEdit;
    seuilEdit->setPlaceholderText("Ex: 10");
    seuilEdit->setStyleSheet(inputStyle);
    seuilEdit->setValidator(new QIntValidator(0, 999999, this));

    dateEdit = new QDateEdit;
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setStyleSheet(inputStyle);

    fournisseurEdit = new QLineEdit;
    fournisseurEdit->setPlaceholderText("Ex: BoisCorp");
    fournisseurEdit->setStyleSheet(inputStyle);

    statutBox = new QComboBox;
    statutBox->addItems({"Disponible", "Rupture", "En commande", "En maintenance"});
    statutBox->setStyleSheet(inputStyle);

    QLabel *idLabel = new QLabel("ID :");
    idLabel->setStyleSheet("font-weight: bold; color: #2c3e50; min-width: 100px;");
    QLabel *nomLabel = new QLabel("Nom * :");
    nomLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
    QLabel *typeLabel = new QLabel("Type :");
    typeLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
    QLabel *quantiteLabel = new QLabel("Quantité * :");
    quantiteLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
    QLabel *seuilLabel = new QLabel("Seuil * :");
    seuilLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
    QLabel *dateLabelForm = new QLabel("Date d'ajout :");
    dateLabelForm->setStyleSheet("font-weight: bold; color: #2c3e50;");
    QLabel *fournisseurLabel = new QLabel("Fournisseur :");
    fournisseurLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");
    QLabel *statutLabelForm = new QLabel("Statut :");
    statutLabelForm->setStyleSheet("font-weight: bold; color: #2c3e50;");

    inputForm->addRow(idLabel, idEdit);
    inputForm->addRow(nomLabel, nomEdit);
    inputForm->addRow(typeLabel, typeBox);
    inputForm->addRow(quantiteLabel, quantiteEdit);
    inputForm->addRow(seuilLabel, seuilEdit);
    inputForm->addRow(dateLabelForm, dateEdit);
    inputForm->addRow(fournisseurLabel, fournisseurEdit);
    inputForm->addRow(statutLabelForm, statutBox);

    formLayout->addLayout(inputForm);

    // Boutons du formulaire
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

    ajouterBtn = new QPushButton("➕ Ajouter");
    ajouterBtn->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border: none; border-radius: 8px;"
        "padding: 12px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #27ae60; }");

    modifierBtn = new QPushButton("✏️ Modifier");
    modifierBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; border-radius: 8px;"
        "padding: 12px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }");

    supprimerBtn = new QPushButton("🗑️ Supprimer");
    supprimerBtn->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; border: none; border-radius: 8px;"
        "padding: 12px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #c0392b; }");

    viderBtn = new QPushButton("🔄 Vider");
    viderBtn->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 8px;"
        "padding: 12px; font-size: 13px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7f8c8d; }");

    buttonLayout->addWidget(ajouterBtn);
    buttonLayout->addWidget(modifierBtn);
    buttonLayout->addWidget(supprimerBtn);
    buttonLayout->addWidget(viderBtn);

    formLayout->addLayout(buttonLayout);

    // Boutons supplémentaires
    QHBoxLayout *extraButtonLayout = new QHBoxLayout;
    extraButtonLayout->setSpacing(12);

    exporterBtn = new QPushButton("📄 Exporter CSV");
    exporterBtn->setStyleSheet(
        "QPushButton { background-color: #9b59b6; color: white; border: none; border-radius: 8px;"
        "padding: 10px; font-size: 12px; font-weight: bold; }"
        "QPushButton:hover { background-color: #8e44ad; }");

    importerBtn = new QPushButton("📂 Importer CSV");
    importerBtn->setStyleSheet(
        "QPushButton { background-color: #16a085; color: white; border: none; border-radius: 8px;"
        "padding: 10px; font-size: 12px; font-weight: bold; }"
        "QPushButton:hover { background-color: #138d75; }");

    statistiquesBtn = new QPushButton("📊 Statistiques");
    statistiquesBtn->setStyleSheet(
        "QPushButton { background-color: #f39c12; color: white; border: none; border-radius: 8px;"
        "padding: 10px; font-size: 12px; font-weight: bold; }"
        "QPushButton:hover { background-color: #e67e22; }");

    extraButtonLayout->addWidget(exporterBtn);
    extraButtonLayout->addWidget(importerBtn);
    extraButtonLayout->addWidget(statistiquesBtn);

    formLayout->addLayout(extraButtonLayout);

    QLabel *infoNote = new QLabel("⚠️ Les champs avec * sont obligatoires");
    infoNote->setStyleSheet("color: #e67e22; font-size: 11px; padding-top: 10px;");
    formLayout->addWidget(infoNote);

    contentLayout->addWidget(formFrame);

    // Tableau
    QFrame *tableFrame = new QFrame;
    tableFrame->setStyleSheet("QFrame { background-color: white; border-radius: 15px; }");

    tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setContentsMargins(15, 15, 15, 15);

    QLabel *tableTitle = new QLabel("📋 Liste du matériel");
    tableTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding-bottom: 10px;");
    tableLayout->addWidget(tableTitle);

    tableView = new QTableView;
    tableView->setStyleSheet(
        "QTableView { background-color: white; border: none; font-size: 13px; gridline-color: #e9ecef; }"
        "QTableView::item { padding: 8px; }"
        "QTableView::item:selected { background-color: #e3f2fd; }"
        "QHeaderView::section { background-color: #2c3e50; color: white; padding: 10px; border: none; font-weight: bold; }");
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->setVisible(false);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);

    tableLayout->addWidget(tableView);
    contentLayout->addWidget(tableFrame, 2);

    mainLayout->addLayout(contentLayout);

    // Status bar
    statusBar = new QStatusBar;
    setStatusBar(statusBar);
    statusLabel = new QLabel("✅ Prêt");
    connexionLabel = new QLabel("🔌 Connexion Oracle: Active");
    statusBar->addWidget(statusLabel);
    statusBar->addPermanentWidget(connexionLabel);
    statusBar->setStyleSheet("QStatusBar { background-color: #2c3e50; color: white; padding: 5px; }");
}



void MaterielOracle::creerTableSiExiste()
{
    QSqlQuery query;

    // Vérifier si la table existe
    QString checkTable = "SELECT COUNT(*) FROM user_tables WHERE table_name = 'MATERIEL'";
    query.exec(checkTable);

    if (query.next() && query.value(0).toInt() == 0) {
        // Créer la table
        QString createTable =
            "CREATE TABLE materiel ("
            "id_materiel NUMBER PRIMARY KEY,"
            "nom VARCHAR2(100) NOT NULL,"
            "type VARCHAR2(50),"
            "quantite NUMBER NOT NULL,"
            "seuil_min NUMBER NOT NULL,"
            "date_ajout DATE,"
            "fournisseur VARCHAR2(100),"
            "statut VARCHAR2(20),"
            "created_at DATE DEFAULT SYSDATE,"
            "updated_at DATE DEFAULT SYSDATE"
            ")";

        if (!query.exec(createTable)) {
            afficherMessage("Erreur création table: " + query.lastError().text(), true);
        } else {
            // Créer une séquence pour l'auto-incrémentation
            QString createSequence = "CREATE SEQUENCE materiel_seq START WITH 1 INCREMENT BY 1";
            query.exec(createSequence);

            // Créer un trigger pour l'auto-incrémentation
            QString createTrigger =
                "CREATE OR REPLACE TRIGGER materiel_trigger "
                "BEFORE INSERT ON materiel "
                "FOR EACH ROW "
                "BEGIN "
                "SELECT materiel_seq.NEXTVAL INTO :NEW.id_materiel FROM dual; "
                "END;";
            query.exec(createTrigger);
        }
    }
}

void MaterielOracle::insererDonneesExemple()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM materiel");

    if (query.next() && query.value(0).toInt() == 0) {
        QStringList exemples;
        exemples << "('Poutre en chêne', 'Bois', 50, 10, SYSDATE, 'BoisCorp', 'Disponible')";
        exemples << "('Marteau perforateur', 'Outil', 15, 5, SYSDATE, 'TechTools', 'Disponible')";
        exemples << "('Vis à bois 5x60', 'Consommable', 500, 100, SYSDATE, 'FixPro', 'Disponible')";
        exemples << "('Perceuse électrique', 'Électrique', 8, 3, SYSDATE, 'Bosch', 'Disponible')";
        exemples << "('Câble HDMI 2m', 'Électronique', 120, 20, SYSDATE, 'CablePro', 'Disponible')";
        exemples << "('Scie circulaire', 'Outil', 12, 4, SYSDATE, 'Makita', 'Disponible')";

        for (const QString& exemple : exemples) {
            QString insertQuery = "INSERT INTO materiel (nom, type, quantite, seuil_min, date_ajout, fournisseur, statut) VALUES " + exemple;
            query.exec(insertQuery);
        }

        afficherMessage("Données d'exemple insérées avec succès !", false);
    }
}

void MaterielOracle::setupConnections()
{
    connect(ajouterBtn, &QPushButton::clicked, this, &MaterielOracle::ajouterMateriel);
    connect(modifierBtn, &QPushButton::clicked, this, &MaterielOracle::modifierMateriel);
    connect(supprimerBtn, &QPushButton::clicked, this, &MaterielOracle::supprimerMateriel);
    connect(actualiserBtn, &QPushButton::clicked, this, &MaterielOracle::actualiserTable);
    connect(viderBtn, &QPushButton::clicked, this, &MaterielOracle::viderChamps);
    connect(rechercherBtn, &QPushButton::clicked, this, &MaterielOracle::rechercherMateriel);
    connect(exporterBtn, &QPushButton::clicked, this, &MaterielOracle::exporterCSV);
    connect(importerBtn, &QPushButton::clicked, this, &MaterielOracle::importerCSV);
    connect(statistiquesBtn, &QPushButton::clicked, this, &MaterielOracle::afficherStatistiques);
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MaterielOracle::onTableSelectionChanged);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MaterielOracle::rechercherMateriel);
}

void MaterielOracle::loadData()
{
    model = new QSqlTableModel(this);
    model->setTable("materiel");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    // Configuration des en-têtes
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nom");
    model->setHeaderData(2, Qt::Horizontal, "Type");
    model->setHeaderData(3, Qt::Horizontal, "Quantité");
    model->setHeaderData(4, Qt::Horizontal, "Seuil");
    model->setHeaderData(5, Qt::Horizontal, "Date d'ajout");
    model->setHeaderData(6, Qt::Horizontal, "Fournisseur");
    model->setHeaderData(7, Qt::Horizontal, "Statut");

    tableView->setModel(model);
    tableView->hideColumn(8); // Masquer created_at
    tableView->hideColumn(9); // Masquer updated_at

    tableView->resizeColumnsToContents();
}

void MaterielOracle::ajouterMateriel()
{
    if (nomEdit->text().trimmed().isEmpty()) {
        afficherMessage("Veuillez saisir le nom du matériel !", true);
        nomEdit->setFocus();
        return;
    }

    if (quantiteEdit->text().trimmed().isEmpty()) {
        afficherMessage("Veuillez saisir la quantité !", true);
        quantiteEdit->setFocus();
        return;
    }

    bool ok;
    int quantite = quantiteEdit->text().toInt(&ok);
    if (!ok || quantite < 0) {
        afficherMessage("Quantité invalide !", true);
        quantiteEdit->setFocus();
        return;
    }

    int seuil = seuilEdit->text().toInt(&ok);
    if (!ok || seuil < 0) {
        afficherMessage("Seuil invalide !", true);
        seuilEdit->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO materiel (nom, type, quantite, seuil_min, date_ajout, fournisseur, statut) "
                  "VALUES (:nom, :type, :quantite, :seuil, TO_DATE(:date, 'YYYY-MM-DD'), :fournisseur, :statut)");

    query.bindValue(":nom", nomEdit->text().trimmed());
    query.bindValue(":type", typeBox->currentText());
    query.bindValue(":quantite", quantite);
    query.bindValue(":seuil", seuil);
    query.bindValue(":date", dateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", fournisseurEdit->text().trimmed());

    QString statut = (quantite <= seuil) ? "Rupture" : "Disponible";
    query.bindValue(":statut", statut);

    if (!query.exec()) {
        afficherMessage("Erreur Oracle: " + query.lastError().text(), true);
    } else {
        afficherMessage("✓ Matériel ajouté avec succès dans Oracle !", false);
        actualiserTable();
        viderChamps();
    }
}

void MaterielOracle::modifierMateriel()
{
    int row = tableView->currentIndex().row();
    if (row < 0) {
        afficherMessage("Veuillez sélectionner un matériel à modifier !", true);
        return;
    }

    int id = model->data(model->index(row, 0)).toInt();

    if (nomEdit->text().trimmed().isEmpty()) {
        afficherMessage("Veuillez saisir le nom du matériel !", true);
        nomEdit->setFocus();
        return;
    }

    bool ok;
    int quantite = quantiteEdit->text().toInt(&ok);
    if (!ok || quantite < 0) {
        afficherMessage("Quantité invalide !", true);
        quantiteEdit->setFocus();
        return;
    }

    int seuil = seuilEdit->text().toInt(&ok);
    if (!ok || seuil < 0) {
        afficherMessage("Seuil invalide !", true);
        seuilEdit->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE materiel SET nom = :nom, type = :type, quantite = :quantite, "
                  "seuil_min = :seuil, date_ajout = TO_DATE(:date, 'YYYY-MM-DD'), "
                  "fournisseur = :fournisseur, statut = :statut, updated_at = SYSDATE "
                  "WHERE id_materiel = :id");

    query.bindValue(":id", id);
    query.bindValue(":nom", nomEdit->text().trimmed());
    query.bindValue(":type", typeBox->currentText());
    query.bindValue(":quantite", quantite);
    query.bindValue(":seuil", seuil);
    query.bindValue(":date", dateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":fournisseur", fournisseurEdit->text().trimmed());

    QString statut = (quantite <= seuil) ? "Rupture" : "Disponible";
    query.bindValue(":statut", statut);

    if (!query.exec()) {
        afficherMessage("Erreur Oracle: " + query.lastError().text(), true);
    } else {
        afficherMessage("✓ Matériel modifié avec succès !", false);
        actualiserTable();
        viderChamps();
    }
}

void MaterielOracle::supprimerMateriel()
{
    int row = tableView->currentIndex().row();
    if (row < 0) {
        afficherMessage("Veuillez sélectionner un matériel à supprimer !", true);
        return;
    }

    int id = model->data(model->index(row, 0)).toInt();
    QString nom = model->data(model->index(row, 1)).toString();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmation",
        QString("Voulez-vous vraiment supprimer le matériel :\n\n'%1' (ID: %2) ?\n\nCette action est irréversible !")
            .arg(nom).arg(id),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM materiel WHERE id_materiel = :id");
        query.bindValue(":id", id);

        if (!query.exec()) {
            afficherMessage("Erreur Oracle: " + query.lastError().text(), true);
        } else {
            afficherMessage("✓ Matériel supprimé avec succès !", false);
            actualiserTable();
            viderChamps();
        }
    }
}

void MaterielOracle::rechercherMateriel()
{
    QString searchText = searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        actualiserTable();
        return;
    }

    QSqlQueryModel *searchModel = new QSqlQueryModel(this);
    QSqlQuery query;
    query.prepare("SELECT id_materiel, nom, type, quantite, seuil_min, "
                  "TO_CHAR(date_ajout, 'DD/MM/YYYY') as date_ajout, "
                  "fournisseur, statut "
                  "FROM materiel WHERE UPPER(nom) LIKE UPPER(:search) OR "
                  "UPPER(type) LIKE UPPER(:search) OR "
                  "UPPER(fournisseur) LIKE UPPER(:search) "
                  "ORDER BY id_materiel");
    query.bindValue(":search", "%" + searchText + "%");
    query.exec();

    searchModel->setQuery(std::move(query));

    searchModel->setHeaderData(0, Qt::Horizontal, "ID");
    searchModel->setHeaderData(1, Qt::Horizontal, "Nom");
    searchModel->setHeaderData(2, Qt::Horizontal, "Type");
    searchModel->setHeaderData(3, Qt::Horizontal, "Quantité");
    searchModel->setHeaderData(4, Qt::Horizontal, "Seuil");
    searchModel->setHeaderData(5, Qt::Horizontal, "Date d'ajout");
    searchModel->setHeaderData(6, Qt::Horizontal, "Fournisseur");
    searchModel->setHeaderData(7, Qt::Horizontal, "Statut");

    tableView->setModel(searchModel);

    int count = searchModel->rowCount();
    statusLabel->setText(QString("🔍 %1 résultat(s) trouvé(s)").arg(count));

    if (count == 0) {
        afficherMessage("Aucun résultat trouvé pour: " + searchText, true);
    }
}

void MaterielOracle::actualiserTable()
{
    if (model) {
        model->select();
        tableView->setModel(model);
        tableView->resizeColumnsToContents();
    }
    searchEdit->clear();
    updateStatusBar();
}

void MaterielOracle::viderChamps()
{
    idEdit->clear();
    nomEdit->clear();
    typeBox->setCurrentIndex(0);
    quantiteEdit->clear();
    seuilEdit->clear();
    dateEdit->setDate(QDate::currentDate());
    fournisseurEdit->clear();
    statutBox->setCurrentIndex(0);

    nomEdit->setFocus();
    tableView->clearSelection();
}

void MaterielOracle::onTableSelectionChanged()
{
    int row = tableView->currentIndex().row();
    if (row < 0) return;

    idEdit->setText(model->data(model->index(row, 0)).toString());
    nomEdit->setText(model->data(model->index(row, 1)).toString());
    typeBox->setCurrentText(model->data(model->index(row, 2)).toString());
    quantiteEdit->setText(model->data(model->index(row, 3)).toString());
    seuilEdit->setText(model->data(model->index(row, 4)).toString());

    QString dateStr = model->data(model->index(row, 5)).toString();
    if (!dateStr.isEmpty()) {
        QDate date = QDate::fromString(dateStr, "dd/MM/yyyy");
        if (date.isValid()) {
            dateEdit->setDate(date);
        }
    }

    fournisseurEdit->setText(model->data(model->index(row, 6)).toString());
    statutBox->setCurrentText(model->data(model->index(row, 7)).toString());
}

void MaterielOracle::exporterCSV()
{
    QString filename = QFileDialog::getSaveFileName(this, "Exporter en CSV",
                                                    QDir::homePath() + "/materiel_oracle_export.csv", "CSV Files (*.csv)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        afficherMessage("Impossible de créer le fichier !", true);
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // En-tête
    stream << "ID;NOM;TYPE;QUANTITÉ;SEUIL;DATE D'AJOUT;FOURNISSEUR;STATUT\n";

    QSqlQuery query("SELECT id_materiel, nom, type, quantite, seuil_min, "
                    "TO_CHAR(date_ajout, 'DD/MM/YYYY'), fournisseur, statut "
                    "FROM materiel ORDER BY id_materiel");

    while (query.next()) {
        stream << query.value(0).toString() << ";"
               << query.value(1).toString() << ";"
               << query.value(2).toString() << ";"
               << query.value(3).toString() << ";"
               << query.value(4).toString() << ";"
               << query.value(5).toString() << ";"
               << query.value(6).toString() << ";"
               << query.value(7).toString() << "\n";
    }

    file.close();
    afficherMessage("✓ Export CSV réussi !\nFichier: " + filename, false);
}

void MaterielOracle::importerCSV()
{
    afficherMessage("📂 Import CSV - Sélectionnez un fichier CSV", false);

    QString filename = QFileDialog::getOpenFileName(this, "Importer un fichier CSV",
                                                    QDir::homePath(), "CSV Files (*.csv)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        afficherMessage("Impossible d'ouvrir le fichier !", true);
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    QString header = stream.readLine(); // Lire l'en-tête
    int compteur = 0;

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList fields = line.split(";");

        if (fields.size() >= 7) {
            QSqlQuery query;
            query.prepare("INSERT INTO materiel (nom, type, quantite, seuil_min, date_ajout, fournisseur, statut) "
                          "VALUES (:nom, :type, :quantite, :seuil, SYSDATE, :fournisseur, :statut)");

            query.bindValue(":nom", fields[1]);
            query.bindValue(":type", fields[2]);
            query.bindValue(":quantite", fields[3].toInt());
            query.bindValue(":seuil", fields[4].toInt());
            query.bindValue(":fournisseur", fields[6]);
            query.bindValue(":statut", fields[7]);

            if (query.exec()) {
                compteur++;
            }
        }
    }

    file.close();
    afficherMessage(QString("✓ Import CSV réussi !\n%1 ligne(s) importée(s)").arg(compteur), false);
    actualiserTable();
}

void MaterielOracle::afficherStatistiques()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) as total, SUM(quantite) as total_quantite FROM materiel");

    int total = 0, totalQuantite = 0;
    if (query.next()) {
        total = query.value(0).toInt();
        totalQuantite = query.value(1).toInt();
    }

    query.exec("SELECT COUNT(*) FROM materiel WHERE quantite <= seuil_min");
    int critique = 0;
    if (query.next()) critique = query.value(0).toInt();

    query.exec("SELECT statut, COUNT(*) FROM materiel GROUP BY statut");
    QString statsStatut;
    while (query.next()) {
        statsStatut += QString("• %1: %2\n").arg(query.value(0).toString()).arg(query.value(1).toInt());
    }

    query.exec("SELECT type, COUNT(*) FROM materiel GROUP BY type ORDER BY COUNT(*) DESC");
    QString statsType;
    while (query.next()) {
        statsType += QString("• %1: %2\n").arg(query.value(0).toString()).arg(query.value(1).toInt());
    }

    QString message = QString(
                          "📊 STATISTIQUES ORACLE\n"
                          "═══════════════════════════════\n\n"
                          "📦 Vue d'ensemble :\n"
                          "• Total matériels: %1\n"
                          "• Quantité totale: %2 unités\n"
                          "• Stock critique: %3\n\n"
                          "📌 Par statut:\n%4\n"
                          "🏷️ Par type:\n%5"
                          ).arg(total).arg(totalQuantite).arg(critique).arg(statsStatut).arg(statsType);

    QMessageBox::information(this, "Statistiques Oracle", message);
}

void MaterielOracle::updateStatusBar()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*), SUM(quantite) FROM materiel");
    if (query.next()) {
        int total = query.value(0).toInt();
        int totalQuantite = query.value(1).toInt();

        query.exec("SELECT COUNT(*) FROM materiel WHERE quantite <= seuil_min");
        int critique = 0;
        if (query.next()) critique = query.value(0).toInt();

        statusLabel->setText(QString("✅ Prêt - %1 matériel(s) | Stock total: %2 unités | ⚠️ Stock critique: %3")
                                 .arg(total).arg(totalQuantite).arg(critique));
    }
}

void MaterielOracle::afficherMessage(const QString& message, bool erreur)
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(erreur ? "❌ Erreur Oracle" : "✅ Succès Oracle");
    msgBox->setText(message);
    msgBox->setIcon(erreur ? QMessageBox::Warning : QMessageBox::Information);
    QTimer::singleShot(3000, msgBox, &QMessageBox::accept);
    msgBox->exec();
}

bool MaterielOracle::verifierConnexion()
{
    return QSqlDatabase::database().isOpen();
}
