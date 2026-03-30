#include <QApplication>
#include "matriele.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("GestionMatériel");
    app.setApplicationName("Matriele");
    app.setApplicationVersion("2.0.0");
    app.setStyle("Fusion");

    Matriele window;
    window.show();

    return app.exec();
}
