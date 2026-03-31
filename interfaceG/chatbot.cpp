#include "chatbot.h"

// =============================================
// REMPLACEZ PAR VOTRE CLE GROQ
// 1. Allez sur console.groq.com
// 2. API Keys → Create API Key
// 3. Copiez la clé (gsk_...) ici
// =============================================
const QString Chatbot::GROQ_API_KEY =
    "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";

// =============================================
// PROMPT — Réponses ni trop courtes ni trop longues
// =============================================
const QString Chatbot::SYSTEM_PROMPT =
    "Tu es l'assistant virtuel officiel de Smart Menuiserie, "
    "entreprise tunisienne specialisee en menuiserie aluminium et PVC.\n\n"

    "STYLE DE REPONSE:\n"
    "- Toujours en francais\n"
    "- 3 a 5 phrases par reponse (ni trop court ni trop long)\n"
    "- Chaleureux, professionnel, clair\n"
    "- Utilise des tirets pour lister si necessaire\n"
    "- Jamais de prix exacts, oriente vers un devis gratuit\n\n"

    "INFORMATIONS ENTREPRISE:\n"
    "- Nom: Smart Menuiserie, Tunis, Tunisie\n"
    "- Horaires: Lundi-Vendredi 8h-18h | Samedi 9h-14h | Dimanche ferme\n"
    "- Tel: +216 99740020 \n"
    "- Email: contact@smartmenuiserie.tn\n\n"

    "PRODUITS ET SERVICES:\n"
    "- Portes interieures et exterieures (bois, aluminium, PVC)\n"
    "- Fenetres double vitrage (isolation thermique et acoustique)\n"
    "- Volets roulants electriques et manuels\n"
    "- Parquets et revetements de sol\n"
    "- Escaliers sur mesure\n"
    "- Cuisines et dressings sur mesure\n"
    "- Pergolas et abris de jardin\n"
    "- Installation, pose et service apres-vente\n"
    "- Garantie 2 ans sur tous les produits\n"
    "- Devis gratuit sous 48h\n\n"

    "REGLES STRICTES:\n"
    "- Reponds UNIQUEMENT sur Smart Menuiserie et la menuiserie\n"
    "- Si question hors sujet, redirige poliment vers les services\n"
    "- Pour les prix, dis toujours 'devis gratuit sous 48h'\n"
    "- Si tu ne sais pas, invite a appeler ou emailer";

// =============================================
// CONSTRUCTEUR
// =============================================
Chatbot::Chatbot(QObject *parent)
    : QObject(parent),
    manager(new QNetworkAccessManager(this))
{
    connect(manager, &QNetworkAccessManager::finished,
            this,    &Chatbot::onReplyFinished);
}

// =============================================
// ENVOYER LA QUESTION
// =============================================
void Chatbot::askQuestion(const QString &question)
{
    // Vérification clé non configurée
    if (GROQ_API_KEY == "VOTRE_CLE_GROQ_ICI" || GROQ_API_KEY.isEmpty()) {
        emit erreurReseau(
            "Cle API non configuree.\n"
            "Ouvrez chatbot.cpp et remplacez VOTRE_CLE_GROQ_ICI "
            "par votre cle sur console.groq.com");
        return;
    }

    QJsonObject userMsg;
    userMsg["role"]    = "user";
    userMsg["content"] = question;
    conversationHistory.append(userMsg);

    QNetworkRequest request(
        QUrl("https://api.groq.com/openai/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         ("Bearer " + GROQ_API_KEY).toUtf8());

    QJsonObject systemMsg;
    systemMsg["role"]    = "system";
    systemMsg["content"] = SYSTEM_PROMPT;

    QJsonArray messages;
    messages.append(systemMsg);
    for (const QJsonValue &v : conversationHistory)
        messages.append(v);

    QJsonObject body;
    body["model"] = "llama-3.1-8b-instant";
    body["max_tokens"]  = 250;   // ni trop court, ni trop long
    body["temperature"] = 0.6;
    body["messages"]    = messages;

    manager->post(request,
                  QJsonDocument(body).toJson(QJsonDocument::Compact));
}

// =============================================
// RÉPONSE DE L'API
// =============================================
void Chatbot::onReplyFinished(QNetworkReply *reply)
{
    int httpCode = reply->attribute(
                            QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        QString msg;
        if (httpCode == 400)
            msg = "Erreur 400 : Cle API invalide ou expiree.\n"
                  "Allez sur console.groq.com/keys et creez une nouvelle cle.";
        else if (httpCode == 401)
            msg = "Erreur 401 : Authentification echouee.\n"
                  "Verifiez votre cle API dans chatbot.cpp.";
        else if (httpCode == 429)
            msg = "Limite de requetes atteinte. Reessayez dans quelques secondes.";
        else if (reply->error() == QNetworkReply::HostNotFoundError)
            msg = "Pas de connexion internet. Verifiez votre reseau.";
        else
            msg = QString("Erreur de connexion (%1). Reessayez.").arg(httpCode);

        emit erreurReseau(msg);
        reply->deleteLater();
        return;
    }

    QByteArray    data = reply->readAll();
    QJsonDocument doc  = QJsonDocument::fromJson(data);
    QJsonObject   obj  = doc.object();
    QString       rep;

    if (obj.contains("choices")) {
        QJsonArray ch = obj["choices"].toArray();
        if (!ch.isEmpty())
            rep = ch[0].toObject()["message"]
                      .toObject()["content"].toString().trimmed();
    }

    if (rep.isEmpty()) {
        if (obj.contains("error"))
            rep = "Erreur API: " + obj["error"].toObject()["message"].toString();
        else
            rep = "Je n'ai pas pu generer une reponse. Reessayez.";
    }

    // Ajouter au contexte
    QJsonObject asst;
    asst["role"]    = "assistant";
    asst["content"] = rep;
    conversationHistory.append(asst);
    while (conversationHistory.size() > 16)
        conversationHistory.removeFirst();

    emit reponseRecue(rep);
    reply->deleteLater();
}
