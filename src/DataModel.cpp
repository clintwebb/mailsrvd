//-----------------------------------------------------------------------------
// Mailsrvd
//  (c) Copyright Hyper-Active Systems, 2006.
//
// Hyper-Active Systems
// ABN: 98 400 498 123
// 66 Brown Cr
// Seville Grove, WA, 6112
// Australia
// Phone: +61 434 895 695
//        0434 895 695
// contact@hyper-active.com.au
// 
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "DataModel.h"
#include "Logger.h"

//-----------------------------------------------------------------------------
// CJW: Constructor.  
DataModel::DataModel() 
{	
	Lock();
	_pDB = NULL;
	Unlock();
}
		
//-----------------------------------------------------------------------------
// CJW: 
DataModel::~DataModel() 
{
	Lock();
	if (_pDB != NULL) {
		delete _pDB;
		_pDB = NULL;
	}
	Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Using the information in the ini file, we will connect to the database.
bool DataModel::Connect(DpIniFile *pIni)
{
	Logger log;
	char *szServer, *szUsername, *szPassword, *szDB;
	bool bConnected = false;

	ASSERT(pIni != NULL);
	
	Lock();
	ASSERT(_pDB == NULL);

	// We need to connect to the database, so we need to get the details 
	// from the configuration so that we know what server to connect to, 
	// and the username and password.
	if (pIni->SetGroup("mailsrvd") == false) {
		log.Log("'mailsrvd' section in config file is missing.\n");
	}
	else {

		if (pIni->GetValue("db_server", &szServer) == true) {
			if (pIni->GetValue("db_user", &szUsername) == true) {
				if (pIni->GetValue("db_pass", &szPassword) == true) {
					if (pIni->GetValue("db_db", &szDB) == true) {
				
						log.Log("Connecting to database: %s@%s", szUsername, szServer);
						_pDB = new DpMySqlDB;
						ASSERT(_pDB != NULL);
						if (_pDB->Connect(szServer, szUsername, szPassword, szDB) == true) {
						
							log.Log("Database connected.");
							bConnected = true;
						}
						else {
							log.Log("Unable to connect to database");
							delete _pDB;
							_pDB = NULL;
						}
						
						free(szDB);
					}
					free(szPassword);
				}
				free(szUsername);	

			}
			free(szServer);
		}
	
		log.Log("Finished Launching Process.");
	}
	
	Unlock();
	
	return(bConnected);
}




//-----------------------------------------------------------------------------
// CJW: Given a username and password, this function will return the userID.
int DataModel::GetUserID(char *szUser, char *szPass)
{
	DpMySqlDB *pResult;
	char *vUser, *vPass;
	int nUserID = 0;
	
	ASSERT(szUser != NULL && szPass != NULL);
	
	Lock();
	ASSERT(_pDB != NULL);
	vUser = _pDB->Quote(szUser);
	vPass = _pDB->Quote(szPass);
	ASSERT(vUser != NULL && vPass != NULL);
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("SELECT UserID FROM Users WHERE Account='%s' AND Password='%s' LIMIT 1", vUser, vPass) == true) {
		if(pResult->NextRow()) {
			 pResult->GetData("UserID", &nUserID);
		}
	}
	delete pResult;
	Unlock();
	free(vUser);
	free(vPass);
	return (nUserID);
}


//-----------------------------------------------------------------------------
// CJW: Given the domain name, we return the domainID.  We will store the information internally, 
int DataModel::GetDomainID(char *szDomain)
{
	DpMySqlDB *pResult;
	char *vDomain;
	int nDomainID=0;
	
	ASSERT(szDomain != NULL);
	
	Lock();
	ASSERT(_pDB != NULL);
	vDomain = _pDB->Quote(szDomain);
	ASSERT(vDomain != NULL);

	pResult = _pDB->Spawn();
	if (pResult->Execute("SELECT DomainID, Reject FROM Domains WHERE Name LIKE '%s'", vDomain) == true) {
		if(pResult->NextRow()) {
			pResult->GetData("DomainID", &nDomainID);
			_Cache.domain.nID = nDomainID;
			pResult->GetData("Reject", &_Cache.domain.nReject);
		}
	}
	delete pResult;
	Unlock();
	free(vDomain);
	return(nDomainID);
}


//-----------------------------------------------------------------------------
// CJW: We need to know if we should reject emails for usernames that arent 
// 		specifically created for this domain.  If we dont then the email would 
// 		go to the postmaster.  We should have this information in our cache.  
// 		If we dont, then we need to look it up in the database.
int DataModel::GetDomainReject(int nDomainID)
{
	DpMySqlDB *pResult;
	int nReject = 1;
	
	ASSERT(nDomainID > 0);
	Lock();
	if (nDomainID == _Cache.domain.nID) {
		nReject = _Cache.domain.nReject;
	}
	else {
		ASSERT(_pDB != NULL);
		pResult = _pDB->Spawn();
		if (pResult->Execute("SELECT Reject FROM Domains WHERE DomainID=%d", nDomainID) == true) {
			if(pResult->NextRow()) {
				pResult->GetData("Reject", &nReject);
			}
		}
		delete pResult;
	}
	Unlock();
	return(nReject);
}


//-----------------------------------------------------------------------------
// CJW: We have the domain ID, and the username, so we want to get the list of 
// users for this address (there may be more than one).  For this we use a 
// DataList.
DataList* DataModel::GetUserFromAddress(int nDomainID, char *szUser)
{
	DataList *pList = NULL;
	DpMySqlDB *pResult;
	char *vUser;
	int nUserID;
	
	ASSERT(nDomainID > 0 && szUser != NULL);
	ASSERT(strlen(szUser) < 255);

	Lock();
	ASSERT(_pDB != NULL);
	vUser = _pDB->Quote(szUser);

	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("SELECT UserID FROM Addresses WHERE DomainID=%d AND Name LIKE '%s'", nDomainID, vUser) == true) {
		pList = new DataList;
		pList->AddColumn("UserID");
		while(pResult->NextRow()) {
			pResult->GetData("UserID", &nUserID);
			pList->AddRow();
			pList->AddData(0, nUserID);
		}
		pList->Complete();
	}
	delete pResult;
	Unlock();
	free(vUser);

	return(pList);
}





//-----------------------------------------------------------------------------
// CJW: We have a message we need to insert. 
int DataModel::InsertMessage(int nUserID)
{
	DpMySqlDB *pResult;
	Logger log;
	int nMsgID=0;
	
	ASSERT(nUserID > 0);
	
	Lock();
	ASSERT(_pDB != NULL);
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	log.Log("INSERT INTO Messages (UserID, Incoming) VALUES (%d, 1)", nUserID);
	pResult->Execute("INSERT INTO Messages (UserID, Incoming) VALUES (%d, 1)", nUserID);
	nMsgID = pResult->GetInsertID();
	ASSERT(nMsgID > 0);
	delete pResult;
	Unlock();
	
	return(nMsgID);
}


//-----------------------------------------------------------------------------
// CJW: We have created the message record, and so now we insert each line for 
// 		the message.
void DataModel::InsertBodyLine(int nMsgID, int lineno, char *szLine)
{
	DpMySqlDB *pResult;
	char szLineCopy[1025];
	char *vLine;
	ASSERT(nMsgID > 0 && lineno > 0 && szLine != NULL);
	
	strncpy(szLineCopy, szLine, 1024);
	
	Lock();
	ASSERT(_pDB != NULL);
	vLine = _pDB->Quote(szLineCopy);
	ASSERT(vLine != NULL);
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	pResult->Execute("INSERT INTO Bodies (MessageID, Line, Body) VALUES (%d, %d, '%s')", nMsgID, lineno, vLine);
	delete pResult;
	Unlock();
	free(vLine);
}




void DataModel::InsertOutgoing(int nMsgID, char *szFrom, char *szRemote)
{
	DpMySqlDB *pResult;
	char *vFrom, *vRemote;
	
	ASSERT(nMsgID > 0 && szFrom != NULL && szRemote != NULL);
	
	Lock();
	ASSERT(_pDB != NULL);
	vFrom = _pDB->Quote(szFrom);
	vRemote = _pDB->Quote(szRemote);
	ASSERT(vFrom != NULL && vRemote != NULL);

	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	
	pResult->Execute("INSERT INTO Outgoing (MessageID, SendTime, MsgFrom, MsgTo) VALUES (%d, DATETIME('now'), '%q', '%q')", nMsgID, vFrom, vRemote);

	delete pResult;
	Unlock();
	free(vFrom);
	free(vRemote);
}



void DataModel::DeleteMessage(int nMsgID)
{
	DpMySqlDB *pResult;
	
	ASSERT(nMsgID > 0);
				
	Lock();
	ASSERT(_pDB != NULL);
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	pResult->Execute("DELETE FROM Bodies WHERE MessageID=%d", nMsgID);
	delete pResult;
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	pResult->Execute("DELETE FROM Messages WHERE MessageID=%d", nMsgID);
	delete pResult;
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	pResult->Execute("DELETE FROM Summaries WHERE MessageID=%d", nMsgID);
	delete pResult;

	Unlock();
}



DataList * DataModel::GetMessageList(int nUserID, int nMax)
{
	DpMySqlDB *pResult;
	DataList *pList = NULL;
	int nMsgID, nSize;
	
	ASSERT(nUserID > 0 && nMax > 0);
	
	Lock();
	ASSERT(_pDB != NULL);
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("SELECT MessageID,BodySize FROM Messages WHERE UserID=%d AND Incoming=1 LIMIT %d", nUserID, nMax) == true) {
		pList = new DataList;
		pList->AddColumn("MessageID");
		pList->AddColumn("Size");
		while(pResult->NextRow()) {
			pResult->GetData("MessageID", &nMsgID);
			pResult->GetData("BodySize", &nSize);
			pList->AddRow();
			pList->AddData(0, nMsgID);
			pList->AddData(1, nSize);
		}
		pList->Complete();
	}
	delete pResult;
	Unlock();

	return(pList);
}


int DataModel::GetMessageLength(int nMsgID)
{
	DpMySqlDB *pResult;
	int nSize = -1;
	
	ASSERT(nMsgID > 0);
			
	Lock();
	ASSERT(_pDB != NULL);
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("SELECT SUM(LENGTH(Body)) AS Size FROM Bodies WHERE MessageID=%d", nMsgID) == true) {
		if(pResult->NextRow()) {
			pResult->GetData("Size", &nSize);
		}
	}
	delete pResult;
	Unlock();
	
	return(nSize);
}



DataList * DataModel::GetMessageBody(int nMsgID)
{
	DpMySqlDB *pResult;
	DataList *pList=NULL;
	char line[1025];
	int slen;
	
	ASSERT(nMsgID > 0);
			
	Lock();
	ASSERT(_pDB != NULL);
	
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("SELECT Body FROM Bodies WHERE MessageID=%d ORDER BY Line", nMsgID) == true) {
		pList = new DataList;
		pList->AddColumn("Line");
		while(pResult->NextRow()) {
			pResult->GetData("Body", line, 1024);
			slen = strlen(line);
			if (slen > 1024) line[1024] = '\0';
			pList->AddRow();
			pList->AddData(0, line);
		}
		pList->Complete();
	}
	delete pResult;
	Unlock();
	
	return(pList);
}



//-----------------------------------------------------------------------------
// CJW: We have a message we need to insert. 
void DataModel::UpdateMessageSize(int nMsgID, int nSize)
{
	DpMySqlDB *pResult;
	
	ASSERT(nMsgID > 0 && nSize >= 0);
	
	Lock();
	ASSERT(_pDB != NULL);
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	pResult->Execute("UPDATE Messages SET BodySize=%d WHERE MessageID=%d", nSize, nMsgID);
	delete pResult;
	Unlock();
}


//-----------------------------------------------------------------------------
// CJW: The data stored did not originally contain any message size details, it 
// 		was calculated when messages were retrieved.  Howevere, this can be  a 
// 		heavy resource that is unnecessary because we actually know the size of 
// 		the data when we receive the message and before it is actually put in 
// 		the database.  We merely need to store this data.
void DataModel::ConvertPop3()
{
	Logger log;
	DpMySqlDB *pResult;
	DataList *pList = NULL;
	int nMsgID, nSize, nCount, nTotal;
	
	
	// First lets get the list of messages
	Lock();
	ASSERT(_pDB != NULL);
	pResult = _pDB->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("SELECT MessageID FROM Messages WHERE Incoming=1 AND BodySize=0") == true) {
		pList = new DataList;
		pList->AddColumn("MessageID");
		while(pResult->NextRow()) {
			pResult->GetData("MessageID", &nMsgID);
			pList->AddRow();
			pList->AddData(0, nMsgID);
		}
		pList->Complete();
	}
	delete pResult;
	Unlock();


	

	if (pList != NULL) {
		nTotal = pList->GetRowCount();
		ASSERT(nTotal >= 0);
		log.Log("ConvertPop - %d items to calculate.", nTotal);
		
		nCount = 0;
		while (pList->NextRow()) {
			nCount ++;
			nMsgID = pList->GetInt(0);
			ASSERT(nMsgID > 0);
			log.Log("ConvertPop - Calculating %d (%d of %d)", nMsgID, nCount, nTotal);
			nSize = GetMessageLength(nMsgID);
			ASSERT(nSize >= 0);
			UpdateMessageSize(nMsgID, nSize);
			log.Log("ConvertPop - Finished Calculating %d", nMsgID);
			Sleep(10);
		}
		
		delete pList;
	}
}


