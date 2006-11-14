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