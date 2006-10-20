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

#include "PopServer.h"
#include "PopSession.h"

PopServer::PopServer() 
{	
	_log.SetName("PopServer");
	_log.Log("PopServer()");
}
		
PopServer::~PopServer() 
{
	_log.Log("~PopServer()");
}

//-----------------------------------------------------------------------------
// CJW: A new connection has arrived.  We need to create an object to handle 
// 		it, and add it to the list of objects.
void PopServer::OnAccept(SOCKET nSocket)
{
	PopSession *pSession;
	
	ASSERT(nSocket > 0);
	
	_log.Log("Connection received on socket %d.", nSocket);
		
	pSession = new PopSession;
	ASSERT(pSession != NULL); 
	pSession->Accept(nSocket);
	AddSession(pSession);
}







