#ifndef ARDUINO_H
#define ARDUINO_H

#include <QString>
#include <QByteArray>
#include <QObject>
#include <QTimer>
#include <windows.h>
#include <QDebug>

class Arduino : public QObject
{
    Q_OBJECT
public:
    Arduino();
    ~Arduino();
    int connect_arduino();
    int close_arduino();
    int write_to_arduino(QByteArray);
    QByteArray read_from_arduino();
    QString getarduino_port_name();

signals:
    void readyRead();

private slots:
    void pollSerial();

private:
    HANDLE hSerial;
    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray data;
    QTimer *timer;
};

#endif // ARDUINO_H
