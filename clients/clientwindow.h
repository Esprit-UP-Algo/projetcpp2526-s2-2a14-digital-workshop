#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTableWidget>
#include <QDateEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QSqlQueryModel>
#include "client.h"
#include "chatbot.h"
#include "chatwidget.h"

class ClientWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

    void showListeClients();
    void showStatistiques();
    void onDeconnexion();
    void setCurrentUserId(int id) { currentUserId = id; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onAjouterClientSubmit();
    void onAnnulerAjout();
    void onEditClient(int row);
    void onDeleteClient(int row);
    void onSearchClient();
    void onSortClients();
    void onExportPDF();
    void onExportExcel();
    void onShowGraphique();

private:
    void createListeClientsPage();
    void createStatistiquesPage();
    void updateStatistiques();
    void refreshClientTable();
    QWidget* createActionButtons(int row);
    void showValidationError(const QString &erreur);
    void resetFieldStyles();
    void highlightInvalidField(const QString &erreur);
    int currentUserId;

    // Pages
    QStackedWidget *stackedWidget;
    QWidget        *pageListeClients;
    QWidget        *pageStatistiques;

    // Formulaire
    QLineEdit   *idEdit, *nomEdit, *prenomEdit;
    QLineEdit   *emailEdit, *telEdit, *adresseEdit;
    QDateEdit   *dateEdit;
    QButtonGroup *statutGroup;
    QRadioButton *radioEnCours, *radioTermine, *radioEnAttente;

    // Table
    QTableWidget *tableClients;
    QLineEdit    *searchEdit;
    QComboBox    *sortComboBox;

    // Statistiques
    QLabel       *statsClientTotal, *statsEnCours, *statsTerminees;
    QTableWidget *statsTable;

    // Chat
    Chatbot    *chatbot;
    ChatWidget *chatWidget;
    QPushButton *chatBtn;

    // Données
    Client Ctmp;
    int    editingId;
    bool   isEditing;
};

#endif // CLIENTWINDOW_H
