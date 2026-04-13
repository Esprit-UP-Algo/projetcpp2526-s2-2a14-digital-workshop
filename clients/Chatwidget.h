#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "chatbot.h"

class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);
    void connectChatbot(Chatbot *bot);

private slots:
    void onSendMessage();
    void onBotReply(const QString &reply);

private:
    void addMessage(const QString &text, bool isUser);

    Chatbot       *chatbot;
    QScrollArea   *scrollArea;
    QWidget       *messagesContainer;
    QVBoxLayout   *messagesLayout;
    QLineEdit     *inputEdit;
    QPushButton   *sendBtn;
};

#endif // CHATWIDGET_H
