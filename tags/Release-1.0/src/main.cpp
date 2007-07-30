//-----------------------------------------------------------------------------
// Mailsrv
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


#include <unistd.h>

#include <DpIniFile.h>
#include <DpMain.h>
#include <DpMySql.h>
#include <DpTextTools.h>

#include "Defaults.h"
#include "Logger.h"
#include "Server.h"
#include "SmtpServer.h"
#include "PopServer.h"
#include "DataModel.h"


//-----------------------------------------------------------------------------
// This value can be provided at compile time, but if not, then the default 
// here will be used, which should be more than suitable.
#ifndef CONFIG_DIR
#define CONFIG_DIR "/etc"
#endif

#ifndef LOG_DIR
#define LOG_DIR "/var/log/mailsrvd"
#endif


#ifndef DEVPLUS_VERSION
#error For performance and stability, DevPlus v0.2.15 or higher is required.
#endif
#if (DEVPLUS_VERSION < 215) 
#error For performance and stability, DevPlus v0.2.15 or higher is required.
#endif


class theApp : public DpMain
{
	private:
	
	protected:
		DpIniFile	  *_pIni;
		Logger		  *_pLogger;
		Server		 **_pServers;	// list of servers.
		int            _nServers;	// number of servers in the list.
		DataModel     *_pData;		// main database interface object.

	public:
		
		theApp()
		{
			_pIni       = NULL;
			_pServers   = NULL;
			_nServers   = 0;
			_pLogger    = NULL;
			_pData      = NULL;
		}
		
		
		//---------------------------------------------------------------------
		// The final peice.  We dont actually want to do anything here, so we 
		// just verify that everything was cleaned up properly.
		virtual ~theApp()
		{
			ASSERT(_pIni     == NULL);
			ASSERT(_pServers == NULL);
			ASSERT(_pLogger  == NULL);
			ASSERT(_pData    == NULL);
			ASSERT(_nServers == 0);
		}


	protected:
		
		//---------------------------------------------------------------------
		// Load the config file and return a true if we could do it.  Otherwise 
		// we will return a false.  We dont need to pull out any information 
		// from the file, we just need to make sure we can load it.
		// First we need to check the current directory for the conf file, and 
		// if it is not there, then we check the CONFIG_DIR.
		bool LoadConfig(void)
		{
			bool bLoaded;
			ASSERT(_pIni     == NULL);
			ASSERT(_pServers == NULL);
			ASSERT(_pData    == NULL);
			ASSERT(_nServers == 0);
			
			_pIni = new DpIniFile;
			if (_pIni->Load("." "/mailsrv.conf") == false) {
				delete _pIni;
				
				_pIni = new DpIniFile;
				if (_pIni->Load(CONFIG_DIR "/mailsrv.conf") == false) {
					delete _pIni;
					_pIni = NULL;
					bLoaded = false;
				}
				else {
					bLoaded = true;
				}
			}
			else {
				bLoaded = true;
			}
			
			return (bLoaded);
		}
		
		
		//---------------------------------------------------------------------
		// Because we have the potential of multiple SMTP and POP3 servers 
		// listening on multiple ports, we need to add each instance to our 
		// list.  After adding the server to the list, we will attach the 
		// database instance to it.
		void AddServer(Server *pServer) 
		{
			ASSERT(pServer != NULL);
			ASSERT((_pServers == NULL && _nServers == 0) || (_pServers != NULL && _nServers > 0));

			_pServers = (Server **) realloc(_pServers, sizeof(Server *) * (_nServers+1));
			ASSERT(_pServers != NULL);
			_pServers[_nServers] = pServer;
			_nServers++;
			
			ASSERT(_pServers != NULL && _nServers > 0);
			
			ASSERT(_pData != NULL);
			pServer->AttachData(_pData);
		}
		
		
		//---------------------------------------------------------------------
		// Check the INI, and if the SMTP module is enabled, then we need to 
		// fork, and let the SmtpServer object handle it from that moment on.
		virtual void StartSMTP(void)
		{
			SmtpServer *pServer;
			DpTextTools text;
			char *szPorts;
			char **szList;
			int nPort;
			int i,k;
			
			ASSERT(_pIni != NULL && _pLogger != NULL);
			
			if (_pIni->SetGroup("smtp") == false) {
				_pLogger->Log("'smtp' section in config file is missing.\n");
			}
			else {
			
				if (_pIni->GetValue("ports", &szPorts) == true) {
					ASSERT(szPorts != NULL);
					text.Load(szPorts);
					szList = text.GetWordArray();
					
					i=0; 
					while(szList[i] != NULL) {
						
						nPort = atoi(szList[i]);
						ASSERT(nPort > 0);
						
						_pLogger->Log("Activating Incoming SMTP Server on port %d.", nPort);

						k = 15;
						while(k > 0) {
							pServer = new SmtpServer;
							ASSERT(pServer != NULL);
							if (pServer->Listen(nPort) == false) {
								delete pServer;
								k--;
								if (k == 0) { _pLogger->Log("Unable to listen on socket %d. Giving up. ", nPort); }
								else { 
									_pLogger->Log("Unable to listen on socket %d. Will try again in 30 seconds", nPort); 
									sleep(30);
								}
							}
							else {
								AddServer(pServer);
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
			PopServer *pServer;
			DpTextTools text;
			char *szPorts;
			char **szList;
			int nPort;
			int i,k;
			
			ASSERT(_pIni != NULL && _pLogger != NULL);
			
			if (_pIni->SetGroup("pop3") == false) {
				_pLogger->Log("'pop3' section in config file is missing.\n");
			}
			else {
			
				if (_pIni->GetValue("ports", &szPorts) == true) {
					ASSERT(szPorts != NULL);
					text.Load(szPorts);
					szList = text.GetWordArray();
					
					i=0;
					while(szList[i] != NULL) {
						
						nPort = atoi(szList[i]);
						ASSERT(nPort > 0);
						
						_pLogger->Log("Activating Incoming POP3 Server on port %d.", nPort);
							
						k = 15;
						while(k > 0) {
							pServer = new PopServer;
							ASSERT(pServer != NULL);
							if (pServer->Listen(nPort) == false) {
								delete pServer;
								k--;
								if (k == 0) { _pLogger->Log("Unable to listen on socket %d.  Giving Up.", nPort); }
								else {
									_pLogger->Log("Unable to listen on socket %d.  Will try again in 30 seconds", nPort);
									sleep(30);
								}
							}
							else {
								AddServer(pServer);
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
/*		virtual void StartOut(void)
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
*/
		
		
/*		virtual void OnTimer(int nTimerID) 
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
*/		
		
		
		void ConvertPop3()
		{
			ASSERT(_pData != NULL);
			_pData->ConvertPop3();
		}
		

		//---------------------------------------------------------------------
		// This is where everything is started up.  So we need to start up data 
		// object which creates a connection with the database.  We also start 
		// the server object which will listen on a parMessageticular port.  
		// Finally, we will start up the controller object which checks the 
		// database and makes the API request to our sms service host.
		//
		// Once everything is started, then the main thread will idle while the 
		// worker threads continue to function.
		void OnStartup(void)
		{
			ASSERT(_pIni == NULL);
			ASSERT(_pData == NULL);
			ASSERT(_pLogger == NULL);
		
			// Create and initialise the logger.
			_pLogger = new Logger;
			_pLogger->Init();
			_pLogger->Log("Starting Main Process.");
			
			// initialise the randomiser.
			InitRandom();
			
			// Load the configuration... we will use it later.
			_pLogger->Log("Loading Config.");
			if (LoadConfig() == false) {
				_pLogger->Log("Failed to load config file: %s%s.", CONFIG_DIR, "/mailsrv.conf");
				Shutdown();
			}
			else {
			
				_pData = new DataModel;
				if (_pData->Connect(_pIni) == false) {
					_pLogger->Log("Unable to connect to data store");
					Shutdown();
				}
				else {
 					
					StartPop3();
					StartSMTP();
// 					ConvertPop3();
				}
			}
		}
		
		


		//---------------------------------------------------------------------
		// When we need to shutdown the server, this function will be run.  
		// When this function exits, the main thread will be closed down.
		void OnShutdown(void)
		{
			int i;
			
			ASSERT(_pLogger != NULL);
			
			_pLogger->Log("Shutdown Initiated.");
			
// 			printf("OnShutdown()\n");
			ASSERT((_pServers == NULL || _nServers == 0) || (_pServers != NULL && _nServers > 0));
			if (_pServers != NULL) {
				ASSERT(_nServers > 0);
// 				printf("OnShutdown() - %d servers to delete.\n", _nServers);
				for(i=0; i < _nServers; i++) {
// 					printf("OnShutdown() - deleting pServer[%d]\n", i);
					delete _pServers[i];
					_pServers[i] = NULL;
				}
				
				free(_pServers);
				_pServers = NULL;
				_nServers = 0;
			}
// 			printf("OnShutdown() - Passed the deletion of Servers\n");
			
			if (_pIni != NULL) {
				_pLogger->Log("Deleting INI object.");
				delete _pIni;
				_pIni = NULL;
			}
			
			if (_pData != NULL) {
				_pLogger->Log("Deleting Database connection.");
				delete _pData;
				_pData = NULL;
			}
			
			_pLogger->Log("Shutting Down.");
			_pLogger->Close();
			delete _pLogger;
			_pLogger = NULL;
		}
		
		
		
		//---------------------------------------------------------------------
		// If the user or the system wants to shut down the daemon, this 
		// function will be run.  
		void OnCtrlBreak(void)
		{
			printf("OnCtrlBreak();\n");
		}
		

} myApp;


		





		



