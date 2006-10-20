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
#include <DpSqlite3.h>
#include <DpDataQueue.h>
#include <DpThreadObject.h>

#include "Logger.h"
#include "Defaults.h"

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
		DpLock _lock;
		DpSocket  *_pSocket;
		enum State_e _SessionState;
		int _nIdleCount;
		
	protected:
		DpSqlite3  *_pDB;
		DpDataQueue _Qin, _Qout;
		Logger _log;
		
	public:
		Session(); 
		virtual ~Session();
		
		void Accept(SOCKET nSocket);
		bool IsDone(void)
		{
			bool b;
			_lock.Lock();
			if (_SessionState == Done)	{ b = true; }
			else 						{ b = false; }
			_lock.Unlock();
	
			return (b);
		}

		
		bool Connect(char *ip, int port);
		
	protected:
		virtual void OnThreadRun();
		virtual void OnIdle(void);
		virtual void OnBusy(void);
		
		virtual void OpenDB(void);
		
		virtual void OnStart(void)			= 0;
		virtual void OnCommand(char *line)	= 0;
		virtual void OnData(char *line)		= 0;
		virtual void OnClose(void)			= 0;
		
		void ChangeState(State_e state)
		{
			_lock.Lock();
			_SessionState = state;
			_lock.Unlock();
		}
		
		State_e GetState(void)
		{
			State_e e;
			_lock.Lock();
			e = _SessionState;
			_lock.Unlock();
			
			return(e);
		}

		
		
	private:
		void ReceiveData(void);
		void SendData(void);
		
		
};



#endif
