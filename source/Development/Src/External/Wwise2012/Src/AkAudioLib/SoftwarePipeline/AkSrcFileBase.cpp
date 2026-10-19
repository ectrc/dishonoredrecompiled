/*=============================================================================
	AkSrcFileBase.cpp - CAkSrcFileBase (aksrcfilebase.cpp, 2012 0x95f280..0x95fae0, the aksrcfilebase.h inlines
	ReleaseStreamBuffer 0x955c00 / OnLoopComplete 0x955c50) and AK::SrcFileServices::IsPrebufferingReady (0x955bb0);
	2013 matched by bytes.

	DISHONORED(port): agent IH2 fork S (wave 26). The streams are IH1's AK::IAkStreamMgr automatic streams.
	Deviation (named): AkSrcTypeInfo keeps its +20 union as pInMemoryMedia only; for a streamed source retail reads the
	same slot as the external file name (CreateAuto by name when set), which CreateStream does through a cast.
=============================================================================*/
#include "AkSrcFileBase.h"
#include "AkPBI.h"

namespace AK
{
	namespace SrcFileServices
	{
		/** 2012 0x955bb0: ready once the stream holds its nominal buffering (or has reached the end of the file). */
		AKRESULT IsPrebufferingReady( AK::IAkAutoStream * in_pStream, AkUInt32 in_uSizeLeft )
		{
			AkUInt32 uBuffering;
			const AKRESULT eResult = in_pStream->QueryBufferingStatus( uBuffering );
			if( eResult == AK_DataReady || eResult == AK_NoDataReady )
			{
				return ( uBuffering + in_uSizeLeft < in_pStream->GetNominalBuffering() ) ? AK_NoDataReady : AK_DataReady;
			}
			if( eResult == AK_NoMoreData )
			{
				return AK_DataReady;
			}
			return eResult;
		}
	}
}

/** 2012 0x95f2b0. */
CAkSrcFileBase::CAkSrcFileBase( CAkPBI * in_pCtx )
	: CAkSrcBaseEx( in_pCtx )
	, m_pStream( NULL )
	, m_pNextAddress( NULL )
	, m_ulSizeLeft( 0 )
	, m_ulFileOffset( 0 )
	, m_uiCorrection( 0 )
	, m_ulLoopStart( 0 )
	, m_ulLoopEnd( 0 )
	, m_uStreamLoopCntAhead( 0 )
	, m_bIsLastStmBuffer( 0 )
	, m_bSkipBufferRelease( 0 )
	, m_bFormatReady( 0 )
{
}

/** 2012 0x95f520. */
CAkSrcFileBase::~CAkSrcFileBase()
{
	if( m_pStream )
	{
		m_pStream->Destroy();
		m_pStream = NULL;
		CAkSrcBaseEx::StopStream();
	}
}

/** 2012 0x955c00: a prefetched buffer (the PBI's media, not the stream's) is skipped once instead of released. */
void CAkSrcFileBase::ReleaseStreamBuffer()
{
	if( m_bSkipBufferRelease )
	{
		m_bSkipBufferRelease = 0;
	}
	else
	{
		m_pStream->ReleaseBuffer();
	}
}

/** 2012 0x955c50. */
AKRESULT CAkSrcFileBase::OnLoopComplete( bool in_bEndOfFile )
{
	if( !in_bEndOfFile )
	{
		--m_uStreamLoopCntAhead;
	}
	return CAkSrcBaseEx::OnLoopComplete( in_bEndOfFile );
}

/** 2012 0x95f280. */
AKRESULT CAkSrcFileBase::IsInitialPrebufferingReady()
{
	if( !m_bWaitForBuffering )
	{
		return AK_Success;
	}
	const AKRESULT eResult = AK::SrcFileServices::IsPrebufferingReady( m_pStream, m_ulSizeLeft );
	if( eResult == AK_NoDataReady )
	{
		return AK_FormatNotReady;
	}
	if( eResult == AK_DataReady )
	{
		return AK_Success;
	}
	return eResult;
}

/** 2012 0x95f2f0: an automatic stream on the source's file, by name when externally supplied, else by ID. */
AKRESULT CAkSrcFileBase::CreateStream( AkAutoStmBufSettings & in_bufSettings, AkUInt8 in_uMinNumBuffers )
{
	const AkSrcTypeInfo & srcType = m_pCtx->m_pSource->m_sSrcTypeInfo;
	if( !srcType.pInMemoryMedia && srcType.mediaInfo.uFileID == AK_INVALID_FILE_ID )
	{
		return AK_Fail;
	}

	AkAutoStmHeuristics heuristics;
	heuristics.uMinNumBuffers = in_uMinNumBuffers;
	heuristics.fThroughput = 1.f;
	heuristics.uLoopStart = 0;
	heuristics.uLoopEnd = 0;
	heuristics.priority = (AkPriority)m_pCtx->GetPriority();

	AkFileSystemFlags fileSystemFlags;
	fileSystemFlags.uCompanyID = srcType.mediaInfo.bExternallySupplied;
	fileSystemFlags.uCodecID = CODECID_FROM_PLUGINID( srcType.dwID );
	fileSystemFlags.uCustomParamSize = 0;
	fileSystemFlags.pCustomParam = NULL;
	fileSystemFlags.bIsLanguageSpecific = srcType.mediaInfo.bIsLanguageSpecific != 0;
	fileSystemFlags.bIsFromRSX = srcType.mediaInfo.bIsFromRSX != 0;
	fileSystemFlags.bIsAutomaticStream = false;

	if( srcType.pInMemoryMedia )
	{
		return AK::IAkStreamMgr::Get()->CreateAuto( (const AkOSChar *)srcType.pInMemoryMedia, &fileSystemFlags, heuristics, &in_bufSettings, m_pStream, false );
	}
	return AK::IAkStreamMgr::Get()->CreateAuto( srcType.mediaInfo.uFileID, &fileSystemFlags, heuristics, &in_bufSettings, m_pStream, false );
}

/** 2012 0x95f3d0. */
void CAkSrcFileBase::StopStream()
{
	if( m_pStream )
	{
		m_pStream->Destroy();
		m_pStream = NULL;
	}
	CAkSrcBaseEx::StopStream();
}

/** 2012 0x95f3f0: finish the loop the stream is already reading, then stop looping the stream. */
AKRESULT CAkSrcFileBase::StopLooping()
{
	m_uLoopCnt = m_uStreamLoopCntAhead + 1;
	if( m_pStream )
	{
		AkAutoStmHeuristics heuristics;
		m_pStream->GetHeuristics( heuristics );
		heuristics.uLoopEnd = 0;
		m_pStream->SetHeuristics( heuristics );
	}
	return AK_Success;
}

/** 2012 0x95f440. */
void CAkSrcFileBase::VirtualOn( AkVirtualQueueBehavior eBehavior )
{
	m_pStream->Stop();
	if( eBehavior <= AkVirtualQueueBehavior_FromElapsedTime && m_ulSizeLeft )
	{
		ReleaseStreamBuffer();
		m_pNextAddress = NULL;
		m_ulSizeLeft = 0;
	}
}

/** 2012 0x95f4a0: the stream lands on its block boundary; the rest is the correction applied to the next buffer. */
AKRESULT CAkSrcFileBase::SetStreamPosition( AkUInt32 in_uPosition )
{
	AkInt64 lRealOffset;
	if( m_pStream->SetPosition( in_uPosition, AK_MoveBegin, &lRealOffset ) != AK_Success )
	{
		return AK_Fail;
	}
	m_uiCorrection = in_uPosition - (AkUInt32)lRealOffset;
	m_ulFileOffset = (AkUInt32)lRealOffset;
	return AK_Success;
}

/** 2012 0x95f4f0. */
void CAkSrcFileBase::GetStreamLoopHeuristic( bool in_bIsLooping, AkAutoStmHeuristics & io_heuristics )
{
	if( in_bIsLooping )
	{
		io_heuristics.uLoopStart = m_ulLoopStart;
		io_heuristics.uLoopEnd = m_ulLoopEnd;
	}
	else
	{
		io_heuristics.uLoopStart = 0;
		io_heuristics.uLoopEnd = 0;
	}
}

/** 2012 0x95f550: points at the buffer's data, cuts it at the loop end (the data end on the last loop) and rewinds the stream to the loop start. */
AKRESULT CAkSrcFileBase::ProcessStreamBuffer( AkUInt8 * in_pBuffer )
{
	const AkUInt32 ulSizeLeft = m_ulSizeLeft;
	m_ulFileOffset += ulSizeLeft;
	m_pNextAddress = in_pBuffer + m_uiCorrection;
	m_ulSizeLeft = ulSizeLeft - m_uiCorrection;

	const bool bIsLastLoop = m_uLoopCnt && m_uLoopCnt - m_uStreamLoopCntAhead == 1;
	const AkUInt32 ulEndLimit = bIsLastLoop ? m_uDataSize + m_uDataOffset : m_ulLoopEnd;
	if( m_ulFileOffset < ulEndLimit )
	{
		m_uiCorrection = 0;
		return AK_Success;
	}
	m_ulSizeLeft = ulEndLimit + m_ulSizeLeft - m_ulFileOffset;
	if( bIsLastLoop )
	{
		m_bIsLastStmBuffer = 1;
		return AK_Success;
	}
	if( SetStreamPosition( m_ulLoopStart ) != AK_Success )
	{
		return AK_Fail;
	}
	++m_uStreamLoopCntAhead;
	if( m_uLoopCnt && m_uLoopCnt - m_uStreamLoopCntAhead == 1 )
	{
		AkAutoStmHeuristics heuristics;
		m_pStream->GetHeuristics( heuristics );
		heuristics.uLoopEnd = 0;
		m_pStream->SetHeuristics( heuristics );
	}
	return AK_Success;
}

/** 2012 0x95f660. */
void CAkSrcFileBase::ResetStreamingAfterSeek()
{
	AkAutoStmHeuristics heuristics;
	m_pStream->GetHeuristics( heuristics );
	GetStreamLoopHeuristic( m_uLoopCnt != 1, heuristics );
	m_pStream->SetHeuristics( heuristics );
	m_bIsLastStmBuffer = 0;
	m_bWaitForBuffering = m_pCtx->m_bRequiresPreBuffering;
}

/** 2012 0x95f720: a prefetched source starts on the PBI's in-memory media and streams from where it ends. */
AKRESULT CAkSrcFileBase::HandlePrefetch( bool & out_bUsePrefetchedData )
{
	out_bUsePrefetchedData = false;
	if( !m_pCtx->m_pSource->m_sSrcTypeInfo.mediaInfo.bPrefetch || m_pCtx->m_bSeekDirty )
	{
		return AK_Success;
	}
	AkUInt8 * pDataPtr = m_pCtx->m_pDataPtr;
	m_ulSizeLeft = m_pCtx->m_uDataSize;
	out_bUsePrefetchedData = pDataPtr && m_ulSizeLeft;
	m_bSkipBufferRelease = out_bUsePrefetchedData;
	if( !out_bUsePrefetchedData )
	{
		return AK_Success;
	}
	AKRESULT eResult = ParseHeader( pDataPtr );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = ProcessStreamBuffer( pDataPtr );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( !m_uStreamLoopCntAhead )
	{
		eResult = SetStreamPosition( m_ulSizeLeft );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	m_pNextAddress += m_uDataOffset;
	m_ulSizeLeft -= m_uDataOffset;
	return AK_Success;
}

/** 2012 0x95f7c0. */
AKRESULT CAkSrcFileBase::FetchStreamBuffer()
{
	m_pNextAddress = NULL;
	AkAutoStmHeuristics heuristics;
	m_pStream->GetHeuristics( heuristics );
	heuristics.priority = (AkPriority)m_pCtx->GetPriority();
	m_pStream->SetHeuristics( heuristics );

	void * pBuffer;
	AKRESULT eResult = m_pStream->GetBuffer( pBuffer, m_ulSizeLeft, false );
	if( eResult != AK_DataReady && eResult != AK_NoMoreData )
	{
		return eResult;
	}
	if( !m_ulSizeLeft )
	{
		return AK_Fail;
	}
	eResult = ProcessStreamBuffer( (AkUInt8 *)pBuffer );
	return eResult == AK_Success ? AK_DataReady : eResult;
}

/** 2012 0x95f850. */
AKRESULT CAkSrcFileBase::SeekStream( AkUInt32 in_uDesiredSample, AkUInt32 & out_uSeekedSample )
{
	if( in_uDesiredSample >= m_uTotalSamples )
	{
		return AK_Fail;
	}
	AkUInt32 uFileOffset;
	if( FindClosestFileOffset( in_uDesiredSample, out_uSeekedSample, uFileOffset ) != AK_Success )
	{
		return AK_Fail;
	}
	m_uStreamLoopCntAhead = 0;
	if( SetStreamPosition( uFileOffset ) != AK_Success )
	{
		return AK_Fail;
	}
	ResetStreamingAfterSeek();
	return AK_Success;
}

/** 2012 0x95f8b0: what the stream could not reach (codec granularity) stays in the PBI as the remainder. */
AKRESULT CAkSrcFileBase::SeekToSourceOffset()
{
	const AkUInt32 uSourceOffset = GetSourceOffset();
	if( SeekStream( uSourceOffset, m_uCurSample ) != AK_Success )
	{
		return AK_Fail;
	}
	m_pCtx->SetSourceOffsetRemainder( uSourceOffset - m_uCurSample );
	return AK_Success;
}

/** 2012 0x95f900. */
AKRESULT CAkSrcFileBase::ChangeSourcePosition()
{
	const AkUInt32 uSourceOffset = GetSourceOffset();
	if( SeekStream( uSourceOffset, m_uCurSample ) != AK_Success )
	{
		return AK_Fail;
	}
	m_pCtx->SetSourceOffsetRemainder( uSourceOffset - m_uCurSample );
	ReleaseStreamBuffer();
	m_ulSizeLeft = 0;
	m_pNextAddress = NULL;
	return AK_Success;
}

/** 2012 0x95f970: the first stream buffer carries the header; a pending seek re-positions the stream instead of reading on. */
AKRESULT CAkSrcFileBase::ProcessFirstBuffer()
{
	m_bWaitForBuffering = m_pCtx->m_bRequiresPreBuffering;
	void * pBuffer;
	const AKRESULT eStmResult = m_pStream->GetBuffer( pBuffer, m_ulSizeLeft, false );
	if( eStmResult == AK_NoDataReady )
	{
		return AK_FormatNotReady;
	}
	if( eStmResult != AK_DataReady && eStmResult != AK_NoMoreData )
	{
		return AK_Fail;
	}
	AKRESULT eResult = ParseHeader( (AkUInt8 *)pBuffer );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( m_pCtx->m_bSeekDirty )
	{
		eResult = SeekToSourceOffset();
		if( m_ulSizeLeft )
		{
			ReleaseStreamBuffer();
			m_bFormatReady = 1;
			m_ulSizeLeft = 0;
			return eResult;
		}
	}
	else
	{
		const AkUInt32 uBufferFileOffset = m_uiCorrection + (AkUInt32)m_pStream->GetPosition( NULL );
		eResult = ProcessStreamBuffer( (AkUInt8 *)pBuffer );
		const AkUInt32 uSkip = m_uDataOffset - uBufferFileOffset;
		m_pNextAddress += uSkip;
		m_ulSizeLeft -= uSkip;
	}
	m_bFormatReady = 1;
	return eResult;
}

/** 2012 0x95fa40. */
AKRESULT CAkSrcFileBase::VirtualOff( AkVirtualQueueBehavior eBehavior, bool in_bUseSourceOffset )
{
	AKRESULT eResult;
	if( eBehavior == AkVirtualQueueBehavior_FromElapsedTime )
	{
		if( in_bUseSourceOffset )
		{
			eResult = SeekToSourceOffset();
		}
		else
		{
			if( SeekStream( m_uCurSample, m_uCurSample ) == AK_Success )
			{
				return m_pStream->Start();
			}
			eResult = SeekStream( 0, m_uCurSample );
		}
		if( eResult != AK_Success )
		{
			return eResult;
		}
		return m_pStream->Start();
	}
	if( eBehavior != AkVirtualQueueBehavior_FromBeginning )
	{
		if( eBehavior == AkVirtualQueueBehavior_Resume )
		{
			m_bWaitForBuffering = m_pCtx->m_bRequiresPreBuffering;
		}
		return m_pStream->Start();
	}
	m_uCurSample = 0;
	m_uLoopCnt = m_pCtx->m_LoopCount;
	eResult = SeekStream( 0, m_uCurSample );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return m_pStream->Start();
}

/** 2012 0x95fae0. */
AKRESULT CAkSrcFileBase::StartStream()
{
	if( m_bFormatReady )
	{
		return IsInitialPrebufferingReady();
	}
	if( !m_pStream )
	{
		AkAutoStmBufSettings bufSettings;
		bufSettings.uBufferSize = 0;
		bufSettings.uMinBufferSize = 2048;
		bufSettings.uBlockSize = 0;
		AKRESULT eResult = CreateStream( bufSettings, 0 );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		bool bUsePrefetchedData;
		eResult = HandlePrefetch( bUsePrefetchedData );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		eResult = m_pStream->Start();
		if( eResult != AK_Success || bUsePrefetchedData )
		{
			return eResult;
		}
	}
	const AKRESULT eResult = ProcessFirstBuffer();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return IsInitialPrebufferingReady();
}
