//------------------------------------------------------------------------------
// File: MemFile.h
//
// Desc: DirectShow sample code - header file for application using async 
//	   filter.
//
// Copyright (c) 1996-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//  Define an internal filter that wraps the base CBaseReader stuff
//
#include "wincriemu.h"

class CMemStream : public CAsyncStream
{
public:
	CMemStream(MW_PLY_OBJ2	*pbData, LONGLONG llLength, DWORD dwKBPerSec = INFINITE) :
		m_pMpo(pbData),
		m_llLength(llLength),
		m_llPosition(0),
		m_dwKBPerSec(dwKBPerSec)
	{
		m_dwTimeStart = timeGetTime();
	}
	HRESULT SetPointer(LONGLONG llPos)
	{
		if (llPos < 0 || llPos > m_llLength) {
			return S_FALSE;
		} else {
//@			m_llPosition = llPos;
			return S_OK;
		}
	}
	HRESULT Read(PBYTE pbBuffer,
				 DWORD dwBytesToRead,
				 BOOL bAlign,
				 LPDWORD pdwBytesRead)
	{
		CAutoLock lck(&m_csLock);
		DWORD dwReadLength;
		DWORD	skip = FALSE;
		/*  Wait until the bytes are here! */
		if (m_llPosition + dwBytesToRead > m_llLength) {
			dwReadLength = (DWORD)(m_llLength - m_llPosition);
		} else {
			dwReadLength = dwBytesToRead;
		}
		{
			char	str[256];
			sprintf(str,"read %d,%d\n",(DWORD)m_llPosition ,dwReadLength);
			OutputDebugString(str);
		}

		if(dwBytesToRead > 62768 && m_llPosition == 0){
			dwReadLength = 0x400;
			skip = TRUE;
		}

//		dwReadLength /= 8;


		{
			int			size = dwReadLength;
			PBYTE	pbBufTmp = pbBuffer;
			SJCK	ck;
			while(1){
				SJ_Lock(m_pMpo->sj);
				if( (DWORD)SJ_GetNumData( m_pMpo->sj, SJ_LIN_DATA ) < dwReadLength ) {
					size = dwReadLength = SJ_GetNumData( m_pMpo->sj, SJ_LIN_DATA );
//					Unlock();
//					WaitSema(m_pMpo->readsema);
//					Lock();
//					Sleep(1);
				} 
				SJ_Unlock(m_pMpo->sj);
				if(dwReadLength > 0){
					do {
						SJ_Lock(m_pMpo->sj);
						SJ_GetChunk( m_pMpo->sj, SJ_LIN_DATA, size, &ck );
						CopyMemory((PVOID)pbBufTmp, (PVOID)(ck.data),
						ck.len);
						if(skip){
							SJ_UngetChunk(m_pMpo->sj, SJ_LIN_DATA, &ck );
							size -= ck.len;
						}else{
							SJ_PutChunk( m_pMpo->sj, SJ_LIN_FREE, &ck );
							size -= ck.len;
						}
						SJ_Unlock(m_pMpo->sj);
					} while ( size > 0 );
					break;
				}else{
//					WaitSema(m_pMpo->readsema);
					break;
				}
			}
		}

		{
			char	str[256];
			sprintf(str,"read %d,%d\n",(DWORD)m_llPosition ,dwReadLength);
			OutputDebugString(str);
		}

		if(!skip){
			m_llPosition += dwReadLength;
			*pdwBytesRead = dwReadLength;
	   	}else{
			*pdwBytesRead = dwBytesToRead;
		}



		return S_OK;
	}
	LONGLONG Size(LONGLONG *pSizeAvailable)
	{
		LONGLONG llCurrentAvailable =
			Int32x32To64((timeGetTime() - m_dwTimeStart),m_dwKBPerSec);
		*pSizeAvailable = min(m_llLength, llCurrentAvailable);
		return m_llLength;
	}
	DWORD Alignment()
	{
		return 1;
	}
	void Lock()
	{
		m_csLock.Lock();
	}
	void Unlock()
	{
		m_csLock.Unlock();
	}

private:
	CCritSec	   m_csLock;
	MW_PLY_OBJ2		*m_pMpo;
	const LONGLONG m_llLength;
	LONGLONG	   m_llPosition;
	DWORD		  m_dwKBPerSec;
	DWORD		  m_dwTimeStart;
};

class CMemReader : public CAsyncReader
{
public:

	//  We're not going to be CoCreate'd so we don't need registration
	//  stuff etc
	STDMETHODIMP Register()
	{
		return S_OK;
	}
	STDMETHODIMP Unregister()
	{
		return S_OK;
	}
	CMemReader(CMemStream *pStream, CMediaType *pmt, HRESULT *phr) :
		CAsyncReader(NAME("Mem Reader"), NULL, pStream, phr)
	{
		m_mt = *pmt;
	}
};
