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

#include <string.h>

#include <DevPlus.h>

struct DataItem {
	union {
		int nValue;
		char *szValue;
	} data;
	bool bInteger;
};


class DataRow {

	private:
		int _nColumns;
		DataItem *_pItems;

	public:
		DataRow()
		{
			_nColumns = 0;
			_pItems = NULL;
		}
	
		DataRow(int nColumns) 
		{
			ASSERT(nColumns > 0);
			_pItems = (DataItem *) malloc(sizeof(DataItem) * nColumns);
			ASSERT(_pItems != NULL);
			_nColumns = 0;
			while (_nColumns < nColumns) {
				_pItems[_nColumns].bInteger = true;
				_pItems[_nColumns].data.nValue = 0;
				_nColumns ++;
			}
		}
		
		virtual ~DataRow() 
		{
			ASSERT(_pItems != NULL && _nColumns > 0);
			while (_nColumns > 0) {
				_nColumns --;
				if (_pItems[_nColumns].bInteger == false) {
					ASSERT(_pItems[_nColumns].data.szValue != NULL);
					free(_pItems[_nColumns].data.szValue);
					_pItems[_nColumns].data.szValue = NULL;
					_pItems[_nColumns].bInteger = true;
				}
			}
			free(_pItems);
			_pItems = NULL;
			ASSERT(_nColumns == 0);
		}
		
		//---------------------------------------------------------------------
		// CJW: Compare the date provided, against the data stored, and if 
		// 		there is a match, return the position.  If it isnt found, then 
		// 		return a -1.
		int GetIndex(char *szValue) 
		{ 
			int nIndex = -1;
			int i;
			
			ASSERT(szValue != NULL);
			ASSERT(_nColumns > 0 && _pItems != NULL);
			
			for(i=0; i<_nColumns; i++) {
				ASSERT(_pItems[i].bInteger == false);
				ASSERT(_pItems[i].data.szValue != NULL);
				
				if (strcmp(_pItems[i].data.szValue, szValue) == 0) {
					nIndex = i;
					i = _nColumns;
				}
			}
			
			return(nIndex);
		}
		
		
		//---------------------------------------------------------------------
		// CJW: Return the integer for the column in the row.  If it is stored 
		// 		as a string, then do an atoi on it and return that.  
		int GetInt(int nIndex) 
		{
			int nResult = 0;
			ASSERT(nIndex >= 0);
			ASSERT(nIndex < _nColumns);
			if (_pItems[nIndex].bInteger == true) {
				nResult = _pItems[nIndex].data.nValue;
			}
			else {
				ASSERT(_pItems[nIndex].data.szValue != NULL);
				nResult = atoi(_pItems[nIndex].data.szValue);
			}
			return(nResult);
		}
		
		//---------------------------------------------------------------------
		// CJW: Return the integer for the column in the row.  If it is stored 
		// 		as a string, then do an atoi on it and return that.  
		char * GetStr(int nIndex) 
		{
			char *str = NULL;
			
			ASSERT(nIndex >= 0);
			ASSERT(nIndex < _nColumns);
			if (_pItems[nIndex].bInteger == false ) {
				str = _pItems[nIndex].data.szValue;
			}
			return(str);
		}
		
		
		void SetInt(int nIndex, int nValue)
		{
			ASSERT(nIndex >= 0 && nIndex < _nColumns);
			ASSERT(_pItems != NULL);
			_pItems[nIndex].bInteger = true;
			_pItems[nIndex].data.nValue = nValue;
		}
		
		
		void SetStr(int nIndex, char *szValue)
		{
			int len;
			ASSERT(szValue != NULL);
			ASSERT(nIndex >= 0 && nIndex < _nColumns);
			ASSERT(_pItems != NULL);
			_pItems[nIndex].bInteger = false;
			len = strlen(szValue);
			_pItems[nIndex].data.szValue = (char *) malloc(len + 1);
			strcpy(_pItems[nIndex].data.szValue, szValue);
		}
		
		int AddColumn(char *szName)
		{
			int nIndex = -1;
			ASSERT(szName != NULL);
			ASSERT((_nColumns == 0 && _pItems == NULL) || (_nColumns > 0 && _pItems != NULL));
			_pItems = (DataItem *) realloc(_pItems, sizeof(DataItem) * (_nColumns + 1));
			ASSERT(_pItems != NULL);
			
			_pItems[_nColumns].bInteger = false;
			_pItems[_nColumns].data.szValue = (char *) malloc(strlen(szName)+1);
			strcpy(_pItems[_nColumns].data.szValue, szName);
			nIndex = _nColumns;
			_nColumns ++;
			return(nIndex);
		}
};


class DataList
{
	private:
		bool _bComplete;
		int  _nCurrentRow;
		int  _nRowCount;
		int  _nColumns;
		DataRow *_pHeaders, **_pRows;
		
	protected:
		
	public:
		DataList(); 
		virtual ~DataList();
		
		void AddRow();
		void AddColumn(char *szName);
		void AddData(int nIndex, int nValue);
		void AddData(int nIndex, char *szValue);
		void Complete();
		
		int GetRowCount() { return (_nRowCount); }
		
		bool NextRow();
		int GetInt(char *szName);
		int GetInt(int nIndex);
		char *GetStr(int nIndex);
		
	protected:
		
	private:
};



#endif
