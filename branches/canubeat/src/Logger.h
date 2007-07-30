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

#ifndef __LOGGER_H
#define __LOGGER_H


#include <stdio.h>
#include <string.h>

#include <DpLogger.h>
#include <DpLock.h>


class Logger 
{
	private:
		static DpLock _lock;
		static DpLogger *_pLogger;
		
		char         *_szName;
		int           _nID;
		char		 *_szBuffer;
		
	public:
		Logger(); 
		virtual ~Logger();
		
		void Init();
		virtual void Log(char *text, ...);
		virtual void LogStr(char *text);
		void SetName(const char *name);
		void SetID(int nID);
		
		void Close();
		
	protected:
		
	private:
		
};



#endif
