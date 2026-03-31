#include "chatwidget.h"

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent), chatbot(nullptr), isOpen(false)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setStyleSheet("background: transparent;");

    // =========================
    // 💬 BOUTON BULLE
    // =========================
    floatBtn = new QPushButton("💬", this);
    floatBtn->setFixedSize(60, 60);
    floatBtn->setStyleSheet(R"(
    QPushButton {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                    stop:0 #4f46e5, stop:1 #06b6d4);
        color: white;
        border-radius: 30px;
        font-size: 22px;
        border: none;
    }

    QPushButton:hover {
        background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
                    stop:0 #4338ca, stop:1 #0891b2);
    }

    QPushButton:pressed {
        background: #1e3a8a;
    }
)");
    connect(floatBtn, &QPushButton::clicked, this, &ChatWidget::toggleChat);

    // =========================
    // 📦 PANEL CHAT
    // =========================
    panel = new QWidget(this);
    panel->setFixedSize(400, 500);
    panel->setStyleSheet("background:white;border-radius:12px;");
    panel->hide();

    QVBoxLayout *layout = new QVBoxLayout(panel);

    display = new QTextEdit();
    display->setReadOnly(true);

    // ✅ TEST MESSAGE (évite écran blanc)
    display->append("🤖 Bonjour ! Je suis votre assistant.");

    input = new QLineEdit();
    input->setPlaceholderText("Écrire un message...");

    sendBtn = new QPushButton("Envoyer");

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(input);
    inputLayout->addWidget(sendBtn);

    layout->addWidget(display);
    layout->addLayout(inputLayout);

    connect(sendBtn, &QPushButton::clicked, this, &ChatWidget::onSend);
    connect(input, &QLineEdit::returnPressed, this, &ChatWidget::onSend);
}

// =========================
// POSITION
// =========================
void ChatWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    floatBtn->move(width() - 80, height() - 80);
    panel->move(width() - 420, height() - 600);
}

// =========================
// OUVERTURE / FERMETURE
// =========================
void ChatWidget::toggleChat()
{
    isOpen = !isOpen;
    panel->setVisible(isOpen);
}

// =========================
// ENVOI MESSAGE
// =========================
void ChatWidget::onSend()
{
    QString txt = input->text();
    if (txt.isEmpty()) return;

    display->append("👤 " + txt);
    input->clear();

    if (chatbot)
        chatbot->askQuestion(txt);
}

// =========================
// RÉPONSE BOT (FIX PRINCIPAL)
// =========================
void ChatWidget::onReponse(const QString &rep)
{
    display->append("🤖 " + rep);
}

// =========================
// CONNECT BOT
// =========================
void ChatWidget::connectChatbot(Chatbot *bot)
{
    chatbot = bot;

    connect(chatbot, &Chatbot::reponseRecue,
            this, &ChatWidget::onReponse);
}
