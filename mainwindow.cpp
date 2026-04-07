#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QHeaderView>

MainWindow::MainWindow(CommandeService *service, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_service(service)
{
    ui->setupUi(this);

    // Table setup
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->verticalHeader()->setVisible(false);

    ui->dateEditOrderDate->setDate(QDate::currentDate());

    // Sidebar nav (placeholder — extend for multi-page if needed)
    connect(ui->navCommandes,  &QPushButton::clicked, this, [this]{ refreshTable(); });

    // Top bar
    connect(ui->btnSearch,      &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(ui->btnClearSearch, &QPushButton::clicked, this, &MainWindow::onClearSearch);
    connect(ui->lineEditSearch, &QLineEdit::returnPressed, this, &MainWindow::onSearch);

    // CRUD buttons
    connect(ui->btnAdd,     &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(ui->btnUpdate,  &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(ui->btnDelete,  &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(ui->btnRefresh, &QPushButton::clicked, this, [this]{ refreshTable(); });

    // Row click → fill form
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::onTableClicked);

    refreshTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------------------

void MainWindow::refreshTable(const QString &filter)
{
    QSqlQueryModel *model = m_service->getAllCommandes(filter);
    if (!model) {
        QMessageBox::critical(this, "Erreur DB",
                              "Impossible de charger les données:\n" + m_service->lastError());
        statusBar()->showMessage("Erreur: " + m_service->lastError());
        return;
    }
    auto *old = ui->tableView->model();
    ui->tableView->setModel(model);
    delete old;
    ui->tableView->resizeColumnsToContents();
    statusBar()->showMessage(
        QString("%1 commande(s) chargée(s).").arg(model->rowCount()), 4000);
}

bool MainWindow::validateFields()
{
    if (ui->lineEditId->text().trimmed().isEmpty()         ||
        ui->lineEditClientName->text().trimmed().isEmpty() ||
        ui->lineEditProduct->text().trimmed().isEmpty()    ||
        ui->lineEditQuantity->text().trimmed().isEmpty()   ||
        ui->lineEditPrice->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Validation", "Tous les champs sont obligatoires.");
        return false;
    }
    bool idOk, qtyOk, priceOk;
    ui->lineEditId->text().toInt(&idOk);
    ui->lineEditQuantity->text().toInt(&qtyOk);
    ui->lineEditPrice->text().toDouble(&priceOk);

    if (!idOk)    { QMessageBox::warning(this, "Validation", "L'ID doit être un entier.");        return false; }
    if (!qtyOk)   { QMessageBox::warning(this, "Validation", "La quantité doit être un entier."); return false; }
    if (!priceOk) { QMessageBox::warning(this, "Validation", "Le prix doit être un nombre.");     return false; }
    return true;
}

void MainWindow::onAdd()
{
    if (!validateFields()) return;

    if (m_service->addCommande(
            ui->lineEditId->text().toInt(),
            ui->lineEditClientName->text().trimmed(),
            ui->lineEditProduct->text().trimmed(),
            ui->lineEditQuantity->text().toInt(),
            ui->lineEditPrice->text().toDouble(),
            ui->dateEditOrderDate->date()))
    {
        QMessageBox::information(this, "Succès", "Commande ajoutée.");
        ui->lineEditId->clear(); ui->lineEditClientName->clear();
        ui->lineEditProduct->clear(); ui->lineEditQuantity->clear();
        ui->lineEditPrice->clear();
        ui->dateEditOrderDate->setDate(QDate::currentDate());
        refreshTable();
    } else {
        QMessageBox::critical(this, "Erreur", m_service->lastError());
    }
}

void MainWindow::onUpdate()
{
    if (!validateFields()) return;

    if (m_service->updateCommande(
            ui->lineEditId->text().toInt(),
            ui->lineEditClientName->text().trimmed(),
            ui->lineEditProduct->text().trimmed(),
            ui->lineEditQuantity->text().toInt(),
            ui->lineEditPrice->text().toDouble(),
            ui->dateEditOrderDate->date()))
    {
        QMessageBox::information(this, "Succès", "Commande modifiée.");
        refreshTable();
    } else {
        QMessageBox::critical(this, "Erreur", m_service->lastError());
    }
}

void MainWindow::onDelete()
{
    if (ui->lineEditId->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Sélectionnez une commande à supprimer.");
        return;
    }
    bool ok;
    int id = ui->lineEditId->text().toInt(&ok);
    if (!ok) { QMessageBox::warning(this, "Validation", "ID invalide."); return; }

    if (QMessageBox::question(this, "Confirmer",
            QString("Supprimer la commande ID %1 ?").arg(id),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    if (m_service->deleteCommande(id)) {
        QMessageBox::information(this, "Succès", "Commande supprimée.");
        ui->lineEditId->clear(); ui->lineEditClientName->clear();
        ui->lineEditProduct->clear(); ui->lineEditQuantity->clear();
        ui->lineEditPrice->clear();
        refreshTable();
    } else {
        QMessageBox::critical(this, "Erreur", m_service->lastError());
    }
}

void MainWindow::onSearch()
{
    refreshTable(ui->lineEditSearch->text().trimmed());
}

void MainWindow::onClearSearch()
{
    ui->lineEditSearch->clear();
    refreshTable();
}

void MainWindow::onTableClicked(const QModelIndex &index)
{
    auto *model = qobject_cast<QSqlQueryModel *>(ui->tableView->model());
    if (!model) return;
    int row = index.row();
    ui->lineEditId->setText(model->data(model->index(row, 0)).toString());
    ui->lineEditClientName->setText(model->data(model->index(row, 1)).toString());
    ui->lineEditProduct->setText(model->data(model->index(row, 2)).toString());
    ui->lineEditQuantity->setText(model->data(model->index(row, 3)).toString());
    ui->lineEditPrice->setText(model->data(model->index(row, 4)).toString());
    ui->dateEditOrderDate->setDate(
        QDate::fromString(model->data(model->index(row, 5)).toString(), "yyyy-MM-dd"));
}
