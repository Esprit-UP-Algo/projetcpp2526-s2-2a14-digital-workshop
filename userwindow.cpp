#include "userwindow.h"
#include "utilisateur.h"
#include "materiel.h"
#include "client.h"
#include "commande.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QListWidget>
#include <QStackedWidget>

UserWindow::UserWindow(QWidget *parent)
    : QMainWindow(parent)
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

    QLabel *appTitle = new QLabel("🏢 GESTION");
    appTitle->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(appTitle);

    navList = new QListWidget;
    navList->addItem("👥 Utilisateurs");
    navList->addItem("🛠️ Matériel");
    navList->addItem("👨‍💼 Clients");
    navList->addItem("📦 Commandes");
    navList->setCurrentRow(0);

    connect(navList, &QListWidget::currentRowChanged, this, &UserWindow::changePage);

    sidebarLayout->addWidget(navList);
    sidebarLayout->addStretch();

    QLabel *footer = new QLabel("v1.0 - En développement");
    footer->setStyleSheet("color: #7f8c8d; font-size: 11px; padding: 10px;");
    footer->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(footer);

    // === CONTENU PRINCIPAL ===
    pagesWidget = new QStackedWidget;

    // Ajouter les pages extraites
    pagesWidget->addWidget(new Utilisateur(this));
    pagesWidget->addWidget(new Materiel(this));
    pagesWidget->addWidget(new ClientWidget(this));
    pagesWidget->addWidget(new Commande(this));

    // === ASSEMBLAGE ===
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(pagesWidget, 1);

    setWindowTitle("Système de Gestion - Version collaborative");
    resize(1200, 700);
}

void UserWindow::changePage(int index)
{
    pagesWidget->setCurrentIndex(index);
}
