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
//	This object is used by the DataModel, when it needs to return more than a 
//	simple value.  It returns rows in a grid, similar to the usual results by 
//	a dataquery.  Each column is labelled and given a type.  
//
//-----------------------------------------------------------------------------


#ifndef __DATALIST_H
#define __DATALIST_H


class DataList
{
	private:
		bool _bComplete;
		int  _nCurrentRow;
		
	protected:
		
	public:
		DataList(); 
		virtual ~DataList();
		
		void AddRow();
		void Complete();
		
	protected:
		
	private:
};



#endif
