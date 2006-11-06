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



#include "main.h"


theApp::theApp()
{
	_pIni     = NULL;
	_pServers = NULL;
	_pLogger  = NULL;
	_pData    = NULL;
}


		
//---------------------------------------------------------------------
// The final peice.  We dont actually want to do anything here, so we 
// just verify that everything was cleaned up properly.
theApp::~theApp()
{
	ASSERT(_pIni == NULL);
	ASSERT(_pServers == NULL);
	ASSERT(_pLogger == NULL);
	ASSERT(_pData == NULL);
}

//---------------------------------------------------------------------
// Load the config file and return a true if we could do it.  Otherwise 
// we will return a false.  We dont need to pull out any information 
// from the file, we just need to make sure we can load it.
bool theApp::LoadConfig(void)
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
// This is where everything is started up.  So we need to start up data 
// object which creates a connection with the database.  We also start 
// the server object which will listen on a parMessageticular port.  Finally, 
// we will start up the controller object which checks the database and 
// makes the API request to our sms service host.
//
// Once everything is started, then the main thread will idle while the 
// worker threads continue to function.
void theApp::OnStartup(void)
{
	Logger log;

	ASSERT(_pIni == NULL);
	ASSERT(_pData == NULL);

	// Create and initialise the logger.

	log.Log("Starting Main Process.");
	
	// initialise the randomiser.
	InitRandom();
	
	// Load the configuration... we will use it later.
	log.Log("Loading Config.");
	if (LoadConfig() == false) {
		log.Log("Failed to load config file: %s%s.", CONFIG_DIR, "/mailsrv.conf");
		Shutdown();
	}
	else {
	
		// We need to connect to the database, so we need to get the details 
		// from the configuration so that we know what server to connect to, 
		// and the username and password.
		
		
		
		// start the data server.
// 		// *** should get the db path from the config file.
// 		_pData = new DpDataServer;
// 		if (_pData->Load("
	
		// setup and initialise our named-pipes for external integration.
// 		OnStarted();
	
		log.Log("Finished Launching Process.");
	}
}





//---------------------------------------------------------------------
// When we need to shutdown the server, this function will be run.  
// When this function exits, the main thread will be closed down.
void theApp::OnShutdown(void)
{
	Logger log;
	int i;
	
	if (_pServers != NULL) {
		for(i=0; _pServers[i] != NULL; i++) {
			delete _pServers[i];
			_pServers[i] = NULL;
		}
		_pServers = NULL;
	}
	
	if (_pIni != NULL) {
		log.Log("Deleting INI object.");
		delete _pIni;
		_pIni = NULL;
	}
	
	if (_pData != NULL) {
		delete _pData;
		_pData = NULL;
	}
	
	log.Log("Shutting Down.");
	log.Close();
}

		

//---------------------------------------------------------------------
// If the user or the system wants to shut down the daemon, this 
// function will be run.  
void theApp::OnCtrlBreak(void)
{
	printf("OnCtrlBreak();\n");
}



