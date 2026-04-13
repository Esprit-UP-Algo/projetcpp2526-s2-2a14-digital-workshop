#ifndef CHATBOT_H
#define CHATBOT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrl>

class Chatbot : public QObject
{
    Q_OBJECT
public:
    explicit Chatbot(QObject *parent = nullptr);
    void askQuestion(const QString &question);

signals:
    void reponseRecue(const QString &reponse);
    void erreurReseau(const QString &erreur);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QJsonArray             conversationHistory;
    static const QString   GROQ_API_KEY;
    static const QString   SYSTEM_PROMPT;
};

#endif // CHATBOT_H
