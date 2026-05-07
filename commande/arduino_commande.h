#ifndef ARDUINO_COMMANDE_H
#define ARDUINO_COMMANDE_H
#include <QtSerialPort>
class arduino_commande
{
public:
    arduino_commande();
      ~arduino_commande();
      int connect_arduino();
    int close_arduino();
    int write_to_arduino(QByteArray);
    QByteArray read_from_arduino();
    QSerialPort* get_serial(){return serial;}
    QString getarduino_port_name(){return arduino_port_name;}

private:
    QSerialPort* serial;
    static const quint16 arduino_uno_vendor_id=9025;
    static const quint16 arduino_uno_producy_id=67;
    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray data;

};

#endif // ARDUINO_COMMANDE_H
