-- ==============================
-- TABLE UTILISATEUR
-- ==============================

CREATE TABLE utilisateur (
    id_utilisateur NUMBER PRIMARY KEY,
    nom VARCHAR2(100) NOT NULL,
    prenom VARCHAR2(100) NOT NULL,
    email VARCHAR2(150) UNIQUE NOT NULL,
    mot_de_passe VARCHAR2(255) NOT NULL,
    role VARCHAR2(50),
    date_creation DATE
);

CREATE SEQUENCE seq_utilisateur START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_utilisateur
BEFORE INSERT ON utilisateur
FOR EACH ROW
BEGIN
    SELECT seq_utilisateur.NEXTVAL
    INTO :NEW.id_utilisateur
    FROM dual;
END;
/

-- ==============================
-- TABLE CLIENT
-- ==============================

CREATE TABLE client (
    id_client NUMBER PRIMARY KEY,
    nom VARCHAR2(100) NOT NULL,
    prenom VARCHAR2(100) NOT NULL,
    telephone VARCHAR2(20),
    email VARCHAR2(150),
    adresse VARCHAR2(255),
    date_inscription DATE
);

CREATE SEQUENCE seq_client START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_client
BEFORE INSERT ON client
FOR EACH ROW
BEGIN
    SELECT seq_client.NEXTVAL
    INTO :NEW.id_client
    FROM dual;
END;
/

-- ==============================
-- TABLE CATEGORIE
-- ==============================

CREATE TABLE categorie (
    id_categorie NUMBER PRIMARY KEY,
    nom_categorie VARCHAR2(100) NOT NULL,
    description VARCHAR2(255)
);

CREATE SEQUENCE seq_categorie START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_categorie
BEFORE INSERT ON categorie
FOR EACH ROW
BEGIN
    SELECT seq_categorie.NEXTVAL
    INTO :NEW.id_categorie
    FROM dual;
END;
/

-- ==============================
-- TABLE PRODUIT
-- ==============================

CREATE TABLE produit (
    id_produit NUMBER PRIMARY KEY,
    nom_produit VARCHAR2(100) NOT NULL,
    type VARCHAR2(50),
    prix NUMBER(10,2),
    stock NUMBER,
    date_ajout DATE,
    id_categorie NUMBER,
    CONSTRAINT fk_produit_categorie
        FOREIGN KEY (id_categorie)
        REFERENCES categorie(id_categorie)
);

CREATE SEQUENCE seq_produit START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_produit
BEFORE INSERT ON produit
FOR EACH ROW
BEGIN
    SELECT seq_produit.NEXTVAL
    INTO :NEW.id_produit
    FROM dual;
END;
/

-- ==============================
-- TABLE COMMANDE
-- ==============================

CREATE TABLE commande (
    id_commande NUMBER PRIMARY KEY,
    date_commande DATE,
    total NUMBER(10,2),
    id_client NUMBER,
    CONSTRAINT fk_commande_client
        FOREIGN KEY (id_client)
        REFERENCES client(id_client)
);

CREATE SEQUENCE seq_commande START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_commande
BEFORE INSERT ON commande
FOR EACH ROW
BEGIN
    SELECT seq_commande.NEXTVAL
    INTO :NEW.id_commande
    FROM dual;
END;
/

-- ==============================
-- TABLE LIGNE_COMMANDE
-- ==============================

CREATE TABLE ligne_commande (
    id_ligne NUMBER PRIMARY KEY,
    id_commande NUMBER,
    id_produit NUMBER,
    quantite NUMBER,
    prix_unitaire NUMBER(10,2),
    CONSTRAINT fk_ligne_commande
        FOREIGN KEY (id_commande)
        REFERENCES commande(id_commande),
    CONSTRAINT fk_ligne_produit
        FOREIGN KEY (id_produit)
        REFERENCES produit(id_produit)
);

CREATE SEQUENCE seq_ligne START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_ligne
BEFORE INSERT ON ligne_commande
FOR EACH ROW
BEGIN
    SELECT seq_ligne.NEXTVAL
    INTO :NEW.id_ligne
    FROM dual;
END;
/