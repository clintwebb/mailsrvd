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

#include <DevPlus.h>

#include "Defaults.h"
#include "Logger.h"
// #include "Server.h"
#include "SmtpServer.h"
#include "PopServer.h"
#include "SenderSession.h"


//-----------------------------------------------------------------------------
// This value can be provided at compile time, but if not, then the default 
// here will be used, which should be more than suitable.
#ifndef CONFIG_DIR
#define CONFIG_DIR "/etc"
#endif


// ---
// The main instance is basically a launcher for forked copies that do the different things.   First it loads in the .ini file and determines what to fork.   This way it can also fork multiple servers to listen on multiple ports.

// Todo.  Need to add this information into the config file.


// The outgoing emails will be sent using the SMTP system.  The difference is that if a RCPT address is received that is not handled by us, it will only be forwarded on, if the user authenticated.  Without authentication, only addresses we control will be accepted.


class theApp : public DpMain
{
	private:
		DpIniFile *_pIni;
		Server **_pServers;
		int _nOutTimerID;

	public:
		
		//---------------------------------------------------------------------
		// Everything that happens here is done before main(), so we just want 
		// to initialise our variables.
		theApp()
		{
			_pIni = NULL;
			_pServers = NULL;
			_nOutTimerID = 0;
		}


		
		//---------------------------------------------------------------------
		// The final peice.  We dont actually want to do anything here, so we 
		// just verify that everything was cleaned up properly.
		virtual ~theApp()
		{
			ASSERT(_pIni == NULL);
			ASSERT(_pServers == NULL);
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
		// Check the INI, and if the SMTP module is enabled, then we need to 
		// fork, and let the SmtpServer object handle it from that moment on.
		virtual void StartSMTP(void)
		{
			Logger log;
			SmtpServer *pServer;
			DpTextTools text;
			char *szPorts;
			char **szList;
			int nPort;
			int i,j,k;
			
			ASSERT(_pIni != NULL);
			
			if (_pIni->SetGroup("smtp") == false) {
				log.Log("'smtp' section in config file is missing.\n");
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
							pServer = new SmtpServer;
							ASSERT(pServer != NULL);
							if (pServer->Listen(nPort) == false) {
								delete pServer;
								k--;
								if (k == 0) { log.Log("Unable to listen on socket %d. Giving up. ", nPort); }
								else { 
									log.Log("Unable to listen on socket %d. Will try again in 30 seconds", nPort); 
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
		// This will setup the part that sends the emails out.  It is not the 
		// same as our SMTP and POP3 servers, because here we basically setup 
		// a timer, and that is all.  The timer will fire every so often and 
		// it will do its work there.
		virtual void StartOut(void)
		{
			DpSqlite3 *pDB;
			Logger log;
			
			log.SetName("StartOut");
			
			pDB = new DpSqlite3;
			ASSERT(pDB != NULL);
			if (pDB->Open("/data/mail/db/mailsrv.db") == false) {
				log.Log("Unable to load database.");
			}
			else {
				// All the messages that were incomplete, should now be restarted.
				log.Log("Resetting outgoing messages that are incomplete");
				pDB->ExecuteNR("UPDATE Messages SET Incoming=0 WHERE Incoming>1");
				
				// start the timer that will fire every 10 seconds.
				_nOutTimerID = SetTimer(10000);
				ASSERT(_nOutTimerID > 0);
			}
			delete pDB;
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

			log.Log("Starting Main Process.");
			
			InitRandom();
			
			log.Log("Loading Config.");
			if (LoadConfig() == false) {
				log.Log("Failed to load config file: %s%s.", CONFIG_DIR, "/mailsrv.conf");
			}
			else {
			
				StartSMTP(); 
				StartPop3();
				StartOut();
							
				log.Log("Finished Launching Processes.");
			}
		}
		
		
		
		virtual void OnTimer(int nTimerID) 
		{
			DpSqlite3 *pDB;
			Logger log;
			DpSqlite3Result *pResult;
			int nMessageID;
			SenderSession *pSession;
			
			log.SetName("main::OnTimer");
			
			ASSERT(_nOutTimerID > 0);
			ASSERT(nTimerID > 0);
			
			if (nTimerID == _nOutTimerID) {
			
				log.SetName("main::OnTimer(OUT)");
				
				pDB = new DpSqlite3;
				ASSERT(pDB != NULL);
				if (pDB->Open("/data/mail/db/mailsrv.db") == false) {
					log.Log("Unable to load database.");
				}
				else {
			
					pResult = pDB->Execute("SELECT MessageID FROM Messages WHERE Incoming=0");
					ASSERT(pResult != NULL);
					while(pResult->NextRow()) {
						nMessageID = pResult->GetInt("MessageID");
						log.Log("Found message to send: %d", nMessageID);
						ASSERT(nMessageID > 0);
					
						pDB->ExecuteNR("UPDATE Messages SET Incoming=2 WHERE MessageID=%d", nMessageID);
						
						pSession = new SenderSession;
						pSession->SendMessage(nMessageID);
						while (pSession->IsDone() == false) {
							log.Log("Waiting for outgoing message %d to be sent.", nMessageID);
							Sleep(2000);
						}
						log.Log("Outgoing Message %d done.", nMessageID);
						delete pSession;
						
						Sleep(20);
					}
					delete pResult;
				}
				delete pDB;
			}
		}

		
		
		
		
		//---------------------------------------------------------------------
		// When we need to shutdown the server, this function will be run.  
		// When this function exits, the main thread will be closed down.
		virtual void OnShutdown(void)
		{
			Logger log;
			int i;
			
			if (_pServers != NULL) {
				for(i=0; _pServers[i] != NULL; i++) {
					delete _pServers[i];
					_pServers[i] = NULL;
				}
				free(_pServers);
				_pServers = NULL;
			}
			
			if (_pIni != NULL) {
				log.Log("Deleting INI object.");
				delete _pIni;
				_pIni = NULL;
			}
			
			log.Log("Shutting Down.");
		}

		
		
		//---------------------------------------------------------------------
		// If the user or the system wants to shut down the daemon, this 
		// function will be run.  
		virtual void OnCtrlBreak(void)
		{
			printf("OnCtrlBreak();\n");
		}

} myApp;



