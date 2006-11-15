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

#ifndef __SERVER_H
#define __SERVER_H

// Abstraction object.  This object is placed in case we end up having some common functionality we want to keep active.

#include <DpLock.h>
#include <DpServerInterface.h>

#include "Session.h"
#include "Logger.h"
#include "DataModel.h"


class Server : public DpServerInterface 
{
	private:
		DpLock _lock;
		int _nItems;
		Session **_pList;
		DataModel *_pData;
		
	protected:
		Logger _log;
		
	public:
		Server(); 
		virtual ~Server();
		
		virtual bool Listen(int nPort);
		void AttachData(DataModel *pData);
		
	protected:
		virtual void OnAccept(SOCKET nSocket) = 0;
		virtual void AddSession(Session *ptr);
		
	private:
		
};



#endif
