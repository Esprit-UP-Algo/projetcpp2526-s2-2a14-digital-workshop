#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include "commandeservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(CommandeService *service, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAdd();
    void onUpdate();
    void onDelete();
    void onSearch();
    void onClearSearch();
    void onTableClicked(const QModelIndex &index);

private:
    void refreshTable(const QString &filter = QString());
    bool validateFields();

    Ui::MainWindow  *ui;
    CommandeService *m_service;
};

#endif // MAINWINDOW_H
