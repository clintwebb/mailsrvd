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
// CJW: Constructor.   Initialise everything so that we start in a known state.
DataList::DataList() 
{	
	_bComplete = false;
	_nCurrentRow = 0;
	_nRowCount = 0;
	_pHeaders = NULL;
	_pRows = NULL;
	_nColumns = 0;
}


//-----------------------------------------------------------------------------
// CJW: Clean up stuff that was created by this object.
DataList::~DataList() 
{
	ASSERT((_nRowCount == 0 && _pRows == NULL) || (_nRowCount > 0 && _pRows != NULL));
	
	while(_nRowCount > 0) {
		_nRowCount--;
		ASSERT(_pRows[_nRowCount] != NULL);
		delete _pRows[_nRowCount];
		_pRows[_nRowCount] = NULL;
	}
}


void DataList::AddColumn(char *szName)
{
	ASSERT(szName != NULL);
	ASSERT(_bComplete == false);
	
}


//-----------------------------------------------------------------------------
// CJW: Add a new row to the grid.
void DataList::AddRow()
{
	ASSERT((_nCurrentRow == 0 && _nRowCount == 0) || (_nCurrentRow > 0 && _nRowCount > 0));
	ASSERT(_nColumns > 0);
	_nRowCount ++;
	_pRows = (DataRow **) realloc(_pRows, sizeof(DataRow*) * _nRowCount);
	_pRows[_nRowCount - 1] = new DataRow(_nColumns);
}


//-----------------------------------------------------------------------------
// CJW: If there is another row of data available, then return a true and 
// 		update the row counters.  Otherwise return a false.
bool DataList::NextRow()
{
	bool bValid = false;
	
	_nCurrentRow++;
	ASSERT(_nCurrentRow <= _nRowCount);
	if (_nCurrentRow < _nRowCount) {
		bValid = true;
	}
	
	return (bValid);
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



//-----------------------------------------------------------------------------
// CJW: Since we are given a name, we need to use that name to lookup the name 
// 		in the headers first.
int DataList::GetInt(char *szName)
{
	int nResult = 0;
	int nIndex;
	
	ASSERT(szName != NULL);
	ASSERT(_pHeaders != NULL);
	
	nIndex = _pHeaders->GetIndex(szName);
	ASSERT(nIndex >= 0);
	
	nResult = GetInt(nIndex);
	return(nResult);
}


//-----------------------------------------------------------------------------
// CJW: get an integer from the row.  The index is basically the column number, 
// 		starting from 0.
int DataList::GetInt(int nIndex)
{
	int nResult = 0;
	
	ASSERT(nIndex >= 0);
	ASSERT(_nCurrentRow < _nRowCount);
	ASSERT(_pRows != NULL);
	ASSERT(_pRows[nIndex] != NULL);
	
	nResult = _pRows[nIndex]->GetInt(nIndex);
	
	return(nResult);
}


