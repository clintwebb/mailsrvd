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
#include <unistd.h>

#include "SenderSession.h"



//-----------------------------------------------------------------------------
SenderSession::SenderSession() 
{	
	_log.SetName("SenderSession");
	_log.Log("SenderSession()");
	_Data.nMessageID = 0;
	_Data.szFrom = NULL;
	_Data.szTo   = NULL;
	_State = Starting;
	
	_pAddressList = NULL;
	_nAddresses = 0;
}
		
//-----------------------------------------------------------------------------
// CJW: Deconstructor.   Clean up everything before this object is destroyed.
SenderSession::~SenderSession() 
{
	_log.Log("~SenderSession() - Start");
	ASSERT(_Data.nMessageID > 0);	
	
	ASSERT((_pAddressList == NULL && _nAddresses == 0) || (_pAddressList != NULL && _nAddresses > 0));
	if (_pAddressList != NULL) {
		free(_pAddressList);
		_pAddressList = NULL;
	}
	
	_log.Log("~SenderSession() - End");
}


//-----------------------------------------------------------------------------
// CJW: This is the function that launches the thread.  We need to first change 
// 		the field in the database, and then we can start the actual thread that 
// 		will process the itnformation.
void SenderSession::SendMessage(int nMessageID)
{
	ASSERT(nMessageID > 0);
	
	ASSERT(_Data.nMessageID == 0);
	_Data.nMessageID = nMessageID;
	
	_log.SetID(nMessageID);
	_log.Log("Processing Message: %d", nMessageID);
	
	// modify the state of the message.  We will wait for a result so that we 
	// know its state is valid before we start the thread.  Also, we want to 
	// block the existing thread until that is done.
	
	ChangeState(Started);
	
	Start();
}


//-----------------------------------------------------------------------------
// CJW: The session has started.  We need to get the information out of the 
// 		database and make the connection to the appropriate mail server.  We 
// 		should not exit this function until we have either connected, or failed 
// 		to connect.  If we have failed to connect, then we need to set the 
// 		SessionState to Done so that the thread exits.
void SenderSession::OnStart(void) 
{
	DpSqlite3Result *pResult;
	int nStatus;
	bool bContinue;
	int i;
	
	ASSERT(_Data.nMessageID > 0 && _Data.szFrom == NULL && _Data.szTo == NULL);
	ASSERT(_State == Starting);
	ASSERT(_pDB != NULL);
	
	pResult = _pDB->Execute("SELECT * FROM Outgoing WHERE MessageID=%d", _Data.nMessageID);
	
	ASSERT(pResult != NULL);
	if (pResult->NextRow()) {
		nStatus = pResult->GetInt("Status");
		_Data.szFrom  = pResult->GetStr("MsgFrom");
		_Data.szTo    = pResult->GetStr("MsgTo");
		
		_log.Log("Session: Message from: %s", _Data.szFrom);
		_log.Log("Session: Message to: %s", _Data.szTo);
		
		// get the target system from the TO entry.
		bContinue = GetServerFromAddress(_Data.szTo);
		
		if (bContinue == false) {
			// server could not be retrieved.
			// we could not find a server to send it to.  What do we do with it?
			_log.Log("Unable to send message to %s", _Data.szTo);
			
			// we will need to send a reply email to the From address.
		}
		else {
			// server was retrieved, so we need to connect to it.
			
			ASSERT(_pAddressList != NULL && _nAddresses > 0);
			
			i = 0;
			while (i < _nAddresses && bContinue == true) {
				ASSERT(_pAddressList[i].szAddress != NULL);
				_log.Log("Session: Try #%d - Server for %s is %s (%d)", i, _Data.szTo, _pAddressList[i].szAddress, _pAddressList[i].nPriority);
			
				if (Connect(_pAddressList[i].szAddress, 25) == true) {
					_State = Connected;
					bContinue = false;
					_log.Log("Session: Connected to %s for %s", _pAddressList[i].szAddress, _Data.szTo);
				}
				
				i++;
			}
			
			if (bContinue == true) {
				// unable to connect to the server.  
				
				_log.Log("Session: Unable to Connect for %s", _Data.szTo);
			}
		}
	}
}



//-----------------------------------------------------------------------------
// CJW: We are given an actual address.  We need to parse the address so that 
// 		we can get the mailbox and the domain seperate.  Once we have the 
// 		domain information, we will check the dns cache table to see if we 
// 		already know the mail server for this domain.   If it isnt in the 
// 		database, then we need to perform a DNS lookup for it.  For now we will 
// 		just use the output of 'dig'.  If it is in the database, but the status 
// 		indicates that a lookup is being performed, we wait until the result is 
// 		available.   If it takes longer than 60 seconds for a result to be 
// 		returned, we reset the status and look it up ourselves.
bool SenderSession::GetServerFromAddress(char *szAddress)
{
	bool bOK = false;
	int len, pos=0, i=0;
	char szDomain[2048];
	enum {
		Starting,
		User,
		Domain,
		Done,
		Waiting
	} status = Starting;
	DpSqlite3Result *pResult;
	struct {
		int nID;
		int nStatus;
		char *szServer;
		int nPriority;
		int nEntries;
	} data;
	int nLoops;
	
	_log.Log("Session: Resolve Address '%s'", szAddress);
	len = strlen(szAddress);
	
	ASSERT(_pDB != NULL);
	
	// get the domain out of the address (find the '@' and go to the end (make sure no < or > is in there).
	
	while (status != Done && pos < len) {
		switch(status) {
			case Starting:
				status = User;
				
			case User:
				if (szAddress[pos] == '@') 
					status = Domain;
				break;
				
			case Domain:
				if ((szAddress[pos] != '@') && (szAddress[pos] != '>')) {
					szDomain[i] = szAddress[pos];
					i++;
				}
				else {
					status = Done;
				}
				break;
				
			case Done:
			default:
				_log.Log("Unexpected state: %d", status);
				break;
		}
		
		pos++;
	}
	
	szDomain[i] = '\0';
	_log.Log("Session: Domain for %s resolved to %s", szAddress, szDomain);
	
	if (status == Done || status == Domain) {
		ASSERT(szDomain != NULL);
		ASSERT(strlen(szDomain) < len);
		
		status = Waiting;
		nLoops = 5;
		data.nID = 0;
		
		// See if that domain is in the database.
		while (status == Waiting) {
			ASSERT(_pAddressList == NULL && _nAddresses == 0);
			
			pResult = _pDB->Execute("SELECT CacheID,Status,Entries FROM DomainCache WHERE Domain LIKE '%q'", szDomain);
			ASSERT(pResult != NULL);
			if (pResult->NextRow()) {
				data.nID       = pResult->GetInt("CacheID");
				data.nStatus   = pResult->GetInt("Status");
				data.nEntries  = pResult->GetInt("Entries");
	
				if (data.nStatus < 2) {
					// sleep for 1 second, and then we will try again.
					_log.Log("Session: Domain '%s' is being processed (%d).", szDomain, data.nStatus);
					Sleep(1000);
					nLoops --;
					if (nLoops == 0) { status = Done; }
				}
				else {
					// YES, use it.
					ASSERT(data.nStatus == 2);
					ASSERT(data.nID > 0);
					ASSERT(data.nEntries > 0);
					ASSERT(_pAddressList == NULL && _nAddresses == 0);
					
					_pAddressList = (ServerAddress *) malloc(sizeof(ServerAddress) * data.nEntries);
					
					delete pResult;
					pResult = _pDB->Execute("SELECT Server,Priority FROM DomainCacheEntries WHERE CacheID=%d ORDER BY Priority", data.nID);
					ASSERT(pResult != NULL);
					while (pResult->NextRow()) {
						data.szServer  = pResult->GetStr("Server");
						data.nPriority = pResult->GetInt("Priority");
					
						ASSERT(_nAddresses < data.nEntries);
						
						_log.Log("Session: Got sender address of %s (%d)", data.szServer, data.nPriority);
						
						strncpy(_pAddressList[_nAddresses].szAddress, data.szServer, MAX_ADDRESS);
						_pAddressList[_nAddresses].nPriority = data.nPriority;
						_nAddresses ++;
					}
					delete pResult;
					pResult = NULL;
					
					if (_nAddresses == 0) {
						// We have a cache, but we dont have any entries.   So we will delete the cache entry.
						_pDB->ExecuteNR("DELETE FROM DomainCache WHERE CacheID=%d", data.nID);
						_pDB->ExecuteNR("DELETE FROM DomainCacheEntries WHERE CacheID=%d", data.nID);
					}
					else {
						bOK = true;
					}
					
					status = Done;
				}
			}
			else {
				// NO, there was nothing in the database for this domain. Add 
				// 	   it to the database, sleep for a very short time, and 
				// 	   then loop around again.
				
				delete pResult;
				pResult = _pDB->Execute("INSERT INTO DomainCache (Domain) VALUES ('%q')", szDomain);
				data.nID = pResult->GetInsertID();
				ASSERT(data.nID > 0);
				delete pResult;  
				pResult = NULL;
				
				Resolve(data.nID, szDomain);
			}
			
			// If we havent already deleted our result set, then we will do so now.
			if (pResult != NULL) {
				delete pResult;  
				pResult = NULL;
			}
		}
	}

	return(bOK);
}






//-----------------------------------------------------------------------------
// CJW: Since all of these sessions are command based (one command per line), 
// 		then we need to process the command.
void SenderSession::OnCommand(char *line)
{
	int len;
	int result;
	
	ASSERT(line != NULL);
	len = strlen(line);
	_log.Log("New Command. (%s)[%d]", line, len);
				
	if (len < 3 && len > 0) {
		_log.Log("Unexpected reply.");
		_Qout.Print("QUIT\r\n");
		ChangeState(Close);
	}
	else if (len > 0) {
		result = atoi(line);
		ASSERT(_State != Starting);
		switch (_State) {
			case Connected:		ProcessConnected(result);				break;
			case SentHelo:		ProcessHelo(result);					break;
			case SentMailFrom:	ProcessMailFrom(result);				break;
			case SentRcpt:		ProcessRcpt(result);					break;
			case SentData:		ProcessData(result);					break;
			case SentDot:		ProcessDot(result);						break;
			case SentQuit:		ProcessQuit(result);					break;
			default:			_log.Log("Unexpected state: %d", _State); 	break;
		}
	}
	else {
		_log.Log("Filtered out blank line");
	}

}

//-----------------------------------------------------------------------------
// CJW: When we are in data mode we store all the data that we get.  When a 
// 		line is passed to this function, the caller does not free the memory, 
// 		it is up to us.
void SenderSession::OnData(char *line)
{
	ASSERT(0);
	// we should never actually get into a data state with the sender object.  We are only using the session object to easily access the result codes.
}

//-----------------------------------------------------------------------------
// CJW: When the client has issued the QUIT command...
void SenderSession::OnClose(void)
{
	_log.Log("SenderSession Closed");
	// there is nothing that we are going to do here.
}




//-----------------------------------------------------------------------------
// CJW: We have connected with the server, and we have received the initial 
// 		responce.  It should be a 220, and then we can send a HELO.
void SenderSession::ProcessConnected(int nResult)
{
	ASSERT(nResult > 0);
	
	if (nResult == 220) {
		_log.Log("CONNECT Result: %d - Sending HELO", nResult);
		_Qout.Print("HELO mail.cjdj.org\r\n");
		_State = SentHelo;
	}
	else {
		_log.Log("CONNECT Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
		
		// We should make sure that if we werent able to send the message, but there are more than one mail server address we can send it to, then we should retry to connect to the next one.
	}
}


void SenderSession::ProcessHelo(int nResult)
{
	ASSERT(nResult > 0);
	ASSERT(_Data.szFrom != NULL);
	
	if (nResult == 250) {
		_log.Log("HELO Result: %d - Sending MAIL FROM", nResult);
		_Qout.Print("MAIL FROM:<%s>\r\n", _Data.szFrom);
		_State = SentMailFrom;
	}
	else {
		_log.Log("HELO Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
	}
}

void SenderSession::ProcessMailFrom(int nResult)
{
	ASSERT(nResult > 0);
	ASSERT(_Data.szFrom != NULL);
	ASSERT(_Data.szTo != NULL);
	
	if (nResult == 250) {
		_log.Log("MAIL FROM Result: %d - Sending RCPT TO", nResult);
		_Qout.Print("RCPT TO:<%s>\r\n", _Data.szTo);
		_State = SentRcpt;
	}
	else {
		_log.Log("MAIL FROM Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
	}
}


void SenderSession::ProcessRcpt(int nResult)
{
	ASSERT(nResult > 0);
	ASSERT(_Data.szFrom != NULL);
	ASSERT(_Data.szTo != NULL);
	
	if (nResult == 250) {
		_log.Log("RCPT Result: %d - Sending RCPT TO", nResult);
		_Qout.Print("DATA\r\n");
		_State = SentData;
	}
	else {
		_log.Log("RCPT Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
	}
}

void SenderSession::ProcessData(int nResult)
{
	DpSqlite3Result *pResult;
	char *szLine;
	
	ASSERT(nResult > 0);
	ASSERT(_Data.nMessageID > 0);
	ASSERT(_pDB != NULL);
	
	if (nResult == 354) {
		_log.Log("DATA Result: %d - Sending Body", nResult);
		
		
		pResult = _pDB->Execute("SELECT Body FROM Bodies WHERE MessageID=%d ORDER BY Line", _Data.nMessageID);
		ASSERT(pResult != NULL);
		while (pResult->NextRow()) {
			szLine = pResult->GetStr("Body");
			_Qout.Print("%s\r\n", szLine);
		}
		delete pResult;
		_Qout.Print(".\r\n");
		_State = SentDot;
	}
	else {
		_log.Log("DATA Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
	}
}


void SenderSession::ProcessDot(int nResult)
{
	ASSERT(nResult > 0);
	ASSERT(_Data.nMessageID > 0);
	ASSERT(_pDB != NULL);
	
	if (nResult == 250) {
		_log.Log("DATA Complete Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
		
		_pDB->Execute("DELETE FROM Bodies WHERE MessageID=%d", _Data.nMessageID);
		_pDB->Execute("DELETE FROM Messages WHERE MessageID=%d", _Data.nMessageID);
	}
	else {
		_log.Log("RCPT Result: %d - Sending QUIT", nResult);
		_Qout.Print("QUIT\r\n");
		_State = SentQuit;
	}
}



void SenderSession::ProcessQuit(int nResult)
{
	ASSERT(nResult > 0);
	_log.Log("QUIT Result: %d", nResult);
}



//-----------------------------------------------------------------------------
// CJW: We have found a domain in the database that needs to be processed.  We 
// 		have the cache ID, and we have the domain name.  We need to do what we 
// 		can to get the list of MX addresses for this domain.
void SenderSession::Resolve(int nID, char *szDomain)
{
	char szFilename[2048], szCommand[2048];
	FILE *fp;
	char buf[2048], buf2[2048];
	int pri, entries;

	ASSERT(nID > 0 && szDomain != NULL);
	ASSERT(_pDB != NULL);
	
	_log.Log("Resolve: Processing ID: %d '%s'", nID, szDomain);
	
	// first set the status of this entry to '1'
	_pDB->ExecuteNR("UPDATE DomainCache SET Status=1 WHERE CacheID=%d", nID);
	_pDB->ExecuteNR("DELETE FROM DomainCacheEntries WHERE CacheID=%d", nID);
	
	// Get the name of our temporary filename that will hold the information for this domain.
	sprintf(szFilename, "/data/mail/tmp/dns-%d", nID);
	
	// run the command that gives us the details of the domain.
	sprintf(szCommand, "host -t MX %s>%s", szDomain, szFilename);
	system(szCommand);
	
	// Open the file that has our data in it.
	fp = fopen(szFilename, "r");
	if (fp == NULL) {
		// there should be a file here now, but there wasnt, so we should log an error or something.
		printf("File '%s' doesnt exist.  What happened to it?\n", szFilename);
	}
	else {
		// read in each line of the file, and get out the information that we need.
		
		entries = 0;
		while (fscanf(fp, "%s mail is handled by %d %s", buf, &pri, buf2) != EOF) {
			printf("Server: %s (%d)\n", buf2, pri);
			
			_pDB->ExecuteNR("INSERT INTO DomainCacheEntries (CacheID, Server, Priority) VALUES (%d, '%q', %d)", nID, buf2, pri);
			
			entries ++;
		}
		
		_pDB->ExecuteNR("UPDATE DomainCache SET Created=DATETIME('now'),Entries=%d,Status=2 WHERE CacheID=%d", entries, nID);
		
		fclose(fp);
		Sleep(50);
		unlink(szFilename);
	}
}


