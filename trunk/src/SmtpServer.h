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

#ifndef __SMTPSERVER_H
#define __SMTPSERVER_H


#include <stdio.h>
#include <string.h>

#include <DevPlus.h>

#include "Server.h"


class SmtpServer : public Server
{
	private:
		
	public:
		SmtpServer(); 
		virtual ~SmtpServer();
		
	protected:
		virtual void OnAccept(SOCKET nSocket);
		virtual bool OnObjectDelete(DpThreadObject *pObject);
		
	private:
		
};



#endif
