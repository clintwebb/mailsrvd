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
	pResult = _pDB->Spawn();
	ASSERT(pResult = NULL);
	vUser = pResult->Quote(_Data.auth.szUser);
	vPass = pResult->Quote(_Data.auth.szPass);
	ASSERT(vUser != NULL && vPass != NULL);
	if (pResult->Execute("SELECT UserID FROM Users WHERE Account='%s' AND Password='%s' LIMIT 1", vUser, vPass) == true) {
		if(pResult->NextRow()) {
			 pResult->GetData("UserID", &nUserID);
		}
	}
	delete pResult;
	free(vUser);
	free(vPass);
	Unlock();
	return (nUserID);
}


//-----------------------------------------------------------------------------
// CJW: Given the domain name, we return the domainID.  We will store the information internally, 
int DataModel::GetDomainID(char *szDomain)
{
	DpMySqlDB *pResult;
	char *vDomain;
	int nDomainID;
	
	ASSERT(szDomain != NULL);
	
	Lock();
	ASSERT(_pDB != NULL);
	pResult = _pDB->Spawn();
	vDomain = pResult->Quote(domain);
	if (pResult->Execute("SELECT DomainID, Reject FROM Domains WHERE Name LIKE '%s'", vDomain) == true) {
		if(pResult->NextRow()) {
			pResult->GetData("DomainID", &nDomainID);
			_Cache.domain.nID = nDomainID;
			pResult->GetData("Reject", &_Cache.domain.nReject);
		}
	}
	free(vDomain);
	delete pResult;
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
		ASSERT(_pData != NULL);
		pResult = _pData->Spawn();
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

	Lock();
	ASSERT(_pData != NULL);
	pResult = _pData->Spawn();
	ASSERT(pResult != NULL);
	vUser = pResult->Quote(szUser);
	if (pResult->Execute("SELECT UserID FROM Addresses WHERE DomainID=%d AND Name LIKE '%s'", nDomainID, vUser) == true) {
		pList = new DataList;
		pList->AddColumn("UserID");
		while(pResult->NextRow()) {
			nUserID = pResult->GetInt("UserID");
			pList->AddRow();
			pList->AddData(0, nUserID);
		}
	}
	free(vUser);
	delete pResult;
	Unlock();
	
	pList->Complete();
	return(pList);
}





//-----------------------------------------------------------------------------
// CJW: We have a message we need to insert. 
int DataModel::InsertMessage(int nUserID)
{
	DpMySqlDB *pResult;
	int nMsgID=0;
	
	ASSERT(nUserID > 0);
	
	Lock();
	ASSERT(_pData != NULL);
	pResult = _pData->Spawn();
	ASSERT(pResult != NULL);
	if (pResult->Execute("INSERT INTO Messages (UserID, Incoming) VALUES (%d, 1)", nUserID) == true) {
		nMsgID = pResult->GetInsertID();
		ASSERT(nMsgID > 0);
	}
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
	char *vLine;
	ASSERT(nMsgID > 0 && lineno > 0 && szLine != NULL);
	
	Lock();
	ASSERT(_pData != NULL);
	pResult = _pData->Spawn();
	ASSERT(pResult != NULL);
	vLine = pResult->Quote(szLine);
	ASSERT(vLine != NULL);
	pResult->ExecuteNR("INSERT INTO Bodies (MessageID, Line, Body) VALUES (%d, %d, '%s')", nMsgID, lineno, vLine);
	free(vLine);
	delete pResult);
	Unlock();
}


