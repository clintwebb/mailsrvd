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


#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "Logger.h"

//-----------------------------------------------------------------------------
DpLock Logger::_lock;


//-----------------------------------------------------------------------------
Logger::Logger() 
{	
	_lock.Lock();
	_szName = "mailsrvd";
	_nID = 0;
	_lock.Unlock();
}
		
//-----------------------------------------------------------------------------
Logger::~Logger() 
{
}


void Logger::Init()
{
	
}



//-----------------------------------------------------------------------------
void Logger::Log(char *text, ...)
{
	va_list ap;
	char *buffer;

	ASSERT(text != NULL);
	ASSERT(_szName != NULL);

	buffer = (char *) malloc(32767);
	if (buffer != NULL) {
		va_start(ap, text);
		vsprintf(buffer, text, ap);
		va_end(ap);
		
		_lock.Lock();
		printf("%s(%d): %s\n", _szName, _nID, buffer);
		_lock.Unlock();
		
		free(buffer);
		
// 		sleep(1);
	}
}


//-----------------------------------------------------------------------------
void Logger::SetName(const char *name)
{
	ASSERT(name != NULL);
	_lock.Lock();
	_szName = (char *) name;
	_lock.Unlock();
}


//-----------------------------------------------------------------------------
void Logger::SetID(int nID)
{
	ASSERT(nID > 0);
	_lock.Lock();
	_nID = nID;
	_lock.Unlock();
}


// close the log so that no other operations will write to it.  Should be in a state as if it was a newly created object.
void Logger::Close(void)
{
	
}
