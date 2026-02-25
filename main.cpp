#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Créer et afficher directement la fenêtre principale
    MainWindow *mainWindow = new MainWindow();
    mainWindow->show();
    mainWindow->showMaximized();  // Afficher en plein écran

    return app.exec();
}
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Style
    app.setStyle("Fusion");

    // Application
    MainWindow window;
    window.show();

    return app.exec();
}
