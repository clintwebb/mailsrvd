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
#include "Defaults.h"

//-----------------------------------------------------------------------------
DpLock Logger::_lock;
FILE *Logger::_fp = NULL;
int Logger::_nLength = 0;

//-----------------------------------------------------------------------------
Logger::Logger() 
{	
	_lock.Lock();
	_szName = "mailsrvd";
	_nID = 0;
	_szBuffer = (char *) malloc(32767);
	ASSERT(_szBuffer != NULL);
	_lock.Unlock();
}
		
//-----------------------------------------------------------------------------
Logger::~Logger() 
{
	_lock.Lock();
	ASSERT(_szBuffer != NULL);
	if (_szBuffer != NULL) {
		free(_szBuffer);
		_szBuffer = NULL;
	}
	_lock.Unlock();
}


void Logger::Init()
{
	_lock.Lock();
	unlink("/var/log/mailsrvd.4");
	rename("/var/log/mailsrvd.3", "/var/log/mailsrvd.4");
	rename("/var/log/mailsrvd.2", "/var/log/mailsrvd.3");
	rename("/var/log/mailsrvd.1", "/var/log/mailsrvd.2");
	rename("/var/log/mailsrvd",   "/var/log/mailsrvd.1");
	
	_fp = fopen("/var/log/mailsrvd", "a");
	ASSERT(_fp);
	_nLength = 0;
	_lock.Unlock();
}



//-----------------------------------------------------------------------------
void Logger::Log(char *text, ...)
{
	va_list ap;

	ASSERT(text != NULL);
	ASSERT(_szName != NULL);

	va_start(ap, text);
	vsprintf(_szBuffer, text, ap);
	va_end(ap);
		
	_lock.Lock();
	ASSERT(_fp != NULL && _nLength >= 0);
	_nLength += fprintf(_fp, "%s(%d): %s\n", _szName, _nID, _szBuffer);
	fflush(_fp);
	
	ASSERT(_nLength > 0);
	if (_nLength >= MAX_LOG_LENGTH) {
		fclose(_fp);
		_fp = NULL;
	
		Init();
		ASSERT(_nLength == 0);
	}
	
	_lock.Unlock();
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
	_lock.Lock();
	ASSERT(_fp != NULL);
	fclose(_fp);
	_fp = NULL;
	_lock.Unlock();	
}
