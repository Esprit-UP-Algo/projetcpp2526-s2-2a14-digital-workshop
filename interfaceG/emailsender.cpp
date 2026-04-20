#include "emailsender.h"

EmailSender* EmailSender::instance = nullptr;

bool EmailSender::sendEmail(const QString &to,
                            const QString &toName,
                            const QString &subject,
                            const QString &message)
{
    QNetworkAccessManager manager;

    QNetworkRequest request(
        QUrl("https://api.emailjs.com/api/v1.0/email/send"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("origin", "http://localhost");

    QJsonObject templateParams;
    templateParams["to_email"] = to;
    templateParams["to_name"]  = toName;
    templateParams["subject"]  = subject;
    templateParams["message"]  = message;
    templateParams["date"]     = QDateTime::currentDateTime()
                                 .toString("dd/MM/yyyy HH:mm");

    QJsonObject body;
    body["service_id"]      = serviceId;
    body["template_id"]     = templateId;
    body["user_id"]         = publicKey;
    body["template_params"] = templateParams;

    QByteArray jsonData = QJsonDocument(body).toJson(QJsonDocument::Compact);

    qDebug() << "[EMAIL] Envoi a:" << to;

    QNetworkReply *reply = manager.post(request, jsonData);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished,
                     &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(
                              QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString response = QString::fromUtf8(reply->readAll());

    qDebug() << "[EMAIL] Status:" << statusCode << "| Reponse:" << response;
    reply->deleteLater();

    return (statusCode == 200);
}

bool EmailSender::sendEmailAjout(const QString &to,
                                 const QString &nomComplet,
                                 int clientId)
{
    return sendEmail(to, nomComplet,
                     "Bienvenue chez Smart Menuiserie !",
                     QString("Bonjour %1,\n\n"
                             "Votre compte a ete cree avec succes.\n\n"
                             "ID Client : #%2\nDate : %3\n\n"
                             "Cordialement,\nSmart Menuiserie")
                         .arg(nomComplet).arg(clientId)
                         .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm")));
}

bool EmailSender::sendEmailModification(const QString &to,
                                        const QString &nomComplet,
                                        int clientId)
{
    return sendEmail(to, nomComplet,
                     "Smart Menuiserie - Profil mis a jour",
                     QString("Bonjour %1,\n\n"
                             "Votre profil (ID: #%2) a ete modifie le %3.\n\n"
                             "Si vous n'etes pas a l'origine de cette action,\n"
                             "contactez-nous immediatement.\n\n"
                             "Cordialement,\nSmart Menuiserie")
                         .arg(nomComplet).arg(clientId)
                         .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm")));
}

bool EmailSender::sendEmailSuppression(const QString &to,
                                       const QString &nomComplet,
                                       int clientId)
{
    return sendEmail(to, nomComplet,
                     "Smart Menuiserie - Compte supprime",
                     QString("Bonjour %1,\n\n"
                             "Votre compte (ID: #%2) a ete supprime le %3.\n\n"
                             "Merci de nous avoir fait confiance.\n\n"
                             "Cordialement,\nSmart Menuiserie")
                         .arg(nomComplet).arg(clientId)
                         .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm")));
}
