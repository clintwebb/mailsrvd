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

#ifndef __POPSERVER_H
#define __POPSERVER_H


#include "Server.h"

class PopServer : public Server 
{
	private:
		
	public:
		PopServer(); 
		virtual ~PopServer();
		
	protected:
		virtual void OnAccept(SOCKET nSocket);
		
	private:
		
};



#endif
