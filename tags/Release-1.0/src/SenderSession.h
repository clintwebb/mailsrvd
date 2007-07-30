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

#ifndef __SENDERSESSION_H
#define __SENDERSESSION_H

//-----------------------------------------------------------------------------
// We are going to use the Session framework because it provides a certain 
// level of functionality that we need.  However it is kind of the oposite of 
// what we need.  So we will be sending commands, and then the reply will come 
// back as a Message that we process.  Therefore we will need to know the last 
// command that we gave, and process the result code that was returned.
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>

#include "Session.h"

#define MAX_ADDRESS	256

struct ServerAddress {
	char szAddress[MAX_ADDRESS];
	int nPriority;
}; 


class SenderSession : public Session
{
	private:
		struct {
			int nMessageID;
			char *szFrom;
			char *szTo;

		} _Data;
		enum {
			Starting,
			Connected,
			SentHelo,
			SentMailFrom,
			SentRcpt,
			SentData,
			SentDot,
			SentQuit
		} _State;
		
		ServerAddress *_pAddressList;
		int _nAddresses;
		
	public:
		SenderSession(); 
		virtual ~SenderSession();
		
		void SendMessage(int nMessageID);
		
	protected:
		virtual void OnStart(void);
		virtual void OnCommand(char *line);
		virtual void OnData(char *line);
		virtual void OnClose(void);
				
	private:
		bool GetServerFromAddress(char *szAddress);
		void Resolve(int nID, char *szDomain);
// 		int GetLocalMX(int nID, char *szDomain);

		void ProcessConnected(int nResult);
		void ProcessHelo(int nResult);
		void ProcessMailFrom(int nResult);
		void ProcessRcpt(int nResult);
		void ProcessData(int nResult);
		void ProcessDot(int nResult);
		void ProcessQuit(int nResult);
};



#endif
