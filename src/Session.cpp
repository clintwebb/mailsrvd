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

#include <stdio.h>
#include <stdlib.h>

#include "Session.h"

#define PACKET_SIZE		3000
// #define THREAD_CYCLE	50

//-----------------------------------------------------------------------------
Session::Session() 
{	
	ASSERT(sizeof(int) == sizeof(SOCKET));
	
	_log.SetName("Session");
	_log.Log("Session()");
	
	_lock.Lock();
	_pSocket = NULL;
	ChangeState(Starting);
	_pDB = NULL;
	_nIdleCount = 0;
	_lock.Unlock();
}
		
//-----------------------------------------------------------------------------
// CJW: Deconstructor.   Clean up everything before this object is destroyed.
Session::~Session() 
{
	_log.Log("~Session() - Start");
	
	if (GetState() != Done) {
		_log.Log("~Session() - Setting state to ForceClose.  Currently is %d", GetState());
		ChangeState(ForceClose);
	}
	_log.Log("~Session() - Waiting for thread to exit.");
	WaitForThread();
	
	ASSERT(_pSocket == NULL);
	if (_pSocket != NULL) {
		_log.Log("~Session() - deleting socket object");
		delete _pSocket;
		_pSocket = NULL;
	}
	
	if (_pDB != NULL) {
		_log.Log("~Session() - deleting database object");
		delete _pDB;
		_pDB = NULL;
	}
	
	_log.Log("~Session() - End");
}


void Session::OpenDB(void)
{
	ASSERT(_pDB == NULL);
	
	_log.Log("Creating Database Handler.");
	_pDB = new DpSqlite3;
	ASSERT(_pDB != NULL);
	if (_pDB->Open("/data/mail/db/mailsrv.db") == false) {
		_log.Log("Unable to load database.");
		delete _pDB;
		ChangeState(ForceClose);
	}
}



//-----------------------------------------------------------------------------
// CJW: Accept the socket, and then start the thread so that it can process the 
// 		data.  We will need to receive all the data from the client, validate 
// 		it against the information in the database.  The data we receive will 
// 		need to be put in a data queue.  We will need to process the commands 
// 		that are received from the client, and a state engine will be needed to 
// 		keep track of the mode that we need to process.
void Session::Accept(SOCKET nSocket)
{
	ASSERT(nSocket > 0);
	
	ASSERT(GetState() == Starting);
	
	_log.SetID(nSocket);
	_log.Log("New Connection Received.");
	
	ASSERT(_pSocket == NULL);
	_pSocket = new DpSocket;
	ASSERT(_pSocket != NULL);
	_pSocket->Accept(nSocket);
	
	Start();
}


//-----------------------------------------------------------------------------

void Session::OnThreadRun(void)
{
	char *ptr;
	ChangeState(Started);
	
	OpenDB();
	
	_log.Log("Ready");
	
	ASSERT(GetState() >= Started);
	
	while (GetState() != Done) {
		// First we check to see if there is any incoming data that we need to add to the data queue.
		if (_pSocket != NULL) ReceiveData();
		
		// Secondly, we send any data that needs to be sent out.
		if (_pSocket != NULL) SendData();
		
		switch (GetState()) {				

			case Starting:
				// should not be in this state at this point.
				ASSERT(GetState() != Starting);		
				ChangeState(ForceClose);
				break;
				
			case Started:
				// The connection has just started.  We need to send an initial message to the client.  We need to call this function in an Unlocked state so that this function can take some time if it wants.  It may need to establish a connection to something.  If this gets held up, the parent object could get paused when checking to see if this object is done.  All other functions should process quickly enough where this doesnt matter.
				OnStart();
				ChangeState(Waiting);
				break;
			
			case Waiting:
				ptr = _Qin.GetLine();
				if (ptr != NULL) {
					OnCommand(ptr);
					free(ptr);
					OnBusy();
				}
				else {
					OnIdle();
				}
				break;
				
			case Data:
				// almost the same as a command, except we do not free the memory used by the line. We let the child object do that.
				ptr = _Qin.GetLine();
				if (ptr != NULL) {
					OnData(ptr);
					OnBusy();
				}
				else {
					OnIdle();
				}
				break;
				
			case ForceClose:
				// The socket closed before we were expecting it.  if we have 
				// saved information to the database, we should delete it.  
				// Additionally, if the socket object hasnt actually been 
				// destroyed, then we should delete it now.
				
				if (_pSocket != NULL) {
					delete _pSocket;
					_pSocket = NULL;
				}
	
				if (_pDB != NULL) {
					delete _pDB;
					_pDB = NULL;
				}
				
				ChangeState(Done);
				break;
				
			case Close:
				_log.Log("Session Closing");
				ASSERT(_pSocket != NULL);
				if (_Qout.Length() == 0) {
					
					OnClose();
					
					delete _pSocket;
					_pSocket = NULL;
					
					if (_pDB != NULL) {
						delete _pDB;
						_pDB = NULL;
					}
					
					ChangeState(Done);
					_log.Log("Session Closed");
					
				}	
				else {
					_log.Log("Sending %d chars first", _Qout.Length());
				}
				break;
			
			case Done:
				// we must stop the thread.
				ASSERT(_pSocket == NULL);
				break;
				
			default:
				break;
		}
	}
}


//-----------------------------------------------------------------------------
// Whenever the session has nothing to do, then this function will be called.  
// Its purpose is to increment a counter so that we can close the session if it 
// stays idle for too long.  Also, it will sleep for a short time to allow 
// other threads to process.
void Session::OnIdle(void)
{
	_nIdleCount += 50;
	if (_nIdleCount >= IDLE_LIMIT) {
		ChangeState(Close);
	}
	else {
		Sleep(50);
	}
}

//-----------------------------------------------------------------------------
// When the session does something, rather than nothing, this function is 
// called so that we can reset the counter that is used to determine if a 
// connection has been idle too long.
void Session::OnBusy(void)
{
	_nIdleCount	= 0;
}


//-----------------------------------------------------------------------------
// CJW: Receive data from the socket and put it in the data queue.  Return true 
// 		if we actually got data from the socket.
void Session::ReceiveData(void)
{
	char tmp[PACKET_SIZE];
	int len;
	
	ASSERT(_pSocket != NULL);
	
	len = _pSocket->Receive(tmp, PACKET_SIZE);
	if (len < 0) {
		ChangeState(ForceClose);
		delete _pSocket;
		_pSocket = NULL;
		_log.Log("Connection closed while Receiving.");
	}
	else if (len > 0) {
		_Qin.Add(tmp, len);
	}
}


//-----------------------------------------------------------------------------
// CJW: Send data that we have in our outgoing queue.
void Session::SendData(void)
{
	int len, sent;
	char *ptr;
	
	ASSERT(_pSocket != NULL);
	
	len = _Qout.Length();
	if (len > 0) {
		ptr = _Qout.Pop(len);
		ASSERT(ptr != NULL);
		sent = _pSocket->Send(ptr, len);
		if (sent < 0) {
			ChangeState(ForceClose);
			delete _pSocket;
			_pSocket = NULL;
			_log.Log("Connection closed while Sending.");
		}
		else if (sent < len) {
			_Qout.Push(&ptr[sent], len-sent);
			_log.Log("Transport saturated (%d,%d).  Sleeping.", len, sent);
			Sleep(500);
		}
		
		free(ptr);
	}
}



//-----------------------------------------------------------------------------
// CJW: Connect to the specified address and port.  If we manage to connect, 
// 		then we will return a true, otherwise we will return a false.
bool Session::Connect(char *ip, int port)
{
	bool bConnected = false;
	DpSocket *pSocket;
	
	ASSERT(ip != NULL && port > 0);
	
	ASSERT(_pSocket == NULL);
	pSocket = new DpSocket;
	ASSERT(pSocket != NULL);
	
	if (pSocket->Connect(ip, port) == true) {
		_pSocket = pSocket;
		bConnected = true;
	}
	else {
		delete pSocket;
	}
	
	return(bConnected);
}
