#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
#include <QDebug>
#include <QDateTime>

class EmailSender
{
private:
    QString serviceId;
    QString templateId;
    QString publicKey;

    static EmailSender* instance;
    EmailSender() {}

public:
    EmailSender(const EmailSender&)            = delete;
    EmailSender& operator=(const EmailSender&) = delete;

    static EmailSender* getInstance()
    {
        if (instance == nullptr)
            instance = new EmailSender();
        return instance;
    }

    void setEmailJSKeys(const QString &svcId,
                        const QString &tmplId,
                        const QString &pubKey)
    {
        serviceId  = svcId;
        templateId = tmplId;
        publicKey  = pubKey;
    }

    bool sendEmail(const QString &to,
                   const QString &toName,
                   const QString &subject,
                   const QString &message);

    bool sendEmailAjout      (const QString &to, const QString &nomComplet, int clientId);
    bool sendEmailModification(const QString &to, const QString &nomComplet, int clientId);
    bool sendEmailSuppression (const QString &to, const QString &nomComplet, int clientId);
};

#endif // EMAILSENDER_H
