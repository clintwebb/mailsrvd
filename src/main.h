//-----------------------------------------------------------------------------
// Mailsrv-pop3
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
// 
//  This project is intended to be a simple, robust mail server.  It was 
//  developed because of a lack of easy to use mail server software out there.  
//  Most existing software was either too complicated, or two difficult to 
//  configure.  This software uses very simple method for domain addresses, and 
//  mailboxes within addresses.   It is incredibly easy to configure it to 
//  handle multiple mailboxes on multiple domains.
//
//  Additionally some extra functionality has been added.   An extention to the 
//  SMTP protocol that allows emails to be picked up from the sending mail 
//  server by using DNS lookup, rather than receiving email from anyone who 
//  tries to send one.   It is not a measure to reduce spam, however it is a 
//  measure to give a bit more control to the receiving mailserver over when it 
//  will receive emails, and from whom.
//
//-----------------------------------------------------------------------------



#include <DpIniFile.h>
#include <DpMain.h>
#include <DpMySql.h>

#include "Defaults.h"
#include "Logger.h"
#include "Server.h"


//-----------------------------------------------------------------------------
// This value can be provided at compile time, but if not, then the default 
// here will be used, which should be more than suitable.
#ifndef CONFIG_DIR
#define CONFIG_DIR "/etc"
#endif

#ifndef LOG_DIR
#define LOG_DIR "/var/log/mailsrvd"
#endif




class theApp : public DpMain
{
	private:
	
	protected:
		DpIniFile	  *_pIni;
		Logger		  *_pLogger;
		Server		 **_pServers;
		DpMySqlDB     *_pData;

	public:
		
		theApp();
		virtual ~theApp();

	protected:
		
		virtual bool LoadConfig(void);
		virtual void OnStartup(void);
		virtual void OnShutdown(void);
		virtual void OnCtrlBreak(void);

		virtual void OnStarted(void) = 0;
} ;



