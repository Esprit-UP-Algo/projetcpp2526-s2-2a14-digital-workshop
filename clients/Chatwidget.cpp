#include "chatwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTimer>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent), chatbot(nullptr)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("ChatWidget { background-color: #ffffff; border-radius: 16px; "
                  "border: 1px solid #e2e8f0; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === EN-TETE ===
    QWidget *header = new QWidget();
    header->setFixedHeight(55);
    header->setStyleSheet("background-color: #1e40af; border-radius: 16px 16px 0 0;");
    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(15, 0, 15, 0);

    QLabel *avatar = new QLabel("🤖");
    avatar->setStyleSheet("font-size: 22px; background: transparent;");
    hl->addWidget(avatar);

    QVBoxLayout *titleLayout = new QVBoxLayout();
    QLabel *title = new QLabel("Assistant Smart Menuiserie");
    title->setStyleSheet("color: white; font-weight: bold; font-size: 13px; background: transparent;");
    QLabel *status = new QLabel("● En ligne");
    status->setStyleSheet("color: #86efac; font-size: 11px; background: transparent;");
    titleLayout->addWidget(title);
    titleLayout->addWidget(status);
    titleLayout->setSpacing(2);
    hl->addLayout(titleLayout);
    hl->addStretch();
    mainLayout->addWidget(header);

    // === ZONE MESSAGES ===
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { background: #f8fafc; border: none; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 3px; }"
        );

    messagesContainer = new QWidget();
    messagesContainer->setStyleSheet("background: #f8fafc;");
    messagesLayout = new QVBoxLayout(messagesContainer);
    messagesLayout->setContentsMargins(10, 10, 10, 10);
    messagesLayout->setSpacing(8);
    messagesLayout->addStretch();

    scrollArea->setWidget(messagesContainer);
    mainLayout->addWidget(scrollArea);

    // === BARRE DE SAISIE ===
    QWidget *inputBar = new QWidget();
    inputBar->setFixedHeight(55);
    inputBar->setStyleSheet("background: white; border-top: 1px solid #e2e8f0; "
                            "border-radius: 0 0 16px 16px;");
    QHBoxLayout *il = new QHBoxLayout(inputBar);
    il->setContentsMargins(10, 8, 10, 8);
    il->setSpacing(8);

    inputEdit = new QLineEdit();
    inputEdit->setPlaceholderText("Posez votre question...");
    inputEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #e2e8f0; border-radius: 20px; "
        "padding: 8px 15px; font-size: 13px; background: #f8fafc; }"
        "QLineEdit:focus { border: 1px solid #1e40af; background: white; }"
        );
    connect(inputEdit, &QLineEdit::returnPressed, this, &ChatWidget::onSendMessage);
    il->addWidget(inputEdit);

    sendBtn = new QPushButton("➤");
    sendBtn->setFixedSize(36, 36);
    sendBtn->setCursor(Qt::PointingHandCursor);
    sendBtn->setStyleSheet(
        "QPushButton { background-color: #1e40af; color: white; border: none; "
        "border-radius: 18px; font-size: 16px; }"
        "QPushButton:hover { background-color: #1e3a8a; }"
        );
    connect(sendBtn, &QPushButton::clicked, this, &ChatWidget::onSendMessage);
    il->addWidget(sendBtn);

    mainLayout->addWidget(inputBar);

    // Message de bienvenue
    addMessage("Bonjour ! Comment puis-je vous aider ?", false);
}

void ChatWidget::connectChatbot(Chatbot *bot)
{
    chatbot = bot;
    if (chatbot)
        connect(chatbot, &Chatbot::reponseRecue, this, &ChatWidget::onBotReply);}

void ChatWidget::addMessage(const QString &text, bool isUser)
{
    // Conteneur de la ligne
    QWidget *row = new QWidget();
    row->setStyleSheet("background: transparent;");
    QHBoxLayout *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);

    // Bulle
    QLabel *bubble = new QLabel(text);
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(260);
    bubble->setMinimumWidth(160);
    bubble->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    if (isUser) {
        bubble->setStyleSheet(
            "QLabel { background-color: #1e40af; color: white; "
            "border-radius: 16px 16px 4px 16px; "
            "padding: 10px 14px; font-size: 13px; }"
            );
        rowLayout->addStretch();
        rowLayout->addWidget(bubble);
    } else {
        bubble->setStyleSheet(
            "QLabel { background-color: #e2e8f0; color: #1a202c; "
            "border-radius: 16px 16px 16px 4px; "
            "padding: 10px 14px; font-size: 13px; }"
            );
        rowLayout->addWidget(bubble);
        rowLayout->addStretch();
    }

    // Insérer avant le stretch final
    messagesLayout->insertWidget(messagesLayout->count() - 1, row);

    // Scroll vers le bas
    QTimer::singleShot(50, this, [this]() {
        scrollArea->verticalScrollBar()->setValue(
            scrollArea->verticalScrollBar()->maximum());
    });
}

void ChatWidget::onSendMessage()
{
    QString text = inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    addMessage(text, true);
    inputEdit->clear();

    if (chatbot)
        chatbot->askQuestion(text);
    else
        addMessage("Chatbot non disponible.", false);
}

void ChatWidget::onBotReply(const QString &reply)
{
    addMessage(reply, false);
}
