#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QMessageBox>
#include "loginwindow.h"
#include "userwindow.h"
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle("Fusion");

    // Enhanced global stylesheet with larger elements
    app.setStyleSheet(
        "QMessageBox {"
        "    background-color: white;"
        "    min-width: 400px;"        /* Increased minimum width */
        "}"
        "QMessageBox QLabel {"
        "    color: #2c3e50;"
        "    font-size: 14px;"          /* Increased font size */
        "    padding: 15px;"             /* Added padding */
        "}"
        "QMessageBox QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 25px;"        /* Increased padding */
        "    font-size: 14px;"            /* Increased font size */
        "    min-width: 100px;"           /* Increased minimum width */
        "    min-height: 35px;"           /* Added minimum height */
        "}"
        "QMessageBox QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        );

    Connection* conn = Connection::getInstance();

    bool dbConnected = conn->establishConnection();

    if (!dbConnected) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Mode hors ligne");
        msgBox.setText("Impossible de se connecter à la base de données.");
        msgBox.setInformativeText("Voulez-vous continuer en mode démo (données locales) ?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Question);

        if (msgBox.exec() == QMessageBox::No) {
            Connection::deleteInstance();
            return -1;
        }
    }

    LoginWindow loginWindow;
    UserWindow mainWindow;

    // Center the login window on screen
    if (QScreen *screen = QApplication::primaryScreen()) {
        QRect screenGeometry = screen->availableGeometry();
        loginWindow.setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                loginWindow.size(),
                screenGeometry
                )
            );
    }

    QObject::connect(&loginWindow, &LoginWindow::loginSuccess, [&]() {
        mainWindow.show();
    });

    loginWindow.show();

    int result = app.exec();

    Connection::deleteInstance();

    return result;
}
