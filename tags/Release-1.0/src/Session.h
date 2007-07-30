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

#ifndef __Session_H
#define __Session_H


#include <DpLock.h>
#include <DpSocket.h>
#include <DpMySql.h>
#include <DpDataQueue.h>
#include <DpThreadObject.h>

#include "Logger.h"
#include "Defaults.h"
#include "DataModel.h"

enum State_e {
			Starting,
			Started,
			Waiting,
			Data,
			ForceClose,
			Close,
			Done
};

class Session : public DpThreadObject 
{
	private:
		DpSocket    *_pSocket;
		enum State_e _SessionState;
		int          _nIdleCount;
		char 		 _pBuffer[PACKET_SIZE + 1];
		static int   _nNextSessionID;
		static DpLock _BigLock;
		
	protected:
		DataModel   *_pData;
		DpDataQueue  _Qin, _Qout;
		Logger       _log;
		
	public:
		Session(); 
		virtual ~Session();
		
		void Accept(SOCKET nSocket);
		bool IsDone(void)
		{
			bool b;
			Lock();
			if (_SessionState == Done)	{ b = true; }
			else 						{ b = false; }
			Unlock();
	
			return (b);
		}

		virtual void AttachData(DataModel *pData) {
		    ASSERT(pData != NULL);
		    Lock();
			ASSERT(_pData == NULL);
			_pData = pData;
			Unlock();
		}

		
		bool Connect(char *ip, int port);
		
	protected:
		virtual void OnThreadRun();
		virtual void OnIdle(void);
		virtual void OnBusy(void);
		
		
		virtual void OnStart(void)			= 0;
		virtual void OnCommand(char *line)	= 0;
		virtual void OnData(char *line)		= 0;
		virtual void OnClose(void)			= 0;
		
		void ChangeState(State_e state);
		State_e GetState(void);
		

		
		
	private:
		void ReceiveData(void);
		void SendData(void);
		
		
};



#endif
