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

#include <DpBase64.h>

#include "Message.h"
#include "DataList.h"


//-----------------------------------------------------------------------------
Message::Message() 
{	
	_log.SetName("Message");
	
	_log.Log("Message()");
	
	_Data.szHelo   = NULL;
	_Data.szFrom   = NULL;
	_Data.szBodies = NULL;
	_Data.nBodies  = 0;
	_Data.nRemote  = 0;
	
	_Data.auth.bAuthenticating = false;
	_Data.auth.bAuthenticated  = false;
	_Data.auth.szUser = NULL;
	_Data.auth.szPass = NULL;
	_Data.auth.nUserID = 0;
	
	Reset();
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.   Clean up everything before this object is destroyed.
Message::~Message() 
{
	_log.Log("~Message() - Start");
	Reset();							
	
	if (_Data.szHelo != NULL) {
		free(_Data.szHelo);
		_Data.szHelo = NULL;
	}
	
	ASSERT(_Data.szFrom == NULL);
	ASSERT(_Data.szBodies == NULL);
	
	if (_Data.auth.szUser != NULL) {
		free(_Data.auth.szUser);
		_Data.auth.szUser = NULL;
	}
	
	if (_Data.auth.szPass != NULL) {
		free(_Data.auth.szPass);
		_Data.auth.szPass = NULL;
	}
	_log.Log("~Message() - Done");
}



//-----------------------------------------------------------------------------
// CJW: A new connection was established, so we need to send some text which 
// 		indicates what kind of service this is.
void Message::OnStart(void) 
{
	_Qout.Print("220 srv02 ESMTP mailsrv\r\n");
}

//-----------------------------------------------------------------------------
// CJW: Since all of these sessions are command based (one command per line), 
// 		then we need to process the command.
void Message::OnCommand(char *line)
{
	int len;
	
	ASSERT(line != NULL);
	len = strlen(line);
	_log.Log("New Command. (%s)[%d]", line, len);
				
	if (len < 4 && len > 0) {
		_Qout.Print("500 Command not recognised.\r\n");
	}
	else if (len > 0) {
		if 		(strncasecmp(line, "EHLO", 4) == 0) 		{ ProcessEHLO(line, len); }
		else if (strncasecmp(line, "HELO", 4) == 0)			{ ProcessHELO(line, len); }
		else if (strncasecmp(line, "MAIL FROM:", 10) == 0)	{ ProcessMAIL(line, len); }
		else if (strncasecmp(line, "RCPT TO:", 8) == 0)		{ ProcessRCPT(line, len); }
		else if (strncasecmp(line, "DATA", 4) == 0)			{ ProcessDATA(line, len); }
		else if (strncasecmp(line, "RSET", 4) == 0)			{ ProcessRSET(line, len); }
		else if (strncasecmp(line, "NOOP", 4) == 0)			{ ProcessNOOP(line, len); }
		else if (strncasecmp(line, "QUIT", 4) == 0)			{ ProcessQUIT(line, len); }
		else if (strncasecmp(line, "AUTH LOGIN", 10) == 0)	{ ProcessAUTH(line, len); }
		else { _Qout.Print("500 Command not recognised.\r\n"); }
	}
}

//-----------------------------------------------------------------------------
// CJW: When we are in data mode we store all the data that we get.  When a 
// 		line is passed to this function, the caller does not free the memory, 
// 		it is up to us.
void Message::OnData(char *line)
{
	int len;
	DpBase64 base;
	char *result;
	
	ASSERT(_pData != NULL);
	
	if (_Data.auth.bAuthenticating == true) {
		// we are getting authentication data, so we need to check to see what stage we are in, and validate the information received.
		
		
		ASSERT(_Data.auth.szPass == NULL && _Data.auth.nUserID == 0);
		if (_Data.auth.szUser == NULL) {
			result = (char *) base.Decode(line, &len);
			ASSERT(len > 0 && result != NULL);
			_Data.auth.szUser = (char *) malloc(len+1);
			strncpy(_Data.auth.szUser, result, len);
			_Data.auth.szUser[len] = '\0';
			
			_Qout.Print("334 UGFzc3dvcmQ6\r\n");
			_log.Log("Received username '%s', requesting password", _Data.auth.szUser);
		}
		else {
			result = (char *) base.Decode(line, &len);
			ASSERT(len > 0 && result != NULL);
			_Data.auth.szPass = (char *) malloc(len+1);
			strncpy(_Data.auth.szPass, result, len);
			_Data.auth.szPass[len] = '\0';
			
			// now validate the account.
			_Data.auth.nUserID = _pData->GetUserID(_Data.auth.szUser, _Data.auth.szPass);
			
			ASSERT(_Data.auth.nUserID >= 0);
			if (_Data.auth.nUserID > 0) {
				_Qout.Print("250 OK\r\n");
				_Data.auth.bAuthenticated = true;
				_log.Log("Authenticated");
			}
			else {
				_Qout.Print("535 authentication failed (#5.7.1)\r\n");
				_log.Log("--> 535");
				ASSERT(_Data.auth.bAuthenticated == false);
			}
			
			ChangeState(Waiting);
			_Data.auth.bAuthenticating = false;
		}
	}
	else {
	
		ASSERT(line != NULL);
//		_log.Log("Data received (%s)", line);
		len = strlen(line);
		if (len > 1024) {
			// line is too long, we need to close the connection.
			line[1023] = '\0';
			line = (char *) realloc(line, 1024);
		}
					
		if (line[0] == '.' && len == 1) {
			ASSERT(_Data.szBodies != NULL && _Data.nBodies > 0);
						
			SaveMessage();
			ChangeState(Waiting);
			free(line);
						
			_Qout.Print("250 OK\r\n");
		}
		else {
			_Data.szBodies = (char **) realloc(_Data.szBodies, sizeof(char *) * (_Data.nBodies + 1));
			_Data.szBodies[_Data.nBodies] = line;
			_Data.nBodies ++;
								
			line = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// CJW: When the client has issued the QUIT command...
void Message::OnClose(void)
{
	// there is nothing that we are going to do here.
	_log.Log("OnClose()");
}





//-----------------------------------------------------------------------------
// CJW: We are going to wait for a HELO command.  We will fail if the HELO 
// 		command doesnt have an actual parameter.
void Message::ProcessHELO(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("HELO received.");
	if (len < 6) {
		_Qout.Print("501 Invalid Parameter\r\n");
	}
	else if (_Data.szHelo != NULL) {
		_Qout.Print("503 Bad sequence of commands.\r\n");
	}
	else {
		_Data.szHelo = (char *) malloc(len);
		ASSERT(_Data.szHelo != NULL);
		
		strcpy(_Data.szHelo , &ptr[5]);
		_log.Log("HELO: (%s)", _Data.szHelo);
		_Qout.Print("250 OK\r\n");
	}
}


//-----------------------------------------------------------------------------
// CJW: We are going to wait for a HELO command.  We will fail if the HELO 
// 		command doesnt have an actual parameter.
void Message::ProcessEHLO(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("EHLO received.");
	if (len < 6) {
		_Qout.Print("501 Invalid Parameter\r\n");
	}
	else if (_Data.szHelo != NULL) {
		_Qout.Print("503 Bad sequence of commands.\r\n");
	}
	else {
		_Data.szHelo = (char *) malloc(len);
		ASSERT(_Data.szHelo != NULL);
		
		strcpy(_Data.szHelo , &ptr[5]);
		_log.Log("EHLO: (%s)", _Data.szHelo);
		_Qout.Print("250-mail.hyper-active.com.au\r\n250 AUTH LOGIN\r\n");
	}
}


//-----------------------------------------------------------------------------
// CJW: The client is wanting to authenticate so that they can send emails to 
// 		other systems (relay), which we only allow to authenticated users.
void Message::ProcessAUTH(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("AUTH received.");
	if (len < 10) {
		_log.Log("--> 501");
		_Qout.Print("501 Invalid Parameter\r\n");
	}
	else if (_Data.szHelo == NULL) {
		_log.Log("--> 503");
		_Qout.Print("503 Bad sequence of commands.\r\n");
	}
	else {
		_Data.auth.bAuthenticating = true;
		_Data.auth.bAuthenticated = false;
		
		_Qout.Print("334 VXNlcm5hbWU6\r\n");
		ChangeState(Data);
		_log.Log("Sent Login request");
	}
}




//-----------------------------------------------------------------------------
// CJW: The client is telling us who they are, so we need to record that 
// 		information, because we are going to need it later when we add our 
// 		informative header information.
void Message::ProcessMAIL(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("MAIL received.");
	if (len < 11) {
		_Qout.Print("501 Invalid Parameter\r\n");
	}
	else if (_Data.szHelo == NULL) {
		_Qout.Print("503 Bad sequence of commands.\r\n");
	}
	else if (_Data.szFrom != NULL) {
		_Qout.Print("503 Bad sequence of commands.\r\n");
	}
	else {
		_Data.szFrom = (char *) malloc(len);
		ASSERT(_Data.szFrom != NULL);
		
		strcpy(_Data.szFrom , &ptr[11]);
		_log.Log("MAIL FROM: (%s)", _Data.szFrom);
		
		_Qout.Print("250 OK\r\n");
	}
}


//-----------------------------------------------------------------------------
// CJW: The client is telling us who the email is for.  Remember that there can 
// 		be more than one of them, and we will need to validate each one.  If 
// 		the domain is one that we control, and that domain doesnt reject 
// 		unknown users, we will send this email to the postmaster user for that 
// 		domain.
void Message::ProcessRCPT(char *ptr, int len)
{
	char *user, *domain;
	int i,j;
	int nDomainID, nReject, nUserID=0;
	bool bFound;
	DataList *pList;

	ASSERT(_pData != NULL);
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("RCPT received.");
	if (len < 9) {
		_Qout.Print("501 Invalid Parameter\r\n");
	}
	else if (_Data.szFrom == NULL) {
		_Qout.Print("503 Bad sequence of commands.\r\n");
	}
	else {
		user = (char *) malloc(len);
		domain = (char *) malloc(len);
		
		ASSERT(user != NULL && domain != NULL);
		
		i = 9;
		j = 0;
		while (i < len && ptr[i] != '@') {
			if (ptr[i] != '<') { user[j++] = ptr[i]; }
			i++;
		}
		user[j] = '\0';
		ASSERT(j < len);
		
		i++;	// to skip the @.
		j=0;
		while (i < len) {
			if (ptr[i] == '>' || ptr[i] == ' ') { i = len; }
			else { domain[j++] = ptr[i]; }
			i++;
		}
		domain[j] = '\0';
		ASSERT(j < len);
		
		_log.Log("RCPT TO: %s@%s", user, domain);
		
		// now we need to verify that this is a domain that we control.
		
		
		ASSERT(_pData != NULL);
		nReject = 0; 
		nDomainID = _pData->GetDomainID(domain);
		ASSERT(nDomainID > 0);
		nReject   = _pData->GetDomainReject(nDomainID);
		ASSERT(nReject == 0 || nReject == 1);
		_log.Log("DomainID: %d", nDomainID);
		_log.Log("Reject: %d", nReject);
		
		if (nDomainID == 0) {
			if (_Data.auth.bAuthenticated == false) {
				_Qout.Print("450 Mailbox unavaliable.  No relay.\r\n");
			}
			else {
				ASSERT(_Data.auth.bAuthenticating == false);
				ASSERT(_Data.auth.nUserID > 0);
				
				// we need to store the details so that the email can be sent out of our system.
				ASSERT(_Data.nRemote <= MAX_REMOTE_ADDRESSES);
				ASSERT(_Data.szRemote[_Data.nRemote] == NULL);
				if (_Data.nRemote < MAX_REMOTE_ADDRESSES) {
					i = strlen(user) + strlen(domain) + 2;
					_Data.szRemote[_Data.nRemote] = (char *) malloc(i);
					j = sprintf(_Data.szRemote[_Data.nRemote], "%s@%s", user, domain);
					ASSERT((j+1) <= i);
					_Data.nRemote ++;
					_Qout.Print("250 OK\r\n");
				}
				else {
					_Qout.Print("554 Transaction failed.\r\n");
				}
			}
		}
		else {
		
			// now that we know we handle this domain, we need to see if this username exists.
			
			ASSERT(_pData != NULL);
			bFound = false;
			pList = _pData->GetUserFromAddress(nDomainID, user);
			if (pList != NULL) {
				while(pList->NextRow()) {
					nUserID = pList->GetInt("UserID");
					AddUser(nUserID);
					bFound = true;
				}
				delete pList;
			}
			
			if (bFound == false) {
				if (nReject == 0) {
					pList = _pData->GetUserFromAddress(nDomainID, "postmaster");
					ASSERT(pList != NULL);
					while(pList->NextRow()) {
						nUserID = pList->GetInt("UserID");
						AddUser(nUserID);
						bFound = true;
					}
					delete pList;
				}
			}
			
			ASSERT(_Data.nUsers <= MAX_LOCAL_ADDRESSES);
			if (bFound == false) {
				ASSERT(nUserID == 0);
				_Qout.Print("450 Mailbox does not exist.\r\n");
			}
			else {
				ASSERT(bFound == true);
				ASSERT(nUserID > 0);
				if (_Data.nUsers == MAX_LOCAL_ADDRESSES) {
					_Qout.Print("554 Transaction failed.\r\n");
				}
				else {
					_Qout.Print("250 OK\r\n");
				}
			}
		}
		
		free(domain);
		free(user);
	}
}


//-----------------------------------------------------------------------------
// CJW: 
void Message::AddUser(int nUserID)
{
	int i,j;
	
	ASSERT(nUserID > 0);
	
	if (_Data.nUsers < MAX_LOCAL_ADDRESSES) {
		j = -1;
		for (i=0; i<_Data.nUsers; i++) {
			ASSERT(_Data.nUserID[i] > 0);
			if (_Data.nUserID[i] == nUserID) {
				j = i;
				i = _Data.nUsers;
			}
		}
		if (j < 0) {
			_Data.nUserID[_Data.nUsers] = nUserID;
			_Data.nUsers++;
		}
	}
}



//-----------------------------------------------------------------------------
// CJW: If the client is ready to send data, then we need to have at least one 
// 		valid recipient.
void Message::ProcessDATA(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	ASSERT(_Data.nUsers >= 0);
	ASSERT(_Data.auth.bAuthenticating == false);
	
	if (_Data.nUsers == 0 && _Data.nRemote == 0) {
		_log.Log("--> 503");
		_Qout.Print("503 No recipients.\r\n");
	}
	else {
		_Qout.Print("354 Start mail input; end with <CRLF>.<CRLF>\r\n");
		ChangeState(Data);
	}
}


//-----------------------------------------------------------------------------
// CJW: 
void Message::ProcessRSET(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	ASSERT(GetState() == Waiting);
	
	_Qout.Print("250 OK\r\n");
	Reset();
}


//-----------------------------------------------------------------------------
// CJW: 
void Message::ProcessNOOP(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	ASSERT(GetState() == Waiting);
	
	_Qout.Print("250 OK\r\n");
}


//-----------------------------------------------------------------------------
// CJW: The client has issued the quit command.  We will quit right now.  This 
// 		will cause the thread to stop processing, and our parent process to 
// 		delete this object (which will clean everything out.
void Message::ProcessQUIT(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	ChangeState(Close);
}


//-----------------------------------------------------------------------------
// CJW: The process has been completed, and we need to save this email in the 
// 		database.  To do this, we need to first create the message record, and 
// 		then create all the header and body records.
void Message::SaveMessage(void)
{
	int i, j, nID;
	
	ASSERT(_Data.szBodies != NULL  && _Data.nBodies > 0);
	ASSERT(_pData != NULL);
	
	ASSERT(_Data.nUsers > 0 || _Data.nRemote > 0);
	for (i=0; i<_Data.nUsers; i++) {
		ASSERT(_Data.nUserID[i] > 0);
		
		// create the message.
		nID = _pData->InsertMessage(_Data.nUserID[i]);
		ASSERT(nID > 0);
		
		for (j=0; j<_Data.nBodies; j++) {
			ASSERT(_Data.szBodies[j] != NULL);
			_pData->InsertBodyLine(nID, j+1, _Data.szBodies[j]);
		}
	}
	
	if (_Data.nRemote > 0) {
		ASSERT(_Data.auth.bAuthenticated == true);
		ASSERT(_Data.auth.bAuthenticating == false);
		ASSERT(_Data.auth.nUserID > 0);
		ASSERT(_Data.szFrom != NULL);
	
		for (i=0; i < _Data.nRemote; i++) {
			ASSERT(_Data.szRemote[i] != NULL);
			
			// *** is that right?  
			nID = _pData->InsertMessage(_Data.auth.nUserID);
			ASSERT(nID > 0);
			
			for (j=0; j<_Data.nBodies; j++) {
				ASSERT(_Data.szBodies[j] != NULL);
				_pData->InsertBodyLine(nID, j+1, _Data.szBodies[j]);
			}
			
			_pData->InsertOutgoing(nID, _Data.szFrom, _Data.szRemote[i]);
			
		}
	}	
}


//-----------------------------------------------------------------------------
// CJW: Reset the data that we have stored for any messages.  
void Message::Reset(void)
{
	int i;
	
	if (_Data.szFrom != NULL) {
		free(_Data.szFrom);
		_Data.szFrom = NULL;
	}
	
	for (i=0; i<MAX_LOCAL_ADDRESSES; i++) { _Data.nUserID[i] = 0; }
	_Data.nUsers = 0;
	
	for (i=0; i<MAX_REMOTE_ADDRESSES; i++) { 
		if (_Data.szRemote[i] != NULL) {
			if (i < _Data.nRemote) { free(_Data.szRemote[i]); }
			_Data.szRemote[i] = NULL;
		}
	}
	_Data.nRemote = 0;
	
	while(_Data.nBodies > 0) {
		_Data.nBodies--;
		ASSERT(_Data.szBodies[_Data.nBodies] != NULL);
		free(_Data.szBodies[_Data.nBodies]);
	}
	if (_Data.szBodies != NULL) {
		free(_Data.szBodies);
		_Data.szBodies = NULL;
	}
	
	_Data.auth.bAuthenticating = false;
}


