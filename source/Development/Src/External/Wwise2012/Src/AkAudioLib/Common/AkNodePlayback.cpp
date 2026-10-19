/*=============================================================================
	AkNodePlayback.cpp - the playback half of the node hierarchy that the play / stop / pause path reaches:
	play counts and activity chunks, voice limiters, the mixing-bus walk, GetAudioParameters, the sound's Play /
	ExecuteAction, the active parents' and buses' ExecuteAction, CAkPBIAware::CreatePBI, CAkSource's media lock.

	DISHONORED(port): agent IH2 (wave 26), pulled from IH3's list (akparameternodebase.cpp, akparameternode.cpp,
	akbus.cpp, aksound.cpp, aksoundbase.cpp, akactiveparent.h, akpbiaware.cpp, aksource.cpp); rva on each.
	Deviations (named, every one an IH3 seam - the parameter graph):
	  * GetAudioParameters / GetBypassAllFX / GetPriority / GetMaxNumInstances read the nodes' own properties and
	    ranges only: no RTPC (m_RTPCBitArrayMax32 is ignored), no state values (GetAudioStateParams), no SIS
	    (m_pGlobalSIS / m_pMapSIS - nothing sets them before IH3's SetValue actions), no bus ducking
	    (m_DuckedVolumeList), no layer crossfades (m_pAssociatedLayers). Volume / pitch / LPF therefore sum the
	    authored values along the actor-mixer chain and the output bus chain, as retail does without game syncs.
	  * CAkBus::IncrementPlayCount / DecrementPlayCount do not run the ducking state machine (UpdateDuckedBus).
	  * CAkSoundBase::PlayToEnd, the seek and the dynamic-sequence / feedback branches of AllExec are not reached
	    by the glue's calls and are ported only as far as the decompile shows without IH3 classes.
=============================================================================*/
#include "AkActivityChunk.h"
#include "AkURenderer.h"
#include "AkBankMgr.h"
#include <stdlib.h>

static inline AkReal32 AkRandRange( AkReal32 in_fMin, AkReal32 in_fMax )
{
	const AkReal32 fRange = in_fMax - in_fMin;
	return fRange != 0.f ? (AkReal32)( (double)rand() * ( 1.0 / 32767.0 ) * fRange ) : 0.f;
}

// ------------------------------------------------------------------------------------------------------------
// CAkPBIAware
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94bed0. */
CAkPBI * CAkPBIAware::CreatePBI( CAkSoundBase * in_pSound, CAkSource * in_pSource, AkPBIParams & in_rPBIParams, const PriorityInfoCurrent & in_rPriority, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter ) const
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkPBI ) );
	if( !pMem )
	{
		return NULL;
	}
	return AkPlacementNew( pMem ) CAkPBI( in_pSound, in_pSource, in_rPBIParams.pGameObj, in_rPBIParams.userParams, in_rPBIParams.playHistory, 0, in_rPriority, in_rPBIParams.bTargetFeedback, 0, in_pAMLimiter, in_pBusLimiter );
}

// ------------------------------------------------------------------------------------------------------------
// AkActivityChunk / StructMaxInst / CAkLimiter
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x943ff0. */
AkActivityChunk::AkActivityChunk( AkUInt16 in_u16MaxNumInstance, bool in_bIsGlobalLimit, bool in_bDoesKillNewest, bool in_bAllowUseVirtualBehavior )
	: m_Limiter( in_u16MaxNumInstance, in_bDoesKillNewest, in_bAllowUseVirtualBehavior )
	, m_PlayCount( 0 )
	, m_uActivityCount( 0 )
	, m_iPlayCountValid( 0 )
	, m_iVirtualCountValid( 0 )
	, m_bIsGlobalLimit( in_bIsGlobalLimit )
{
}

/** 2012 0x944950. */
AkActivityChunk::~AkActivityChunk()
{
	m_Limiter.Term();
	m_listPBI.m_pFirst = NULL;
	m_ListPlayCountPerObj.Term();
}

/** 2012 0x93f730. */
bool AkActivityChunk::ChunkIsUseless() const
{
	return !m_PlayCount && !m_uActivityCount && !m_iPlayCountValid && !m_iVirtualCountValid && !m_listPBI.m_pFirst && m_ListPlayCountPerObj.IsEmpty();
}

/** 2012 0x943f90. */
StructMaxInst::StructMaxInst( AkUInt16 in_u16Current, AkUInt16 in_u16Max, bool in_bDoesKillNewest, bool in_bAllowUseVirtualBehavior )
	: u16Current( in_u16Current )
	, u16CurrentVirtual( 0 )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkLimiter ) );
	m_pLimiter = pMem ? AkPlacementNew( pMem ) CAkLimiter( in_u16Max, in_bDoesKillNewest, in_bAllowUseVirtualBehavior ) : NULL;
}

/** 2012 0x9448f0. */
void StructMaxInst::DisableLimiter()
{
	if( m_pLimiter )
	{
		m_pLimiter->Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pLimiter );
		m_pLimiter = NULL;
	}
}

/** 2012 0x9435b0 (AkPriorityStruct_INC) and its _DEC twin: priority descending, then PBIID ascending / descending. */
CAkPBI ** CAkLimiter::BinarySearch( const AkPriorityStruct & in_key, bool & out_bFound )
{
	AkInt32 iTop = 0;
	AkInt32 iBottom = (AkInt32)m_sortedPBIList.Length() - 1;
	while( iTop <= iBottom )
	{
		const AkInt32 iThis = iTop + ( iBottom - iTop ) / 2;
		const AkPriorityStruct & itemKey = m_sortedPBIList[ iThis ]->m_PriorityInfoCurrent.currentPriority;
		bool bGoLeft;
		if( in_key.priority == itemKey.priority )
		{
			if( in_key.PBIID == itemKey.PBIID )
			{
				out_bFound = true;
				return &m_sortedPBIList[ iThis ];
			}
			bGoLeft = m_bDoesKillNewest ? in_key.PBIID < itemKey.PBIID : in_key.PBIID > itemKey.PBIID;
		}
		else
		{
			bGoLeft = in_key.priority > itemKey.priority;
		}
		if( bGoLeft )
		{
			iBottom = iThis - 1;
		}
		else
		{
			iTop = iThis + 1;
		}
	}
	out_bFound = false;
	return m_sortedPBIList.m_pItems ? &m_sortedPBIList[ iTop ] : NULL;
}

/** 2012 0x9449a0: the first PBI of an AM / bus limiter registers the limiter with the renderer. */
AKRESULT CAkLimiter::Add( CAkPBI * in_pPBI, AKVoiceLimiterType in_Type )
{
	const bool bWasEmpty = m_sortedPBIList.IsEmpty();
	bool bFound;
	CAkPBI ** ppSlot = BinarySearch( in_pPBI->m_PriorityInfoCurrent.currentPriority, bFound );
	ppSlot = ppSlot ? m_sortedPBIList.Insert( (AkUInt32)( ppSlot - m_sortedPBIList.m_pItems ) ) : m_sortedPBIList.AddLast();
	if( !ppSlot )
	{
		return AK_Fail;
	}
	if( bWasEmpty )
	{
		if( in_Type == AKVoiceLimiter_AM )
		{
			CAkURenderer::AddAMLimiter( this );
		}
		else if( in_Type == AKVoiceLimiter_Bus )
		{
			CAkURenderer::AddBusLimiter( this );
		}
	}
	*ppSlot = in_pPBI;
	return AK_Success;
}

/** 2012 0x9440d0. */
void CAkLimiter::Remove( CAkPBI * in_pPBI, AKVoiceLimiterType in_Type )
{
	bool bFound;
	CAkPBI ** ppSlot = BinarySearch( in_pPBI->m_PriorityInfoCurrent.currentPriority, bFound );
	if( bFound )
	{
		m_sortedPBIList.Erase( (AkUInt32)( ppSlot - m_sortedPBIList.m_pItems ) );
	}
	if( m_sortedPBIList.IsEmpty() )
	{
		if( in_Type == AKVoiceLimiter_AM )
		{
			CAkURenderer::RemoveAMLimiter( this );
		}
		else if( in_Type == AKVoiceLimiter_Bus )
		{
			CAkURenderer::RemoveBusLimiter( this );
		}
	}
}

/** 2012 0x944130 (AkSortedKeyArray::Reorder): take the PBI out at its old key and put it back at the new one. */
void CAkLimiter::Update( AkReal32 in_NewPriority, CAkPBI * in_pPBI )
{
	bool bFound;
	CAkPBI ** ppSlot = BinarySearch( in_pPBI->m_PriorityInfoCurrent.currentPriority, bFound );
	if( !bFound )
	{
		return;
	}
	m_sortedPBIList.Erase( (AkUInt32)( ppSlot - m_sortedPBIList.m_pItems ) );
	AkPriorityStruct newKey;
	newKey.priority = in_NewPriority;
	newKey.PBIID = in_pPBI->m_PriorityInfoCurrent.currentPriority.PBIID;
	ppSlot = BinarySearch( newKey, bFound );
	ppSlot = ppSlot ? m_sortedPBIList.Insert( (AkUInt32)( ppSlot - m_sortedPBIList.m_pItems ) ) : m_sortedPBIList.AddLast();
	if( ppSlot )
	{
		*ppSlot = in_pPBI;
	}
}

/** 2012 0x943730: past the first m_u16LimiterMax real voices, every voice is virtualized or kicked. */
void CAkLimiter::UpdateFlags()
{
	if( !m_u16LimiterMax || m_sortedPBIList.Length() <= m_u16LimiterMax )
	{
		return;
	}
	CAkPBI ** pp = m_sortedPBIList.m_pItems;
	AkUInt32 uCount = 0;
	while( pp != m_sortedPBIList.m_pItemsEnd && uCount < m_u16LimiterMax )
	{
		CAkPBI * pPBI = *pp;
		if( ( !pPBI->m_bIsVirtual || pPBI->m_bWasForcedToVirtualize ) && !pPBI->m_bWasKicked && !pPBI->m_bIsForcedToVirtualize )
		{
			++uCount;
		}
		++pp;
	}
	const KickFrom eReason = this == &CAkURenderer::m_GlobalLimiter ? KickFrom_OverGlobalLimit : KickFrom_OverNodeLimit;
	for( ; pp != m_sortedPBIList.m_pItemsEnd; ++pp )
	{
		if( !( *pp )->m_bWasKicked )
		{
			if( m_bAllowUseVirtualBehavior )
			{
				( *pp )->ForceVirtualize( eReason );
			}
			else
			{
				( *pp )->Kick( eReason );
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNodeBase
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x942ca0. */
CAkBus * CAkParameterNodeBase::GetMixingBus()
{
	if( m_pBusOutputNode )
	{
		return m_pBusOutputNode->GetMixingBus();
	}
	if( m_pParentNode )
	{
		return m_pParentNode->GetMixingBus();
	}
	return NULL;
}

/** 2012 0x944d80: the first node overriding its parent's priority answers. DISHONORED(IH3 seam): no RTPC. */
PriorityInfo CAkParameterNodeBase::GetPriority( CAkRegisteredObj * in_GameObjPtr )
{
	if( m_pParentNode && !m_bPriorityOverrideParent )
	{
		return m_pParentNode->GetPriority( in_GameObjPtr );
	}
	PriorityInfo priorityInfo;
	const AkPropValue * pPriority = m_props.FindProp( AkPropID_Priority );
	priorityInfo.priority = pPriority ? pPriority->fValue : (AkReal32)AK_DEFAULT_PRIORITY;
	priorityInfo.distanceOffset = 0.f;
	if( m_bPriorityApplyDistFactor )
	{
		const AkPropValue * pOffset = m_props.FindProp( AkPropID_PriorityDistanceOffset );
		priorityInfo.distanceOffset = pOffset ? pOffset->fValue : -10.f;
	}
	return priorityInfo;
}

/** 2012 0x943380. DISHONORED(IH3 seam): no RTPC on the instance limit. */
AkUInt16 CAkParameterNodeBase::GetMaxNumInstances( CAkRegisteredObj * in_GameObjPtr )
{
	return m_u16MaxNumInstance;
}

/** 2012 0x944050. */
bool CAkParameterNodeBase::EnableActivityChunk()
{
	if( !m_pActivityChunk )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkActivityChunk ) );
		m_pActivityChunk = pMem ? AkPlacementNew( pMem ) AkActivityChunk( GetMaxNumInstances( NULL ), m_bIsGlobalLimit, m_bKillNewest, m_bUseVirtualBehavior ) : NULL;
	}
	return m_pActivityChunk != NULL;
}

/** 2012 0x945a20. */
void CAkParameterNodeBase::DeleteActivityChunk()
{
	if( m_pActivityChunk )
	{
		m_pActivityChunk->~AkActivityChunk();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pActivityChunk );
	}
	m_pActivityChunk = NULL;
}

/** 2012 0x9452f0: a mixing bus counts its mix bus nodes up its own chain; flag 1 also forwards to the output bus
    (flag 2, the feedback bus, needs a feedback bus: none exists in the banks). */
bool CAkParameterNodeBase::IncrementActivityCount( AkUInt16 in_flagForwardToBus )
{
	bool bResult = EnableActivityChunk();
	if( bResult )
	{
		++m_pActivityChunk->m_uActivityCount;
	}
	if( ( in_flagForwardToBus & 1 ) && m_pBusOutputNode )
	{
		in_flagForwardToBus &= ~1;
		bResult = m_pBusOutputNode->IncrementActivityCount( 3 ) && bResult;
	}
	if( m_pParentNode )
	{
		bResult = m_pParentNode->IncrementActivityCount( in_flagForwardToBus ) && bResult;
	}
	return bResult;
}

/** 2012 0x9464e0 (+ DecrementActivityCountValue 0x946320). */
void CAkParameterNodeBase::DecrementActivityCount( AkUInt16 in_flagForwardToBus )
{
	if( m_pActivityChunk )
	{
		--m_pActivityChunk->m_uActivityCount;
		if( m_pActivityChunk->ChunkIsUseless() )
		{
			DeleteActivityChunk();
		}
	}
	if( ( in_flagForwardToBus & 1 ) && m_pBusOutputNode )
	{
		in_flagForwardToBus &= ~1;
		m_pBusOutputNode->DecrementActivityCount( 3 );
	}
	if( m_pParentNode )
	{
		m_pParentNode->DecrementActivityCount( in_flagForwardToBus );
	}
}

/** 2012 0x93b8b0: a bus forwards only to its output bus. */
bool CAkBus::IncrementActivityCount( AkUInt16 in_flagForwardToBus )
{
	bool bResult = EnableActivityChunk();
	if( bResult )
	{
		++m_pActivityChunk->m_uActivityCount;
	}
	if( m_pBusOutputNode )
	{
		bResult = m_pBusOutputNode->IncrementActivityCount( 3 ) && bResult;
	}
	return bResult;
}

/** 2012 0x93b900. */
void CAkBus::DecrementActivityCount( AkUInt16 in_flagForwardToBus )
{
	if( m_pActivityChunk )
	{
		--m_pActivityChunk->m_uActivityCount;
		if( m_pActivityChunk->ChunkIsUseless() )
		{
			DeleteActivityChunk();
		}
	}
	if( m_pBusOutputNode )
	{
		m_pBusOutputNode->DecrementActivityCount( 3 );
	}
}

/** 2012 0x944480. */
AKRESULT CAkParameterNodeBase::IncrementPlayCountValue()
{
	if( !EnableActivityChunk() )
	{
		return AK_Fail;
	}
	++m_pActivityChunk->m_PlayCount;
	return AK_Success;
}

/** 2012 0x9462b0. */
void CAkParameterNodeBase::DecrementPlayCountValue()
{
	if( m_pActivityChunk )
	{
		--m_pActivityChunk->m_PlayCount;
		if( m_pActivityChunk->ChunkIsUseless() )
		{
			DeleteActivityChunk();
		}
	}
}

/** 2012 0x9433d0: over the node's global instance limit, the weakest voice of the node is kicked. */
AKRESULT CAkParameterNodeBase::IncrementPlayCountGlobal( AkReal32 in_fPriority, AkUInt16 & io_ui16NumKickedOrRevived, CAkLimiter *& io_pLimiter )
{
	if( m_pActivityChunk )
	{
		++m_pActivityChunk->m_iPlayCountValid;
	}
	const AkUInt16 u16MaxNumInstance = GetMaxNumInstances( NULL );
	if( !u16MaxNumInstance )
	{
		return AK_Success;
	}
	io_pLimiter = &m_pActivityChunk->m_Limiter;
	const AkUInt16 uPlayCountValid = m_pActivityChunk ? m_pActivityChunk->m_iPlayCountValid : 0;
	const AkUInt16 uVirtualCountValid = m_pActivityChunk ? m_pActivityChunk->m_iVirtualCountValid : 0;
	if( (AkInt32)uPlayCountValid - io_ui16NumKickedOrRevived - uVirtualCountValid <= (AkInt32)u16MaxNumInstance )
	{
		return AK_Success;
	}
	CAkParameterNodeBase * pKicked = NULL;
	const AKRESULT eResult = CAkURenderer::Kick( in_fPriority, NULL, key, m_bKillNewest, m_bUseVirtualBehavior, pKicked, KickFrom_OverNodeLimit );
	++io_ui16NumKickedOrRevived;
	return eResult;
}

/** 2012 0x945f60. */
void CAkParameterNodeBase::DecrementPlayCountGlobal()
{
	if( m_pActivityChunk )
	{
		--m_pActivityChunk->m_iPlayCountValid;
		if( m_pActivityChunk->ChunkIsUseless() )
		{
			DeleteActivityChunk();
		}
	}
}

/** 2012 0x945390: the per game object count and limiter. */
AKRESULT CAkParameterNodeBase::IncrementPlayCountGameObject( AkReal32 in_fPriority, AkUInt16 & io_ui16NumKickedOrRevived, CAkRegisteredObj * in_pGameObj, CAkLimiter *& io_pLimiter )
{
	AKRESULT eResult = AK_Success;
	AkUInt16 u16Max;
	StructMaxInst * pPerObj = m_pActivityChunk->m_ListPlayCountPerObj.Exists( in_pGameObj );
	if( pPerObj )
	{
		++pPerObj->u16Current;
		u16Max = pPerObj->m_pLimiter ? pPerObj->m_pLimiter->m_u16LimiterMax : 0;
		if( u16Max && (AkInt32)pPerObj->u16Current - pPerObj->u16CurrentVirtual - io_ui16NumKickedOrRevived > (AkInt32)u16Max )
		{
			CAkParameterNodeBase * pKicked = NULL;
			eResult = CAkURenderer::Kick( in_fPriority, in_pGameObj, key, m_bKillNewest, m_bUseVirtualBehavior, pKicked, KickFrom_OverNodeLimit );
			++io_ui16NumKickedOrRevived;
		}
	}
	else
	{
		u16Max = GetMaxNumInstances( in_pGameObj );
		StructMaxInst structMaxInst( 1, u16Max, m_bKillNewest, m_bUseVirtualBehavior );
		pPerObj = m_pActivityChunk->m_ListPlayCountPerObj.Set( in_pGameObj, structMaxInst );
		if( !pPerObj )
		{
			structMaxInst.DisableLimiter();
			return AK_Fail;
		}
	}
	if( u16Max )
	{
		io_pLimiter = pPerObj->m_pLimiter;
	}
	return eResult;
}

/** 2012 0x9460d0. */
void CAkParameterNodeBase::DecrementPlayCountGameObject( CAkRegisteredObj * in_pGameObj )
{
	StructMaxInst * pPerObj = m_pActivityChunk->m_ListPlayCountPerObj.Exists( in_pGameObj );
	if( !pPerObj )
	{
		return;
	}
	if( !--pPerObj->u16Current && !pPerObj->u16CurrentVirtual )
	{
		pPerObj->DisableLimiter();
		m_pActivityChunk->m_ListPlayCountPerObj.Unset( in_pGameObj );
	}
	if( m_pActivityChunk->ChunkIsUseless() )
	{
		DeleteActivityChunk();
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNode
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x937890: the node that holds the instance limit (overrides its parent, or the top) counts against it; the
    count then goes to the output bus (once) and up the parents. */
AKRESULT CAkParameterNode::IncrementPlayCount( CounterParameters & io_params )
{
	AKRESULT eResult = IncrementPlayCountValue();
	if( m_bIsMaxNumInstOverrideParent || !m_pParentNode )
	{
		if( !io_params.bMaxConsidered && eResult == AK_Success )
		{
			if( m_pActivityChunk && !m_pActivityChunk->m_bIsGlobalLimit )
			{
				eResult = IncrementPlayCountGameObject( io_params.fPriority, io_params.ui16NumKicked, io_params.pGameObj, io_params.pAMLimiter );
			}
			else
			{
				eResult = IncrementPlayCountGlobal( io_params.fPriority, io_params.ui16NumKicked, io_params.pAMLimiter );
			}
		}
		io_params.bMaxConsidered = true;
	}
	const bool bMaxConsideredBackup = io_params.bMaxConsidered;
	if( ( io_params.uiFlagForwardToBus & 1 ) && m_pBusOutputNode )
	{
		io_params.uiFlagForwardToBus &= ~1;
		io_params.bMaxConsidered = false;
		const AKRESULT eBusResult = m_pBusOutputNode->IncrementPlayCount( io_params );
		if( eBusResult != AK_Success )
		{
			if( eBusResult == AK_MustBeVirtualized )
			{
				if( eResult == AK_Success )
				{
					eResult = AK_MustBeVirtualized;
				}
			}
			else
			{
				eResult = eBusResult;
			}
		}
	}
	if( ( io_params.uiFlagForwardToBus & 2 ) && m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		io_params.uiFlagForwardToBus &= ~2;
		io_params.bMaxConsidered = false;
		const AKRESULT eFeedbackResult = m_pFeedbackInfo->m_pFeedbackBus->IncrementPlayCount( io_params );
		if( eFeedbackResult != AK_Success )
		{
			if( eFeedbackResult == AK_MustBeVirtualized )
			{
				if( eResult == AK_Success )
				{
					eResult = AK_MustBeVirtualized;
				}
			}
			else
			{
				eResult = eFeedbackResult;
			}
		}
	}
	if( !m_pParentNode )
	{
		return eResult;
	}
	io_params.bMaxConsidered = bMaxConsideredBackup;
	const AKRESULT eParentResult = m_pParentNode->IncrementPlayCount( io_params );
	if( eParentResult == AK_Success )
	{
		return eResult;
	}
	if( eParentResult != AK_MustBeVirtualized )
	{
		return eParentResult;
	}
	return eResult == AK_Success ? AK_MustBeVirtualized : eResult;
}

/** 2012 0x9379e0. */
void CAkParameterNode::DecrementPlayCount( CounterParameters & io_params )
{
	DecrementPlayCountValue();
	if( m_bIsMaxNumInstOverrideParent || !m_pParentNode )
	{
		if( !io_params.bMaxConsidered && m_pActivityChunk )
		{
			if( m_pActivityChunk->m_bIsGlobalLimit )
			{
				DecrementPlayCountGlobal();
			}
			else
			{
				DecrementPlayCountGameObject( io_params.pGameObj );
			}
		}
		io_params.bMaxConsidered = true;
	}
	const bool bMaxConsideredBackup = io_params.bMaxConsidered;
	if( ( io_params.uiFlagForwardToBus & 1 ) && m_pBusOutputNode )
	{
		io_params.uiFlagForwardToBus &= ~1;
		io_params.bMaxConsidered = false;
		m_pBusOutputNode->DecrementPlayCount( io_params );
	}
	if( ( io_params.uiFlagForwardToBus & 2 ) && m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		io_params.uiFlagForwardToBus &= ~2;
		io_params.bMaxConsidered = false;
		m_pFeedbackInfo->m_pFeedbackBus->DecrementPlayCount( io_params );
	}
	if( m_pParentNode )
	{
		io_params.bMaxConsidered = bMaxConsideredBackup;
		m_pParentNode->DecrementPlayCount( io_params );
	}
}

/** 2012 0x938c40. DISHONORED(IH3 seam): no RTPC, no SIS. */
bool CAkParameterNode::GetBypassAllFX( CAkRegisteredObj * in_GameObjPtr )
{
	if( !m_bIsFXOverrideParent && m_pParentNode )
	{
		return m_pParentNode->GetBypassAllFX( in_GameObjPtr );
	}
	return m_pFXChunk ? ( m_pFXChunk->bitsMainFXBypass & 0x10 ) != 0 : false;
}

/** 2012 0x939ab0: the node's own volume / pitch / LPF, its range offsets, then the parent (and the output bus where
    the node overrides it). DISHONORED(IH3 seam): no state values, RTPC, SIS or layers. */
AKRESULT CAkParameterNode::GetAudioParameters( AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr, bool in_bIncludeRange, AkPBIModValues & io_Ranges, bool in_bDoBusCheck )
{
	if( in_ulParamSelect & 1 )
	{
		const AkPropValue * pValue = m_props.FindProp( AkPropID_Volume );
		io_Parameters.Volume += pValue ? pValue->fValue : 0.f;
	}
	if( in_ulParamSelect & 2 )
	{
		const AkPropValue * pValue = m_props.FindProp( AkPropID_Pitch );
		io_Parameters.Pitch += pValue ? pValue->fValue : 0.f;
	}
	if( in_ulParamSelect & 4 )
	{
		const AkPropValue * pValue = m_props.FindProp( AkPropID_LPF );
		io_Parameters.LPF += pValue ? pValue->fValue : 0.f;
	}
	if( in_bIncludeRange && m_ranges.m_pProps )
	{
		const RANGED_MODIFIERS<AkPropValue> * pVolume = m_ranges.FindProp( AkPropID_Volume );
		if( pVolume )
		{
			io_Ranges.VolumeOffset += pVolume->m_min.fValue + AkRandRange( pVolume->m_min.fValue, pVolume->m_max.fValue );
		}
		const RANGED_MODIFIERS<AkPropValue> * pPitch = m_ranges.FindProp( AkPropID_Pitch );
		if( pPitch )
		{
			io_Ranges.PitchOffset += pPitch->m_min.fValue + AkRandRange( pPitch->m_min.fValue, pPitch->m_max.fValue );
		}
		const RANGED_MODIFIERS<AkPropValue> * pLPF = m_ranges.FindProp( AkPropID_LPF );
		if( pLPF )
		{
			io_Ranges.LPFOffset += pLPF->m_min.fValue + AkRandRange( pLPF->m_min.fValue, pLPF->m_max.fValue );
		}
	}
	if( in_bDoBusCheck && m_pBusOutputNode )
	{
		if( m_pParentNode )
		{
			m_pParentNode->GetAudioParameters( io_Parameters, in_ulParamSelect, io_rMutedMap, in_GameObjPtr, in_bIncludeRange, io_Ranges, false );
		}
		m_pBusOutputNode->GetAudioParameters( io_Parameters, in_ulParamSelect, io_rMutedMap, in_GameObjPtr, in_bIncludeRange, io_Ranges, false );
	}
	else if( m_pParentNode )
	{
		m_pParentNode->GetAudioParameters( io_Parameters, in_ulParamSelect, io_rMutedMap, in_GameObjPtr, in_bIncludeRange, io_Ranges, in_bDoBusCheck );
	}
	return AK_Success;
}

/** 2012 0x937810: the first node overriding its parent's virtual voice settings answers. */
AkBelowThresholdBehavior CAkParameterNode::GetVirtualBehavior( AkVirtualQueueBehavior & out_Behavior ) const
{
	const CAkParameterNode * pNode = this;
	while( !pNode->m_bIsVVoicesOptOverrideParent && pNode->m_pParentNode )
	{
		pNode = static_cast<const CAkParameterNode *>( pNode->m_pParentNode );
	}
	out_Behavior = (AkVirtualQueueBehavior)pNode->m_eVirtualQueueBehavior;
	return (AkBelowThresholdBehavior)pNode->m_eBelowThresholdBehavior;
}

// ------------------------------------------------------------------------------------------------------------
// CAkParentNode<T> (CAkActiveParent<T>)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94de50: from a bus, children routed to their own output bus are left to that bus. */
template <class T>
AKRESULT CAkParentNode<T>::ExecuteAction( ActionParams & in_rAction )
{
	AkActivityChunk * pChunk = this->m_pActivityChunk;
	if( !pChunk || ( !pChunk->m_PlayCount && !pChunk->m_uActivityCount ) )
	{
		return AK_Success;
	}
	if( in_rAction.bIsMasterCall )
	{
		this->PauseTransitions( in_rAction.eType == ActionParamType_Pause );
	}
	for( AkUInt32 i = m_mapChildId.Length(); i; --i )
	{
		CAkParameterNodeBase * pChild = m_mapChildId[ i - 1 ];
		if( !in_rAction.bIsFromBus || !pChild->m_pBusOutputNode )
		{
			pChild->ExecuteAction( in_rAction );
		}
	}
	return AK_Success;
}

/** 2012 0x94dee0: the except list skips the listed children. */
template <class T>
AKRESULT CAkParentNode<T>::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	AkActivityChunk * pChunk = this->m_pActivityChunk;
	if( !pChunk || ( !pChunk->m_PlayCount && !pChunk->m_uActivityCount ) )
	{
		return AK_Success;
	}
	for( AkUInt32 i = m_mapChildId.Length(); i; --i )
	{
		CAkParameterNodeBase * pChild = m_mapChildId[ i - 1 ];
		if( in_rAction.pExeceptionList && in_rAction.pExeceptionList->Exists( pChild->key ) )
		{
			continue;
		}
		if( !in_rAction.bIsFromBus || !pChild->m_pBusOutputNode )
		{
			pChild->ExecuteActionExcept( in_rAction );
		}
	}
	return AK_Success;
}

template AKRESULT CAkParentNode<CAkParameterNode>::ExecuteAction( ActionParams & );
template AKRESULT CAkParentNode<CAkParameterNode>::ExecuteActionExcept( ActionParamsExcept & );
template AKRESULT CAkParentNode<CAkParameterNodeBase>::ExecuteAction( ActionParams & );
template AKRESULT CAkParentNode<CAkParameterNodeBase>::ExecuteActionExcept( ActionParamsExcept & );

/** 2012 0x94e240 / 0x93ccf0: every child, the ones already prepared are unprepared again on a failure. */
template <class T>
AKRESULT CAkParentNode<T>::PrepareData()
{
	for( AkUInt32 i = 0; i < m_mapChildId.Length(); ++i )
	{
		const AKRESULT eResult = m_mapChildId[ i ]->PrepareData();
		if( eResult != AK_Success )
		{
			while( i )
			{
				m_mapChildId[ --i ]->UnPrepareData();
			}
			return eResult;
		}
	}
	return AK_Success;
}

/** 2012 0x94e2a0 / 0x93cd50. */
template <class T>
void CAkParentNode<T>::UnPrepareData()
{
	for( AkUInt32 i = 0; i < m_mapChildId.Length(); ++i )
	{
		m_mapChildId[ i ]->UnPrepareData();
	}
}

template AKRESULT CAkParentNode<CAkParameterNode>::PrepareData();
template void CAkParentNode<CAkParameterNode>::UnPrepareData();
template AKRESULT CAkParentNode<CAkParameterNodeBase>::PrepareData();
template void CAkParentNode<CAkParameterNodeBase>::UnPrepareData();

/** 2012 0x945560: the node keeps the reference it is looked up with while prepared. */
AKRESULT CAkParameterNodeBase::PrepareNodeData( AkUniqueID in_NodeID )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_NodeID );
	if( !pNode )
	{
		return AK_Fail;
	}
	const AKRESULT eResult = pNode->PrepareData();
	if( eResult != AK_Success )
	{
		pNode->Release();
	}
	return eResult;
}

/** 2012 0x9455a0: the lookup reference and the one PrepareNodeData kept. */
void CAkParameterNodeBase::UnPrepareNodeData( AkUniqueID in_NodeID )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_NodeID );
	if( pNode )
	{
		pNode->UnPrepareData();
		pNode->Release();
		pNode->Release();
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkSoundBase / CAkSound / CAkSource
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x946bb0: the loop count (1 = once, 0 = infinite) with its random range. */
AkInt16 CAkSoundBase::Loop()
{
	const AkPropValue * pLoop = m_props.FindProp( AkPropID_Loop );
	AkInt32 iLoop = pLoop ? pLoop->iValue : 1;
	const RANGED_MODIFIERS<AkPropValue> * pRange = m_ranges.FindProp( AkPropID_Loop );
	if( pRange )
	{
		const AkInt32 iRange = pRange->m_max.iValue - pRange->m_min.iValue;
		AkInt32 iRand = 0;
		if( iRange )
		{
			iRand = (AkInt32)( (double)rand() * ( 1.0 / 32767.0 ) * (double)iRange + 0.5 );
		}
		iLoop += iRand + pRange->m_min.iValue;
	}
	return (AkInt16)iLoop;
}

/** 2012 0x946c70: every PBI of this sound on the object / playing id plays to its end (no more loops). */
AKRESULT CAkSoundBase::PlayToEnd( CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, AkPlayingID in_PlayingID )
{
	if( !m_pActivityChunk )
	{
		return AK_Success;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( ( !in_pGameObj || pPBI->GetGameObjectPtr() == in_pGameObj ) && ( !in_PlayingID || pPBI->GetPlayingID() == in_PlayingID ) )
		{
			pPBI->PlayToEnd( this );
		}
	}
	return AK_Success;
}

/** 2012 0x94a6c0. */
AKRESULT CAkSound::Play( AkPBIParams & in_rPBIParams )
{
	return CAkURenderer::Play( this, &m_Source, in_rPBIParams );
}

/** 2012 0x94a970. DISHONORED(IH3 seam): ActionParamType_Seek (SeekSound) is not ported. */
AKRESULT CAkSound::ExecuteAction( ActionParams & in_rAction )
{
	if( in_rAction.bIsMasterCall )
	{
		PauseTransitions( in_rAction.eType == ActionParamType_Pause );
	}
	if( !m_pActivityChunk || !m_pActivityChunk->m_PlayCount )
	{
		return AK_Success;
	}
	switch( in_rAction.eType )
	{
	case ActionParamType_Stop:
		return CAkURenderer::Stop( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	case ActionParamType_Pause:
		return CAkURenderer::Pause( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	case ActionParamType_Resume:
		return CAkURenderer::Resume( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.bIsMasterResume, in_rAction.playingID );
	case ActionParamType_Break:
		PlayToEnd( in_rAction.pGameObj, in_rAction.targetNodeID, in_rAction.playingID );
		return AK_Success;
	default:
		return AK_Success;
	}
}

/** 2012 0x94aa70. */
AKRESULT CAkSound::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	if( !m_pActivityChunk || !m_pActivityChunk->m_PlayCount )
	{
		return AK_Success;
	}
	switch( in_rAction.eType )
	{
	case ActionParamType_Stop:
		return CAkURenderer::Stop( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	case ActionParamType_Pause:
		return CAkURenderer::Pause( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.playingID );
	case ActionParamType_Resume:
		return CAkURenderer::Resume( this, in_rAction.pGameObj, in_rAction.transParams, in_rAction.bIsMasterResume, in_rAction.playingID );
	default:
		return AK_Success;
	}
}

/** 2012 0x94a660. */
bool CAkSound::IsPlayable()
{
	return m_Source.m_sSrcTypeInfo.mediaInfo.sourceID != 0;
}

/** 2012 0x9465c0: in-memory media of the sound itself, else the bank manager's (which pins the bank's usage slot in the PBI). */
void CAkSource::LockDataPtr( void *& out_ppvBuffer, AkUInt32 & out_ulSize, CAkPBI * in_pPBI )
{
	if( m_sSrcTypeInfo.pInMemoryMedia )
	{
		out_ulSize = m_sSrcTypeInfo.mediaInfo.uInMemoryMediaSize;
		out_ppvBuffer = m_sSrcTypeInfo.pInMemoryMedia;
		return;
	}
	CAkUsageSlot * pSlot = NULL;
	const AkMediaInfo mediaInfo = g_pBankManager->GetMedia( m_sSrcTypeInfo.mediaInfo.sourceID, pSlot );
	if( pSlot )
	{
		in_pPBI->m_pUsageSlot = pSlot;
	}
	out_ppvBuffer = mediaInfo.pInMemoryData;
	out_ulSize = mediaInfo.uInMemoryDataSize;
}

/** 2012 0x946650: in-bank media that is not already in memory is prepared through the bank manager. */
AKRESULT CAkSource::PrepareData()
{
	if( !m_sSrcTypeInfo.mediaInfo.uInMemoryMediaSize || m_sSrcTypeInfo.pInMemoryMedia )
	{
		return AK_Success;
	}
	return g_pBankManager->PrepareSingleMedia( m_sSrcTypeInfo );
}

/** 2012 0x946680. */
void CAkSource::UnPrepareData()
{
	if( m_sSrcTypeInfo.mediaInfo.uInMemoryMediaSize && !m_sSrcTypeInfo.pInMemoryMedia )
	{
		g_pBankManager->UnprepareSingleMedia( m_sSrcTypeInfo.mediaInfo.sourceID );
	}
}

/** 2012 0x946630. */
void CAkSource::UnLockDataPtr()
{
	if( !m_sSrcTypeInfo.pInMemoryMedia )
	{
		g_pBankManager->ReleaseMedia( m_sSrcTypeInfo.mediaInfo.sourceID );
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkBus
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x93c110. */
CAkBus * CAkBus::GetMasterBusAndAddRef()
{
	AkAutoLock<CAkLock> gate( g_pIndex->m_idxAudioNode.m_IndexLock );
	if( s_pMasterBus )
	{
		s_pMasterBus->AddRef();
	}
	return s_pMasterBus;
}

/** Inlined in CAkBus::GetMixingBus / GetAudioParameters: any of the four effect slots set. */
bool CAkBus::HasEffect() const
{
	if( !m_pFXChunk )
	{
		return false;
	}
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		if( m_pFXChunk->aFX[ i ].id )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x93bfd0: a bus mixes on its own only with an effect or as an environmental (aux) bus; the master bus never. */
CAkBus * CAkBus::GetMixingBus()
{
	if( ( HasEffect() || m_bIsEnvironmental ) && this != s_pMasterBus )
	{
		return this;
	}
	return CAkParameterNodeBase::GetMixingBus();
}

/** 2012 0x93bcf0. */
AKRESULT CAkBus::ExecuteAction( ActionParams & in_rAction )
{
	if( !m_pActivityChunk || ( !m_pActivityChunk->m_PlayCount && !m_pActivityChunk->m_uActivityCount ) )
	{
		return AK_Success;
	}
	if( in_rAction.bIsMasterCall )
	{
		PauseTransitions( in_rAction.eType == ActionParamType_Pause );
	}
	in_rAction.bIsFromBus = true;
	for( AkUInt32 i = m_mapChildId.Length(); i; --i )
	{
		m_mapChildId[ i - 1 ]->ExecuteAction( in_rAction );
	}
	return AK_Success;
}

/** 2012 0x93bd70. */
AKRESULT CAkBus::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	if( !m_pActivityChunk || ( !m_pActivityChunk->m_PlayCount && !m_pActivityChunk->m_uActivityCount ) )
	{
		return AK_Success;
	}
	in_rAction.bIsFromBus = true;
	for( AkUInt32 i = m_mapChildId.Length(); i; --i )
	{
		CAkParameterNodeBase * pChild = m_mapChildId[ i - 1 ];
		if( !in_rAction.pExeceptionList || !in_rAction.pExeceptionList->Exists( pChild->key ) )
		{
			pChild->ExecuteActionExcept( in_rAction );
		}
	}
	return AK_Success;
}

/** 2012 0x93d900. DISHONORED(IH3 seam): the ducking state machine (UpdateDuckedBus) is not run. */
AKRESULT CAkBus::IncrementPlayCount( CounterParameters & io_params )
{
	AKRESULT eResult = IncrementPlayCountValue();
	if( m_bIsMaxNumInstOverrideParent || !m_pBusOutputNode )
	{
		if( !io_params.bMaxConsidered && eResult == AK_Success )
		{
			eResult = IncrementPlayCountGlobal( io_params.fPriority, io_params.ui16NumKicked, io_params.pBusLimiter );
		}
		io_params.bMaxConsidered = true;
	}
	if( m_pBusOutputNode )
	{
		const AKRESULT eParentResult = m_pBusOutputNode->IncrementPlayCount( io_params );
		if( eParentResult != AK_Success )
		{
			if( eParentResult == AK_MustBeVirtualized )
			{
				if( eResult == AK_Success )
				{
					eResult = AK_MustBeVirtualized;
				}
			}
			else
			{
				eResult = eParentResult;
			}
		}
	}
	if( m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		const AKRESULT eFeedbackResult = m_pFeedbackInfo->m_pFeedbackBus->IncrementPlayCount( io_params );
		if( eFeedbackResult != AK_Success )
		{
			if( eFeedbackResult == AK_MustBeVirtualized )
			{
				if( eResult == AK_Success )
				{
					eResult = AK_MustBeVirtualized;
				}
			}
			else
			{
				eResult = eFeedbackResult;
			}
		}
	}
	return eResult;
}

/** 2012 0x93d9e0. DISHONORED(IH3 seam): no duck release notification. */
void CAkBus::DecrementPlayCount( CounterParameters & io_params )
{
	DecrementPlayCountValue();
	if( ( m_bIsMaxNumInstOverrideParent || !m_pBusOutputNode ) && !io_params.bMaxConsidered )
	{
		DecrementPlayCountGlobal();
		io_params.bMaxConsidered = true;
	}
	if( m_pBusOutputNode )
	{
		m_pBusOutputNode->DecrementPlayCount( io_params );
	}
	if( m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		m_pFeedbackInfo->m_pFeedbackBus->DecrementPlayCount( io_params );
	}
}

/** 2012 0x93bb50. DISHONORED(IH3 seam): no RTPC, no SIS. */
bool CAkBus::GetBypassAllFX( CAkRegisteredObj * in_GameObjPtr )
{
	return m_pFXChunk ? ( m_pFXChunk->bitsMainFXBypass & 0x10 ) != 0 : false;
}

/** 2012 0x93cfd0: a mixing bus (effect or environmental) applies its own volume in its VPL, so only a plain bus adds
    its volume to the voice; pitch and LPF always. DISHONORED(IH3 seam): no ducking, states, RTPC or SIS. */
AKRESULT CAkBus::GetAudioParameters( AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr, bool in_bIncludeRange, AkPBIModValues & io_Ranges, bool in_bDoBusCheck )
{
	if( HasEffect() || m_bIsEnvironmental )
	{
		in_ulParamSelect &= ~1;
	}
	if( in_ulParamSelect & 1 )
	{
		const AkPropValue * pValue = m_props.FindProp( AkPropID_Volume );
		io_Parameters.Volume += pValue ? pValue->fValue : 0.f;
	}
	if( in_ulParamSelect & 2 )
	{
		const AkPropValue * pValue = m_props.FindProp( AkPropID_Pitch );
		io_Parameters.Pitch += pValue ? pValue->fValue : 0.f;
	}
	if( in_ulParamSelect & 4 )
	{
		const AkPropValue * pValue = m_props.FindProp( AkPropID_LPF );
		io_Parameters.LPF += pValue ? pValue->fValue : 0.f;
	}
	if( m_pBusOutputNode )
	{
		m_pBusOutputNode->GetAudioParameters( io_Parameters, in_ulParamSelect, io_rMutedMap, in_GameObjPtr, in_bIncludeRange, io_Ranges, true );
	}
	return AK_Success;
}

/** 2012 0x937c50: is the node (or its output bus chain, checked once) the limiting node in_NodeIDToTest or below it? */
bool CAkParameterNode::IsInstanceCountCompatible( AkUniqueID in_NodeIDToTest, bool in_bIsBusChecked, bool in_bIgnoreNonBus )
{
	if( !in_bIgnoreNonBus && in_NodeIDToTest == key )
	{
		return true;
	}
	if( !in_bIsBusChecked && m_pBusOutputNode )
	{
		if( static_cast<CAkBus *>( m_pBusOutputNode )->IsInstanceCountCompatible( in_NodeIDToTest ) )
		{
			return true;
		}
		in_bIsBusChecked = true;
	}
	if( !m_pParentNode )
	{
		return false;
	}
	if( m_bIsMaxNumInstOverrideParent )
	{
		in_bIgnoreNonBus = true;
	}
	return static_cast<CAkParameterNode *>( m_pParentNode )->IsInstanceCountCompatible( in_NodeIDToTest, in_bIsBusChecked, in_bIgnoreNonBus );
}

/** 2012 0x93b950. */
bool CAkBus::IsInstanceCountCompatible( AkUniqueID in_NodeIDToTest )
{
	if( in_NodeIDToTest == key )
	{
		return true;
	}
	if( m_pBusOutputNode && !m_bIsMaxNumInstOverrideParent )
	{
		return static_cast<CAkBus *>( m_pBusOutputNode )->IsInstanceCountCompatible( in_NodeIDToTest );
	}
	return false;
}

/** 2012 0x93a400: the first node with 3D parameters (or overriding positioning) gives the PBI its own copy, and the
    base (2D) parameters come from it. */
void CAkParameterNode::Get3DParams( CAkGen3DParams *& io_p3DParams, CAkRegisteredObj * in_GameObj, bool in_bUpdateOnly, BaseGenParams * io_pBasePosParams )
{
	CAkParameterNode * pNode = this;
	CAkParameterNode * pOwner;
	do
	{
		pOwner = pNode;
		if( pNode->m_p3DParameters )
		{
			if( !io_p3DParams )
			{
				void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkGen3DParams ) );
				io_p3DParams = pMem ? AkPlacementNew( pMem ) CAkGen3DParams() : NULL;
				if( io_p3DParams && !in_bUpdateOnly )
				{
					io_p3DParams->m_Params = pNode->m_p3DParameters->m_Params;
				}
			}
			else if( !in_bUpdateOnly )
			{
				io_p3DParams->m_Params = pNode->m_p3DParameters->m_Params;
			}
		}
		pNode = static_cast<CAkParameterNode *>( pNode->m_pParentNode );
	}
	while( !pOwner->m_p3DParameters && !pOwner->m_bPositioningInfoOverrideParent && pNode );
	pOwner->UpdateBaseParamsFromRTPC( in_GameObj, io_pBasePosParams );
}

/** 2012 0x93a4a0: the node that owns the positioning (overrides its parent, or the top). */
bool CAkParameterNode::UpdateBaseParams( CAkRegisteredObj * in_GameObj, BaseGenParams * io_pBasePosParams )
{
	CAkParameterNode * pNode = this;
	while( !pNode->m_bPositioningInfoOverrideParent && pNode->m_pParentNode )
	{
		pNode = static_cast<CAkParameterNode *>( pNode->m_pParentNode );
	}
	return pNode->UpdateBaseParamsFromRTPC( in_GameObj, io_pBasePosParams );
}

/** 2012 0x93a0d0: PAN_LR / PAN_FR / CenterPCT and the panner flag. DISHONORED(IH3 seam): the RTPC-driven pan
    (RTPC ids 20 / 21, scaled by the max radius for 3D) is not evaluated, so this always returns false. */
bool CAkParameterNode::UpdateBaseParamsFromRTPC( CAkRegisteredObj * in_GameObj, BaseGenParams * io_pBasePosParams )
{
	const AkPropValue * pPanLR = m_props.FindProp( AkPropID_PAN_LR );
	const AkPropValue * pPanFR = m_props.FindProp( AkPropID_PAN_FR );
	const AkPropValue * pCenter = m_props.FindProp( AkPropID_CenterPCT );
	io_pBasePosParams->m_fPAN_RL = pPanLR ? pPanLR->fValue : 0.f;
	io_pBasePosParams->m_fPAN_FR = pPanFR ? pPanFR->fValue : 0.f;
	io_pBasePosParams->m_fCenterPCT = pCenter ? pCenter->fValue : 0.f;
	io_pBasePosParams->bIsPannerEnabled = m_bPositioningEnablePanner != 0;
	return false;
}
