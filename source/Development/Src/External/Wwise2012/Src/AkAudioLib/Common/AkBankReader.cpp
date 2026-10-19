/*=============================================================================
	AkBankReader.cpp - CAkBankReader: reads a bank either from memory or through a standard stream of the
	stream manager (which reaches the game's IAkIOHookBlocking).

	DISHONORED(port): agent IH1 (wave 26); akbankreader.cpp, 2012 0x91e240..0x91e9c0, function by function.
	The read buffer is max(block size, 32 KB), Malign'd to 32 in the default pool; every stream read is
	blocking with the bank throughput / priority as its deadline heuristics.
=============================================================================*/
#include "AkBankMgr.h"

/** 2012 0x91e240. */
CAkBankReader::CAkBankReader()
	: m_pBuffer( NULL )
	, m_pReadBufferPtr( NULL )
	, m_ulRemainingBytes( 0 )
	, m_ulBufferSize( 0 )
	, m_ulDeviceBlockSize( 0 )
	, m_pUserReadBuffer( NULL )
	, m_pInMemoryBankReaderPtr( NULL )
	, m_pStream( NULL )
	, m_fThroughput( AK_DEFAULT_BANK_THROUGHPUT )
	, m_priority( AK_DEFAULT_BANK_IO_PRIORITY )
	, m_bIsInitDone( false )
{
}

/** 2012 0x91e260. */
AKRESULT CAkBankReader::Init()
{
	if( m_bIsInitDone )
	{
		return AK_Fail;
	}
	m_ulRemainingBytes = 0;
	m_ulBufferSize = 0;
	m_pBuffer = NULL;
	m_pReadBufferPtr = NULL;
	m_bIsInitDone = true;
	m_fThroughput = AK_DEFAULT_BANK_THROUGHPUT;
	m_priority = AK_DEFAULT_BANK_IO_PRIORITY;
	return AK_Success;
}

/** 2012 0x91e2a0. */
void CAkBankReader::Reset()
{
	m_ulRemainingBytes = 0;
	m_pReadBufferPtr = m_pBuffer;
	m_pInMemoryBankReaderPtr = NULL;
}

/** 2012 0x91e2b0. */
AKRESULT CAkBankReader::Term()
{
	if( m_pBuffer )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pBuffer );
		m_pBuffer = NULL;
	}
	m_pReadBufferPtr = NULL;
	return AK_Success;
}

/** 2012 0x91e2e0. */
AKRESULT CAkBankReader::SetBankLoadIOSettings( AkReal32 in_fThroughput, AkPriority in_priority )
{
	if( in_fThroughput < 0.f || (AkUInt8)in_priority > AK_MAX_PRIORITY )
	{
		return AK_InvalidParameter;
	}
	m_priority = in_priority;
	m_fThroughput = in_fThroughput;
	return AK_Success;
}

/** 2012 0x91e310. */
AKRESULT CAkBankReader::SetFile( const void * in_pInMemoryFile, AkUInt32 in_ui32FileSize )
{
	m_pInMemoryBankReaderPtr = (const AkUInt8 *)in_pInMemoryFile;
	m_ulRemainingBytes = in_ui32FileSize;
	return AK_Success;
}

/** The common tail of both stream SetFile overloads: size the read buffer on the device block size. */
AKRESULT CAkBankReader::OpenedStream()
{
	m_ulDeviceBlockSize = m_pStream->GetBlockSize();
	if( !m_ulDeviceBlockSize )
	{
		return AK_Fail;
	}
	if( m_ulBufferSize < m_ulDeviceBlockSize )
	{
		if( m_pBuffer )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, m_pBuffer );
		}
		m_ulBufferSize = m_ulDeviceBlockSize <= 0x8000 ? 0x8000 : m_ulDeviceBlockSize;
		m_pBuffer = (AkUInt8 *)AK::MemoryMgr::Malign( g_DefaultPoolId, m_ulBufferSize, 32 );
	}
	return m_pBuffer ? AK_Success : AK_InsufficientMemory;
}

/** 2012 0x91e6e0: language-specific first, then common. */
AKRESULT CAkBankReader::SetFile( const char * in_pszFilename, AkUInt32 in_uFileOffset, void * in_pCookie )
{
	AkFileSystemFlags fsFlags;
	memset( &fsFlags, 0, sizeof( fsFlags ) );
	fsFlags.bIsFromRSX = false;
	fsFlags.bIsLanguageSpecific = true;
	fsFlags.pCustomParam = in_pCookie;

	const size_t uLen = strlen( in_pszFilename );
	wchar_t szFileName[ AK_DEFAULT_PATH_LENGTH ];
	MultiByteToWideChar( CP_ACP, 0, in_pszFilename, -1, szFileName, (int)( uLen + 1 < AK_DEFAULT_PATH_LENGTH ? uLen + 1 : AK_DEFAULT_PATH_LENGTH ) );
	szFileName[ AK_DEFAULT_PATH_LENGTH - 1 ] = 0;

	AKRESULT eResult = AK::IAkStreamMgr::Get()->CreateStd( szFileName, &fsFlags, AK_OpenModeRead, m_pStream, true );
	if( eResult != AK_Success )
	{
		fsFlags.bIsLanguageSpecific = false;
		eResult = AK::IAkStreamMgr::Get()->CreateStd( szFileName, &fsFlags, AK_OpenModeRead, m_pStream, true );
	}
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = OpenedStream();
	if( eResult != AK_Success || !in_uFileOffset )
	{
		return eResult;
	}
	AkInt64 lRealOffset = 0;
	eResult = m_pStream->SetPosition( in_uFileOffset, AK_MoveBegin, &lRealOffset );
	if( eResult == AK_Success )
	{
		const AkUInt32 uToSkip = in_uFileOffset - (AkUInt32)lRealOffset;
		AkUInt32 uSkipped = 0;
		eResult = Skip( uToSkip, uSkipped );
		if( uSkipped != uToSkip )
		{
			return AK_Fail;
		}
	}
	return eResult;
}

/** 2012 0x91e870. */
AKRESULT CAkBankReader::SetFile( AkFileID in_FileID, AkUInt32 in_uFileOffset, AkUInt32 in_codecID, void * in_pCookie, bool in_bIsLanguageSpecific )
{
	AkFileSystemFlags fsFlags;
	memset( &fsFlags, 0, sizeof( fsFlags ) );
	fsFlags.uCodecID = in_codecID;
	fsFlags.uCompanyID = 0;
	fsFlags.pCustomParam = in_pCookie;
	fsFlags.uCustomParamSize = 0;
	fsFlags.bIsFromRSX = false;
	fsFlags.bIsLanguageSpecific = in_bIsLanguageSpecific;

	AKRESULT eResult = AK::IAkStreamMgr::Get()->CreateStd( in_FileID, &fsFlags, AK_OpenModeRead, m_pStream, true );
	if( eResult != AK_Success && in_bIsLanguageSpecific )
	{
		fsFlags.bIsLanguageSpecific = false;
		eResult = AK::IAkStreamMgr::Get()->CreateStd( in_FileID, &fsFlags, AK_OpenModeRead, m_pStream, true );
	}
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = OpenedStream();
	if( eResult != AK_Success || !in_uFileOffset )
	{
		return eResult;
	}
	AkInt64 lRealOffset = 0;
	eResult = m_pStream->SetPosition( in_uFileOffset, AK_MoveBegin, &lRealOffset );
	if( eResult == AK_Success )
	{
		const AkUInt32 uToSkip = in_uFileOffset - (AkUInt32)lRealOffset;
		AkUInt32 uSkipped = 0;
		eResult = Skip( uToSkip, uSkipped );
		if( uSkipped != uToSkip )
		{
			return AK_Fail;
		}
	}
	return eResult;
}

/** 2012 0x91e330. */
AKRESULT CAkBankReader::CloseFile()
{
	if( m_pStream )
	{
		m_pStream->Destroy();
		m_pStream = NULL;
	}
	m_pInMemoryBankReaderPtr = NULL;
	m_ulRemainingBytes = 0;
	return AK_Success;
}

/** 2012 0x91e360. */
void CAkBankReader::ReleaseData()
{
	if( m_pUserReadBuffer )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pUserReadBuffer );
		m_pUserReadBuffer = NULL;
	}
}

/** 2012 0x91e390. */
AKRESULT CAkBankReader::FillData( void * in_pBufferToFill, AkUInt32 in_ulSizeToRead, AkUInt32 & out_rulSizeRead )
{
	out_rulSizeRead = 0;
	AkUInt8 * pBufferToFill = (AkUInt8 *)in_pBufferToFill;
	if( !m_pStream )
	{
		const AkUInt32 uSize = in_ulSizeToRead < m_ulRemainingBytes ? in_ulSizeToRead : m_ulRemainingBytes;
		out_rulSizeRead = uSize;
		m_ulRemainingBytes -= uSize;
		memcpy( pBufferToFill, m_pInMemoryBankReaderPtr, uSize );
		m_pInMemoryBankReaderPtr += uSize;
		return AK_Success;
	}
	AkUInt32 uLeft = in_ulSizeToRead;
	while( uLeft )
	{
		if( m_ulRemainingBytes )
		{
			const AkUInt32 uCopy = m_ulRemainingBytes >= uLeft ? uLeft : m_ulRemainingBytes;
			memcpy( pBufferToFill, m_pReadBufferPtr, uCopy );
			m_pReadBufferPtr += uCopy;
			out_rulSizeRead += uCopy;
			pBufferToFill += uCopy;
			m_ulRemainingBytes -= uCopy;
			uLeft -= uCopy;
		}
		else if( uLeft < m_ulBufferSize )
		{
			const AkUInt32 uReadSize = m_ulDeviceBlockSize * ( m_ulBufferSize / m_ulDeviceBlockSize );
			const AKRESULT eResult = m_pStream->Read( m_pBuffer, uReadSize, true, m_priority, (AkReal32)uReadSize / m_fThroughput, m_ulRemainingBytes );
			if( eResult != AK_Success || m_pStream->GetStatus() != AK_StmStatusCompleted )
			{
				return eResult;
			}
			m_pReadBufferPtr = m_pBuffer;
			if( !m_ulRemainingBytes )
			{
				return AK_Success;
			}
		}
		else
		{
			const AkUInt32 uReadSize = m_ulDeviceBlockSize * ( uLeft / m_ulDeviceBlockSize );
			AkUInt32 uSizeRead = 0;
			const AKRESULT eResult = m_pStream->Read( pBufferToFill, uReadSize, true, m_priority, (AkReal32)uReadSize / m_fThroughput, uSizeRead );
			if( eResult != AK_Success || m_pStream->GetStatus() != AK_StmStatusCompleted )
			{
				return eResult;
			}
			pBufferToFill += uSizeRead;
			out_rulSizeRead += uSizeRead;
			uLeft -= uSizeRead;
			if( uLeft >= m_ulBufferSize )
			{
				bool bEndOfStream = false;
				m_pStream->GetPosition( &bEndOfStream );
				return AK_Fail;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x91e540. */
AKRESULT CAkBankReader::FillDataEx( void * in_pBufferToFill, AkUInt32 in_ulSizeToRead )
{
	AkUInt32 ulSizeRead = 0;
	const AKRESULT eResult = FillData( in_pBufferToFill, in_ulSizeToRead, ulSizeRead );
	if( eResult == AK_Success && in_ulSizeToRead != ulSizeRead )
	{
		return AK_BankReadError;
	}
	return eResult;
}

/** 2012 0x91e580: a skip past the buffer seeks the stream; a short one reads a buffer and moves into it. */
AKRESULT CAkBankReader::Skip( AkUInt32 in_ulNumBytesToSkip, AkUInt32 & out_rulSizeSkipped )
{
	out_rulSizeSkipped = 0;
	if( !m_pStream )
	{
		const AkUInt32 uSize = in_ulNumBytesToSkip < m_ulRemainingBytes ? in_ulNumBytesToSkip : m_ulRemainingBytes;
		out_rulSizeSkipped = uSize;
		m_ulRemainingBytes -= uSize;
		m_pInMemoryBankReaderPtr += uSize;
		return AK_Success;
	}
	AkUInt32 uLeft = in_ulNumBytesToSkip;
	while( uLeft )
	{
		if( m_ulRemainingBytes )
		{
			const AkUInt32 uSkip = m_ulRemainingBytes >= uLeft ? uLeft : m_ulRemainingBytes;
			m_pReadBufferPtr += uSkip;
			out_rulSizeSkipped += uSkip;
			m_ulRemainingBytes -= uSkip;
			uLeft -= uSkip;
			continue;
		}
		if( uLeft > m_ulBufferSize )
		{
			AkInt64 lRealOffset = 0;
			const AKRESULT eResult = m_pStream->SetPosition( uLeft, AK_MoveCurrent, &lRealOffset );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			out_rulSizeSkipped += (AkUInt32)lRealOffset;
			uLeft -= (AkUInt32)lRealOffset;
			continue;
		}
		const AkUInt32 uReadSize = m_ulDeviceBlockSize * ( m_ulBufferSize / m_ulDeviceBlockSize );
		const AKRESULT eResult = m_pStream->Read( m_pBuffer, uReadSize, true, m_priority, (AkReal32)uReadSize / m_fThroughput, m_ulRemainingBytes );
		if( eResult != AK_Success || m_pStream->GetStatus() != AK_StmStatusCompleted )
		{
			return eResult;
		}
		bool bEndOfStream = false;
		m_pStream->GetPosition( &bEndOfStream );
		if( bEndOfStream && m_ulRemainingBytes < uLeft )
		{
			return AK_Fail;
		}
		m_pReadBufferPtr = m_pBuffer + uLeft;
		out_rulSizeSkipped += uLeft;
		m_ulRemainingBytes -= uLeft;
		return AK_Success;
	}
	return AK_Success;
}

/** 2012 0x91e9c0: a pointer into the read buffer when the section is already there, else a 32-aligned copy. */
const void * CAkBankReader::GetData( AkUInt32 in_uSize )
{
	if( !m_pStream )
	{
		const AkUInt32 uSize = in_uSize >= m_ulRemainingBytes ? m_ulRemainingBytes : in_uSize;
		m_ulRemainingBytes -= uSize;
		const AkUInt8 * pData = m_pInMemoryBankReaderPtr;
		m_pInMemoryBankReaderPtr += uSize;
		return pData;
	}
	if( in_uSize <= m_ulRemainingBytes )
	{
		AkUInt8 * pData = m_pReadBufferPtr;
		m_pReadBufferPtr += in_uSize;
		m_ulRemainingBytes -= in_uSize;
		return pData;
	}
	AkUInt32 uAlignOffset = 0;
	AkUInt32 uAllocSize = in_uSize;
	const AkUInt32 uMisalign = m_ulRemainingBytes & 0x1F;
	if( uMisalign )
	{
		uAllocSize = in_uSize + 31;
		uAlignOffset = 32 - uMisalign;
	}
	m_pUserReadBuffer = AK::MemoryMgr::Malign( g_DefaultPoolId, uAllocSize, 32 );
	if( m_pUserReadBuffer )
	{
		AkUInt8 * pData = (AkUInt8 *)m_pUserReadBuffer + uAlignOffset;
		AkUInt32 uSizeRead = 0;
		if( FillData( pData, in_uSize, uSizeRead ) == AK_Success && uSizeRead == in_uSize )
		{
			return pData;
		}
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pUserReadBuffer );
		m_pUserReadBuffer = NULL;
	}
	return m_pUserReadBuffer;
}
