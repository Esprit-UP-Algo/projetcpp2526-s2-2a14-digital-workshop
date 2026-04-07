#include "emailjsservice.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

static const char *EMAILJS_ENDPOINT = "https://api.emailjs.com/api/v1.0/email/send";

EmailJSService::EmailJSService(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this,      &EmailJSService::onReplyFinished);
}

void EmailJSService::sendCommandeEmail(const QString &client,
                                       const QString &product,
                                       int quantity)
{
    // Build JSON payload
    QJsonObject templateParams;
    templateParams["client_name"] = client;
    templateParams["product"]     = product;
    templateParams["quantity"]    = QString::number(quantity);

    QJsonObject payload;
    payload["service_id"]       = serviceId;
    payload["template_id"]      = templateId;
    payload["user_id"]          = userId;
    payload["accessToken"]      = accessToken;
    payload["template_params"]  = templateParams;

    QNetworkRequest request{QUrl(EMAILJS_ENDPOINT)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

void EmailJSService::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "[EmailJS] Email sent successfully. Response:"
                 << reply->readAll();
    } else {
        qWarning() << "[EmailJS] Failed to send email. HTTP status:"
                   << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                   << "| Error:" << reply->errorString()
                   << "| Body:" << reply->readAll();
    }
    reply->deleteLater();
}
