/*=============================================================================
	AkRanSeqCntr.cpp - the playback half of the random / sequence container (akranseqcntr.cpp, 2012 0x94d640..0x94f8e0),
	its playlists (akplaylist.cpp, 0x95e630..0x95e910) and per-game-object selection state (akranseqbaseinfo.cpp,
	0x95dee0..0x95e280); 2013 matched by bytes (ih1_inventory.csv).

	DISHONORED(port): agent IH3, fork C (wave 26).
	Deviations (named):
	  * RandomizerModifier::GetModValue (a header template of the SDK) is the file-static GetModValue below.
	  * AkTimeConv::MillisecondsToSamples (header inline) is not used here.
	  * The selection's rand() is the CRT's, as retail (the audio thread seeds it, AkAudioMgr.cpp).
=============================================================================*/
#include "AkContainers.h"
#include "AkContinuousPBI.h"
#include "AkAudioMgr.h"
#include <stdlib.h>

static AkReal32 GetModValue( const RANGED_PARAMETER<AkReal32> & in_rParam )
{
	AkReal32 fValue = in_rParam.m_base;
	const AkReal32 fRange = in_rParam.m_mod.m_max - in_rParam.m_mod.m_min;
	if( fRange != 0.f )
	{
		fValue += (AkReal32)( (double)rand() * ( 1.0 / 32767.0 ) * fRange );
	}
	return fValue + in_rParam.m_mod.m_min;
}

// ------------------------------------------------------------------------------------------------------------
// CAkPlayListRandom / CAkPlayListSequence
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95e650. */
AkUInt32 CAkPlayListRandom::Length()
{
	return m_PlayList.Length();
}

/** 2012 0x95e660. */
AkUniqueID CAkPlayListRandom::ID( AkUInt16 in_wPosition )
{
	return m_PlayList[ in_wPosition ].ulID;
}

/** 2012 0x95e670. */
void CAkPlayListRandom::Remove( AkUniqueID in_ID )
{
	AkPlaylistItem * pItem = m_PlayList.m_pItems;
	while( pItem != m_PlayList.m_pItemsEnd )
	{
		if( pItem->ulID == in_ID )
		{
			for( AkPlaylistItem * p = pItem; p < m_PlayList.m_pItemsEnd - 1; ++p )
			{
				*p = *( p + 1 );
			}
			--m_PlayList.m_pItemsEnd;
		}
		else
		{
			++pItem;
		}
	}
}

/** 2012 0x95e6c0. */
void CAkPlayListRandom::SetWeight( AkUInt16 in_wPosition, AkUInt32 in_weight )
{
	m_PlayList[ in_wPosition ].weight = in_weight;
}

/** 2012 0x95e6e0. */
AkUInt32 CAkPlayListRandom::GetWeight( AkUInt16 in_wPosition )
{
	return m_PlayList[ in_wPosition ].weight;
}

/** 2012 0x95e6f0. */
AkUInt32 CAkPlayListRandom::CalculateTotalWeight()
{
	AkUInt32 uTotal = 0;
	for( AkPlaylistItem * p = m_PlayList.m_pItems; p != m_PlayList.m_pItemsEnd; ++p )
	{
		uTotal += p->weight;
	}
	return uTotal;
}

/** 2012 0x95e710. */
bool CAkPlayListRandom::Exists( AkUniqueID in_ID )
{
	for( AkPlaylistItem * p = m_PlayList.m_pItems; p != m_PlayList.m_pItemsEnd; ++p )
	{
		if( p->ulID == in_ID )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x95e740. */
bool CAkPlayListRandom::GetPosition( AkUniqueID in_ID, AkUInt16 & out_rwPosition )
{
	AkUInt16 wPosition = 0;
	for( AkPlaylistItem * p = m_PlayList.m_pItems; p != m_PlayList.m_pItemsEnd; ++p, ++wPosition )
	{
		if( p->ulID == in_ID )
		{
			out_rwPosition = wPosition;
			return true;
		}
	}
	return false;
}

/** 2012 0x95e7c0. */
AKRESULT CAkPlayListRandom::Add( AkUniqueID in_ulID, AkUInt32 in_weight )
{
	AkPlaylistItem * pItem = m_PlayList.AddLast();
	if( !pItem )
	{
		return AK_Fail;
	}
	pItem->ulID = in_ulID;
	pItem->weight = in_weight;
	return AK_Success;
}

/** 2012 0x95e830 (shared by both playlists). */
void CAkPlayListRandom::RemoveAll()
{
	m_PlayList.m_pItemsEnd = m_PlayList.m_pItems;
}

/** The SDK's CAkPlayListRandom::Destroy (same body as 0x95e8b0 on the random array). */
void CAkPlayListRandom::Destroy()
{
	m_PlayList.Term();
	this->~CAkPlayListRandom();
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x95e790. */
AkUInt32 CAkPlayListSequence::GetWeight( AkUInt16 )
{
	return 50000;
}

/** 2012 0x95e7a0. */
AkUInt32 CAkPlayListSequence::Length()
{
	return m_PlayList.Length();
}

/** 2012 0x95e7b0. */
AkUniqueID CAkPlayListSequence::ID( AkUInt16 in_wPosition )
{
	return m_PlayList[ in_wPosition ];
}

/** 2012 0x95e820. */
void CAkPlayListSequence::Remove( AkUniqueID in_ulID )
{
	m_PlayList.Remove( in_ulID );
}

void CAkPlayListSequence::RemoveAll()
{
	m_PlayList.m_pItemsEnd = m_PlayList.m_pItems;
}

/** 2012 0x95e840. */
bool CAkPlayListSequence::Exists( AkUniqueID in_ulID )
{
	return m_PlayList.Exists( in_ulID ) != NULL;
}

/** 2012 0x95e870. */
bool CAkPlayListSequence::GetPosition( AkUniqueID in_ID, AkUInt16 & out_rwPosition )
{
	for( AkUniqueID * p = m_PlayList.m_pItems; p != m_PlayList.m_pItemsEnd; ++p )
	{
		if( *p == in_ID )
		{
			out_rwPosition = (AkUInt16)( p - m_PlayList.m_pItems );
			return true;
		}
	}
	return false;
}

/** 2012 0x95e8b0. */
void CAkPlayListSequence::Destroy()
{
	m_PlayList.Term();
	this->~CAkPlayListSequence();
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x95e910. */
AKRESULT CAkPlayListSequence::Add( AkUniqueID in_ulID, AkUInt32 )
{
	AkUniqueID * pItem = m_PlayList.AddLast();
	if( !pItem )
	{
		return AK_Fail;
	}
	*pItem = in_ulID;
	return AK_Success;
}

// ------------------------------------------------------------------------------------------------------------
// CAkRandomInfo / CAkSequenceInfo
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95e120. */
CAkRandomInfo::CAkRandomInfo( AkUInt16 in_wItemCount )
	: m_ulTotalWeight( 50000 * in_wItemCount )
	, m_ulRemainingWeight( 50000 * in_wItemCount )
	, m_wRemainingItemsToPlay( in_wItemCount )
	, m_wCounter( in_wItemCount )
	, m_pcArrayBeenPlayedFlag( NULL )
	, m_pcArrayBlockedFlag( NULL )
{
}

/** 2012 0x95e200. */
CAkRandomInfo::~CAkRandomInfo()
{
	if( m_pcArrayBeenPlayedFlag )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pcArrayBeenPlayedFlag );
	}
	m_listAvoid.Term();
}

/** 2012 0x95e160: the played and blocked flag arrays in one allocation, the avoid list up to 4 entries. */
AKRESULT CAkRandomInfo::Init( AkUInt16 in_wAvoidRepeatCount )
{
	const AkUInt32 uFlagBytes = ( m_wCounter + 7 ) / 8;
	m_pcArrayBeenPlayedFlag = (char *)AK::MemoryMgr::Malloc( g_DefaultPoolId, 2 * uFlagBytes );
	if( !m_pcArrayBeenPlayedFlag )
	{
		return AK_Fail;
	}
	m_pcArrayBlockedFlag = m_pcArrayBeenPlayedFlag + uFlagBytes;
	memset( m_pcArrayBeenPlayedFlag, 0, 2 * uFlagBytes );
	const AkUInt32 uAvoid = in_wAvoidRepeatCount >= 4 ? 4 : in_wAvoidRepeatCount;
	if( uAvoid && m_listAvoid.Reserve( uAvoid ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	return AK_Success;
}

void CAkRandomInfo::Destroy()
{
	this->~CAkRandomInfo();
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x95e280. */
CAkContainerBaseInfo * CAkRandomInfo::Clone( AkUInt16 in_wItemCount )
{
	if( !in_wItemCount )
	{
		return NULL;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRandomInfo ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkRandomInfo * pClone = AkPlacementNew( pMem ) CAkRandomInfo( in_wItemCount );
	pClone->m_ulTotalWeight = m_ulTotalWeight;
	pClone->m_ulRemainingWeight = m_ulRemainingWeight;
	pClone->m_wRemainingItemsToPlay = m_wRemainingItemsToPlay;
	pClone->m_wCounter = m_wCounter;
	if( m_pcArrayBeenPlayedFlag )
	{
		const AkUInt32 uFlagBytes = ( in_wItemCount + 7 ) / 8;
		pClone->m_pcArrayBeenPlayedFlag = (char *)AK::MemoryMgr::Malloc( g_DefaultPoolId, 2 * uFlagBytes );
		if( !pClone->m_pcArrayBeenPlayedFlag )
		{
			pClone->Destroy();
			return NULL;
		}
		pClone->m_pcArrayBlockedFlag = pClone->m_pcArrayBeenPlayedFlag + uFlagBytes;
		memcpy( pClone->m_pcArrayBeenPlayedFlag, m_pcArrayBeenPlayedFlag, 2 * uFlagBytes );
	}
	pClone->m_listAvoid.Reserve( m_listAvoid.Length() );
	for( AkUInt16 * p = m_listAvoid.m_pItems; p != m_listAvoid.m_pItemsEnd; ++p )
	{
		if( !pClone->m_listAvoid.AddLast( *p ) )
		{
			pClone->Destroy();
			return NULL;
		}
	}
	return pClone;
}

/** 2012 0x95dee0. */
void CAkRandomInfo::FlagSetPlayed( AkUInt16 in_wPosition )
{
	m_pcArrayBeenPlayedFlag[ in_wPosition >> 3 ] |= 1 << ( in_wPosition & 7 );
}

/** 2012 0x95df10. */
bool CAkRandomInfo::IsFlagSetPlayed( AkUInt16 in_wPosition ) const
{
	return ( m_pcArrayBeenPlayedFlag[ in_wPosition >> 3 ] & ( 1 << ( in_wPosition & 7 ) ) ) != 0;
}

/** 2012 0x95df50. */
void CAkRandomInfo::ResetFlagsPlayed( AkUInt32 in_PlaylistSize )
{
	memset( m_pcArrayBeenPlayedFlag, 0, ( in_PlaylistSize + 7 ) >> 3 );
	m_wRemainingItemsToPlay = (AkUInt16)in_PlaylistSize;
}

/** 2012 0x95df80. */
void CAkRandomInfo::FlagAsBlocked( AkUInt16 in_wPosition )
{
	m_pcArrayBlockedFlag[ in_wPosition >> 3 ] |= 1 << ( in_wPosition & 7 );
}

/** 2012 0x95dfb0. */
void CAkRandomInfo::FlagAsUnBlocked( AkUInt16 in_wPosition )
{
	m_pcArrayBlockedFlag[ in_wPosition >> 3 ] &= ~( 1 << ( in_wPosition & 7 ) );
}

/** 2012 0x95dfe0. */
bool CAkRandomInfo::IsFlagBlocked( AkUInt16 in_wPosition ) const
{
	return ( m_pcArrayBlockedFlag[ in_wPosition >> 3 ] & ( 1 << ( in_wPosition & 7 ) ) ) != 0;
}

/** 2012 0x95e020. */
AkInt32 CAkRandomInfo::GetRandomValue()
{
	return (AkInt32)( (AkUInt32)( ( rand() << 15 ) + rand() ) % m_ulRemainingWeight );
}

/** 2012 0x95e0f0. */
void CAkSequenceInfo::Destroy()
{
	this->~CAkSequenceInfo();
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x95e070. */
CAkContainerBaseInfo * CAkSequenceInfo::Clone( AkUInt16 )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSequenceInfo ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSequenceInfo * pClone = AkPlacementNew( pMem ) CAkSequenceInfo();
	pClone->m_bIsForward = m_bIsForward;
	pClone->m_i16LastPositionChosen = m_i16LastPositionChosen;
	return pClone;
}

// ------------------------------------------------------------------------------------------------------------
// CAkRanSeqCntr
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94d640: the special-transition RTPC (param 4) is read by TransitionTime, never pushed to PBIs. */
AKRESULT CAkRanSeqCntr::SetParamComplexFromRTPCManager( void * in_pToken, AkUInt32 in_Param_id, AkRtpcID in_RTPCid, AkReal32 in_value, CAkRegisteredObj * in_GameObj, void * in_pGameObjExceptArray )
{
	if( in_Param_id == RTPC_PlayMechanismSpecialTransitionsValue )
	{
		return AK_Success;
	}
	return CAkParameterNodeBase::SetParamComplexFromRTPCManager( in_pToken, in_Param_id, in_RTPCid, in_value, in_GameObj, in_pGameObjExceptArray );
}

/** 2012 0x94d680. */
CAkSequenceInfo * CAkRanSeqCntr::CreateSequenceInfo()
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSequenceInfo ) );
	return pMem ? AkPlacementNew( pMem ) CAkSequenceInfo() : NULL;
}

/** 2012 0x94d6a0. */
AkTransitionMode CAkRanSeqCntr::TransitionMode()
{
	return (AkTransitionMode)m_RanSeqParams.m_eTransitionMode;
}

/** 2012 0x94d6b0. */
AkUInt32 CAkRanSeqCntr::GetPlaylistLength()
{
	return m_pPlayList ? m_pPlayList->Length() : 0;
}

/** 2012 0x94d6d0. */
bool CAkRanSeqCntr::CanContinueAfterCompleteLoop( AkLoop * io_pLoopingInfo )
{
	if( !io_pLoopingInfo )
	{
		return true;
	}
	if( !io_pLoopingInfo->bIsEnabled )
	{
		return false;
	}
	return io_pLoopingInfo->bIsInfinite || --io_pLoopingInfo->lLoopCount != 0;
}

/** 2012 0x94d700. */
bool CAkRanSeqCntr::CanPlayPosition( const CAkRandomInfo * in_pRandomInfo, AkUInt16 in_wPosition ) const
{
	if( m_RanSeqParams.m_eRandomMode != RandomMode_Normal )
	{
		return !in_pRandomInfo->IsFlagSetPlayed( in_wPosition ) && !in_pRandomInfo->IsFlagBlocked( in_wPosition );
	}
	return !m_RanSeqParams.m_wAvoidRepeatCount || !in_pRandomInfo->IsFlagBlocked( in_wPosition );
}

/** 2012 0x94d770: the continuous play path makes a CAkContinuousPBI. */
CAkPBI * CAkRanSeqCntr::CreatePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams, const PriorityInfoCurrent & in_rPriority, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter ) const
{
	if( in_rPBIParams.eType == AkPBIParams::ContinuousPBI )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkContinuousPBI ) );
		if( !pMem )
		{
			return NULL;
		}
		return AkPlacementNew( pMem ) CAkContinuousPBI( in_pSound, in_pSource, in_rPBIParams.pGameObj, *in_rPBIParams.pContinuousParams, in_rPBIParams.userParams, in_rPBIParams.playHistory, in_rPBIParams.bIsFirst, in_rPBIParams.sequenceID, in_rPBIParams.pInstigator, in_rPriority, in_rPBIParams.bTargetFeedback, in_pAMLimiter, in_pBusLimiter );
	}
	return CAkPBIAware::CreatePBI( in_pSound, in_pSource, in_rPBIParams, in_rPriority, in_pAMLimiter, in_pBusLimiter );
}

/** 2012 0x94d9f0: weights from the playlist when the container uses them. */
CAkRandomInfo * CAkRanSeqCntr::CreateRandomInfo( AkUInt16 in_uPlaylistSize )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRandomInfo ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkRandomInfo * pInfo = AkPlacementNew( pMem ) CAkRandomInfo( in_uPlaylistSize );
	if( pInfo->Init( m_RanSeqParams.m_wAvoidRepeatCount ) != AK_Success )
	{
		pInfo->Destroy();
		return NULL;
	}
	if( m_RanSeqParams.m_bIsUsingWeight )
	{
		const AkUInt32 uTotal = static_cast<CAkPlayListRandom *>( m_pPlayList )->CalculateTotalWeight();
		pInfo->m_ulRemainingWeight = uTotal;
		pInfo->m_ulTotalWeight = uTotal;
	}
	return pInfo;
}

/** 2012 0x94da90: the authored time and its range, plus the special-transition RTPC in seconds. */
AkReal32 CAkRanSeqCntr::TransitionTime( CAkRegisteredObj * in_GameObj )
{
	AkReal32 fTime;
	if( m_RTPCBitArrayMax32.IsSet( RTPC_PlayMechanismSpecialTransitionsValue ) )
	{
		const AkReal32 fRTPCTime = g_pRTPCMgr->GetRTPCConvertedValue( this, RTPC_PlayMechanismSpecialTransitionsValue, in_GameObj ) * 1000.f;
		fTime = GetModValue( m_RanSeqParams.m_TransitionTime ) + fRTPCTime;
	}
	else
	{
		fTime = GetModValue( m_RanSeqParams.m_TransitionTime );
	}
	return fTime >= 0.f ? fTime : 0.f;
}

/** 2012 0x94db10. */
AkUInt16 CAkRanSeqCntr::SelectSequentially( CAkSequenceInfo * in_pSeqInfo, bool & out_bIsAnswerValid, AkLoop * io_pLoopCount )
{
	out_bIsAnswerValid = true;
	if( in_pSeqInfo->m_bIsForward )
	{
		const AkInt16 iLast = in_pSeqInfo->m_i16LastPositionChosen;
		if( (AkUInt32)( iLast + 1 ) != m_pPlayList->Length() )
		{
			in_pSeqInfo->m_i16LastPositionChosen = iLast + 1;
			return (AkUInt16)( iLast + 1 );
		}
		if( m_RanSeqParams.m_bIsRestartBackward )
		{
			in_pSeqInfo->m_i16LastPositionChosen = iLast - 1;
			in_pSeqInfo->m_bIsForward = false;
			return (AkUInt16)( iLast - 1 );
		}
		in_pSeqInfo->m_i16LastPositionChosen = 0;
		if( !CanContinueAfterCompleteLoop( io_pLoopCount ) )
		{
			out_bIsAnswerValid = false;
			return 0;
		}
	}
	else if( in_pSeqInfo->m_i16LastPositionChosen )
	{
		--in_pSeqInfo->m_i16LastPositionChosen;
	}
	else
	{
		in_pSeqInfo->m_i16LastPositionChosen = 1;
		in_pSeqInfo->m_bIsForward = true;
		if( io_pLoopCount && ( !io_pLoopCount->bIsEnabled || ( !io_pLoopCount->bIsInfinite && !--io_pLoopCount->lLoopCount ) ) )
		{
			out_bIsAnswerValid = false;
			return 0;
		}
	}
	return (AkUInt16)in_pSeqInfo->m_i16LastPositionChosen;
}

/** 2012 0x94e470. */
void CAkRanSeqCntr::Unregister( CAkRegisteredObj * in_pGameObj )
{
	CAkParameterNode::Unregister( in_pGameObj );
	CAkContainerBaseInfo * pInfo = m_mapObjectCntrInfo.Unset( in_pGameObj );
	if( pInfo )
	{
		pInfo->Destroy();
	}
}

/** 2012 0x94e4c0. */
void CAkRanSeqCntr::DestroySpecificInfo()
{
	for( AkUInt32 i = 0; i < 31; ++i )
	{
		CAkContainerBaseInfo * pItem = m_mapObjectCntrInfo.m_table[ i ];
		while( pItem )
		{
			CAkContainerBaseInfo * pNext = pItem->pNextItem;
			pItem->Destroy();
			pItem = pNext;
		}
	}
	m_mapObjectCntrInfo.RemoveAll();
	if( m_pGlobalContainerInfo )
	{
		m_pGlobalContainerInfo->Destroy();
		m_pGlobalContainerInfo = NULL;
	}
}

/** 2012 0x94e6c0. */
CAkSequenceInfo * CAkRanSeqCntr::GetExistingSequenceInfo( CAkRegisteredObj * in_pGameObj )
{
	if( m_RanSeqParams.m_bIsGlobal )
	{
		if( !m_pGlobalContainerInfo )
		{
			m_pGlobalContainerInfo = CreateSequenceInfo();
		}
		return static_cast<CAkSequenceInfo *>( m_pGlobalContainerInfo );
	}
	CAkContainerBaseInfo * pInfo = m_mapObjectCntrInfo.Exists( in_pGameObj );
	if( pInfo )
	{
		return static_cast<CAkSequenceInfo *>( pInfo );
	}
	CAkSequenceInfo * pSeqInfo = CreateSequenceInfo();
	if( !pSeqInfo )
	{
		return NULL;
	}
	if( in_pGameObj->SetNodeAsModified( this ) != AK_Success )
	{
		pSeqInfo->Destroy();
		return NULL;
	}
	pSeqInfo->key = in_pGameObj;
	m_mapObjectCntrInfo.Set( pSeqInfo );
	return pSeqInfo;
}

/** 2012 0x94e790. */
CAkRandomInfo * CAkRanSeqCntr::GetExistingRandomInfo( AkUInt16 in_uPlaylistSize, CAkRegisteredObj * in_pGameObj )
{
	if( m_RanSeqParams.m_bIsGlobal )
	{
		if( !m_pGlobalContainerInfo )
		{
			m_pGlobalContainerInfo = CreateRandomInfo( in_uPlaylistSize );
		}
		return static_cast<CAkRandomInfo *>( m_pGlobalContainerInfo );
	}
	CAkContainerBaseInfo * pInfo = m_mapObjectCntrInfo.Exists( in_pGameObj );
	if( pInfo )
	{
		return static_cast<CAkRandomInfo *>( pInfo );
	}
	CAkRandomInfo * pRandomInfo = CreateRandomInfo( in_uPlaylistSize );
	if( !pRandomInfo )
	{
		return NULL;
	}
	if( in_pGameObj->SetNodeAsModified( this ) != AK_Success )
	{
		pRandomInfo->Destroy();
		return NULL;
	}
	pRandomInfo->key = in_pGameObj;
	m_mapObjectCntrInfo.Set( pRandomInfo );
	return pRandomInfo;
}

/** 2012 0x94e840: forget every selection state; a container that ever played also drops its pending actions and stops. */
void CAkRanSeqCntr::ResetSpecificInfo()
{
	DestroySpecificInfo();
	if( m_bContainerBeenPlayed )
	{
		if( g_pAudioMgr )
		{
			g_pAudioMgr->RemovePausedPendingAction( key );
			g_pAudioMgr->RemovePendingAction( key );
		}
		Stop( NULL, AK_INVALID_PLAYING_ID, 0, AkCurveInterpolation_Linear );
	}
}

/** 2012 0x94e890. */
AKRESULT CAkRanSeqCntr::UpdateNormalAvoidRepeat( CAkRandomInfo * in_pRandomInfo, AkUInt16 in_uPosition )
{
	CAkPlayList * pPlayList = m_pPlayList;
	if( !m_RanSeqParams.m_wAvoidRepeatCount )
	{
		return AK_Success;
	}
	--in_pRandomInfo->m_wRemainingItemsToPlay;
	if( !in_pRandomInfo->m_listAvoid.AddLast( in_uPosition ) )
	{
		in_pRandomInfo->m_wCounter = 0;
		return AK_Fail;
	}
	in_pRandomInfo->FlagAsBlocked( in_uPosition );
	in_pRandomInfo->m_ulRemainingWeight -= pPlayList->GetWeight( in_uPosition );
	AkUInt32 uAvoid = m_pPlayList->Length() - 1;
	if( m_RanSeqParams.m_wAvoidRepeatCount < uAvoid )
	{
		uAvoid = m_RanSeqParams.m_wAvoidRepeatCount;
	}
	if( in_pRandomInfo->m_listAvoid.Length() > uAvoid )
	{
		const AkUInt16 wFreed = in_pRandomInfo->m_listAvoid[ 0 ];
		in_pRandomInfo->FlagAsUnBlocked( wFreed );
		in_pRandomInfo->m_ulRemainingWeight += pPlayList->GetWeight( wFreed );
		++in_pRandomInfo->m_wRemainingItemsToPlay;
		in_pRandomInfo->m_listAvoid.Erase( 0 );
	}
	return AK_Success;
}

/** 2012 0x94e9a0: a weighted or uniform pick among the positions still allowed (shuffle: not yet played in this pass). */
AkUInt16 CAkRanSeqCntr::SelectRandomly( CAkRandomInfo * in_pRandomInfo, bool & out_bIsAnswerValid, AkLoop * io_pLoopCount )
{
	out_bIsAnswerValid = true;
	CAkPlayList * pPlayList = m_pPlayList;
	AkUInt16 wPosition = 0;
	if( !in_pRandomInfo->m_wCounter )
	{
		if( io_pLoopCount && ( !io_pLoopCount->bIsEnabled || ( !io_pLoopCount->bIsInfinite && !--io_pLoopCount->lLoopCount ) ) )
		{
			out_bIsAnswerValid = false;
			return 0;
		}
		in_pRandomInfo->m_wCounter = (AkUInt16)m_pPlayList->Length();
		in_pRandomInfo->ResetFlagsPlayed( m_pPlayList->Length() );
		if( m_RanSeqParams.m_eRandomMode == RandomMode_Shuffle )
		{
			in_pRandomInfo->m_ulRemainingWeight = in_pRandomInfo->m_ulTotalWeight;
			for( AkUInt16 * p = in_pRandomInfo->m_listAvoid.m_pItems; p != in_pRandomInfo->m_listAvoid.m_pItemsEnd; ++p )
			{
				in_pRandomInfo->m_ulRemainingWeight -= pPlayList->GetWeight( *p );
			}
		}
		in_pRandomInfo->m_wRemainingItemsToPlay -= (AkUInt16)in_pRandomInfo->m_listAvoid.Length();
	}
	if( !m_RanSeqParams.m_bIsUsingWeight )
	{
		const AkInt16 iRandom = (AkInt16)( rand() % in_pRandomInfo->m_wRemainingItemsToPlay );
		AkInt16 iCount = -1;
		do
		{
			if( CanPlayPosition( in_pRandomInfo, wPosition ) )
			{
				++iCount;
			}
			++wPosition;
		}
		while( iCount < iRandom );
	}
	else
	{
		const AkInt32 iRandom = in_pRandomInfo->GetRandomValue();
		AkInt32 iCount = -1;
		if( iRandom > -1 )
		{
			do
			{
				if( CanPlayPosition( in_pRandomInfo, wPosition ) )
				{
					iCount += pPlayList->GetWeight( wPosition );
				}
				++wPosition;
			}
			while( iCount < iRandom );
		}
	}
	--wPosition;
	if( m_RanSeqParams.m_eRandomMode == RandomMode_Normal )
	{
		if( !in_pRandomInfo->IsFlagSetPlayed( wPosition ) )
		{
			in_pRandomInfo->FlagSetPlayed( wPosition );
			--in_pRandomInfo->m_wCounter;
		}
		UpdateNormalAvoidRepeat( in_pRandomInfo, wPosition );
		return wPosition;
	}
	const AkUInt32 uAvoidCount = m_RanSeqParams.m_wAvoidRepeatCount ? m_RanSeqParams.m_wAvoidRepeatCount : 1;
	in_pRandomInfo->m_ulRemainingWeight -= pPlayList->GetWeight( wPosition );
	--in_pRandomInfo->m_wRemainingItemsToPlay;
	--in_pRandomInfo->m_wCounter;
	in_pRandomInfo->FlagSetPlayed( wPosition );
	if( !in_pRandomInfo->m_listAvoid.AddLast( wPosition ) )
	{
		in_pRandomInfo->m_wCounter = 0;
		return wPosition;
	}
	in_pRandomInfo->FlagAsBlocked( wPosition );
	AkUInt32 uAvoid = m_pPlayList->Length() - 1;
	if( uAvoidCount < uAvoid )
	{
		uAvoid = uAvoidCount;
	}
	if( in_pRandomInfo->m_listAvoid.Length() > uAvoid )
	{
		const AkUInt16 wFreed = in_pRandomInfo->m_listAvoid[ 0 ];
		in_pRandomInfo->m_listAvoid.Erase( 0 );
		in_pRandomInfo->FlagAsUnBlocked( wFreed );
		if( !in_pRandomInfo->IsFlagSetPlayed( wFreed ) )
		{
			in_pRandomInfo->m_ulRemainingWeight += pPlayList->GetWeight( wFreed );
			++in_pRandomInfo->m_wRemainingItemsToPlay;
		}
	}
	return wPosition;
}

/** 2012 0x94ecd0: a sequence restarted at each play leaves the shared (global or game object) state one step behind
    the continuous pass's copy, so the next play resumes where this one is. */
void CAkRanSeqCntr::UpdateResetPlayListSetup( CAkSequenceInfo * in_pSeqInfo, CAkRegisteredObj * in_pGameObj )
{
	CAkSequenceInfo * pShared = static_cast<CAkSequenceInfo *>( m_RanSeqParams.m_bIsGlobal ? m_pGlobalContainerInfo : m_mapObjectCntrInfo.Exists( in_pGameObj ) );
	pShared->m_bIsForward = in_pSeqInfo->m_bIsForward;
	pShared->m_i16LastPositionChosen = pShared->m_bIsForward ? in_pSeqInfo->m_i16LastPositionChosen - 1 : in_pSeqInfo->m_i16LastPositionChosen + 1;
	const AkInt16 iLast = pShared->m_i16LastPositionChosen;
	if( (AkUInt32)( iLast + 1 ) == m_pPlayList->Length() && !m_RanSeqParams.m_bIsRestartBackward )
	{
		pShared->m_i16LastPositionChosen = -1;
	}
	else if( !iLast && !pShared->m_bIsForward )
	{
		pShared->m_bIsForward = true;
	}
}

/** 2012 0x94edf0: the next child for a step play; a random pick that is not playable retries the whole playlist in
    order once (deterministic mode) unless the container shuffles. */
CAkParameterNodeBase * CAkRanSeqCntr::GetNextToPlay( CAkRegisteredObj * in_pGameObj, AkUInt16 & out_rwPositionSelected, AkUniqueID & out_uSelectedNodeID )
{
	out_rwPositionSelected = 0;
	out_uSelectedNodeID = AK_INVALID_UNIQUE_ID;
	const AkUInt32 uPlaylistLength = m_pPlayList->Length();
	if( !uPlaylistLength )
	{
		return NULL;
	}
	if( uPlaylistLength == 1 )
	{
		out_uSelectedNodeID = m_pPlayList->ID( 0 );
		return g_pIndex->m_idxAudioNode.GetPtrAndAddRef( out_uSelectedNodeID );
	}
	CAkSequenceInfo * pSeqInfo = NULL;
	CAkRandomInfo * pRandomInfo = NULL;
	if( m_RanSeqParams.m_eMode == ContainerMode_Sequence )
	{
		pSeqInfo = GetExistingSequenceInfo( in_pGameObj );
		if( !pSeqInfo )
		{
			return NULL;
		}
	}
	else
	{
		pRandomInfo = GetExistingRandomInfo( (AkUInt16)uPlaylistLength, in_pGameObj );
		if( !pRandomInfo )
		{
			return NULL;
		}
	}
	CAkParameterNodeBase * pNode = NULL;
	bool bDeterministicMode = false;
	AkUInt16 wPosition = 0;
	for( AkUInt32 numIterations = 0; ; )
	{
		bool bIsValid = true;
		if( bDeterministicMode )
		{
			if( ++wPosition >= uPlaylistLength )
			{
				wPosition = 0;
			}
			bIsValid = CanPlayPosition( pRandomInfo, wPosition );
			if( bIsValid )
			{
				UpdateNormalAvoidRepeat( pRandomInfo, wPosition );
			}
		}
		else if( m_RanSeqParams.m_eMode == ContainerMode_Sequence )
		{
			wPosition = SelectSequentially( pSeqInfo, bIsValid, NULL );
		}
		else
		{
			wPosition = SelectRandomly( pRandomInfo, bIsValid, NULL );
		}
		if( !bIsValid )
		{
			if( !bDeterministicMode )
			{
				return pNode;
			}
		}
		else
		{
			out_uSelectedNodeID = m_pPlayList->ID( wPosition );
			pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( out_uSelectedNodeID );
			if( pNode )
			{
				if( pNode->IsPlayable() )
				{
					out_rwPositionSelected = wPosition;
					return pNode;
				}
				pNode->Release();
				pNode = NULL;
			}
		}
		if( !numIterations && m_RanSeqParams.m_eMode == ContainerMode_Random && m_RanSeqParams.m_eRandomMode != RandomMode_Shuffle )
		{
			bDeterministicMode = true;
		}
		if( ++numIterations >= uPlaylistLength )
		{
			return pNode;
		}
	}
}

/** 2012 0x94efd0: the next child of a continuous pass, against the pass's own selection state (io_pContainerInfo) and
    loop count; a sequence that does not restart at each play continues from the shared state. */
CAkParameterNodeBase * CAkRanSeqCntr::GetNextToPlayContinuous( CAkRegisteredObj * in_pGameObj, AkUInt16 & out_rwPositionSelected, AkUniqueID & out_uSelectedNodeID, CAkContainerBaseInfo *& io_pContainerInfo, AkLoop & io_rLoopInfo )
{
	out_uSelectedNodeID = AK_INVALID_UNIQUE_ID;
	out_rwPositionSelected = 0;
	const AkUInt32 uLength = m_pPlayList->Length();
	if( !uLength )
	{
		return NULL;
	}
	if( uLength == 1 )
	{
		if( io_rLoopInfo.lLoopCount <= 0 )
		{
			return NULL;
		}
		if( !io_rLoopInfo.bIsInfinite )
		{
			--io_rLoopInfo.lLoopCount;
		}
		out_uSelectedNodeID = m_pPlayList->ID( 0 );
		return g_pIndex->m_idxAudioNode.GetPtrAndAddRef( out_uSelectedNodeID );
	}
	bool bIsValid = true;
	AkUInt16 wPosition;
	if( !m_RanSeqParams.m_bResetPlayListAtEachPlay && !io_pContainerInfo && m_RanSeqParams.m_eMode == ContainerMode_Sequence )
	{
		CAkSequenceInfo * pShared = GetExistingSequenceInfo( in_pGameObj );
		if( !pShared )
		{
			return NULL;
		}
		CAkSequenceInfo * pSeqInfo = CreateSequenceInfo();
		if( !pSeqInfo )
		{
			return NULL;
		}
		pSeqInfo->m_bIsForward = pShared->m_bIsForward;
		pSeqInfo->m_i16LastPositionChosen = pShared->m_i16LastPositionChosen;
		wPosition = SelectSequentially( pSeqInfo, bIsValid, &io_rLoopInfo );
		io_pContainerInfo = pSeqInfo;
	}
	else if( m_RanSeqParams.m_eMode != ContainerMode_Sequence )
	{
		CAkRandomInfo * pRandomInfo = static_cast<CAkRandomInfo *>( io_pContainerInfo );
		if( !pRandomInfo )
		{
			pRandomInfo = CreateRandomInfo( (AkUInt16)uLength );
			if( !pRandomInfo )
			{
				return NULL;
			}
			io_pContainerInfo = pRandomInfo;
		}
		wPosition = SelectRandomly( pRandomInfo, bIsValid, &io_rLoopInfo );
	}
	else
	{
		if( !io_pContainerInfo )
		{
			CAkSequenceInfo * pSeqInfo = CreateSequenceInfo();
			io_pContainerInfo = pSeqInfo;
			if( !pSeqInfo )
			{
				return NULL;
			}
			if( m_pGlobalContainerInfo )
			{
				CAkSequenceInfo * pGlobal = static_cast<CAkSequenceInfo *>( m_pGlobalContainerInfo );
				pSeqInfo->m_i16LastPositionChosen = pGlobal->m_i16LastPositionChosen;
				pGlobal->m_i16LastPositionChosen = -1;
			}
		}
		CAkSequenceInfo * pSeqInfo = static_cast<CAkSequenceInfo *>( io_pContainerInfo );
		wPosition = SelectSequentially( pSeqInfo, bIsValid, &io_rLoopInfo );
		if( !m_RanSeqParams.m_bResetPlayListAtEachPlay )
		{
			UpdateResetPlayListSetup( pSeqInfo, in_pGameObj );
		}
	}
	if( !bIsValid )
	{
		return NULL;
	}
	out_rwPositionSelected = wPosition;
	out_uSelectedNodeID = m_pPlayList->ID( wPosition );
	return g_pIndex->m_idxAudioNode.GetPtrAndAddRef( out_uSelectedNodeID );
}

/** 2012 0x94f250: a step play - one child, its position pushed on the history as non-continuous. */
AKRESULT CAkRanSeqCntr::_Play( AkPBIParams & in_rPBIParams )
{
	AkUInt16 wPositionSelected;
	AkUniqueID uSelectedNodeID;
	CAkParameterNodeBase * pNode = GetNextToPlay( in_rPBIParams.pGameObj, wPositionSelected, uSelectedNodeID );
	if( pNode )
	{
		AkCntrHistArray & rHist = in_rPBIParams.playHistory.HistArray;
		if( rHist.uiArraySize < 32 )
		{
			in_rPBIParams.playHistory.arrayIsContinuous.UnsetBit( rHist.uiArraySize );
			rHist.aCntrHist[ rHist.uiArraySize ] = wPositionSelected;
		}
		++rHist.uiArraySize;
		AKDIAG( L"akdiag: ranseq 0x%08x %s step: child 0x%08x at position %u", key, m_RanSeqParams.m_eMode == ContainerMode_Sequence ? L"sequence" : L"random", uSelectedNodeID, wPositionSelected );
		const AKRESULT eResult = pNode->Play( in_rPBIParams );
		pNode->Release();
		return eResult;
	}
	AKDIAG( L"akdiag: ranseq 0x%08x step: nothing to play (playlist %u)", key, m_pPlayList->Length() );
	if( in_rPBIParams.eType == AkPBIParams::PBI )
	{
		return AK_Fail;
	}
	const AKRESULT eResult = PlayAndContinueAlternate( in_rPBIParams );
	return eResult == AK_PartialSuccess ? AK_Success : eResult;
}

/** 2012 0x94f2f0: a continuous pass - this container pushed on the continuation list with its loop count, then the
    first child played as a continuous PBI. */
AKRESULT CAkRanSeqCntr::_PlayContinuous( AkPBIParams & in_rPBIParams )
{
	CAkContinuationList * pContList = in_rPBIParams.pContinuousParams->spContList;
	CAkContinueListItem * pItem = pContList->m_listItems.AddLast();
	if( pItem )
	{
		AddRef();
		if( pItem->m_pContainer )
		{
			pItem->m_pContainer->Release();
		}
		pItem->m_pContainer = this;
		pItem->m_LoopingInfo.bIsEnabled = m_Loop != 1;
		pItem->m_LoopingInfo.bIsInfinite = m_Loop == 0;
		pItem->m_LoopingInfo.lLoopCount = pItem->m_LoopingInfo.bIsEnabled ? ( pItem->m_LoopingInfo.bIsInfinite ? 1 : m_Loop ) : 1;
		AkUInt16 wPositionSelected;
		AkUniqueID uSelectedNodeID;
		CAkParameterNodeBase * pNode = pItem->m_pContainer->GetNextToPlayContinuous( in_rPBIParams.pGameObj, wPositionSelected, uSelectedNodeID, pItem->m_pContainerInfo, pItem->m_LoopingInfo );
		if( pNode )
		{
			AkCntrHistArray & rHist = in_rPBIParams.playHistory.HistArray;
			if( rHist.uiArraySize < 32 )
			{
				in_rPBIParams.playHistory.arrayIsContinuous.SetBit( rHist.uiArraySize );
				rHist.aCntrHist[ rHist.uiArraySize ] = wPositionSelected;
			}
			++rHist.uiArraySize;
			AKDIAG( L"akdiag: ranseq 0x%08x %s continuous: child 0x%08x at position %u, transition mode %u, loop %d", key, m_RanSeqParams.m_eMode == ContainerMode_Sequence ? L"sequence" : L"random", uSelectedNodeID, wPositionSelected, m_RanSeqParams.m_eTransitionMode, (AkInt32)m_Loop );
			const AKRESULT eResult = pNode->Play( in_rPBIParams );
			pNode->Release();
			return eResult;
		}
		pContList = in_rPBIParams.pContinuousParams->spContList;
		( pContList->m_listItems.m_pItemsEnd - 1 )->~CAkContinueListItem();
		--pContList->m_listItems.m_pItemsEnd;
	}
	const AKRESULT eResult = PlayAndContinueAlternate( in_rPBIParams );
	return eResult == AK_PartialSuccess ? AK_Success : eResult;
}

/** 2012 0x94f7e0: a continuous container starts a continuation list when the play is not already continuous. */
AKRESULT CAkRanSeqCntr::Play( AkPBIParams & in_rPBIParams )
{
	m_bContainerBeenPlayed = true;
	if( !m_RanSeqParams.m_bIsContinuous )
	{
		return _Play( in_rPBIParams );
	}
	if( in_rPBIParams.eType != AkPBIParams::PBI )
	{
		ContParams * pContParams = in_rPBIParams.pContinuousParams;
		if( !pContParams || pContParams->spContList )
		{
			return _PlayContinuous( in_rPBIParams );
		}
		pContParams->spContList = CAkContinuationList::Create();
		if( !pContParams->spContList )
		{
			return AK_Fail;
		}
		return _PlayContinuous( in_rPBIParams );
	}
	in_rPBIParams.eType = AkPBIParams::ContinuousPBI;
	in_rPBIParams.pInstigator = this;
	AkPathInfo PathInfo;
	PathInfo.pPBPath = NULL;
	PathInfo.PathOwnerID = AK_INVALID_UNIQUE_ID;
	ContParams continuousParams;
	continuousParams.pPlayStopTransition = NULL;
	continuousParams.pPauseResumeTransition = NULL;
	continuousParams.pPathInfo = &PathInfo;
	continuousParams.bIsPlayStopTransitionFading = false;
	continuousParams.bIsPauseResumeTransitionFading = false;
	continuousParams.ulPauseCount = 0;
	continuousParams.spContList = CAkContinuationList::Create();
	if( !continuousParams.spContList )
	{
		return AK_Fail;
	}
	in_rPBIParams.pContinuousParams = &continuousParams;
	const AKRESULT eResult = _PlayContinuous( in_rPBIParams );
	if( continuousParams.spContList )
	{
		continuousParams.spContList->Release();
	}
	return eResult;
}
