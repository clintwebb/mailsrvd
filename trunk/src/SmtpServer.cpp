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

#include "SmtpServer.h"
#include "Message.h"

//-----------------------------------------------------------------------------
SmtpServer::SmtpServer() 
{	
	_log.SetName("SmtpServer");
	_log.Log("SmtpServer()");
}
		
//-----------------------------------------------------------------------------
SmtpServer::~SmtpServer() 
{
	_log.Log("~SmtpServer()");
}



//-----------------------------------------------------------------------------
// CJW: A new connection has arrived.  We need to create an object to handle 
// 		it, and add it to the list of objects.
void SmtpServer::OnAccept(SOCKET nSocket)
{
	Message *pMsg;
	int n, s;
	
	ASSERT(nSocket > 0);
	_log.Log("Connection received on socket %d.", nSocket);
	pMsg = new Message; 
	ASSERT(pMsg != NULL);
	ASSERT(_pData != NULL);
	pMsg->AttachData(_pData);
	pMsg->Accept(nSocket);
	AddSession(pMsg);
	n = ActiveSessions();
	s = 1000 + (n * 20);
	_log.Log("SMTP: Active Sessions:%d, Sleeping:%d", n, s);
	Sleep(s);
}

