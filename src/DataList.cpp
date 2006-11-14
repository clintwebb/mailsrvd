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

#include "DataList.h"

//-----------------------------------------------------------------------------
// CJW: Constructor.  
DataList::DataList() 
{	
	_bComplete = false;
	_nCurrentRow = 0;
}


//-----------------------------------------------------------------------------
// CJW: 
DataList::~DataList() 
{
}


//-----------------------------------------------------------------------------
// CJW: Add a new row to the grid.
void DataList::AddRow()
{
	ASSERT(_nCurrentRow >= 0);
	
}


//-----------------------------------------------------------------------------
// CJW: When we are finished adding data to the list, then we lock it so that 
// 		it can only be read from.  We also cause the line counters to be 
// 		restarted so that when we read the data from the list, we start at the 
// 		top.
void DataList::Complete(void)
{
	ASSERT(_bComplete == false);
	_bComplete = true;
	_nCurrentRow = 0;
}	







