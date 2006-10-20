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

#include "PopSession.h"


//-----------------------------------------------------------------------------
PopSession::PopSession() 
{	
	_log.SetName("PopSession");
	_log.Log("PopSession()");
	
	_Data.szUser = NULL;
	_Data.szPass = NULL;
	_Data.nUserID = 0;
	_Data.bLoaded = false;
	_Data.nMessages = 0;
	_Data.nTotalSize = 0;
	_Data.pMsgList = NULL;
}
		
//-----------------------------------------------------------------------------
// CJW: Deconstructor.   Clean up everything before this object is destroyed.
PopSession::~PopSession() 
{
	_log.Log("~PopSession - Start");
	
	ASSERT((_Data.pMsgList == NULL && _Data.nMessages == 0) || (_Data.pMsgList != NULL && _Data.nMessages > 0));
	if (_Data.pMsgList != NULL) {
		_log.Log("~PopSession - Deleting Message list.");
		free(_Data.pMsgList);
		_Data.pMsgList = NULL;
	}
	
	if (_Data.szUser != NULL) {
		_log.Log("~PopSession - Deleting User");
		free(_Data.szUser);
		_Data.szUser = NULL;
	}
	
	if (_Data.szPass != NULL) {
		_log.Log("~PopSession - Deleting Password");
		free(_Data.szPass);
		_Data.szPass = NULL;
	}
	
	if (_Data.bLoaded == true) {
	}
	
	_log.Log("~PopSession() - End");
}



//-----------------------------------------------------------------------------
// CJW: A new connection was established, so we need to send some text which 
// 		indicates what kind of service this is.
void PopSession::OnStart(void) 
{
	_Qout.Print("+OK POP3 server ready\r\n");
}

//-----------------------------------------------------------------------------
// CJW: Since all of these sessions are command based (one command per line), 
// 		then we need to process the command.
void PopSession::OnCommand(char *line)
{
	int len;
	
	len = strlen(line);
	_log.Log("New Command. (%s)[%d]", line, len);
				
	if (len < 3 && len > 0) {
		_Qout.Print("-ERR Command not recognised.\r\n");
	}
	else if (len > 0) {
		if 		(strncasecmp(line, "USER", 4) == 0) 		{ ProcessUSER(line, len); }
		else if (strncasecmp(line, "PASS", 4) == 0)			{ ProcessPASS(line, len); }
		
		else if (strncasecmp(line, "STAT", 4) == 0)			{ ProcessSTAT(line, len); }
		else if (strncasecmp(line, "LIST", 4) == 0)			{ ProcessLIST(line, len); }
		else if (strncasecmp(line, "UIDL", 4) == 0)			{ ProcessUIDL(line, len); }
		else if (strncasecmp(line, "RETR", 4) == 0)			{ ProcessRETR(line, len); }
		else if (strncasecmp(line, "DELE", 4) == 0)			{ ProcessDELE(line, len); }

		else if (strncasecmp(line, "RSET", 4) == 0)			{ ProcessRSET(line, len); }
		else if (strncasecmp(line, "NOOP", 4) == 0)			{ ProcessNOOP(line, len); }
		else if (strncasecmp(line, "QUIT", 4) == 0)			{ ProcessQUIT(line, len); }
		else { 
			_Qout.Print("-ERR Command not recognised.\r\n"); 
			_log.Log("Command (%s) not implemented.", line);
		}
	}
}

//-----------------------------------------------------------------------------
// CJW: When we are in data mode we store all the data that we get.  When a 
// 		line is passed to this function, the caller does not free the memory, 
// 		it is up to us.
void PopSession::OnData(char *line)
{
	ASSERT(line != NULL);
	ASSERT(0);
	// there shouldn't be any data.
	
	free(line);
}

//-----------------------------------------------------------------------------
// CJW: When the client has issued the QUIT command... we need to go thru the 
// 		list of messages and actually delete any that have been marked for 
// 		deletion.
void PopSession::OnClose(void)
{
	int i;
	
	ASSERT(_pDB != NULL);
	
	if (_Data.bLoaded == true) {
	
		_pDB->ExecuteNR("BEGIN");
		
		ASSERT((_Data.nMessages > 0 && _Data.pMsgList != NULL) || (_Data.nMessages == 0 && _Data.pMsgList == NULL));
		for (i=0; i<_Data.nMessages; i++) {
			ASSERT(_Data.pMsgList[i].nMsgID > 0);
			if (_Data.pMsgList[i].bDeleted == true) {
				_pDB->ExecuteNR("DELETE FROM Bodies WHERE MessageID=%d", _Data.pMsgList[i].nMsgID);
				_pDB->ExecuteNR("DELETE FROM Messages WHERE MessageID=%d", _Data.pMsgList[i].nMsgID);
				_pDB->ExecuteNR("DELETE FROM Summaries WHERE MessageID=%d", _Data.pMsgList[i].nMsgID);
			}
		}
		
		_pDB->ExecuteNR("COMMIT");
	}	
}






//-----------------------------------------------------------------------------
// CJW: We are going to wait for a USER command.  We will fail if the HELO 
// 		command doesnt have an actual parameter.
void PopSession::ProcessUSER(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("USER received.");
	if (len < 6) {
		_Qout.Print("-ERR Invalid Parameter\r\n");
	}
	else if (_Data.szUser != NULL || _Data.nUserID > 0 ) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
		ASSERT(_Data.szPass == NULL);
		_Data.szUser = (char *) malloc(len);
		ASSERT(_Data.szUser != NULL);
		
		strcpy(_Data.szUser , &ptr[5]);
		_log.Log("User: (%s)", _Data.szUser);
		_Qout.Print("+OK\r\n");
	}
}


//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessPASS(char *ptr, int len)
{
	DpSqlite3Result *pResult;

	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("PASS received.");
	if (len < 6) {
		_Qout.Print("-ERR Invalid Parameter\r\n");
	}
	else if (_Data.szUser == NULL || _Data.szPass != NULL || _Data.nUserID > 0 ) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
		_Data.szPass = (char *) malloc(len);
		ASSERT(_Data.szPass != NULL);
		
		strcpy(_Data.szPass , &ptr[5]);
		_log.Log("PASS: (%s)", _Data.szPass);
		
		pResult = _pDB->Execute("SELECT UserID FROM Users WHERE Account='%q' AND Password='%q'", _Data.szUser, _Data.szPass);
		ASSERT(pResult != NULL);
		if(pResult->NextRow()) {
			_Data.nUserID  = pResult->GetInt("UserID");
		}
		
		if (_Data.nUserID == 0)	{ _Qout.Print("-ERR Invalid account details.\r\n"); }
		else 					{ _Qout.Print("+OK\r\n"); }
		
		ASSERT(_Data.szUser != NULL);
			
		free(_Data.szUser);
		free(_Data.szPass);
			
		_Data.szUser = _Data.szPass = NULL;
	}
}


//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessSTAT(char *ptr, int len)
{

	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("STAT received.");
	if (_Data.nUserID == 0) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
		
		if (_Data.bLoaded == false) {
			LoadMessages();			
		}	
		
		// return the stats.
		_Qout.Print("+OK %d %d\r\n", _Data.nMessages, _Data.nTotalSize);
	}
}


//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessLIST(char *ptr, int len)
{
	int nMsgID;
	int i;

	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("LIST received.");
	if (_Data.nUserID == 0) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
		
		if (_Data.bLoaded == false) {
			LoadMessages();
		}
				
		// get the parameter if one was supplied.
		nMsgID = 0;
		if (len > 5) {
			nMsgID = atoi(&ptr[5]);
		}
		
		ASSERT(nMsgID >= 0);
		if (nMsgID == 0) {
			// return the stats first.
			_Qout.Print("+OK %d messages (%d octets)\r\n", _Data.nMessages, _Data.nTotalSize);
			
			// now go thru the list and provide a line for each one.
			for (i=0; i<_Data.nMessages; i++) {
				if (_Data.pMsgList[i].bDeleted == false) {
					_Qout.Print("%d %d\r\n", i+1, _Data.pMsgList[i].nSize);
				}
			}
			_Qout.Print(".\r\n");
		}
		else {
			if (nMsgID > _Data.nMessages || nMsgID <= 0) {
				_Qout.Print("-ERR no such message.\r\n");
			}
			else {
				ASSERT(nMsgID > 0 && nMsgID <= _Data.nMessages);
				_Qout.Print("+OK %d %d\r\n", nMsgID, _Data.pMsgList[nMsgID-1].nSize);
			}
		}
	}
}




//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessUIDL(char *ptr, int len)
{
	int nMsgID;
	int i;

	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("UIDL received.");
	if (_Data.nUserID == 0) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
		
		if (_Data.bLoaded == false) {
			LoadMessages();
		}
				
		// get the parameter if one was supplied.
		nMsgID = 0;
		if (len > 5) {
			nMsgID = atoi(&ptr[5]);
		}
		
		ASSERT(nMsgID >= 0);
		if (nMsgID == 0) {
			// return the stats first.
			_Qout.Print("+OK\r\n", _Data.nMessages, _Data.nTotalSize);
			
			// now go thru the list and provide a line for each one.
			for (i=0; i<_Data.nMessages; i++) {
				if (_Data.pMsgList[i].bDeleted == false) {
					_Qout.Print("%d %d\r\n", i+1, _Data.pMsgList[i].nMsgID);
				}
			}
			_Qout.Print(".\r\n");
		}
		else {
			if (nMsgID > _Data.nMessages || nMsgID <= 0) {
				_Qout.Print("-ERR no such message.\r\n");
			}
			else {
				ASSERT(nMsgID > 0 && nMsgID <= _Data.nMessages);
				_Qout.Print("+OK %d %d\r\n", nMsgID, _Data.pMsgList[nMsgID-1].nMsgID);
			}
		}
	}
}




//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessRETR(char *ptr, int len)
{
	DpSqlite3Result *pResult;
	char *line;
	int nMsgID;
	int slen;

	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("RETR received.");
	if (_Data.nUserID == 0 || _Data.bLoaded == false) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
			
		// get the parameter must be supplied.
		nMsgID = 0;
		if (len > 5) { nMsgID = atoi(&ptr[5]); }
		
		if (nMsgID > _Data.nMessages || nMsgID <= 0) {
			_Qout.Print("-ERR no such message.\r\n");
		}
		else {
			ASSERT(nMsgID > 0 && nMsgID <= _Data.nMessages);
			_Qout.Print("+OK %d octets\r\n", _Data.pMsgList[nMsgID-1].nSize);
			
			pResult = _pDB->Execute("SELECT Body FROM Bodies WHERE MessageID=%d", _Data.pMsgList[nMsgID-1].nMsgID);
			ASSERT(pResult != NULL);
			while(pResult->NextRow()) {
				line = pResult->GetStr("Body");
				ASSERT(line != NULL);
				slen = strlen(line);
				if (slen > 1024) line[1024] = '\0';
				_Qout.Print("%s\r\n", line);
			}
			delete pResult;

			_Qout.Print(".\r\n");
		}
	}
}


//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessDELE(char *ptr, int len)
{
	int nMsgID;

	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	
	_log.Log("DELE received.");
	if (_Data.nUserID == 0 || _Data.bLoaded == false) {
		_Qout.Print("-ERR Bad sequence of commands.\r\n");
	}
	else {
			
		// get the parameter must be supplied.
		nMsgID = 0;
		if (len > 5) {
			nMsgID = atoi(&ptr[5]);
		}
		
		if (nMsgID > _Data.nMessages || nMsgID <= 0) {
			_Qout.Print("-ERR no such message.\r\n");
		}
		else {
			
			if (_Data.pMsgList[nMsgID-1].bDeleted == true) {
				_Qout.Print("-ERR message already marked as deleted.\r\n");
			}
			else {
				_Data.pMsgList[nMsgID-1].bDeleted = true;
				_Qout.Print("+OK\r\n");
			}
		}
	}
}




//-----------------------------------------------------------------------------
// CJW: 
void PopSession::ProcessNOOP(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	ASSERT(GetState() == Waiting);
	
	_Qout.Print("+OK\r\n");
}


void PopSession::ProcessRSET(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len >= 4);
	ASSERT(GetState() == Waiting);
	
	Reset();
	
	_Qout.Print("+OK\r\n");
}



//-----------------------------------------------------------------------------
// CJW: The client has issued the quit command.  We will quit right now.  This 
// 		will cause the thread to stop processing, and our parent process to 
// 		delete this object (which will clean everything out.
void PopSession::ProcessQUIT(char *ptr, int len)
{
	ASSERT(ptr != NULL);
	ASSERT(len == 4);
	ASSERT(GetState() == Waiting);
	
	ChangeState(Close);
}




//-----------------------------------------------------------------------------
// CJW: Reset the data that we have stored for any messages.  
void PopSession::Reset(void)
{
}


//-----------------------------------------------------------------------------
// CJW: Load the message information into the object.  Once we have 
// 		messageID's, then we know the size of each message and put that in the 
// 		list.
void PopSession::LoadMessages(void)
{
	DpSqlite3Result *pResult;
	int i;

	ASSERT(_Data.nMessages == 0);
	ASSERT(_Data.nTotalSize == 0);
	ASSERT(_Data.pMsgList == NULL);
	_Data.pMsgList = (PopMsgSize *) malloc(sizeof(PopMsgSize) * MAX_POP_ITEMS);
	ASSERT(_Data.pMsgList != NULL);
	
	// get all the messageID's from the database.
	// *** Note, the limit amount here should be in the config file.
	pResult = _pDB->Execute("SELECT MessageID FROM Messages WHERE UserID=%d AND Incoming=1 LIMIT %d", _Data.nUserID, MAX_POP_ITEMS);
	ASSERT(pResult != NULL);
	while(pResult->NextRow()) {
		ASSERT(_Data.nMessages < MAX_POP_ITEMS);
		_Data.pMsgList[_Data.nMessages].nMsgID = pResult->GetInt("MessageID");
		_Data.pMsgList[_Data.nMessages].nSize = 0;
		_Data.pMsgList[_Data.nMessages].bDeleted = false;
		_Data.nMessages++;
	}
	delete pResult;
	
	if (_Data.nMessages == 0) {
		free(_Data.pMsgList);
		_Data.pMsgList = NULL;
	}
	else {
	
		_log.Log("%d Messages found for this user.  Calculating message sizes.", _Data.nMessages);
				
		ASSERT(_Data.nMessages <= MAX_POP_ITEMS);
		for (i=0; i<_Data.nMessages; i++) {
			ASSERT(i < MAX_POP_ITEMS);
			ASSERT(_Data.pMsgList[i].nMsgID > 0);
			ASSERT(_Data.pMsgList[i].nSize == 0);
			pResult = _pDB->Execute("SELECT SUM(LENGTH(Body)) AS Size FROM Bodies WHERE MessageID=%d", _Data.pMsgList[i].nMsgID);
			ASSERT(pResult != NULL);
			if(pResult->NextRow()) {
				_Data.pMsgList[i].nSize = pResult->GetInt("Size");
				_Data.nTotalSize += (_Data.pMsgList[i].nSize + 2);
			}
			delete pResult;
		}
	}
				
	_log.Log("PopSession::LoadMessages: %d Messages found.", _Data.nMessages);
	_Data.bLoaded = true;
}


