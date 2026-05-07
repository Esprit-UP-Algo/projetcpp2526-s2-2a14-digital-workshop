#include "mainwindow.h"
#include "chatbot.h"
#include "chatwidget.h"
#include "client.h"
#include "emailsender.h"
#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDate>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>

#if QT_VERSION >= 0x050000
#include <QtPrintSupport/QPrinter>
#else
#include <QPrinter>
#endif

#define EMAILJS_SERVICE_ID "service_df0eu2o"
#define EMAILJS_TEMPLATE_ID "template_1nhhpu2"
#define EMAILJS_PUBLIC_KEY "EMzsNrZPqIEjQXH9z"

// =============================================
// CONSTRUCTEUR
// =============================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), statsClientTotal(nullptr), statsClientsMois(nullptr),
    statsClientsSemaine(nullptr), chartContainer(nullptr),
    statsTable(nullptr), chatbot(new Chatbot(this)), chatWidget(nullptr),
    editingId(-1), isEditing(false) {
    setWindowTitle("Smart Menuiserie - Gestion Clients");
    resize(1600, 900);
    setStyleSheet("QMainWindow { background-color: #f7fafc; }");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
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

    refreshClientTable();

    EmailSender::getInstance()->setEmailJSKeys(
        EMAILJS_SERVICE_ID, EMAILJS_TEMPLATE_ID, EMAILJS_PUBLIC_KEY);

    // ChatWidget
    chatWidget = new ChatWidget(this);
    chatWidget->connectChatbot(chatbot);
    chatWidget->setFixedSize(450, 650);
    chatWidget->move(width() - 470, height() - 700);
    chatWidget->show();
    chatWidget->raise();

    // Arduino
    arduino = new Arduino();
    int ret = arduino->connect_arduino();
    switch (ret) {
    case 0:
        qDebug() << "Arduino connecte sur :" << arduino->getarduino_port_name();
        break;
    case 1:
        qDebug() << "Arduino disponible mais non connecte";
        break;
    case -1:
        qDebug() << "Arduino non disponible";
        break;
    }
    QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readFromArduino()));
}

MainWindow::~MainWindow() {}

// =============================================
// RESIZE
// =============================================
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if (chatWidget)
        chatWidget->move(width() - 470, height() - 700);
}

// =============================================
// VALIDATION
// =============================================
void MainWindow::showValidationError(const QString &erreur) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Erreur de saisie");
    msgBox.setText(erreur);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStyleSheet(R"(
        QMessageBox { background-color: white; }
        QMessageBox QLabel {
            color: #dc2626; font-size: 14px;
            min-width: 380px; padding: 10px;
        }
        QPushButton {
            background-color: #1e40af; color: white;
            border-radius: 6px; padding: 8px 25px;
            font-size: 13px; font-weight: bold; min-width: 80px;
        }
        QPushButton:hover { background-color: #1e3a8a; }
    )");
    msgBox.exec();
}

void MainWindow::resetFieldStyles() {
    QString s = R"(
        QLineEdit {
            padding: 12px 15px; border: 2px solid #e2e8f0;
            border-radius: 8px; font-size: 14px;
            background-color: #f9fafb; height: 42px;
        }
        QLineEdit:focus {
            border: 2px solid #1e40af; background-color: white;
        }
    )";
    idEdit->setStyleSheet(s);
    nomEdit->setStyleSheet(s);
    prenomEdit->setStyleSheet(s);
    emailEdit->setStyleSheet(s);
    telEdit->setStyleSheet(s);
    adresseEdit->setStyleSheet(s);
}

void MainWindow::highlightInvalidField(const QString &erreur) {
    QString err = R"(
        QLineEdit {
            padding: 12px 15px; border: 2px solid #dc2626;
            border-radius: 8px; font-size: 14px;
            background-color: #fef2f2; height: 42px;
        }
    )";
    if (erreur.contains("ID")) {
        idEdit->setStyleSheet(err);
        idEdit->setFocus();
    } else if (erreur.contains("Nom") && !erreur.contains("Prenom")) {
        nomEdit->setStyleSheet(err);
        nomEdit->setFocus();
    } else if (erreur.contains("Prenom")) {
        prenomEdit->setStyleSheet(err);
        prenomEdit->setFocus();
    } else if (erreur.contains("Email")) {
        emailEdit->setStyleSheet(err);
        emailEdit->setFocus();
    } else if (erreur.contains("Telephone")) {
        telEdit->setStyleSheet(err);
        telEdit->setFocus();
    } else if (erreur.contains("Adresse")) {
        adresseEdit->setStyleSheet(err);
        adresseEdit->setFocus();
    }
}

// =============================================
// SIDEBAR
// =============================================
void MainWindow::createSidebar() {
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #1e3a8a, stop:1 #1e40af);
        }
    )");

    QVBoxLayout *sl = new QVBoxLayout(sidebar);
    sl->setSpacing(5);
    sl->setContentsMargins(15, 30, 15, 30);

    QLabel *logo = new QLabel("Smart Menuiserie");
    logo->setStyleSheet("font-size:20px;font-weight:bold;color:white;"
                        "padding:20px 10px;background:transparent;");
    logo->setAlignment(Qt::AlignCenter);
    sl->addWidget(logo);
    sl->addSpacing(30);

    QString btnActive = R"(
        QPushButton { background-color:rgba(255,255,255,0.15); color:white;
            border:none; border-radius:10px; padding:15px 20px;
            font-size:15px; text-align:left; font-weight:500; }
        QPushButton:hover { background-color:rgba(255,255,255,0.25); }
    )";
    QString btnDisabled = R"(
        QPushButton { background-color:rgba(255,255,255,0.03);
            color:rgba(255,255,255,0.3); border:none;
            border-radius:10px; padding:15px 20px;
            font-size:15px; text-align:left; }
    )";
    QString btnSub = R"(
        QPushButton { background-color:rgba(255,255,255,0.08);
            color:rgba(255,255,255,0.9); border:none;
            border-radius:8px; padding:12px 20px 12px 45px;
            font-size:14px; text-align:left; }
        QPushButton:hover { background-color:rgba(255,255,255,0.18); }
    )";

    auto addBtn = [&](const QString &txt, const QString &style,
                      bool enabled = true) -> QPushButton * {
        QPushButton *b = new QPushButton(txt);
        b->setStyleSheet(style);
        b->setMinimumHeight(50);
        b->setEnabled(enabled);
        if (enabled)
            b->setCursor(Qt::PointingHandCursor);
        sl->addWidget(b);
        return b;
    };

    addBtn("  Utilisateurs", btnDisabled, false);
    addBtn("  Materiel", btnDisabled, false);
    btnListeClients = addBtn("  Clients", btnActive);

    btnStatistiques = new QPushButton("     Statistiques");
    btnStatistiques->setStyleSheet(btnSub);
    btnStatistiques->setCursor(Qt::PointingHandCursor);
    btnStatistiques->setMinimumHeight(42);
    btnStatistiques->setVisible(false);
    sl->addWidget(btnStatistiques);

    addBtn("  Commandes", btnDisabled, false);
    sl->addStretch();

    btnDeconnexion = new QPushButton("  Deconnexion");
    btnDeconnexion->setStyleSheet(R"(
        QPushButton { background-color:rgba(239,68,68,0.8); color:white;
            border:none; border-radius:10px; padding:15px 20px;
            font-size:15px; text-align:left; }
        QPushButton:hover { background-color:rgba(239,68,68,1); }
    )");
    btnDeconnexion->setCursor(Qt::PointingHandCursor);
    btnDeconnexion->setMinimumHeight(50);
    sl->addWidget(btnDeconnexion);

    sl->addSpacing(10);
    QPushButton *btnRetryArduino = new QPushButton("  🔄 Reconnecter RFID");
    btnRetryArduino->setStyleSheet(R"(
        QPushButton { background-color:rgba(16,185,129,0.2); color:#10b981;
            border:1px solid #10b981; border-radius:10px; padding:10px 15px;
            font-size:13px; text-align:left; font-weight:bold; }
        QPushButton:hover { background-color:rgba(16,185,129,0.3); }
    )");
    btnRetryArduino->setCursor(Qt::PointingHandCursor);
    sl->addWidget(btnRetryArduino);

    connect(btnRetryArduino, &QPushButton::clicked, this, [this]() {
        if(arduino) {
            arduino->close_arduino();
            int ret = arduino->connect_arduino();
            if(ret == 0) QMessageBox::information(this, "RFID", "Connecte avec succes sur " + arduino->getarduino_port_name());
            else QMessageBox::warning(this, "RFID", "Echec de connexion. Verifiez le port et fermez l'IDE Arduino.");
        }
    });

    connect(btnListeClients, &QPushButton::clicked, this, [this]() {
        showListeClients();
        btnStatistiques->setVisible(true);
    });
    connect(btnStatistiques, &QPushButton::clicked, this,
            &MainWindow::showStatistiques);
    connect(btnDeconnexion, &QPushButton::clicked, this,
            &MainWindow::onDeconnexion);

    centralWidget()->layout()->addWidget(sidebar);
}

// =============================================
// PAGE LISTE CLIENTS
// =============================================
void MainWindow::createListeClientsPage() {
    pageListeClients = new QWidget();
    pageListeClients->setStyleSheet("background-color:#f7fafc;");

    QHBoxLayout *ml = new QHBoxLayout(pageListeClients);
    ml->setContentsMargins(20, 20, 20, 20);
    ml->setSpacing(20);

    // ---- FORMULAIRE GAUCHE ----
    QWidget *left = new QWidget();
    left->setFixedWidth(420);
    left->setStyleSheet("QWidget{background-color:white;border-radius:15px;}");

    QVBoxLayout *ll = new QVBoxLayout(left);
    ll->setContentsMargins(30, 30, 30, 30);
    ll->setSpacing(8);

    QLabel *fTitle = new QLabel("Ajouter / Modifier Client");
    fTitle->setStyleSheet("font-size:20px;font-weight:bold;"
                          "color:#1e40af;background:transparent;");
    ll->addWidget(fTitle);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color:#e2e8f0;");
    sep->setFixedHeight(2);
    ll->addWidget(sep);
    ll->addSpacing(5);

    QString inputStyle = R"(
        QLineEdit, QDateEdit {
            padding:8px 12px; border:2px solid #e2e8f0;
            border-radius:8px; font-size:13px;
            background-color:#f9fafb; height:38px;
        }
        QLineEdit:focus, QDateEdit:focus {
            border:2px solid #1e40af; background-color:white;
        }
    )";
    QString lblStyle = "font-size:12px;font-weight:600;"
                       "color:#374151;background:transparent;";

    auto addField = [&](const QString &lbl, QLineEdit *&edit, const QString &ph) {
        QLabel *l = new QLabel(lbl);
        l->setStyleSheet(lblStyle);
        l->setFixedHeight(18);
        ll->addWidget(l);
        edit = new QLineEdit();
        edit->setStyleSheet(inputStyle);
        edit->setPlaceholderText(ph);
        edit->setFixedHeight(38);
        ll->addWidget(edit);
    };

    addField("ID Client *", idEdit, "Ex: 123");
    addField("Nom *", nomEdit, "Lettres uniquement ex: Ben Ali");
    addField("Prenom *", prenomEdit, "Lettres uniquement ex: Mohamed");
    addField("Email *", emailEdit, "exemple@domaine.com");
    addField("Telephone *", telEdit, "8 chiffres ex: 99123456");
    addField("Adresse *", adresseEdit, "Ex: Tunis, Ariana");

    idEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("[0-9]*"), this));
    telEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("[0-9]{0,8}"), this));

    QLabel *lblDate = new QLabel("Date inscription *");
    lblDate->setStyleSheet(lblStyle);
    lblDate->setFixedHeight(18);
    ll->addWidget(lblDate);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    dateEdit->setStyleSheet(inputStyle);
    dateEdit->setFixedHeight(38);
    ll->addWidget(dateEdit);

    ll->addSpacing(12);

    QHBoxLayout *bl = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Enregistrer");
    btnSave->setFixedHeight(42);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(R"(
        QPushButton{background-color:#10b981;color:white;border:none;
            border-radius:8px;font-size:14px;font-weight:bold;}
        QPushButton:hover{background-color:#059669;}
    )");
    connect(btnSave, &QPushButton::clicked, this,
            &MainWindow::onAjouterClientSubmit);

    QPushButton *btnClear = new QPushButton("Effacer");
    btnClear->setFixedHeight(42);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet(R"(
        QPushButton{background-color:#6b7280;color:white;border:none;
            border-radius:8px;font-size:14px;font-weight:bold;}
        QPushButton:hover{background-color:#4b5563;}
    )");
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onAnnulerAjout);

    bl->addWidget(btnSave);
    bl->addWidget(btnClear);
    ll->addLayout(bl);
    ll->addStretch();

    // ---- TABLEAU DROIT ----
    QWidget *right = new QWidget();
    QVBoxLayout *rl = new QVBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setSpacing(15);

    QLabel *rTitle = new QLabel("Liste des Clients");
    rTitle->setStyleSheet("font-size:28px;font-weight:bold;"
                          "color:#1a202c;background:transparent;");
    rl->addWidget(rTitle);

    QHBoxLayout *tl = new QHBoxLayout();
    tl->setSpacing(10);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Rechercher par ID...");
    searchEdit->setFixedWidth(180);
    searchEdit->setStyleSheet(R"(
        QLineEdit{padding:8px 12px;border:2px solid #e2e8f0;
            border-radius:8px;font-size:13px;}
        QLineEdit:focus{border:2px solid #1e40af;}
    )");
    tl->addWidget(searchEdit);

    QPushButton *btnSearch = new QPushButton("Rechercher");
    btnSearch->setFixedHeight(35);
    btnSearch->setCursor(Qt::PointingHandCursor);
    btnSearch->setStyleSheet(R"(
        QPushButton{background-color:#3b82f6;color:white;border:none;
            border-radius:8px;padding:0 15px;font-size:13px;font-weight:bold;}
        QPushButton:hover{background-color:#2563eb;}
    )");
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchClient);
    tl->addWidget(btnSearch);
    tl->addSpacing(15);

    sortComboBox = new QComboBox();
    sortComboBox->addItems({"Trier: ID", "Trier: Date", "Trier: Nom"});
    sortComboBox->setFixedWidth(150);
    sortComboBox->setCursor(Qt::PointingHandCursor);
    sortComboBox->setStyleSheet(
        "QComboBox{padding:8px 12px;border:2px solid #e2e8f0;"
        "border-radius:8px;font-size:13px;}");
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSortClients);
    tl->addWidget(sortComboBox);
    tl->addStretch();

    QPushButton *btnPDF = new QPushButton("PDF");
    btnPDF->setFixedHeight(35);
    btnPDF->setCursor(Qt::PointingHandCursor);
    btnPDF->setStyleSheet(R"(
        QPushButton{background-color:#dc2626;color:white;border:none;
            border-radius:8px;padding:0 15px;font-size:13px;font-weight:bold;}
        QPushButton:hover{background-color:#b91c1c;}
    )");
    connect(btnPDF, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    tl->addWidget(btnPDF);

    QPushButton *btnExcel = new QPushButton("Excel");
    btnExcel->setFixedHeight(35);
    btnExcel->setCursor(Qt::PointingHandCursor);
    btnExcel->setStyleSheet(R"(
        QPushButton{background-color:#059669;color:white;border:none;
            border-radius:8px;padding:0 15px;font-size:13px;font-weight:bold;}
        QPushButton:hover{background-color:#047857;}
    )");
    connect(btnExcel, &QPushButton::clicked, this, &MainWindow::onExportExcel);
    tl->addWidget(btnExcel);
    rl->addLayout(tl);

    tableClients = new QTableWidget();
    tableClients->setColumnCount(8);
    tableClients->setHorizontalHeaderLabels({"ID", "Nom", "Prenom", "Email",
                                             "Telephone", "Adresse", "Date",
                                             "Actions"});
    tableClients->setStyleSheet(R"(
        QTableWidget{background-color:white;border:none;
            border-radius:15px;gridline-color:#f1f5f9;}
        QHeaderView::section{background-color:#1e40af;color:white;
            padding:10px;border:none;font-weight:bold;font-size:13px;}
        QTableWidget::item{padding:5px;border-bottom:1px solid #f1f5f9;font-size:13px;}
        QTableWidget::item:selected{background-color:#dbeafe;color:#1e40af;}
    )");
    tableClients->horizontalHeader()->setStretchLastSection(true);
    tableClients->verticalHeader()->setVisible(true);
    tableClients->verticalHeader()->setDefaultSectionSize(50);
    tableClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableClients->setSelectionMode(QAbstractItemView::SingleSelection);
    tableClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableClients->setAlternatingRowColors(true);

    tableClients->setColumnWidth(0, 60);
    tableClients->setColumnWidth(1, 120);
    tableClients->setColumnWidth(2, 120);
    tableClients->setColumnWidth(3, 200);
    tableClients->setColumnWidth(4, 120);
    tableClients->setColumnWidth(5, 180);
    tableClients->setColumnWidth(6, 110);
    tableClients->setColumnWidth(7, 240);

    rl->addWidget(tableClients);

    ml->addWidget(left);
    ml->addWidget(right);
}

// =============================================
// REFRESH TABLE
// =============================================
void MainWindow::refreshClientTable() {
    QString critere = "ID ASC";
    if (sortComboBox) {
        QString s = sortComboBox->currentText();
        if (s.contains("Date"))
            critere = "DATE_INSCRIPTION DESC";
        else if (s.contains("Nom"))
            critere = "NOM ASC";
        else
            critere = "ID ASC";
    }

    QSqlQueryModel *model = Ctmp.afficher(critere);
    tableClients->setRowCount(0);
    for (int i = 0; i < model->rowCount(); i++) {
        tableClients->insertRow(i);
        for (int col = 0; col < 7; col++) {
            QVariant val = model->record(i).value(col);
            QString text;
            if (col == 6 && (val.userType() == QMetaType::QDate ||
                             val.userType() == QMetaType::QDateTime))
                text = val.toDate().toString("dd/MM/yyyy");
            else
                text = val.toString();
            tableClients->setItem(i, col, new QTableWidgetItem(text));
        }
        // ✅ FIX : boutons avec connexions directes (pas de
        // WA_TransparentForMouseEvents)
        tableClients->setCellWidget(i, 7, createActionButtons(i));
    }
    delete model;
    updateStatistiques();
}

// =============================================
// ✅ BOUTONS ACTIONS — FIX PRINCIPAL
// =============================================
QWidget *MainWindow::createActionButtons(int row) {
    QWidget *w = new QWidget();
    // ✅ PAS de WA_TransparentForMouseEvents !
    w->setStyleSheet("background: transparent;");

    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(5, 3, 5, 3);
    l->setSpacing(10);

    QPushButton *btnEdit = new QPushButton("Modifier");
    btnEdit->setFixedSize(105, 36);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(R"(
        QPushButton {
            background-color: #f59e0b; color: white;
            border-radius: 6px; font-weight: bold; font-size: 13px;
            border: none;
        }
        QPushButton:hover { background-color: #d97706; }
        QPushButton:pressed { background-color: #b45309; }
    )");

    QPushButton *btnDel = new QPushButton("Supprimer");
    btnDel->setFixedSize(105, 36);
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setStyleSheet(R"(
        QPushButton {
            background-color: #ef4444; color: white;
            border-radius: 6px; font-weight: bold; font-size: 13px;
            border: none;
        }
        QPushButton:hover { background-color: #dc2626; }
        QPushButton:pressed { background-color: #b91c1c; }
    )");

    // ✅ Connexions directes avec capture du row
    connect(btnEdit, &QPushButton::clicked, this,
            [this, row]() { onEditClient(row); });
    connect(btnDel, &QPushButton::clicked, this,
            [this, row]() { onDeleteClient(row); });

    l->addWidget(btnEdit);
    l->addWidget(btnDel);
    l->setAlignment(Qt::AlignCenter);
    return w;
}

// =============================================
// AJOUTER / MODIFIER
// =============================================
void MainWindow::onAjouterClientSubmit() {
    QString erreur = Client::validerTout(
        idEdit->text().trimmed(), nomEdit->text().trimmed(),
        prenomEdit->text().trimmed(), emailEdit->text().trimmed(),
        telEdit->text().trimmed(), adresseEdit->text().trimmed());

    if (!erreur.isEmpty()) {
        showValidationError(erreur);
        resetFieldStyles();
        highlightInvalidField(erreur);
        return;
    }
    resetFieldStyles();

    int id = idEdit->text().trimmed().toInt();
    QString nom = nomEdit->text().trimmed();
    QString prenom = prenomEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString tel = telEdit->text().trimmed();
    QString adresse = adresseEdit->text().trimmed();
    QString date = dateEdit->date().toString("dd/MM/yyyy");
    QString nomComplet = nom + " " + prenom;
    bool ok = false;

    if (isEditing) {
        ok = Ctmp.modifier(editingId, nom, prenom, email, tel, adresse, date);
        if (ok) {
            QMessageBox::information(this, "Succes", "Client modifie !");
            EmailSender::getInstance()->sendEmailModification(email, nomComplet,
                                                              editingId);
        } else {
            QMessageBox::critical(this, "Erreur", "Modification echouee !");
        }
        isEditing = false;
        editingId = -1;
    } else {
        Client C(id, nom, prenom, email, tel, adresse, date);
        ok = C.ajouter();
        if (ok) {
            QMessageBox::information(this, "Succes", "Client ajoute !");
            EmailSender::getInstance()->sendEmailAjout(email, nomComplet, id);
        } else {
            QMessageBox::critical(this, "Erreur",
                                  "Ajout echoue ! (ID ou email deja existant ?)");
        }
    }
    if (ok) {
        onAnnulerAjout();
        refreshClientTable();
    }
}

void MainWindow::onAnnulerAjout() {
    idEdit->clear();
    nomEdit->clear();
    prenomEdit->clear();
    emailEdit->clear();
    telEdit->clear();
    adresseEdit->clear();
    dateEdit->setDate(QDate::currentDate());
    isEditing = false;
    editingId = -1;
    idEdit->setReadOnly(false);
    resetFieldStyles();
}

void MainWindow::onEditClient(int row) {
    if (row < 0 || row >= tableClients->rowCount())
        return;
    editingId = tableClients->item(row, 0)->text().toInt();
    isEditing = true;
    idEdit->setText(tableClients->item(row, 0)->text());
    nomEdit->setText(tableClients->item(row, 1)->text());
    prenomEdit->setText(tableClients->item(row, 2)->text());
    emailEdit->setText(tableClients->item(row, 3)->text());
    telEdit->setText(tableClients->item(row, 4)->text());
    adresseEdit->setText(tableClients->item(row, 5)->text());
    dateEdit->setDate(
        QDate::fromString(tableClients->item(row, 6)->text(), "dd/MM/yyyy"));
    idEdit->setReadOnly(true);
    resetFieldStyles();
}

void MainWindow::onDeleteClient(int row) {
    if (row < 0 || row >= tableClients->rowCount())
        return;
    int id = tableClients->item(row, 0)->text().toInt();
    QString nom = tableClients->item(row, 1)->text();
    QString prenom = tableClients->item(row, 2)->text();
    QString email = tableClients->item(row, 3)->text();
    QString nomComplet = nom + " " + prenom;

    if (QMessageBox::question(
            this, "Confirmation",
            QString("Supprimer le client %1 (ID: %2) ?").arg(nomComplet).arg(id),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (Ctmp.supprimer(id)) {
            QMessageBox::information(this, "Succes", "Client supprime !");
            refreshClientTable();
            EmailSender::getInstance()->sendEmailSuppression(email, nomComplet, id);
        } else {
            QMessageBox::critical(this, "Erreur", "Suppression echouee !");
        }
    }
}

void MainWindow::onSearchClient() {
    QString txt = searchEdit->text().trimmed();
    if (txt.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un ID.");
        return;
    }
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i, 0) && tableClients->item(i, 0)->text() == txt) {
            tableClients->selectRow(i);
            tableClients->scrollToItem(tableClients->item(i, 0));
            return;
        }
    }
    QMessageBox::information(this, "Recherche",
                             "Aucun client trouve avec l'ID: " + txt);
}

void MainWindow::onSortClients() { refreshClientTable(); }

void MainWindow::onExportPDF() {
    QString fn = QFileDialog::getSaveFileName(this, "Exporter en PDF", "",
                                              "PDF Files (*.pdf)");
    if (fn.isEmpty())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fn);

    QString html =
        "<h1 style='color:#1e40af'>"
        "Liste des Clients - Smart Menuiserie</h1>"
        "<table border='1' cellspacing='0' cellpadding='5' width='100%'>"
        "<tr style='background:#1e40af;color:white;'>"
        "<th>ID</th><th>Nom</th><th>Prenom</th><th>Email</th>"
        "<th>Tel</th><th>Adresse</th><th>Date</th></tr>";

    QSqlQuery query(
        "SELECT ID, NOM, PRENOM, EMAIL, TEL, ADRESSE, "
        "TO_CHAR(DATE_INSCRIPTION, 'DD/MM/YYYY') FROM CLIENTS ORDER BY ID");
    int r = 0;
    while (query.next()) {
        html += (r % 2 == 0) ? "<tr>" : "<tr style='background:#f3f4f6;'>";
        for (int c = 0; c < 7; ++c)
            html += "<td>" + query.value(c).toString() + "</td>";
        html += "</tr>";
        r++;
    }
    html += "</table>";
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
    QMessageBox::information(this, "Export PDF", "Export PDF reussi !");
}

void MainWindow::onExportExcel() {
    QString fn = QFileDialog::getSaveFileName(this, "Exporter en Excel", "",
                                              "CSV Files (*.csv)");
    if (fn.isEmpty())
        return;
    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible d'ouvrir le fichier !");
        return;
    }
    QTextStream out(&file);
    out << "ID,Nom,Prenom,Email,Telephone,Adresse,Date\n";
    QSqlQuery query(
        "SELECT ID, NOM, PRENOM, EMAIL, TEL, ADRESSE, "
        "TO_CHAR(DATE_INSCRIPTION, 'DD/MM/YYYY') FROM CLIENTS ORDER BY ID");
    while (query.next()) {
        for (int c = 0; c < 7; ++c) {
            out << query.value(c).toString();
            if (c < 6)
                out << ",";
        }
        out << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Excel", "Export CSV reussi !");
}

void MainWindow::showListeClients() { stackedWidget->setCurrentIndex(0); }

void MainWindow::showStatistiques() {
    updateStatistiques();
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onDeconnexion() {
    if (QMessageBox::question(
            this, "Deconnexion", "Voulez-vous vraiment vous deconnecter ?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        qApp->quit();
}

// =============================================
// PAGE STATISTIQUES
// =============================================
void MainWindow::createStatistiquesPage() {
    pageStatistiques = new QWidget();
    pageStatistiques->setStyleSheet("background-color:#f7fafc;");

    QVBoxLayout *mainVLayout = new QVBoxLayout(pageStatistiques);
    mainVLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea{border:none;background-color:transparent;}");
    mainVLayout->addWidget(scroll);

    QWidget *container = new QWidget();
    container->setStyleSheet("background-color:transparent;");
    scroll->setWidget(container);

    QVBoxLayout *l = new QVBoxLayout(container);
    l->setContentsMargins(40, 40, 40, 40);
    l->setSpacing(30);

    QHBoxLayout *tl = new QHBoxLayout();
    QLabel *ttl = new QLabel("Statistiques - Tableau de Bord");
    ttl->setStyleSheet("font-size:32px;font-weight:bold;"
                       "color:#1a202c;background:transparent;");
    tl->addWidget(ttl);
    tl->addStretch();

    QPushButton *btnG = new QPushButton("Clients par Date");
    btnG->setFixedSize(200, 45);
    btnG->setCursor(Qt::PointingHandCursor);
    btnG->setStyleSheet(R"(
        QPushButton{background-color:#8b5cf6;color:white;border:none;
            border-radius:8px;font-size:15px;font-weight:bold;}
        QPushButton:hover{background-color:#7c3aed;}
    )");
    connect(btnG, &QPushButton::clicked, this, &MainWindow::onShowGraphique);
    tl->addWidget(btnG);
    l->addLayout(tl);

    QHBoxLayout *cl = new QHBoxLayout();
    cl->setSpacing(20);

    auto makeCard = [](const QString &titre, QLabel *&val,
                       const QString &grad) -> QWidget * {
        QWidget *c = new QWidget();
        c->setStyleSheet(
            QString("QWidget{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,%1);"
                    "border-radius:15px;}")
                .arg(grad));
        c->setMinimumHeight(150);
        QVBoxLayout *vl = new QVBoxLayout(c);
        QLabel *t = new QLabel(titre);
        t->setStyleSheet(
            "font-size:18px;color:white;background:transparent;font-weight:bold;");
        val = new QLabel("0");
        val->setStyleSheet(
            "font-size:48px;color:white;background:transparent;font-weight:bold;");
        vl->addWidget(t);
        vl->addWidget(val);
        vl->addStretch();
        return c;
    };

    cl->addWidget(makeCard("Total Clients", statsClientTotal,
                           "stop:0 #667eea,stop:1 #764ba2"));
    cl->addWidget(makeCard("Nouveau ce Mois", statsClientsMois,
                           "stop:0 #f093fb,stop:1 #f5576c"));
    cl->addWidget(makeCard("Cette Semaine", statsClientsSemaine,
                           "stop:0 #4facfe,stop:1 #00f2fe"));
    l->addLayout(cl);

    QWidget *chartCanvas = new QWidget();
    chartCanvas->setStyleSheet(
        "QWidget{background-color:white;border-radius:15px;}");
    QVBoxLayout *cvl = new QVBoxLayout(chartCanvas);
    cvl->setContentsMargins(30, 30, 30, 30);

    QLabel *ct = new QLabel("Inscriptions par Mois");
    ct->setStyleSheet("font-size:24px;font-weight:bold;color:#1a202c;");
    cvl->addWidget(ct);

    chartContainer = new QWidget();
    chartContainer->setMinimumHeight(100);
    cvl->addWidget(chartContainer);
    l->addWidget(chartCanvas);
    l->addStretch();
}

void MainWindow::updateStatistiques() {
    if (!statsClientTotal || !statsClientsMois || !statsClientsSemaine ||
        !chartContainer)
        return;

    QSqlQuery qTotal("SELECT COUNT(*) FROM CLIENTS");
    if (qTotal.next())
        statsClientTotal->setText(qTotal.value(0).toString());

    QSqlQuery qMois(
        "SELECT COUNT(*) FROM CLIENTS WHERE "
        "EXTRACT(MONTH FROM DATE_INSCRIPTION)=EXTRACT(MONTH FROM SYSDATE) AND "
        "EXTRACT(YEAR FROM DATE_INSCRIPTION)=EXTRACT(YEAR FROM SYSDATE)");
    if (qMois.next())
        statsClientsMois->setText(qMois.value(0).toString());

    QSqlQuery qSem(
        "SELECT COUNT(*) FROM CLIENTS WHERE "
        "TO_CHAR(DATE_INSCRIPTION,'IW')=TO_CHAR(SYSDATE,'IW') AND "
        "EXTRACT(YEAR FROM DATE_INSCRIPTION)=EXTRACT(YEAR FROM SYSDATE)");
    if (qSem.next())
        statsClientsSemaine->setText(qSem.value(0).toString());

    QMap<QString, int> parMois;
    QSqlQuery qGraph(
        "SELECT TO_CHAR(DATE_INSCRIPTION,'MM'), COUNT(*) FROM CLIENTS "
        "GROUP BY TO_CHAR(DATE_INSCRIPTION,'MM') "
        "ORDER BY TO_CHAR(DATE_INSCRIPTION,'MM')");
    while (qGraph.next()) {
        int m = qGraph.value(0).toInt();
        int c = qGraph.value(1).toInt();
        parMois[QDate(2000, m, 1).toString("MMM")] = c;
    }

    QVBoxLayout *gl = qobject_cast<QVBoxLayout *>(chartContainer->layout());
    if (!gl) {
        gl = new QVBoxLayout(chartContainer);
        gl->setSpacing(10);
    } else {
        QLayoutItem *child;
        while ((child = gl->takeAt(0)) != nullptr) {
            if (child->widget())
                delete child->widget();
            delete child;
        }
    }

    int maxVal = 0;
    for (int v : parMois.values())
        maxVal = qMax(maxVal, v);

    if (parMois.isEmpty()) {
        QLabel *empty = new QLabel("Aucune donnee disponible.");
        empty->setStyleSheet("color:#9ca3af;font-style:italic;padding:20px;");
        gl->addWidget(empty);
    } else {
        for (auto it = parMois.begin(); it != parMois.end(); ++it) {
            QWidget *line = new QWidget();
            QHBoxLayout *hl = new QHBoxLayout(line);
            hl->setContentsMargins(0, 0, 0, 0);

            QLabel *lbl = new QLabel(it.key());
            lbl->setFixedWidth(80);
            lbl->setStyleSheet("font-size:13px;font-weight:bold;color:#4b5563;");
            hl->addWidget(lbl);

            QWidget *bar = new QWidget();
            int bw = (maxVal > 0) ? (it.value() * 500 / maxVal) : 0;
            bar->setFixedSize(qMax(bw, 10), 24);
            QString col = (it.value() >= 10)  ? "#059669"
                          : (it.value() >= 5) ? "#2563eb"
                                              : "#d97706";
            bar->setStyleSheet(
                QString("QWidget{background:%1;border-radius:12px;}").arg(col));
            hl->addWidget(bar);
            hl->addStretch();

            QLabel *val = new QLabel(QString::number(it.value()));
            val->setStyleSheet(
                "font-size:14px;font-weight:bold;color:#1e40af;min-width:30px;");
            hl->addWidget(val);
            gl->addWidget(line);
        }
    }
    gl->addStretch();
}

void MainWindow::onShowGraphique() {
    QMap<QString, int> parMois;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (!tableClients->item(i, 6))
            continue;
        QDate d = QDate::fromString(tableClients->item(i, 6)->text(), "dd/MM/yyyy");
        if (d.isValid()) {
            QLocale loc(QLocale::French);
            parMois[loc.monthName(d.month(), QLocale::LongFormat) + " " +
                    QString::number(d.year())]++;
        }
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Clients par Date d'Inscription");
    dlg->resize(900, 550);
    dlg->setStyleSheet("background-color:#f7fafc;");

    QVBoxLayout *dl = new QVBoxLayout(dlg);
    dl->setContentsMargins(30, 30, 30, 30);
    dl->setSpacing(20);

    QLabel *t = new QLabel("Nombre de Clients Inscrits par Mois");
    t->setStyleSheet("font-size:24px;font-weight:bold;color:#1a202c;");
    t->setAlignment(Qt::AlignCenter);
    dl->addWidget(t);

    QWidget *gw = new QWidget();
    gw->setStyleSheet("QWidget{background-color:white;border-radius:15px;}");
    gw->setMinimumHeight(350);
    QVBoxLayout *gl2 = new QVBoxLayout(gw);
    gl2->setContentsMargins(40, 30, 40, 30);
    gl2->setSpacing(12);

    int maxVal = 0;
    for (auto v : parMois.values())
        maxVal = qMax(maxVal, v);

    for (auto it = parMois.begin(); it != parMois.end(); ++it) {
        QWidget *bc = new QWidget();
        QHBoxLayout *bl2 = new QHBoxLayout(bc);
        bl2->setContentsMargins(0, 0, 0, 0);
        bl2->setSpacing(10);

        QLabel *ml = new QLabel(it.key());
        ml->setFixedWidth(150);
        ml->setStyleSheet("font-size:13px;font-weight:bold;color:#374151;");
        bl2->addWidget(ml);

        QWidget *bar = new QWidget();
        int bw = maxVal > 0 ? (it.value() * 400 / maxVal) : 0;
        bar->setFixedSize(qMax(bw, 10), 30);
        QString col = it.value() >= 5   ? "#10b981"
                      : it.value() >= 3 ? "#3b82f6"
                                        : "#f59e0b";
        bar->setStyleSheet(
            QString("QWidget{background:%1;border-radius:8px;}").arg(col));
        bl2->addWidget(bar);

        QLabel *cl2 = new QLabel(QString::number(it.value()) + " client(s)");
        cl2->setStyleSheet("font-size:14px;font-weight:bold;color:#1e40af;");
        bl2->addWidget(cl2);
        bl2->addStretch();
        gl2->addWidget(bc);
    }

    if (parMois.isEmpty()) {
        QLabel *empty = new QLabel("Aucune donnee disponible.");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("font-size:16px;color:#6b7280;");
        gl2->addWidget(empty);
    }
    gl2->addStretch();
    dl->addWidget(gw);

    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setFixedSize(120, 40);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(R"(
        QPushButton{background-color:#6b7280;color:white;border:none;
            border-radius:8px;font-size:14px;font-weight:bold;}
        QPushButton:hover{background-color:#4b5563;}
    )");
    connect(btnClose, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *bl3 = new QHBoxLayout();
    bl3->addStretch();
    bl3->addWidget(btnClose);
    dl->addLayout(bl3);
    dlg->exec();
}

// =============================================
// ARDUINO: LECTURE RFID
// =============================================
void MainWindow::readFromArduino() {
    QByteArray rawData = arduino->read_from_arduino();
    if (!rawData.isEmpty())
        qDebug() << "Donnees brutes recues de l'Arduino :" << rawData;
    serialData += rawData;

    while (serialData.contains('\n')) {
        int idx = serialData.indexOf('\n');
        QString ligne = QString::fromUtf8(serialData.left(idx)).trimmed();
        serialData.remove(0, idx + 1);

        if (ligne.isEmpty())
            continue;

        // Ignorer les lignes de log Arduino
        if (ligne.contains("Carte", Qt::CaseInsensitive) ||
            ligne.contains("Lecture", Qt::CaseInsensitive) ||
            ligne.contains("OK", Qt::CaseInsensitive))
            continue;

        QString uid = ligne;
        qDebug() << "UID RFID:" << uid;

        QSqlQuery query;
        // Test ultra-simplifie pour isoler le probleme
        query.prepare("SELECT NOM, PRENOM, TO_CHAR(DATE_INSCRIPTION, 'DD/MM/YYYY') FROM CLIENTS WHERE TRIM(UID_RFID) = :uid");
        query.bindValue(":uid", uid.trimmed());

        if (query.exec() && query.next()) {
            QString nom = query.value(0).toString();
            QString prenom = query.value(1).toString();
            QString dateInsc = query.value(2).toString();
            QString statutCmd = query.value(3).toString();
            
            if (statutCmd.isEmpty()) statutCmd = "Aucune commande";

            QString message = QString("<b>Client :</b> %1 %2<br>"
                                      "<b>Inscrit le :</b> %3<br>"
                                      "<b>Statut Commande :</b> <span style='color:#1e40af'>%4</span>")
                                  .arg(prenom, nom, dateInsc, statutCmd);

            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Scanner RFID");
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setText(message);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStyleSheet(R"(
                QMessageBox { background-color: white; }
                QMessageBox QLabel {
                    color: #1e40af; font-size: 16px; font-weight: bold;
                    min-width: 400px; padding: 20px;
                }
                QPushButton {
                    background-color: #10b981; color: white;
                    border-radius: 6px; padding: 10px 30px;
                    font-size: 14px; font-weight: bold;
                }
                QPushButton:hover { background-color: #059669; }
            )");
            msgBox.exec();
        } else {
            QMessageBox::warning(this, "RFID Inconnu",
                                 QString("Aucun client trouvé.\nUID reçu : [%1]\nLongueur : %2")
                                 .arg(uid.trimmed()).arg(uid.trimmed().length()));
        }
    }
}