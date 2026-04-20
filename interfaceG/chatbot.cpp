#include "chatbot.h"

Chatbot::Chatbot(QObject *parent) : QObject(parent)
{
    initialiserReponses();
}

void Chatbot::initialiserReponses()
{
    reponses["bonjour"]    = "Bonjour ! Bienvenue chez Smart Menuiserie. Comment puis-je vous aider aujourd'hui ?";
    reponses["salut"]      = "Salut ! Je suis l'assistant de Smart Menuiserie. Que puis-je faire pour vous ?";
    reponses["bonsoir"]    = "Bonsoir ! Bienvenue chez Smart Menuiserie. Comment puis-je vous aider ?";

    reponses["porte"]      = "Nous proposons des portes intérieures et extérieures en bois, aluminium et PVC. "
                        "Toutes nos portes sont fabriquées sur mesure avec une garantie de 2 ans. "
                        "Contactez-nous au +216 99740020 pour un devis gratuit sous 48h.";

    reponses["fenetre"]    = "Nos fenêtres en double vitrage offrent une excellente isolation thermique et acoustique. "
                          "Disponibles en aluminium et PVC, elles sont posées par nos techniciens spécialisés. "
                          "Demandez votre devis gratuit au +216 99740020.";

    reponses["volet"]      = "Nous installons des volets roulants électriques et manuels de haute qualité. "
                        "Nos volets sont robustes, faciles à utiliser et disponibles en plusieurs coloris. "
                        "Pour un devis, appelez le +216 99740020.";

    reponses["cuisine"]    = "Smart Menuiserie conçoit des cuisines sur mesure adaptées à votre espace et budget. "
                          "Nos cuisinistes vous accompagnent de la conception jusqu'à la pose. "
                          "Contactez-nous pour un rendez-vous gratuit.";

    reponses["dressing"]   = "Nous réalisons des dressings et placards sur mesure selon vos besoins. "
                           "Choix de matériaux, couleurs et configurations personnalisables. "
                           "Appelez le +216 99740020 pour un devis gratuit.";

    reponses["parquet"]    = "Nous posons des parquets et revêtements de sol de qualité. "
                          "Large choix de bois massif, stratifié et vinyle. "
                          "Contactez-nous pour une estimation gratuite.";

    reponses["escalier"]   = "Nos escaliers sur mesure sont fabriqués en bois, métal ou mixte. "
                           "Chaque escalier est conçu selon vos dimensions et votre style. "
                           "Demandez un devis gratuit au +216 99740020.";

    reponses["pergola"]    = "Nous construisons des pergolas et abris de jardin sur mesure. "
                          "Profitez de votre extérieur en toute saison avec nos solutions robustes. "
                          "Appelez le +216 99740020 pour plus d'informations.";

    reponses["prix"]       = "Nous ne communiquons pas de prix fixes car chaque projet est unique. "
                       "Nous vous proposons un devis gratuit sous 48h adapté à votre projet. "
                       "Contactez-nous au +216 99740020 ou par email : contact@smartmenuiserie.tn.";

    reponses["tarif"]      = reponses["prix"];
    reponses["cout"]       = reponses["prix"];
    reponses["combien"]    = reponses["prix"];

    reponses["devis"]      = "Notre service de devis est entièrement gratuit et sans engagement. "
                        "Vous recevrez votre estimation personnalisée sous 48h. "
                        "Contactez-nous au +216 99740020 ou à contact@smartmenuiserie.tn.";

    reponses["horaire"]    = "Nos horaires d'ouverture :\n"
                          "- Lundi au Vendredi : 8h00 - 18h00\n"
                          "- Samedi : 9h00 - 14h00\n"
                          "- Dimanche : Fermé";

    reponses["contact"]    = "Vous pouvez nous contacter :\n"
                          "- Téléphone : +216 99740020\n"
                          "- Email : contact@smartmenuiserie.tn\n"
                          "- Horaires : Lun-Ven 8h-18h | Sam 9h-14h";

    reponses["telephone"]  = reponses["contact"];
    reponses["email"]      = reponses["contact"];
    reponses["adresse"]    = "Smart Menuiserie est basée à Tunis, Tunisie. "
                          "Contactez-nous au +216 99740020 pour connaître notre adresse exacte.";

    reponses["garantie"]   = "Tous nos produits bénéficient d'une garantie de 2 ans. "
                           "Notre service après-vente est disponible du lundi au vendredi. "
                           "En cas de problème, appelez le +216 99740020.";

    reponses["aluminium"]  = "Nous travaillons l'aluminium pour les portes, fenêtres, volets et façades. "
                            "L'aluminium est léger, résistant et ne rouille pas. "
                            "Demandez un devis gratuit au +216 99740020.";

    reponses["pvc"]        = "Nos produits en PVC offrent une excellente isolation et une grande durabilité. "
                      "Le PVC est facile d'entretien et disponible en plusieurs finitions. "
                      "Contactez-nous pour un devis personnalisé.";

    reponses["bois"]       = "Nous utilisons des bois de qualité supérieure pour nos menuiseries. "
                       "Chêne, pin, hêtre — chaque essence est choisie selon votre projet. "
                       "Appelez le +216 99740020 pour en savoir plus.";

    reponses["installation"] = "Nos techniciens qualifiés assurent la pose et l'installation de tous nos produits. "
                               "Nous intervenons à Tunis et ses environs. "
                               "Contactez-nous pour planifier une intervention.";

    reponses["merci"]      = "Merci de votre confiance ! Smart Menuiserie est à votre service. "
                        "N'hésitez pas si vous avez d'autres questions.";

    reponses["aurevoir"]   = "Au revoir ! N'hésitez pas à revenir. Smart Menuiserie reste à votre disposition.";
    reponses["au revoir"]  = reponses["aurevoir"];
}

QString Chatbot::trouverReponse(const QString &question)
{
    QString q = question.toLower().trimmed();

    for (auto it = reponses.begin(); it != reponses.end(); ++it) {
        if (q.contains(it.key()))
            return it.value();
    }

    return "Je suis uniquement l'assistant de Smart Menuiserie. "
           "Je peux vous renseigner sur nos produits (portes, fenêtres, volets, cuisines...), "
           "nos horaires, tarifs et contact. "
           "Puis-je vous aider avec l'un de ces sujets ?";
}

void Chatbot::askQuestion(const QString &question)
{
    QString reponse = trouverReponse(question);
    emit reponseRecue(reponse);
}
