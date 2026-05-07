#ifndef ARDOUINO_H
#define ARDOUINO_H

#include <QObject>
#include <QString>
#include <QByteArray>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

class Ardouino : public QObject
{
    Q_OBJECT
public:
    explicit Ardouino(QObject *parent = nullptr);
    ~Ardouino();

    int connect_arduino();
    int close_arduino();
    int write_to_arduino(int value);
    QString read_from_arduino();

    QString getarduino_port_name() const;

private:
#ifdef Q_OS_WIN
    HANDLE hSerial;
#endif
    QString arduino_port_name;
    bool arduino_is_available;
};

#endif // ARDOUINO_H
