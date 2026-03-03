#ifndef USERWINDOW_H
#define USERWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QListWidget;

class UserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserWindow(QWidget *parent = nullptr);
    ~UserWindow() = default;

private slots:
    void changePage(int index);

private:
    QListWidget *navList;
    QStackedWidget *pagesWidget;
};

#endif // USERWINDOW_H
