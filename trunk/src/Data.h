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

class DataModel : public DpLock
{
	private:
		DpMySqlDB *_pDB;
		
	protected:
		
	public:
		DataModel(); 
		virtual ~DataModel();
		
	protected:
		
	private:
		bool Connect(DpIniFile *pIni);
		
};



#endif
