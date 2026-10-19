/*=============================================================================
	AkVPLSrcNode.cpp - CAkVPLSrcNode (akvplsrcnode.cpp, 2012 0x93dad0..0x93ddf0) and CAkVPLNode::CopyRelevantMarkers
	(akvplnode.cpp, 0x941ed0); 2013 matched by bytes.

	DISHONORED(port): agent IH2 fork S (wave 26). Create dispatches on the source type (1 streamed file, 2 plug-in
	generated, 3 in-memory) and the codec (the high word of the source's plug-in ID): PCM and ADPCM are built in,
	every other codec comes from CAkEffectsMgr::AllocCodec (Vorbis, registered by the glue).
=============================================================================*/
#include "AkVPLNode.h"
#include "AkSrcBankPCM.h"
#include "AkSrcFilePCM.h"
#include "AkSrcBankADPCM.h"
#include "AkSrcFileADPCM.h"
#include "AkSrcPhysModel.h"
#include "AkPositionRepository.h"
#include "AkPlayingMgr.h"
#include "AkHierarchy.h"
#include <math.h>

enum AkSrcType
{
	SrcTypeNone = 0,
	SrcTypeFile = 1,
	SrcTypeModelled = 2,
	SrcTypeMemory = 3
};

/** 2012 0x93ddb0. */
CAkVPLSrcNode::CAkVPLSrcNode( CAkPBI * in_pCtx )
	: m_pCtx( in_pCtx )
	, m_bIOReady( 0 )
	, m_bWaitForBuffering( 0 )
{
}

/** 2012 0x93dad0: a failed creation destroys the PBI (play failed). */
CAkVPLSrcNode * CAkVPLSrcNode::Create( CAkPBI * in_pCtx )
{
	const AkSrcTypeInfo & srcType = in_pCtx->m_pSource->m_sSrcTypeInfo;
	const AkUInt32 uSrcType = srcType.mediaInfo.Type;
	const AkUInt32 uCodecID = CODECID_FROM_PLUGINID( srcType.dwID );
	CAkVPLSrcNode * pSrc = NULL;
	void * pMem = NULL;

	if( uSrcType == SrcTypeModelled )
	{
		pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcPhysModel ) );
		pSrc = pMem ? AkPlacementNew( pMem ) CAkSrcPhysModel( in_pCtx ) : NULL;
	}
	else if( uSrcType != SrcTypeNone && uCodecID != AKCODECID_BANK )
	{
		if( uCodecID == AKCODECID_PCM )
		{
			if( uSrcType == SrcTypeFile )
			{
				pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcFilePCM ) );
				pSrc = pMem ? AkPlacementNew( pMem ) CAkSrcFilePCM( in_pCtx ) : NULL;
			}
			else if( uSrcType == SrcTypeMemory )
			{
				pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcBankPCM ) );
				pSrc = pMem ? AkPlacementNew( pMem ) CAkSrcBankPCM( in_pCtx ) : NULL;
			}
		}
		else if( uCodecID == AKCODECID_ADPCM )
		{
			if( uSrcType == SrcTypeFile )
			{
				pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcFileADPCM ) );
				pSrc = pMem ? AkPlacementNew( pMem ) CAkSrcFileADPCM( in_pCtx ) : NULL;
			}
			else if( uSrcType == SrcTypeMemory )
			{
				pMem = AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( CAkSrcBankADPCM ) );
				pSrc = pMem ? AkPlacementNew( pMem ) CAkSrcBankADPCM( in_pCtx ) : NULL;
			}
		}
		else
		{
			pSrc = static_cast<CAkVPLSrcNode *>( CAkEffectsMgr::AllocCodec( in_pCtx, uSrcType, srcType.dwID ) );
		}
	}

	if( !pSrc )
	{
		in_pCtx->Destroy( CtxDestroyReasonPlayFailed );
	}
	return pSrc;
}

/** 2012 0x93dc00. */
void CAkVPLSrcNode::Term( AkCtxDestroyReason in_eReason )
{
	if( m_pCtx->m_uRegisteredNotif & AK_EnableGetSourcePlayPosition )
	{
		g_pPositionRepository->RemoveSource( m_pCtx->GetPlayingID(), this );
	}
	m_pCtx->Destroy( in_eReason );
	StopStream();
}

/** 2012 0x93dc40: the PBI starts with the source's duration scaled by its pitch; the playing ID is told both. */
void CAkVPLSrcNode::Start()
{
	AkReal32 fOriginalDuration = 0.f;
	if( m_bIOReady )
	{
		fOriginalDuration = GetDuration();
	}
	m_pCtx->CalcEffectiveParams();
	const AkReal32 fEstimatedDuration = (AkReal32)( fOriginalDuration / pow( 2.0, m_pCtx->m_EffectiveParams.Pitch * 0.00083333335f ) );
	m_pCtx->Play( fEstimatedDuration );
	g_pPlayingMgr->NotifyDuration( m_pCtx->GetPlayingID(), fOriginalDuration, fEstimatedDuration, m_pCtx->GetSoundID() );
	if( m_pCtx->m_uRegisteredNotif & AK_EnableGetSourcePlayPosition )
	{
		g_pPositionRepository->AddSource( m_pCtx->GetPlayingID(), this );
	}
}

/** 2012 0x93dcf0. */
void CAkVPLSrcNode::Stop()
{
	m_pCtx->Stop();
}

/** 2012 0x93dd00. */
void CAkVPLSrcNode::Pause()
{
	m_pCtx->Pause();
	g_pPositionRepository->SetRate( m_pCtx->GetPlayingID(), 0.f );
}

/** 2012 0x93dd30. */
void CAkVPLSrcNode::Resume( AkReal32 in_fOutputRate )
{
	m_pCtx->Resume();
	g_pPositionRepository->SetRate( m_pCtx->GetPlayingID(), in_fOutputRate );
}

/** 2012 0x93dd60. */
AKRESULT CAkVPLSrcNode::Seek()
{
	if( m_pCtx->m_bSeekDirty )
	{
		return ChangeSourcePosition();
	}
	return AK_Success;
}

/** 2012 0x93dd80: StartStream until the source has its format and first data. */
AKRESULT CAkVPLSrcNode::FetchStreamedData()
{
	if( m_bIOReady )
	{
		return AK_Success;
	}
	const AKRESULT eResult = StartStream();
	if( eResult == AK_Success )
	{
		m_bIOReady = 1;
	}
	return eResult;
}

/** 2012 0x93ddf0: a virtual voice pulls one buffer and drops it. */
AKRESULT CAkVPLSrcNode::TimeSkip( AkUInt32 & io_uFrames )
{
	AkVPLState state;
	state.buffer.pData = NULL;
	state.buffer.uMaxFrames = (AkUInt16)io_uFrames;
	state.buffer.uNumMarkers = 0;
	state.buffer.pMarkers = NULL;
	GetBuffer( state );
	if( !state.buffer.pData )
	{
		io_uFrames = 0;
		return state.result;
	}
	io_uFrames = state.buffer.uValidFrames;
	ReleaseBuffer();
	return state.result;
}

/** 2012 0x941ed0: appends the input's markers that fall in [start, start + frames) to the output buffer's list. */
void CAkVPLNode::CopyRelevantMarkers( AkPipelineBuffer * in_pInputBuffer, AkPipelineBuffer * io_pBuffer, AkUInt32 in_ulBufferStartOffset, AkUInt32 in_ulNumFrames )
{
	if( !in_pInputBuffer->pMarkers )
	{
		return;
	}
	AkUInt16 l_usNbMarkersToCopy = 0;
	for( AkUInt32 i = 0; i < in_pInputBuffer->uNumMarkers; ++i )
	{
		const AkUInt32 dwPosition = in_pInputBuffer->pMarkers[ i ].dwPositionInBuffer;
		if( dwPosition >= in_ulBufferStartOffset && dwPosition < in_ulBufferStartOffset + in_ulNumFrames )
		{
			++l_usNbMarkersToCopy;
		}
	}
	if( !l_usNbMarkersToCopy )
	{
		return;
	}
	AkBufferMarker * l_pNewList = (AkBufferMarker *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( AkBufferMarker ) * ( io_pBuffer->uNumMarkers + l_usNbMarkersToCopy ) );
	if( !l_pNewList )
	{
		io_pBuffer->FreeMarkers();
		return;
	}
	if( io_pBuffer->pMarkers )
	{
		memcpy( l_pNewList, io_pBuffer->pMarkers, sizeof( AkBufferMarker ) * io_pBuffer->uNumMarkers );
	}
	AkBufferMarker * pNewMarker = l_pNewList + io_pBuffer->uNumMarkers;
	for( AkUInt32 i = 0; i < in_pInputBuffer->uNumMarkers; ++i )
	{
		const AkBufferMarker & rMarker = in_pInputBuffer->pMarkers[ i ];
		if( rMarker.dwPositionInBuffer >= in_ulBufferStartOffset && rMarker.dwPositionInBuffer < in_ulBufferStartOffset + in_ulNumFrames )
		{
			pNewMarker->pContext = rMarker.pContext;
			pNewMarker->dwPositionInBuffer = 0;
			pNewMarker->marker = rMarker.marker;
			++pNewMarker;
		}
	}
	io_pBuffer->FreeMarkers();
	io_pBuffer->uNumMarkers += l_usNbMarkersToCopy;
	io_pBuffer->pMarkers = l_pNewList;
}
