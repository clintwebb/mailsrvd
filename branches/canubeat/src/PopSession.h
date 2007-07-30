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

#ifndef __POPSESSION_H
#define __POPSESSION_H




#include <stdio.h>
#include <string.h>

#include "Session.h"
#include "Defaults.h"



struct PopMsgSize {
	int nMsgID;
	int nSize;
	bool bDeleted;
};

class PopSession : public Session
{
	private:
		struct {
			char *szUser;
			char *szPass;
			int nUserID;
			bool bLoaded;
			int nMessages;
			PopMsgSize *pMsgList;
			int nTotalSize;
		} _Data;
		
	public:
		PopSession(); 
		virtual ~PopSession();
		
	protected:
		virtual void OnStart(void);
		virtual void OnCommand(char *line);
		virtual void OnData(char *line);
		virtual void OnClose(void);
				
	private:
		void LoadMessages(void);
		void Reset(void);
		
		void ProcessUSER(char *ptr, int len);
		void ProcessPASS(char *ptr, int len);
		
		void ProcessSTAT(char *ptr, int len);
		void ProcessLIST(char *ptr, int len);
		void ProcessUIDL(char *ptr, int len);
		void ProcessRETR(char *ptr, int len);
		void ProcessDELE(char *ptr, int len);
		
		void ProcessQUIT(char *ptr, int len);
		void ProcessNOOP(char *ptr, int len);
		void ProcessRSET(char *ptr, int len);
};



#endif
