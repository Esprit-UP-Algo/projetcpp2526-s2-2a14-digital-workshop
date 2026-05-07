#include "ardouino.h"
#include <QDebug>

Ardouino::Ardouino(QObject *parent) : QObject(parent)
{
    arduino_port_name = "";
    arduino_is_available = false;
#ifdef Q_OS_WIN
    hSerial = INVALID_HANDLE_VALUE;
#endif
}

Ardouino::~Ardouino()
{
    close_arduino();
}

int Ardouino::connect_arduino()
{
#ifdef Q_OS_WIN
    if (hSerial != INVALID_HANDLE_VALUE) {
        close_arduino();
    }
    
    bool accessDenied = false;
    
    for (int i = 1; i <= 256; ++i) {
        QString name = QString("\\\\.\\COM%1").arg(i);
        HANDLE h = CreateFileA(name.toStdString().c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            hSerial = h;
            arduino_port_name = QString("COM%1").arg(i);
            arduino_is_available = true;
            break;
        } else {
            if (GetLastError() == ERROR_ACCESS_DENIED) {
                accessDenied = true;
                arduino_port_name = QString("COM%1").arg(i);
            }
        }
    }
    
    if (arduino_is_available) {
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (GetCommState(hSerial, &dcbSerialParams)) {
            dcbSerialParams.BaudRate = CBR_9600;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            SetCommState(hSerial, &dcbSerialParams);
        }
        
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = 50;
        timeouts.ReadTotalTimeoutConstant = 50;
        timeouts.ReadTotalTimeoutMultiplier = 10;
        SetCommTimeouts(hSerial, &timeouts);
        
        return 0; // Success
    } else if (accessDenied) {
        return 1; // Access Denied
    } else {
        return -1; // Not Found
    }
#else
    return -1;
#endif
}

int Ardouino::close_arduino()
{
#ifdef Q_OS_WIN
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        arduino_is_available = false;
        return 0;
    }
    return 1;
#else
    return 1;
#endif
}

int Ardouino::write_to_arduino(int value)
{
#ifdef Q_OS_WIN
    if (hSerial != INVALID_HANDLE_VALUE) {
        QString data = QString::number(value) + "\n";
        DWORD bytesWritten;
        if (WriteFile(hSerial, data.toStdString().c_str(), data.length(), &bytesWritten, NULL)) {
            return 0; // Success
        }
    }
    return 1; // Failure
#else
    return 1;
#endif
}

QString Ardouino::read_from_arduino()
{
#ifdef Q_OS_WIN
    if (hSerial != INVALID_HANDLE_VALUE) {
        char buffer[256];
        DWORD bytesRead = 0;
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                return QString::fromStdString(buffer).trimmed();
            }
        }
    }
#endif
    return "";
}

QString Ardouino::getarduino_port_name() const
{
    return arduino_port_name;
}
