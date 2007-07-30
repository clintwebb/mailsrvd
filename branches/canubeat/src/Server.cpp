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

#include "Server.h"

//-----------------------------------------------------------------------------
// CJW: Constructor.  This abstract class adds functionality to keep a list of 
// 		all the sessions that we have started so that we can remove them when 
// 		they are ready to be deleted.  Sometimes the idea of a garbage 
// 		collector is pretty nice.
Server::Server() 
{	
	Lock();
	_log.SetName("Server");
	_log.Log("Server()");
	_pData = NULL;
	Unlock();
}
		
//-----------------------------------------------------------------------------
// CJW: When the server object is being destroyed, we also need to destroy any 
// 		session that is still active also.
Server::~Server() 
{
// 	WaitForThread();
	
	Lock();
	_log.Log("~Server()");
	Unlock();
}


void Server::AttachData(DataModel *pData)
{
	ASSERT(pData != NULL);
	Lock();
	ASSERT(_pData == NULL);
	_pData = pData;
	Unlock();
}

//-----------------------------------------------------------------------------
// CJW: We over-ride the virtual function so taht we can update our logger 
// 		object to let it know what port we are listening on.  We then pass 
// 		control back to the parent object anyway.
bool Server::Listen(int nPort)
{
	ASSERT(nPort > 0);
	_log.SetID(nPort);
	return(DpServerInterface::Listen(nPort));
}


//-----------------------------------------------------------------------------
// CJW: 
bool Server::OnObjectDelete(DpThreadObject *pObject)
{
	_log.Log("Server::OnObjectDelete");
	return(true);
}


//-----------------------------------------------------------------------------
// CJW: If the server object attempts to accept an incoming socket, but fails, 
// 		we will display a log.  Mostly this is in place to try and resolve 
// 		handle leaks.
void Server::OnAcceptFail(void)
{
// 	_log.Log("Server::OnAcceptFail");
}
