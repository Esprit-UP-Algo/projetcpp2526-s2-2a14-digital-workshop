#include "twiliomanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

TwilioManager::TwilioManager()
{
    manager = new QNetworkAccessManager();
}

void TwilioManager::sendSMS(QString body)
{
    QNetworkRequest request;
    qDebug()<<"sms 3";
    setupRequest(&request);

    QUrlQuery params;
    params.addQueryItem("To", "+21655188036");
    params.addQueryItem("From", "+17406608636");
    params.addQueryItem("Body", body);
    QByteArray postData = params.query().toUtf8();
    qDebug()<<"sms 4";

    qDebug() << "Request URL:" << request.url().toString();
    qDebug() << "Request Headers:" << request.rawHeaderList();
    qDebug() << "Request Body:" << postData;


    QNetworkReply *reply = manager->post(request, postData);

    // Connect to the finished signal
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
            handleReply(reply);
        });
}

void TwilioManager::setupRequest(QNetworkRequest *request)
{
    request->setUrl(QUrl("https://api.twilio.com/2010-04-01/Accounts/AC89328c5678def1f050744275e1c51f95/Messages.json"));

    // Set up headers and authentication
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QString credentials = "AC89328c5678def1f050744275e1c51f95:a4e488aec98dd1c1ac82bdb07fca89c7"; // Replace with your SID and token
    QString base64Credentials = QString(credentials.toUtf8().toBase64());
    request->setRawHeader("Authorization", "Basic " + base64Credentials.toUtf8());
}

void TwilioManager::handleReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString response = QString::fromUtf8(reply->readAll());

        qDebug() << "Response Headers:" << reply->rawHeaderList();
        qDebug() << "Response Body:" << reply->readAll();

        qDebug() << "Success!";
    } else {
        qDebug() << "Error:" << reply->errorString();


        qDebug() << "Response Headers:" << reply->rawHeaderList();
        qDebug() << "Response Body:" << reply->readAll();
    }
    reply->deleteLater();
}
