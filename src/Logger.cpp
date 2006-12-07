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


#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "Logger.h"
#include "Defaults.h"

//-----------------------------------------------------------------------------
DpLock Logger::_lock;
DpLogger *Logger::_pLogger = NULL;

//-----------------------------------------------------------------------------
Logger::Logger() 
{	
	_szName = "mailsrvd";
	_nID = 0;
	_szBuffer = (char *) malloc(32767);
	ASSERT(_szBuffer != NULL);
}
		
//-----------------------------------------------------------------------------
Logger::~Logger() 
{
	ASSERT(_szBuffer != NULL);
	if (_szBuffer != NULL) {
		free(_szBuffer);
		_szBuffer = NULL;
	}
}


void Logger::Init()
{
	_lock.Lock();
	ASSERT(_pLogger == NULL);
	_pLogger = new DpLogger;
	ASSERT(_pLogger != NULL);
	
	_pLogger->Init("/var/log/mailsrvd", 4);
	_pLogger->SetTimestamp();
	_lock.Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Log a string.  Since we are getting only a string of text, and we dont 
// 		have to parse it, it should be a bit quicker (and less dangerous).
void Logger::LogStr(char *text)
{
	ASSERT(text != NULL);
	ASSERT(_szName != NULL);
	
	_lock.Lock();
	ASSERT(_pLogger != NULL);
	_pLogger->Log("%s(%d): %s", _szName, _nID, _szBuffer);
	_lock.Unlock();
}


//-----------------------------------------------------------------------------
void Logger::Log(char *text, ...)
{
	va_list ap;

	ASSERT(text != NULL);

	va_start(ap, text);
	vsprintf(_szBuffer, text, ap);
	va_end(ap);
		
	LogStr(_szBuffer);
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
	ASSERT(_pLogger != NULL);
	delete _pLogger;
	_pLogger = NULL;
	_lock.Unlock();	
}
