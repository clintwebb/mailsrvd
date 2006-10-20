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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <DpIniFile.h>
#include <DpMain.h>
#include <DpTextTools.h>

#include "Defaults.h"
#include "Logger.h"
#include "PopServer.h"
#include "main.h"


//-----------------------------------------------------------------------------
// This value can be provided at compile time, but if not, then the default 
// here will be used, which should be more than suitable.
#ifndef CONFIG_DIR
#define CONFIG_DIR "/etc"
#endif

#ifndef LOG_DIR
#define LOG_DIR "/var/log/mailsrvd"
#endif




class theAppPop3 : public theApp
{
	private:

	public:
		
		//---------------------------------------------------------------------
		// Everything that happens here is done before main(), so we just want 
		// to initialise our variables.
		theAppPop3()
		{
		}


		
		//---------------------------------------------------------------------
		// The final peice.  We dont actually want to do anything here, so we 
		// just verify that everything was cleaned up properly.
		virtual ~theAppPop3()
		{
		}

	protected:
		
		
		//---------------------------------------------------------------------
		// Check the INI, and if the POP3 module is enabled, then we need to 
		// let the PopServer object handle it from that moment on.
		virtual void OnStarted(void)
		{
			Logger log;
			PopServer *pServer;
			DpTextTools text;
			char *szPorts;
			char **szList;
			int nPort;
			int i,j,k;
			
			ASSERT(_pIni != NULL);
			
			if (_pIni->SetGroup("pop3") == false) {
				log.Log("'pop3' section in config file is missing.\n");
			}
			else {
			
				if (_pIni->GetValue("ports", &szPorts) == true) {
					ASSERT(szPorts != NULL);
					text.Load(szPorts);
					szList = text.GetWordArray();
					
					i=0; j=0;
					while(szList[i] != NULL) {
						
						nPort = atoi(szList[i]);
						ASSERT(nPort > 0);
						
						log.Log("Activating Incoming POP3 Server on port %d.", nPort);
							
						k = BUSY_SOCKET_TRIES;
						while(k > 0) {
							pServer = new PopServer;
							ASSERT(pServer != NULL);
							if (pServer->Listen(nPort) == false) {
								delete pServer;
								k--;
								if (k == 0) { log.Log("Unable to listen on socket %d.  Giving Up.", nPort); }
								else {
									log.Log("Unable to listen on socket %d.  Will try again in 30 seconds", nPort);
									sleep(BUSY_SOCKET_PAUSE);
								}
							}
							else {
								_pServers = (Server **) realloc(_pServers, sizeof(Server *) * (j+2));
								ASSERT(_pServers != NULL);
								_pServers[j] = pServer;
								j++;
								_pServers[j] = NULL;
								k = 0;
							}
						}
						
						i++;
					}
					
					free(szPorts);
				}
			}
		}
		
		
		
		//---------------------------------------------------------------------
		// When we need to shutdown the server, this function will be run.  
		// When this function exits, the main thread will be closed down.
		virtual void OnShutdown(void)
		{
			// do anything we need to do to cleanup here.
		
			theApp::OnShutdown();
		}

} myApp;



