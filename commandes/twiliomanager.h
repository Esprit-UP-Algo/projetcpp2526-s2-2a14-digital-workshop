#ifndef TWILIOMANAGER_H
#define TWILIOMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>

class TwilioManager
{
public:
    TwilioManager();
    void sendSMS(QString body);

private:
    QNetworkAccessManager *manager;
    void setupRequest(QNetworkRequest *request);
    static void handleReply(QNetworkReply *reply);
};

#endif // TWILIOMANAGER_H
