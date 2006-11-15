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

#ifndef __DATAMODEL_H
#define __DATAMODEL_H

#include <DpLock.h>
#include <DpIniFile.h>
#include <DpMySql.h>

#include "DataList.h"

class DataModel : public DpLock
{
	private:
		DpMySqlDB *_pDB;
		
		struct {
			struct {
				int nID;
				int nReject;
			} domain;
		} _Cache;
		
	protected:
		
	public:
		DataModel(); 
		virtual ~DataModel();
		
		bool Connect(DpIniFile *pIni);
		int GetUserID(char *szUser, char *szPass);
		int GetDomainID(char *szDomain);
		int GetDomainReject(int nDomainID);
		DataList* GetUserFromAddress(int nDomainID, char *szUser);
		int InsertMessage(int nUserID);
		void InsertBodyLine(int nMsgID, int lineno, char *szLine);
		void InsertOutgoing(int nMsgID, char *szFrom, char *szRemote);

		
	protected:
		
	private:
		
};



#endif
