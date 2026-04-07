#ifndef EMAILJSSERVICE_H
#define EMAILJSSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

class EmailJSService : public QObject
{
    Q_OBJECT

public:
    // API credentials — set these before calling sendCommandeEmail()
    QString serviceId;
    QString templateId;
    QString userId;       // EmailJS public key
    QString accessToken;  // for non-browser API access

    explicit EmailJSService(QObject *parent = nullptr);

    void sendCommandeEmail(const QString &client,
                           const QString &product,
                           int quantity);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
};

#endif // EMAILJSSERVICE_H
