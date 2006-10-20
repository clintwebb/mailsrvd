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

#include "Defaults.h"
#include "Logger.h"
#include "PopServer.h"



//-----------------------------------------------------------------------------
// This value can be provided at compile time, but if not, then the default 
// here will be used, which should be more than suitable.
#ifndef CONFIG_DIR
#define CONFIG_DIR "/etc"
#endif

#ifndef LOG_DIR
#define LOG_DIR "/var/log/mailsrvd"
#endif




// Todo.  Need to add this information into the config file.


// The outgoing emails will be sent using the SMTP system.  The difference is that if a RCPT address is received that is not handled by us, it will only be forwarded on, if the user authenticated.  Without authentication, only addresses we control will be accepted.


class theApp : public DpMain
{
	private:
		DpIniFile  *_pIni;
		Logger     *_pLogger;
		PopServer  *_pServer;

	public:
		
		//---------------------------------------------------------------------
		// Everything that happens here is done before main(), so we just want 
		// to initialise our variables.
		theApp()
		{
			_pIni = NULL;
			_pServer = NULL;
			_pLogger = NULL;
		}


		
		//---------------------------------------------------------------------
		// The final peice.  We dont actually want to do anything here, so we 
		// just verify that everything was cleaned up properly.
		virtual ~theApp()
		{
			ASSERT(_pIni == NULL);
			ASSERT(_pServer == NULL);
			ASSERT(_pLogger == NULL);
		}

	protected:
		
		
		//---------------------------------------------------------------------
		// Load the config file and return a true if we could do it.  Otherwise 
		// we will return a false.  We dont need to pull out any information 
		// from the file, we just need to make sure we can load it.
		virtual bool LoadConfig(void)
		{
			bool bLoaded;
			ASSERT(_pIni == NULL);
			
			_pIni = new DpIniFile;
			if (_pIni->Load(CONFIG_DIR "/mailsrv.conf") == false) {
				delete _pIni;
				_pIni = NULL;
				bLoaded = false;
			}
			else {
				bLoaded = true;
			}
			
			return (bLoaded);
		}

		
		
		
		//---------------------------------------------------------------------
		// Check the INI, and if the POP3 module is enabled, then we need to 
		// let the PopServer object handle it from that moment on.
		virtual void StartPop3(void)
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
						
						log.Log("Activating Incoming SMTP Server on port %d.", nPort);
							
						k = 15;
						while(k > 0) {
							pServer = new PopServer;
							ASSERT(pServer != NULL);
							if (pServer->Listen(nPort) == false) {
								delete pServer;
								k--;
								if (k == 0) { log.Log("Unable to listen on socket %d.  Giving Up.", nPort); }
								else {
									log.Log("Unable to listen on socket %d.  Will try again in 30 seconds", nPort);
									sleep(30);
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
		// This is where everything is started up.  So we need to start up data 
		// object which creates a connection with the database.  We also start 
		// the server object which will listen on a parMessageticular port.  Finally, 
		// we will start up the controller object which checks the database and 
		// makes the API request to our sms service host.
		//
		// Once everything is started, then the main thread will idle while the 
		// worker threads continue to function.
		virtual void OnStartup(void)
		{
			Logger log;

			// Create and initialise the logger.

			log.Log("Starting Main Process.");
			
			// initialise the randomiser.
			InitRandom();
			
			// Load the configuration... we will use it later.
			log.Log("Loading Config.");
			if (LoadConfig() == false) {
				log.Log("Failed to load config file: %s%s.", CONFIG_DIR, "/mailsrv.conf");
			}
			else {
			
				// setup and initialise our named-pipes for external integration.
			
				// start the actual listening server.
				StartPop3();
							
				log.Log("Finished Launching Processes.");
			}
		}
		
		
		
		
		
		//---------------------------------------------------------------------
		// When we need to shutdown the server, this function will be run.  
		// When this function exits, the main thread will be closed down.
		virtual void OnShutdown(void)
		{
			Logger log;
			int i;
			
			if (_pServer != NULL) {
				delete _pServer;
				_pServer = NULL;
			}
			
			if (_pIni != NULL) {
				log.Log("Deleting INI object.");
				delete _pIni;
				_pIni = NULL;
			}
			
			log.Log("Shutting Down.");
			log.Close();
		}

		
		
		//---------------------------------------------------------------------
		// If the user or the system wants to shut down the daemon, this 
		// function will be run.  
		virtual void OnCtrlBreak(void)
		{
			printf("OnCtrlBreak();\n");
		}

} myApp;



