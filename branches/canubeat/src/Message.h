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

#ifndef __MESSAGE_H
#define __MESSAGE_H




#include <stdio.h>
#include <string.h>

#include "Session.h"
#include "Defaults.h"

class Message : public Session
{
	private:
		struct {
			char *szHelo;
			char *szFrom;
			int nUserID[MAX_LOCAL_ADDRESSES];
			int nUsers;
			char *szRemote[MAX_REMOTE_ADDRESSES];
			int nRemote;
			char **szBodies;
			int nBodies;
			struct {
				bool bAuthenticating;
				bool bAuthenticated;
			
				char *szUser;
				char *szPass;
				int  nUserID;
			} auth;
		} _Data;
		
	public:
		Message(); 
		virtual ~Message();
		
	protected:
		virtual void OnStart(void);
		virtual void OnCommand(char *line);
		virtual void OnData(char *line);
		virtual void OnClose(void);
				
	private:
		void AddUser(int nUserID);
		void SaveMessage(void);
		void Reset(void);
		
		void ProcessHELO(char *ptr, int len);
		void ProcessEHLO(char *ptr, int len);
		void ProcessAUTH(char *ptr, int len);
		void ProcessMAIL(char *ptr, int len);
		void ProcessRCPT(char *ptr, int len);
		void ProcessDATA(char *ptr, int len);
		void ProcessRSET(char *ptr, int len);
		void ProcessNOOP(char *ptr, int len);
		void ProcessQUIT(char *ptr, int len);
};



#endif
