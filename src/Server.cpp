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
	_lock.Lock();
	_log.SetName("Server");
	_log.Log("Server()");
	_nItems = 0;
	_pList = NULL;
	_pData = NULL;
	_lock.Unlock();
}
		
//-----------------------------------------------------------------------------
// CJW: When the server object is being destroyed, we also need to destroy any 
// 		session that is still active also.
Server::~Server() 
{
// 	WaitForThread();
	
	_lock.Lock();
	_log.Log("~Server() - Deleting Sessions");
	ASSERT((_nItems == 0 && _pList == NULL) || (_nItems > 0 && _pList != NULL));
	while (_nItems > 0) {
		_nItems --;
		if (_pList[_nItems] != NULL) {
			delete _pList[_nItems];
			_pList[_nItems] = NULL;
		}
	}
	
	if (_pList != NULL) {
		free(_pList);
		_pList = NULL;
	}
	
	_log.Log("~Server()");
	_lock.Unlock();
}


void Server::AttachData(DataModel *pData)
{
	ASSERT(pData != NULL);
	_lock.Lock();
	ASSERT(_pData == NULL);
	_pData = pData;
	_lock.Unlock();
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



//---------------------------------------------------------------------

void Server::AddSession(Session *ptr)
{
	int i;
	bool bFound;
	
	ASSERT(ptr != NULL);
	
	_lock.Lock();
	_log.Log("AddSession() - Start");
	ASSERT((_nItems == 0 && _pList == NULL) || (_nItems > 0 && _pList != NULL));
	
	// first go thru the list and see if there is an empty spot there.  This is where we do our actual list cleanup.
	
	for(i=0; i<_nItems; i++) {
		if (_pList[i] != NULL) {
			if (_pList[i]->IsDone() != false) {
				_log.Log("AddSession(i=%d) - Found a session that is complete. Deleting.", i);
				delete _pList[i];
				_pList[i] = NULL;
			}
			else {
				_log.Log("AddSession(i=%d) - Found a session that is not complete yet.", i);
			}
		}
	}
	
	bFound = false;
	for(i=0; i<_nItems && bFound == false; i++) {
		if (_pList[i] == NULL) {
			_log.Log("AddSession(i=%d) - Found a vacant slot", i);
			_pList[i] = ptr;
			bFound = true;
		}
	}
	
	if (bFound == false) {
		_log.Log("AddSession() - Didn't find a slot, creating a new one.  nItems=%d", _nItems);
		_pList = (Session **) realloc(_pList, sizeof(Session *) * (_nItems+1));
		ASSERT(_pList != NULL);
		_pList[_nItems] = ptr;
		_nItems++;
	}
	
	ASSERT((_nItems == 0 && _pList == NULL) || (_nItems > 0 && _pList != NULL));
	_log.Log("AddSession() - Done (nItems=%d)", _nItems);
	
	_lock.Unlock();
}




