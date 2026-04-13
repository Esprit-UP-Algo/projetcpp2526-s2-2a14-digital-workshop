#include "clientwindow.h"
#include "client.h"
#include "emailsender.h"
#include "chatbot.h"
#include "chatwidget.h"
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
#include <QSqlDatabase>
#include <QSqlError>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMap>
#include <QScrollBar>
#include <QResizeEvent>
#include <QtPrintSupport/QPrinter>

#define EMAILJS_SERVICE_ID  "service_df0eu2o"
#define EMAILJS_TEMPLATE_ID "template_1nhhpu2"
#define EMAILJS_PUBLIC_KEY  "EMzsNrZPqIEjQXH9z"

// =============================================
// CONSTRUCTEUR
// =============================================
ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent),
    statsClientTotal(nullptr),
    statsEnCours(nullptr),
    statsTerminees(nullptr),
    statsTable(nullptr),
    chatbot(new Chatbot(this)),
    chatWidget(nullptr),
    chatBtn(nullptr),
    editingId(-1),
    isEditing(false)

{
    currentUserId = 0;
    setWindowTitle("Smart Menuiserie - Gestion Clients");
    resize(1600, 900);
    setStyleSheet("QMainWindow { background-color: #f7fafc; }");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    stackedWidget = new QStackedWidget();
    createListeClientsPage();
    createStatistiquesPage();
    stackedWidget->addWidget(pageListeClients);
    stackedWidget->addWidget(pageStatistiques);
    stackedWidget->setCurrentIndex(0);
    mainLayout->addWidget(stackedWidget);

    refreshClientTable();

    EmailSender::getInstance()->setEmailJSKeys(
        EMAILJS_SERVICE_ID,
        EMAILJS_TEMPLATE_ID,
        EMAILJS_PUBLIC_KEY);

    // Chat widget — caché par défaut
    chatWidget = new ChatWidget(nullptr);
    chatWidget->connectChatbot(chatbot);
    chatWidget->setFixedSize(350, 450);
    chatWidget->hide();

    // Bouton bulle flottant
    chatBtn = new QPushButton("💬", this);
    chatBtn->setFixedSize(55, 55);
    chatBtn->setCursor(Qt::PointingHandCursor);
    chatBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1e40af;
            color: white;
            border: none;
            border-radius: 27px;
            font-size: 22px;
        }
        QPushButton:hover { background-color: #1e3a8a; }
    )");
    chatBtn->show();
    chatBtn->raise();

    connect(chatBtn, &QPushButton::clicked, this, [this]() {
        if (chatWidget->isVisible()) {
            chatWidget->hide();
        } else {
            QPoint globalPos = mapToGlobal(QPoint(
                width()  - chatWidget->width()  - 20,
                height() - chatWidget->height() - 80));
            chatWidget->move(globalPos);
            chatWidget->show();
            chatWidget->raise();
        }
    });
}

ClientWindow::~ClientWindow() {}

// =============================================
// RESIZE
// =============================================
void ClientWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (chatBtn) {
        chatBtn->move(width() - 75, height() - 75);
        chatBtn->raise();
    }
    if (chatWidget && chatWidget->isVisible()) {
        QPoint globalPos = mapToGlobal(QPoint(
            width()  - chatWidget->width()  - 20,
            height() - chatWidget->height() - 80));
        chatWidget->move(globalPos);
    }
}
// =============================================
// VALIDATION
// =============================================
void ClientWindow::showValidationError(const QString &erreur)
{
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

void ClientWindow::resetFieldStyles()
{
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

void ClientWindow::highlightInvalidField(const QString &erreur)
{
    QString err = R"(
        QLineEdit {
            padding: 12px 15px; border: 2px solid #dc2626;
            border-radius: 8px; font-size: 14px;
            background-color: #fef2f2; height: 42px;
        }
    )";
    if      (erreur.contains("ID"))                                { idEdit->setStyleSheet(err);      idEdit->setFocus(); }
    else if (erreur.contains("Nom") && !erreur.contains("Prenom")) { nomEdit->setStyleSheet(err);     nomEdit->setFocus(); }
    else if (erreur.contains("Prenom"))                            { prenomEdit->setStyleSheet(err);  prenomEdit->setFocus(); }
    else if (erreur.contains("Email"))                             { emailEdit->setStyleSheet(err);   emailEdit->setFocus(); }
    else if (erreur.contains("Telephone"))                         { telEdit->setStyleSheet(err);     telEdit->setFocus(); }
    else if (erreur.contains("Adresse"))                           { adresseEdit->setStyleSheet(err); adresseEdit->setFocus(); }
}

// =============================================
// PAGE LISTE CLIENTS
// =============================================
void ClientWindow::createListeClientsPage()
{
    pageListeClients = new QWidget();
    pageListeClients->setStyleSheet("background-color:#f5f6fa;");

    QVBoxLayout *mainPageLayout = new QVBoxLayout(pageListeClients);
    mainPageLayout->setContentsMargins(20, 20, 20, 20);
    mainPageLayout->setSpacing(15);

    // === EN-TÊTE ===
    QFrame *header = new QFrame;
    header->setStyleSheet(
        "QFrame { background-color: white; border-radius: 8px; padding: 12px; }");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    QLabel *pageTitle = new QLabel("🤝 GESTION DES CLIENTS");
    pageTitle->setStyleSheet("color: #2c3e50; font-size: 18px; font-weight: bold;");
    headerLayout->addWidget(pageTitle);
    headerLayout->addStretch();
    mainPageLayout->addWidget(header);

    // === CONTENU ===
    QHBoxLayout *ml = new QHBoxLayout();
    ml->setContentsMargins(0, 0, 0, 0);
    ml->setSpacing(20);
    mainPageLayout->addLayout(ml);

    // ---- FORMULAIRE GAUCHE ----
    QWidget *left = new QWidget();
    left->setFixedWidth(420);
    left->setStyleSheet("QWidget{background-color:white;border-radius:15px;}");

    QVBoxLayout *ll = new QVBoxLayout(left);
    ll->setContentsMargins(30, 30, 30, 30);
    ll->setSpacing(8);

    QLabel *fTitle = new QLabel("Ajouter / Modifier Client");
    fTitle->setStyleSheet("font-size:20px;font-weight:bold;color:#1e40af;background:transparent;");
    ll->addWidget(fTitle);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color:#e2e8f0;");
    sep->setFixedHeight(2);
    ll->addWidget(sep);
    ll->addSpacing(5);

    QString inputStyle = R"(
        QLineEdit, QDateEdit {
            padding:10px 15px; border:2px solid #e2e8f0;
            border-radius:8px; font-size:13px;
            background-color:#f9fafb; height:38px;
        }
        QLineEdit:focus, QDateEdit:focus {
            border:2px solid #1e40af; background-color:white;
        }
    )";
    QString lblStyle = "font-size:12px;font-weight:600;color:#374151;background:transparent;";

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

    addField("ID Client *",  idEdit,      "Ex: 123");
    addField("Nom *",        nomEdit,     "Lettres uniquement ex: Ben Ali");
    addField("Prenom *",     prenomEdit,  "Lettres uniquement ex: Mohamed");
    addField("Email *",      emailEdit,   "exemple@domaine.com");
    addField("Telephone *",  telEdit,     "8 chiffres ex: 99123456");
    addField("Adresse *",    adresseEdit, "Ex: Tunis, Ariana");

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

    QLabel *lblStatut = new QLabel("Statut *");
    lblStatut->setStyleSheet(lblStyle);
    lblStatut->setFixedHeight(18);
    ll->addWidget(lblStatut);

    QWidget *sw = new QWidget();
    sw->setFixedHeight(32);
    QHBoxLayout *sl2 = new QHBoxLayout(sw);
    sl2->setContentsMargins(0,0,0,0);
    sl2->setSpacing(10);

    statutGroup    = new QButtonGroup(this);
    radioEnCours   = new QRadioButton("En Cours");
    radioTermine   = new QRadioButton("Termine");
    radioEnAttente = new QRadioButton("Attente");
    radioEnCours->setChecked(true);

    QString rb = "QRadioButton{font-size:12px;color:#374151;background:transparent;padding:3px;}"
                 "QRadioButton::indicator{width:16px;height:16px;}"
                 "QRadioButton::indicator:unchecked{background-color:white;"
                 "border:2px solid #d1d5db;border-radius:8px;}";
    radioEnCours->setStyleSheet(rb +
                                "QRadioButton::indicator:checked{background-color:#f59e0b;border:2px solid #f59e0b;border-radius:8px;}");
    radioTermine->setStyleSheet(rb +
                                "QRadioButton::indicator:checked{background-color:#10b981;border:2px solid #10b981;border-radius:8px;}");
    radioEnAttente->setStyleSheet(rb +
                                  "QRadioButton::indicator:checked{background-color:#3b82f6;border:2px solid #3b82f6;border-radius:8px;}");

    statutGroup->addButton(radioEnCours,   0);
    statutGroup->addButton(radioTermine,   1);
    statutGroup->addButton(radioEnAttente, 2);
    sl2->addWidget(radioEnCours);
    sl2->addWidget(radioTermine);
    sl2->addWidget(radioEnAttente);
    sl2->addStretch();
    ll->addWidget(sw);
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
    connect(btnSave, &QPushButton::clicked, this, &ClientWindow::onAjouterClientSubmit);

    QPushButton *btnClear = new QPushButton("Effacer");
    btnClear->setFixedHeight(42);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet(R"(
        QPushButton{background-color:#6b7280;color:white;border:none;
            border-radius:8px;font-size:14px;font-weight:bold;}
        QPushButton:hover{background-color:#4b5563;}
    )");
    connect(btnClear, &QPushButton::clicked, this, &ClientWindow::onAnnulerAjout);

    bl->addWidget(btnSave);
    bl->addWidget(btnClear);
    ll->addLayout(bl);
    ll->addStretch();

    // ---- TABLEAU DROIT ----
    QWidget *right = new QWidget();
    QVBoxLayout *rl = new QVBoxLayout(right);
    rl->setContentsMargins(0,0,0,0);
    rl->setSpacing(15);

    QLabel *rTitle = new QLabel("Liste des Clients");
    rTitle->setStyleSheet("font-size:28px;font-weight:bold;color:#1a202c;background:transparent;");
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
    connect(btnSearch, &QPushButton::clicked, this, &ClientWindow::onSearchClient);
    tl->addWidget(btnSearch);
    tl->addSpacing(15);

    sortComboBox = new QComboBox();
    sortComboBox->addItems({"Trier: ID","Trier: Date","Trier: Statut"});
    sortComboBox->setFixedWidth(150);
    sortComboBox->setCursor(Qt::PointingHandCursor);
    sortComboBox->setStyleSheet(
        "QComboBox{padding:8px 12px;border:2px solid #e2e8f0;border-radius:8px;font-size:13px;}");
    connect(sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClientWindow::onSortClients);
    tl->addWidget(sortComboBox);
    tl->addStretch();

    QPushButton *btnPDF = new QPushButton("📄 PDF");
    btnPDF->setFixedHeight(35);
    btnPDF->setFixedWidth(90);
    btnPDF->setCursor(Qt::PointingHandCursor);
    btnPDF->setStyleSheet(R"(
        QPushButton{background-color:#dc2626;color:white;border:none;
            border-radius:8px;padding:0 15px;font-size:13px;font-weight:bold;}
        QPushButton:hover{background-color:#b91c1c;}
    )");
    connect(btnPDF, &QPushButton::clicked, this, &ClientWindow::onExportPDF);
    tl->addWidget(btnPDF);

    QPushButton *btnExcel = new QPushButton("📊 Excel");
    btnExcel->setFixedHeight(35);
    btnExcel->setFixedWidth(100);
    btnExcel->setCursor(Qt::PointingHandCursor);
    btnExcel->setStyleSheet(R"(
        QPushButton{background-color:#059669;color:white;border:none;
            border-radius:8px;padding:0 15px;font-size:13px;font-weight:bold;}
        QPushButton:hover{background-color:#047857;}
    )");
    connect(btnExcel, &QPushButton::clicked, this, &ClientWindow::onExportExcel);
    tl->addWidget(btnExcel);

    rl->addLayout(tl);

    tableClients = new QTableWidget();
    tableClients->setColumnCount(9);
    tableClients->setHorizontalHeaderLabels({
                                             "ID","Nom","Prenom","Email","Telephone","Adresse","Date","Statut","Actions"});
    tableClients->setStyleSheet(R"(
        QTableWidget{background-color:white;border:none;
            border-radius:15px;gridline-color:#e2e8f0;}
        QHeaderView::section{background-color:#1e40af;color:white;
            padding:10px;border:none;font-weight:bold;font-size:13px;}
        QTableWidget::item{padding:8px;border-bottom:1px solid #e2e8f0;}
        QTableWidget::item:selected{background-color:#dbeafe;color:#1e40af;}
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
    rl->addWidget(tableClients);

    ml->addWidget(left);
    ml->addWidget(right);
}

// =============================================
// REFRESH TABLE
// =============================================
void ClientWindow::refreshClientTable()
{
    QSqlQueryModel *model = Ctmp.afficher();
    tableClients->setRowCount(0);
    for (int i = 0; i < model->rowCount(); i++) {
        tableClients->insertRow(i);
        for (int col = 0; col < 8; col++)
            tableClients->setItem(i, col, new QTableWidgetItem(
                                              model->record(i).value(col).toString()));
        tableClients->setCellWidget(i, 8, createActionButtons(i));
    }
    delete model;
}

// =============================================
// BOUTONS ACTIONS
// =============================================
QWidget* ClientWindow::createActionButtons(int row)
{
    QWidget *w = new QWidget();
    QHBoxLayout *l = new QHBoxLayout(w);
    l->setContentsMargins(5,2,5,2);
    l->setSpacing(8);

    QPushButton *btnEdit = new QPushButton("Modifier");
    btnEdit->setMinimumSize(100, 35);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(R"(
        QPushButton{background-color:#f59e0b;color:white;border-radius:6px;
            padding:8px 15px;font-weight:bold;font-size:13px;}
        QPushButton:hover{background-color:#d97706;}
    )");

    QPushButton *btnDel = new QPushButton("Supprimer");
    btnDel->setMinimumSize(110, 35);
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setStyleSheet(R"(
        QPushButton{background-color:#ef4444;color:white;border-radius:6px;
            padding:8px 15px;font-weight:bold;font-size:13px;}
        QPushButton:hover{background-color:#dc2626;}
    )");

    connect(btnEdit, &QPushButton::clicked, this, [this, row](){ onEditClient(row); });
    connect(btnDel,  &QPushButton::clicked, this, [this, row](){ onDeleteClient(row); });

    l->addWidget(btnEdit);
    l->addWidget(btnDel);
    return w;
}

// =============================================
// AJOUTER / MODIFIER
// =============================================
void ClientWindow::onAjouterClientSubmit()
{
    QString erreur = Client::validerTout(
        idEdit->text().trimmed(),
        nomEdit->text().trimmed(),
        prenomEdit->text().trimmed(),
        emailEdit->text().trimmed(),
        telEdit->text().trimmed(),
        adresseEdit->text().trimmed());

    if (!erreur.isEmpty()) {
        showValidationError(erreur);
        resetFieldStyles();
        highlightInvalidField(erreur);
        return;
    }
    resetFieldStyles();

    QString statut = radioEnCours->isChecked()  ? "En Cours"  :
                         radioTermine->isChecked()   ? "Termine"   : "En Attente";
    int     id      = idEdit->text().trimmed().toInt();
    QString nom     = nomEdit->text().trimmed();
    QString prenom  = prenomEdit->text().trimmed();
    QString email   = emailEdit->text().trimmed();
    QString tel     = telEdit->text().trimmed();
    QString adresse = adresseEdit->text().trimmed();
    QString date    = dateEdit->date().toString("dd/MM/yyyy");
    QString nomComplet = nom + " " + prenom;
    bool ok = false;

    if (isEditing) {
        ok = Ctmp.modifier(editingId, nom, prenom, email, tel, adresse, date, statut);
        if (ok) {
            QMessageBox::information(this, "Succes", "Client modifie !");
            EmailSender::getInstance()->sendEmailModification(email, nomComplet, editingId);
        } else {
            QMessageBox::critical(this, "Erreur", "Modification echouee !");
        }
        isEditing = false; editingId = -1;
    } else {
        Client C(id, nom, prenom, email, tel, adresse, date, statut, currentUserId);
        ok = C.ajouter();
        if (ok) {
            QMessageBox::information(this, "Succes", "Client ajoute !");
            EmailSender::getInstance()->sendEmailAjout(email, nomComplet, id);
        } else {
            QString msg = C.lastError.isEmpty() ? "Verifiez les données saisies." : C.lastError;
            QMessageBox::critical(this, "Erreur", "Ajout échoué !\n\n" + msg);
        }
    }
    if (ok) { onAnnulerAjout(); refreshClientTable(); }
}

void ClientWindow::onAnnulerAjout()
{
    idEdit->clear(); nomEdit->clear(); prenomEdit->clear();
    emailEdit->clear(); telEdit->clear(); adresseEdit->clear();
    dateEdit->setDate(QDate::currentDate());
    radioEnCours->setChecked(true);
    isEditing = false; editingId = -1;
    idEdit->setReadOnly(false);
    resetFieldStyles();
}

void ClientWindow::onEditClient(int row)
{
    editingId = tableClients->item(row, 0)->text().toInt();
    isEditing = true;
    idEdit->setText(tableClients->item(row, 0)->text());
    nomEdit->setText(tableClients->item(row, 1)->text());
    prenomEdit->setText(tableClients->item(row, 2)->text());
    emailEdit->setText(tableClients->item(row, 3)->text());
    telEdit->setText(tableClients->item(row, 4)->text());
    adresseEdit->setText(tableClients->item(row, 5)->text());
    dateEdit->setDate(QDate::fromString(tableClients->item(row, 6)->text(), "dd/MM/yyyy"));
    QString statut = tableClients->item(row, 7)->text();
    if      (statut.contains("Cours"))  radioEnCours->setChecked(true);
    else if (statut.contains("mine"))   radioTermine->setChecked(true);
    else                                radioEnAttente->setChecked(true);
    idEdit->setReadOnly(true);
    resetFieldStyles();
}

void ClientWindow::onDeleteClient(int row)
{
    int     id         = tableClients->item(row, 0)->text().toInt();
    QString nom        = tableClients->item(row, 1)->text();
    QString prenom     = tableClients->item(row, 2)->text();
    QString email      = tableClients->item(row, 3)->text();
    QString nomComplet = nom + " " + prenom;

    if (QMessageBox::question(this, "Confirmation",
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

void ClientWindow::onSearchClient()
{
    QString txt = searchEdit->text().trimmed();
    if (txt.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un ID.");
        return;
    }
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (tableClients->item(i,0) && tableClients->item(i,0)->text() == txt) {
            tableClients->selectRow(i);
            tableClients->scrollToItem(tableClients->item(i,0));
            return;
        }
    }
    QMessageBox::information(this, "Recherche", "Aucun client trouve avec l'ID: " + txt);
}

void ClientWindow::onSortClients()
{
    QString s = sortComboBox->currentText();
    int col = s.contains("Date") ? 6 : s.contains("Statut") ? 7 : 0;
    tableClients->sortItems(col, Qt::AscendingOrder);
    for (int r = 0; r < tableClients->rowCount(); ++r)
        tableClients->setCellWidget(r, 8, createActionButtons(r));
}

// =============================================
// EXPORT PDF
// =============================================
void ClientWindow::onExportPDF()
{
    if (tableClients->rowCount() == 0) {
        QMessageBox::warning(this, "Export PDF", "Aucun client a exporter !");
        return;
    }
    QString fn = QFileDialog::getSaveFileName(
        this, "Exporter en PDF", "clients.pdf", "PDF Files (*.pdf)");
    if (fn.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fn);

    QString html =
        "<h1 style='color:#1e40af;font-family:Arial;'>"
        "Liste des Clients - Smart Menuiserie</h1>"
        "<p style='color:#6b7280;font-family:Arial;font-size:12px;'>"
        "Genere le " + QDate::currentDate().toString("dd/MM/yyyy") + "</p>"
                                                        "<table border='1' cellspacing='0' cellpadding='6' width='100%' "
                                                        "style='border-collapse:collapse;font-family:Arial;font-size:11px;'>"
                                                        "<tr style='background-color:#1e40af;color:white;'>"
                                                        "<th>ID</th><th>Nom</th><th>Prenom</th><th>Email</th>"
                                                        "<th>Telephone</th><th>Adresse</th><th>Date</th><th>Statut</th></tr>";

    for (int r = 0; r < tableClients->rowCount(); ++r) {
        QString bg = (r % 2 == 0) ? "#ffffff" : "#f3f4f6";
        html += QString("<tr style='background-color:%1;'>").arg(bg);
        for (int c = 0; c < 8; ++c) {
            QString val = tableClients->item(r, c) ? tableClients->item(r, c)->text() : "";
            html += "<td style='padding:6px;border:1px solid #e2e8f0;'>" + val + "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);

    QMessageBox::information(this, "Export PDF",
                             QString("PDF exporte avec succes !\n%1 clients exportes.\nFichier: %2")
                                 .arg(tableClients->rowCount()).arg(fn));
}

// =============================================
// EXPORT EXCEL (CSV)
// =============================================
void ClientWindow::onExportExcel()
{
    if (tableClients->rowCount() == 0) {
        QMessageBox::warning(this, "Export Excel", "Aucun client a exporter !");
        return;
    }
    QString fn = QFileDialog::getSaveFileName(
        this, "Exporter en Excel", "clients.csv", "CSV Files (*.csv)");
    if (fn.isEmpty()) return;

    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de creer le fichier !");
        return;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "ID,Nom,Prenom,Email,Telephone,Adresse,Date,Statut\n";
    for (int r = 0; r < tableClients->rowCount(); ++r) {
        for (int c = 0; c < 8; ++c) {
            QString val = tableClients->item(r, c) ? tableClients->item(r, c)->text() : "";
            if (val.contains(",") || val.contains("\""))
                val = "\"" + val.replace("\"", "\"\"") + "\"";
            out << val;
            if (c < 7) out << ",";
        }
        out << "\n";
    }
    file.close();
    QMessageBox::information(this, "Export Excel",
                             QString("Export reussi !\n%1 clients exportes.\nFichier: %2")
                                 .arg(tableClients->rowCount()).arg(fn));
}

void ClientWindow::showListeClients() { stackedWidget->setCurrentIndex(0); }

void ClientWindow::showStatistiques()
{
    updateStatistiques();
    stackedWidget->setCurrentIndex(1);
}

void ClientWindow::onDeconnexion()
{
    if (QMessageBox::question(this, "Deconnexion",
                              "Voulez-vous vraiment vous deconnecter ?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        qApp->quit();
}

// =============================================
// PAGE STATISTIQUES
// =============================================
void ClientWindow::createStatistiquesPage()
{
    pageStatistiques = new QWidget();
    pageStatistiques->setStyleSheet("background-color:#f7fafc;");

    QVBoxLayout *l = new QVBoxLayout(pageStatistiques);
    l->setContentsMargins(40,40,40,40);
    l->setSpacing(30);

    QHBoxLayout *tl = new QHBoxLayout();
    QLabel *ttl = new QLabel("Statistiques - Tableau de Bord");
    ttl->setStyleSheet("font-size:32px;font-weight:bold;color:#1a202c;background:transparent;");
    tl->addWidget(ttl);
    tl->addStretch();

    QPushButton *btnG = new QPushButton("Clients par Date");
    btnG->setFixedSize(200,45);
    btnG->setCursor(Qt::PointingHandCursor);
    btnG->setStyleSheet(R"(
        QPushButton{background-color:#8b5cf6;color:white;border:none;
            border-radius:8px;font-size:15px;font-weight:bold;}
        QPushButton:hover{background-color:#7c3aed;}
    )");
    connect(btnG, &QPushButton::clicked, this, &ClientWindow::onShowGraphique);
    tl->addWidget(btnG);
    l->addLayout(tl);

    QHBoxLayout *cl = new QHBoxLayout();
    cl->setSpacing(20);

    auto makeCard = [](const QString &titre, QLabel *&val, const QString &grad) -> QWidget* {
        QWidget *c = new QWidget();
        c->setStyleSheet(
            QString("QWidget{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,%1);"
                    "border-radius:15px;}").arg(grad));
        c->setMinimumHeight(150);
        QVBoxLayout *vl = new QVBoxLayout(c);
        QLabel *t = new QLabel(titre);
        t->setStyleSheet("font-size:18px;color:white;background:transparent;font-weight:bold;");
        val = new QLabel("0");
        val->setStyleSheet("font-size:48px;color:white;background:transparent;font-weight:bold;");
        vl->addWidget(t); vl->addWidget(val); vl->addStretch();
        return c;
    };

    cl->addWidget(makeCard("Total Clients", statsClientTotal, "stop:0 #667eea,stop:1 #764ba2"));
    cl->addWidget(makeCard("En Cours",      statsEnCours,     "stop:0 #f093fb,stop:1 #f5576c"));
    cl->addWidget(makeCard("Terminees",     statsTerminees,   "stop:0 #4facfe,stop:1 #00f2fe"));
    l->addLayout(cl);

    QWidget *gc = new QWidget();
    gc->setStyleSheet("QWidget{background-color:white;border-radius:15px;}");
    QVBoxLayout *gl = new QVBoxLayout(gc);
    gl->setContentsMargins(30,30,30,30);

    QLabel *gt = new QLabel("Repartition des Statuts");
    gt->setStyleSheet("font-size:24px;font-weight:bold;color:#1a202c;background:transparent;");
    gl->addWidget(gt);

    statsTable = new QTableWidget();
    statsTable->setColumnCount(3);
    statsTable->setHorizontalHeaderLabels({"Statut","Nombre","Pourcentage"});
    statsTable->setStyleSheet(R"(
        QTableWidget{background-color:transparent;border:none;gridline-color:#e2e8f0;}
        QHeaderView::section{background-color:#1e40af;color:white;
            padding:10px;border:none;font-weight:bold;}
        QTableWidget::item{padding:8px;}
    )");
    statsTable->horizontalHeader()->setStretchLastSection(true);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setMaximumHeight(300);
    gl->addWidget(statsTable);
    l->addWidget(gc);
    l->addStretch();
}

void ClientWindow::updateStatistiques()
{
    if (!statsClientTotal || !statsEnCours || !statsTerminees || !statsTable) return;

    int total = tableClients->rowCount();
    statsClientTotal->setText(QString::number(total));

    int enCours = 0, terminees = 0;
    QMap<QString,int> counts;
    for (int i = 0; i < total; ++i) {
        if (!tableClients->item(i,7)) continue;
        QString st = tableClients->item(i,7)->text();
        counts[st]++;
        if (st.toLower().contains("cours"))   enCours++;
        if (st.toLower().contains("termine")) terminees++;
    }
    statsEnCours->setText(QString::number(enCours));
    statsTerminees->setText(QString::number(terminees));

    statsTable->setRowCount(counts.size());
    int row = 0;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        statsTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        statsTable->setItem(row, 2, new QTableWidgetItem(
                                        QString::number(total>0?(it.value()*100.0/total):0,'f',1)+"%"));
        row++;
    }
}

void ClientWindow::onShowGraphique()
{
    QMap<QString,int> parMois;
    for (int i = 0; i < tableClients->rowCount(); ++i) {
        if (!tableClients->item(i,6)) continue;
        QDate d = QDate::fromString(tableClients->item(i,6)->text(), "dd/MM/yyyy");
        if (d.isValid()) {
            QLocale loc(QLocale::French);
            parMois[loc.monthName(d.month(),QLocale::LongFormat)
                    + " " + QString::number(d.year())]++;
        }
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Clients par Date d'Inscription");
    dlg->resize(1000, 600);
    dlg->setStyleSheet("background-color:#f7fafc;");

    QVBoxLayout *dl = new QVBoxLayout(dlg);
    dl->setContentsMargins(30,30,30,30);
    dl->setSpacing(20);

    QLabel *t = new QLabel("Nombre de Clients Inscrits par Mois");
    t->setStyleSheet("font-size:28px;font-weight:bold;color:#1a202c;background:transparent;");
    t->setAlignment(Qt::AlignCenter);
    dl->addWidget(t);

    QWidget *gw = new QWidget();
    gw->setStyleSheet("QWidget{background-color:white;border-radius:15px;}");
    gw->setMinimumHeight(400);
    QVBoxLayout *gl2 = new QVBoxLayout(gw);
    gl2->setContentsMargins(40,40,40,40);
    gl2->setSpacing(15);

    int maxVal = 0;
    for (auto v : parMois.values()) maxVal = qMax(maxVal, v);

    for (auto it = parMois.begin(); it != parMois.end(); ++it) {
        QWidget *bc = new QWidget();
        QHBoxLayout *bl2 = new QHBoxLayout(bc);
        bl2->setContentsMargins(0,0,0,0);
        bl2->setSpacing(10);

        QLabel *ml = new QLabel(it.key());
        ml->setFixedWidth(150);
        ml->setStyleSheet("font-size:13px;font-weight:bold;color:#374151;background:transparent;");
        bl2->addWidget(ml);

        QWidget *bar = new QWidget();
        int bw = maxVal > 0 ? (it.value()*500/maxVal) : 0;
        bar->setFixedSize(bw, 35);
        QString col = it.value()>=5 ? "#10b981" : it.value()>=3 ? "#3b82f6" : "#f59e0b";
        bar->setStyleSheet(QString("QWidget{background:%1;border-radius:8px;}").arg(col));
        bl2->addWidget(bar);

        QLabel *cl2 = new QLabel(QString::number(it.value())+" client(s)");
        cl2->setStyleSheet("font-size:15px;font-weight:bold;color:#1e40af;background:transparent;");
        bl2->addWidget(cl2);
        bl2->addStretch();
        gl2->addWidget(bc);
    }

    if (parMois.isEmpty()) {
        QLabel *empty = new QLabel("Aucune donnee disponible");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("font-size:18px;color:#6b7280;background:transparent;");
        gl2->addWidget(empty);
    }

    dl->addWidget(gw);
    dlg->exec();
}
