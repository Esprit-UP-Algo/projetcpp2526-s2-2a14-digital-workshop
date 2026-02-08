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
