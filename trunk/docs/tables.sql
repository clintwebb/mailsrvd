-- Mailsrv

CREATE TABLE Domains (
  DomainID    INTEGER PRIMARY KEY,
  Name        TEXT,
  Reject      INTEGER    
);


CREATE TABLE Users (
  UserID      INTEGER PRIMARY KEY,
  DomainID    INTEGER,
  Account     TEXT,
  Password    TEXT
);

CREATE TABLE Addresses (
  AddressID   INTEGER PRIMARY KEY,
  DomainID    INTEGER,
  Name        TEXT,
  UserID      INTEGER
);

CREATE TABLE Messages (
  MessageID   INTEGER PRIMARY KEY,
  UserID      INTEGER,
  Incoming    INTEGER
);


CREATE TABLE Summaries (
  MessageID   INTEGER,
  Date        DATETIME,
  MsgFrom     TEXT,
  MsgTo       TEXT,
  MsgCC       TEXT,
  MsgSubject  TEXT,
  MsgDate     DATETIME
);


CREATE TABLE Bodies (
  MessageID   INTEGER,
  Line        INTEGER,
  Body        TEXT
);

CREATE TABLE Outgoing (
  MessageID   INTEGER PRIMARY KEY,
  SendTime    DATETIME,
  Status      INTEGER DEFAULT 0,
  MsgFrom     TEXT,
  MsgTo       TEXT
);

CREATE TABLE DomainCache (
  CacheID     INTEGER PRIMARY KEY,
  Domain      TEXT,
  Status      INTEGER DEFAULT 0,
  Entries     INTEGER DEFAULT 0,
  Created     DATETIME
);

CREATE TABLE DomainCacheEntries (
  CacheID     INTEGER,
  Server      TEXT,
  Priority    INTEGER
);

