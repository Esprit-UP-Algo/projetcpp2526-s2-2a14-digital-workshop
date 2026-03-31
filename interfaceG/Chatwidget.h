#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "chatbot.h"

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    void connectChatbot(Chatbot *bot);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void toggleChat();
    void onSend();
    void onReponse(const QString &rep); // ✅ IMPORTANT

private:
    QPushButton *floatBtn;
    QWidget *panel;
    QTextEdit *display;
    QLineEdit *input;
    QPushButton *sendBtn;

    Chatbot *chatbot;
    bool isOpen;
};

#endif
