-- =========================
-- TABLE FOURNISSEUR
-- =========================
CREATE TABLE fournisseur (
    id_fournisseur NUMBER PRIMARY KEY,
    nom VARCHAR2(100) NOT NULL,
    email VARCHAR2(100),
    telephone VARCHAR2(20),
    adresse VARCHAR2(200),
    ville VARCHAR2(50),
    statut VARCHAR2(50),
    date_creation DATE DEFAULT SYSDATE
);

CREATE SEQUENCE fournisseur_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_fournisseur
BEFORE INSERT ON fournisseur
FOR EACH ROW
BEGIN
    IF :NEW.id_fournisseur IS NULL THEN
        SELECT fournisseur_seq.NEXTVAL INTO :NEW.id_fournisseur FROM dual;
    END IF;
END;
/

-- =========================
-- TABLE CLIENT
-- =========================
CREATE TABLE client (
    id_client NUMBER PRIMARY KEY,
    nom VARCHAR2(100),
    prenom VARCHAR2(100),
    telephone VARCHAR2(20),
    email VARCHAR2(100),
    adresse VARCHAR2(200),
    date_inscription DATE DEFAULT SYSDATE,
    statut VARCHAR2(50)
);

CREATE SEQUENCE client_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_client
BEFORE INSERT ON client
FOR EACH ROW
BEGIN
    IF :NEW.id_client IS NULL THEN
        SELECT client_seq.NEXTVAL INTO :NEW.id_client FROM dual;
    END IF;
END;
/

-- =========================
-- TABLE UTILISATEUR
-- =========================
CREATE TABLE utilisateur (
    id_utilisateur NUMBER PRIMARY KEY,
    nom VARCHAR2(100),
    prenom VARCHAR2(100),
    email VARCHAR2(100),
    mot_de_passe VARCHAR2(100),
    role VARCHAR2(50),
    statut VARCHAR2(50),
    date_creation DATE DEFAULT SYSDATE
);

CREATE SEQUENCE utilisateur_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_utilisateur
BEFORE INSERT ON utilisateur
FOR EACH ROW
BEGIN
    IF :NEW.id_utilisateur IS NULL THEN
        SELECT utilisateur_seq.NEXTVAL INTO :NEW.id_utilisateur FROM dual;
    END IF;
END;
/

-- =========================
-- TABLE MATERIEL
-- =========================
CREATE TABLE materiel (
    id_materiel NUMBER PRIMARY KEY,
    nom VARCHAR2(100) NOT NULL,
    type VARCHAR2(50),
    quantite NUMBER,
    seuil_min NUMBER,
    date_ajout DATE DEFAULT SYSDATE,
    id_fournisseur NUMBER,
    statut VARCHAR2(50),

    CONSTRAINT fk_materiel_fournisseur
    FOREIGN KEY (id_fournisseur)
    REFERENCES fournisseur(id_fournisseur)
);

CREATE SEQUENCE materiel_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_materiel
BEFORE INSERT ON materiel
FOR EACH ROW
BEGIN
    IF :NEW.id_materiel IS NULL THEN
        SELECT materiel_seq.NEXTVAL INTO :NEW.id_materiel FROM dual;
    END IF;
END;
/

-- =========================
-- TABLE COMMANDE
-- =========================
CREATE TABLE commande (
    id_commande NUMBER PRIMARY KEY,
    id_client NUMBER,
    id_utilisateur NUMBER,
    date_commande DATE DEFAULT SYSDATE,
    statut VARCHAR2(50),

    CONSTRAINT fk_commande_client
    FOREIGN KEY (id_client) REFERENCES client(id_client),

    CONSTRAINT fk_commande_user
    FOREIGN KEY (id_utilisateur) REFERENCES utilisateur(id_utilisateur)
);

CREATE SEQUENCE commande_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_commande
BEFORE INSERT ON commande
FOR EACH ROW
BEGIN
    IF :NEW.id_commande IS NULL THEN
        SELECT commande_seq.NEXTVAL INTO :NEW.id_commande FROM dual;
    END IF;
END;
/

-- =========================
-- TABLE LIGNE_COMMANDE
-- =========================
CREATE TABLE ligne_commande (
    id_commande NUMBER,
    id_materiel NUMBER,
    quantite NUMBER,

    PRIMARY KEY (id_commande, id_materiel),

    CONSTRAINT fk_ligne_commande
    FOREIGN KEY (id_commande) REFERENCES commande(id_commande),

    CONSTRAINT fk_ligne_materiel
    FOREIGN KEY (id_materiel) REFERENCES materiel(id_materiel)
);

-- =========================
-- INSERT TEST
-- =========================
INSERT INTO fournisseur (nom, email) VALUES ('BoisCorp', 'bois@mail.com');

INSERT INTO client (nom, prenom) VALUES ('Ali', 'Ahmed');

INSERT INTO utilisateur (nom, prenom, role)
VALUES ('Admin', 'System', 'Admin');

INSERT INTO materiel (nom, type, quantite, seuil_min, id_fournisseur, statut)
VALUES ('Poutre', 'Bois', 50, 10, 1, 'Disponible');

INSERT INTO commande (id_client, id_utilisateur, statut)
VALUES (1, 1, 'En cours');

INSERT INTO ligne_commande (id_commande, id_materiel, quantite)
VALUES (1, 1, 5);

-- =========================
-- SELECT TEST
-- =========================
SELECT * FROM fournisseur;
SELECT * FROM materiel;
SELECT * FROM client;
SELECT * FROM utilisateur;
SELECT * FROM commande;
SELECT * FROM ligne_commande;