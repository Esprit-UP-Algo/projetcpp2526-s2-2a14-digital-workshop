#include <QApplication>
#include "userwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Style
    app.setStyle("Fusion");

    // Application
    UserWindow window;
    window.show();

    return app.exec();
}
