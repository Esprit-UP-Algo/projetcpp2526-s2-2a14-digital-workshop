#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QDateEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QTextEdit>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QRegularExpressionValidator>

#include "client.h"
#include "emailsender.h"
#include "chatbot.h"
#include "chatwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void showListeClients();
    void showStatistiques();
    void onDeconnexion();
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
    void     createSidebar();
    void     createListeClientsPage();
    void     createStatistiquesPage();
    QWidget* createActionButtons(int row);
    void     refreshClientTable();
    void     updateStatistiques();
    void     showValidationError  (const QString &erreur);
    void     resetFieldStyles     ();
    void     highlightInvalidField(const QString &erreur);

    // Pages
    QStackedWidget *stackedWidget;
    QWidget        *pageListeClients;
    QWidget        *pageStatistiques;

    // Formulaire
    QLineEdit    *idEdit;
    QLineEdit    *nomEdit;
    QLineEdit    *prenomEdit;
    QLineEdit    *emailEdit;
    QLineEdit    *telEdit;
    QLineEdit    *adresseEdit;
    QDateEdit    *dateEdit;
    QButtonGroup *statutGroup;
    QRadioButton *radioEnCours;
    QRadioButton *radioTermine;
    QRadioButton *radioEnAttente;

    // Tableau
    QTableWidget *tableClients;
    QLineEdit    *searchEdit;
    QComboBox    *sortComboBox;

    // Statistiques
    QLabel       *statsClientTotal;
    QLabel       *statsEnCours;
    QLabel       *statsTerminees;
    QTableWidget *statsTable;

    // Sidebar
    QPushButton *btnListeClients;
    QPushButton *btnStatistiques;
    QPushButton *btnDeconnexion;

    // Chatbot
    Chatbot    *chatbot;
    ChatWidget *chatWidget;

    // Métier
    Client Ctmp;
    int    editingId;
    bool   isEditing;
};

#endif // MAINWINDOW_H
