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

#include <DpLock.h>

class Logger 
{
	private:
		static DpLock _lock;
		char *_szName;
		int _nID;
		
	public:
		Logger(); 
		virtual ~Logger();
		
		void Log(char *text, ...);
		void SetName(const char *name);
		void SetID(int nID);
		
	protected:
		
	private:
		
};



#endif
