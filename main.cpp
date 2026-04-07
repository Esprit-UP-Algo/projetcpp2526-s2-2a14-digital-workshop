#include <QApplication>
#include <QMessageBox>
#include "commandeservice.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ── Edit these to match your Oracle setup ──────────────────────────────
    const QString host       = "127.0.0.1";
    const int     port       = 1521;
    const QString service    = "XE";                    // or "XE"
    const QString user       = "system";
    const QString password   = "123456";             // ← put your password here
    const QString odbcDriver = "Oracle dans OraDB21Home1";  // exact name from odbcad32.exe
    // ───────────────────────────────────────────────────────────────────────

    CommandeService svc;

    // ── EmailJS credentials ────────────────────────────────────────────────
    svc.emailService()->serviceId   = "service_uitc17t";
    svc.emailService()->templateId  = "template_n23n1zo";
    svc.emailService()->userId      = "Laql6ul1U6hv9cTFO";
    svc.emailService()->accessToken = "XGsLyz2MnLNxhfe-xwrgq";
    // ───────────────────────────────────────────────────────────────────────

    if (!svc.connectToDatabase(host, port, service, user, password, odbcDriver)) {
        QMessageBox::critical(nullptr, "Connexion échouée", svc.lastError());
        return 1;
    }

    MainWindow w(&svc);
    w.show();
    return app.exec();
}
