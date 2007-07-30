-- Mailsrv

CREATE DATABASE MailSrv;
USE MailSrv;

CREATE TABLE Domains (
  DomainID    INTEGER UNIQUE AUTO_INCREMENT,
  Name        VARCHAR(255),
  Reject      INTEGER,
  PRIMARY KEY(DomainID)
);

INSERT INTO Domains (Name, Reject) VALUES ('cjdj.org', 0);
INSERT INTO Domains (Name, Reject) VALUES ('mysearchbar.com', 0);
INSERT INTO Domains (Name, Reject) VALUES ('hyper-active.com.au', 0);
INSERT INTO Domains (Name, Reject) VALUES ('hypersms.com.au', 0);
INSERT INTO Domains (Name, Reject) VALUES ('onestoprealty.com.au', 0);

CREATE TABLE Users (
  UserID      INTEGER UNIQUE AUTO_INCREMENT,
  DomainID    INTEGER,
  Account     VARCHAR(255) UNIQUE,
  Password    CHAR(32),
  PRIMARY KEY(UserID)
);

INSERT INTO Users (DomainID, Account, Password) VALUES (1, '1.postmaster', NULL);
INSERT INTO Users (DomainID, Account, Password) VALUES (2, '2.postmaster', NULL);
INSERT INTO Users (DomainID, Account, Password) VALUES (3, '3.postmaster', NULL);
INSERT INTO Users (DomainID, Account, Password) VALUES (4, '4.postmaster', NULL);
INSERT INTO Users (DomainID, Account, Password) VALUES (5, '5.postmaster', NULL);


CREATE TABLE Addresses (
  AddressID   INTEGER UNIQUE AUTO_INCREMENT,
  DomainID    INTEGER,
  Name        VARCHAR(255),
  UserID      INTEGER,
  PRIMARY KEY(AddressID)
);

CREATE TABLE Messages (
  MessageID   INTEGER UNIQUE AUTO_INCREMENT,
  UserID      INTEGER,
  Incoming    INTEGER,
  BodySize    INTEGER DEFAULT 0,
  PRIMARY KEY(MessageID)
);

ALTER Messages ADD COLUMN BodySize INTEGER DEFAULT 0;


CREATE TABLE Summaries (
  MessageID   INTEGER,
  Date        DATETIME,
  MsgFrom     VARCHAR(255),
  MsgTo       TEXT,
  MsgCC       TEXT,
  MsgSubject  VARCHAR(255),
  MsgDate     DATETIME
);


CREATE TABLE Bodies (
  MessageID   INTEGER,
  Line        INTEGER,
  Body        TEXT
);

CREATE TABLE Outgoing (
  MessageID   INTEGER UNIQUE AUTO_INCREMENT,
  SendTime    DATETIME,
  Status      INTEGER DEFAULT 0,
  MsgFrom     VARCHAR(255),
  MsgTo       TEXT,
  PRIMARY KEY(MessageID)
);

CREATE TABLE DomainCache (
  CacheID     INTEGER UNIQUE AUTO_INCREMENT,
  Domain      VARCHAR(255),
  Status      INTEGER DEFAULT 0,
  Entries     INTEGER DEFAULT 0,
  Created     DATETIME,
  PRIMARY KEY(CacheID)
);

CREATE TABLE DomainCacheEntries (
  CacheID     INTEGER,
  Server      VARCHAR(255),
  Priority    INTEGER
);

