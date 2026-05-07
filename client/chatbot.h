#ifndef CHATBOT_H
#define CHATBOT_H

#include <QObject>
#include <QString>
#include <QMap>

class Chatbot : public QObject
{
    Q_OBJECT
public:
    explicit Chatbot(QObject *parent = nullptr);
    void askQuestion(const QString &question);

signals:
    void reponseRecue(const QString &reponse);
    void erreurReseau(const QString &erreur);

private:
    QMap<QString, QString> reponses;
    void initialiserReponses();
    QString trouverReponse(const QString &question);
};

#endif // CHATBOT_H
