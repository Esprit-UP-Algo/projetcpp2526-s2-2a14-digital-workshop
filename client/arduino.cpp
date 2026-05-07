#include "arduino.h"

Arduino::Arduino()
{
    data = "";
    arduino_port_name = "";
    arduino_is_available = false;
    hSerial = INVALID_HANDLE_VALUE;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(pollSerial()));
}

Arduino::~Arduino()
{
    close_arduino();
}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

int Arduino::connect_arduino()
{
    // On essaie COM4 en priorite
    QStringList portsToTry;
    portsToTry << "COM4";
    for(int i=1; i<=30; ++i) {
        QString p = QString("COM%1").arg(i);
        if(!portsToTry.contains(p)) portsToTry << p;
    }

    for (const QString &portName : portsToTry) {
        QString winPortName = "\\\\.\\" + portName;

        hSerial = CreateFileA(winPortName.toStdString().c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

        if (hSerial != INVALID_HANDLE_VALUE) {
            arduino_is_available = true;
            arduino_port_name = portName;
            qDebug() << "### SUCCES : Arduino connecte sur " << portName;
            break;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_ACCESS_DENIED) {
                qDebug() << "### ERREUR : Accès refusé sur" << portName << "(Port deja utilisé par un autre programme ?)";
            }
        }
    }

    if (!arduino_is_available) {
        qDebug() << "### ECHEC : Aucun port Arduino trouve.";
        return -1;
    }

    // Configuration du port (Baudrate 9600, 8N1)
    DCB dcbSerialParams;
    ZeroMemory(&dcbSerialParams, sizeof(dcbSerialParams));
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        qDebug() << "Erreur GetCommState";
        close_arduino();
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_9600; // Doit correspondre a Serial.begin(9600) cote Arduino
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        qDebug() << "Erreur SetCommState";
        close_arduino();
        return 1;
    }

    // Configuration des timeouts (lecture non bloquante)
    COMMTIMEOUTS timeouts;
    ZeroMemory(&timeouts, sizeof(timeouts));
    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    timer->start(100); // Verifier les donnees toutes les 100 ms
    return 0; // Connecte avec succes
}

int Arduino::close_arduino()
{
    timer->stop();
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        arduino_is_available = false;
        return 0;
    }
    return 1;
}

QByteArray Arduino::read_from_arduino()
{
    QByteArray res = data;
    data.clear();
    return res;
}

void Arduino::pollSerial()
{
    if (hSerial == INVALID_HANDLE_VALUE) return;

    char buffer[256];
    DWORD bytesRead;
    // On lit tout ce qui est disponible
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        if (bytesRead > 0) {
            data.append(buffer, bytesRead);
            emit readyRead(); // Emet le signal pour dire a MainWindow qu'il y a de la donnee
        }
    }
}

int Arduino::write_to_arduino(QByteArray d)
{
    if (hSerial == INVALID_HANDLE_VALUE) return -1;
    DWORD bytesWritten;
    if (WriteFile(hSerial, d.constData(), d.size(), &bytesWritten, NULL)) {
        return 0;
    }
    return 1;
}
