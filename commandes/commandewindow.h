#ifndef COMMANDEWINDOW_H
#define COMMANDEWINDOW_H
#include "commande.h"
#include <QMainWindow>

namespace Ui {
class CommandeWindow;
}

class CommandeWindow : public QMainWindow
{
    Q_OBJECT

public:

    void remplir_comboBox_id_client();
    void remplir_comboBox_id_utilisateur();


    explicit CommandeWindow(QWidget *parent = nullptr);
    ~CommandeWindow();

private slots:
    void on_bt_ajouter_commande_clicked();

    void on_tableView_commande_clicked(const QModelIndex &index);

    void on_bt_modifier_commande_clicked();

    void on_bt_effacer_commande_clicked();

    void on_lineEditSearch_textChanged(const QString &text);

    void on_btnSearch_clicked();

    void on_pdf_clicked();

    void on_stat_clicked();

    void on_btnRefresh_clicked();

    void on_Historique_clicked();

private:
    Ui::CommandeWindow *ui;
    commande co;
};

#endif // COMMANDEWINDOW_H
