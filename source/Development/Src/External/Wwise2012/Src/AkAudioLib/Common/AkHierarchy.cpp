/*=============================================================================
	AkHierarchy.cpp - creation, reference counting and bank loading (SetInitialValues) of the HIRC objects.

	DISHONORED(port): agent IH1 (wave 26). Each function names its 2012 rva; the byte order every reader
	consumes is the decompiled one, including retail's own quirks (CAkSwitchCntr ignores SetNodeBaseParams'
	result, CAkLayer::SetInitialValues skips the layer id it was created with, CAkActorMixer refuses a bus as a
	child, a sound already holding a bank source keeps it). See AkHierarchy.h for the named deviations.
=============================================================================*/
#include "AkHierarchy.h"
#include "AkContainers.h"
#include "AkBankMgr.h"
#include "AkBusCtx.h"

#include <math.h>

const AkUInt8 * g_pHircParseEnd = NULL;

// ------------------------------------------------------------------------------------------------------------
// CAkSource (aksource.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9466a0. */
CAkSource::CAkSource()
{
	memset( &m_sSrcTypeInfo, 0, sizeof( m_sSrcTypeInfo ) );
	m_sSrcTypeInfo.pInMemoryMedia = NULL;
	m_sSrcTypeInfo.dwID = AK_INVALID_PLUGINID;
	m_sSrcTypeInfo.mediaInfo.uFileID = AK_INVALID_FILE_ID;
}

/** 2012 0x946570: Type 1 (file path) owns its string. */
void CAkSource::FreeSource()
{
	if( m_sSrcTypeInfo.mediaInfo.Type == 1 && m_sSrcTypeInfo.pInMemoryMedia )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_sSrcTypeInfo.pInMemoryMedia );
		m_sSrcTypeInfo.pInMemoryMedia = NULL;
	}
	m_sSrcTypeInfo.mediaInfo.bIsLanguageSpecific = 0;
	m_sSrcTypeInfo.mediaInfo.bPrefetch = 0;
	m_sSrcTypeInfo.mediaInfo.Type = 0;
	m_sSrcTypeInfo.mediaInfo.bHasSource = 0;
	m_sSrcTypeInfo.mediaInfo.bExternallySupplied = 0;
	m_sSrcTypeInfo.mediaInfo.bIsFromRSX = 0;
	m_sSrcTypeInfo.mediaInfo.sourceID = 0;
	m_sSrcTypeInfo.mediaInfo.uFileOffset = 0;
	m_sSrcTypeInfo.mediaInfo.uInMemoryMediaSize = 0;
	m_sSrcTypeInfo.mediaInfo.uFileID = AK_INVALID_FILE_ID;
	m_sSrcTypeInfo.pInMemoryMedia = NULL;
}

/** 2012 0x946820: a plug-in source (no media): Type 2, no bHasSource. */
void CAkSource::SetSource( AkUniqueID in_sourceID )
{
	FreeSource();
	m_sSrcTypeInfo.dwID = AK_INVALID_PLUGINID;
	m_sSrcTypeInfo.mediaInfo.uFileID = AK_INVALID_FILE_ID;
	m_sSrcTypeInfo.mediaInfo.sourceID = in_sourceID;
	m_sSrcTypeInfo.mediaInfo.uFileOffset = 0;
	m_sSrcTypeInfo.mediaInfo.uInMemoryMediaSize = 0;
	m_sSrcTypeInfo.mediaInfo.Type = 2;
}

/** 2012 0x946790. */
void CAkSource::SetSource( AkUInt32 in_PluginID, const AkMediaInformation & in_MediaInfo )
{
	FreeSource();
	m_sSrcTypeInfo.mediaInfo = in_MediaInfo;
	m_sSrcTypeInfo.dwID = in_PluginID;
	m_sSrcTypeInfo.mediaInfo.bHasSource = 1;
}

// ------------------------------------------------------------------------------------------------------------
// CAkGen3DParams (akgen3dparams.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x954cb0. */
CAkGen3DParams::CAkGen3DParams()
{
	memset( &m_Params, 0, sizeof( m_Params ) );
	m_Params.m_pAttenuation = NULL;
	m_Params.m_ID = 0;
	m_Params.m_eType = AkUndefined;
	m_Params.m_uAttenuationID = 0;
	m_Params.m_ePathMode = AkStepSequence;
	m_Params.m_TransitionTime = 0;
	m_Params.m_Position = g_DefaultSoundPosition.Position;
	m_Params.m_pArrayVertex = NULL;
	m_Params.m_ulNumVertices = 0;
	m_Params.m_pArrayPlaylist = NULL;
	m_Params.m_ulNumPlaylistItem = 0;
	m_Params.m_bIsSpatialized = 0;
	m_Params.m_bIsConeEnabled = 0;
	m_Params.m_bIsPanningFromRTPC = 0;
	m_Params.m_bIsDynamic = 1;
	m_Params.m_bFollowOrientation = 1;
	m_Params.m_bIsLooping = 0;
	m_Params.m_fConeOutsideVolume = 0.f;
	m_Params.m_fConeLoPass = 0.f;
}

/** 2012 0x954d60. */
void CAkGen3DParams::UpdateTransitionTimeInVertex()
{
	for( AkUInt32 i = 0; i < m_Params.m_ulNumPlaylistItem; ++i )
	{
		AkPathListItem & item = m_Params.m_pArrayPlaylist[ i ];
		if( item.iNumVertices > 0 )
		{
			item.pVertices[ item.iNumVertices - 1 ].Duration = m_Params.m_TransitionTime;
		}
	}
}

/** 2012 0x954e60. */
AKRESULT CAkGen3DParams::SetPath( AkPathVertex * in_pArrayVertex, AkUInt32 in_ulNumVertices, AkPathListItemOffset * in_pArrayPlaylist, AkUInt32 in_ulNumPlaylistItem )
{
	AKRESULT eResult = AK_Success;
	Term();
	if( !in_ulNumVertices || !in_ulNumPlaylistItem || !in_pArrayVertex || !in_pArrayPlaylist )
	{
		UpdateTransitionTimeInVertex();
		return AK_InvalidParameter;
	}
	m_Params.m_pArrayVertex = (AkPathVertex *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkPathVertex ) * in_ulNumVertices );
	if( !m_Params.m_pArrayVertex )
	{
		UpdateTransitionTimeInVertex();
		return AK_InsufficientMemory;
	}
	memcpy( m_Params.m_pArrayVertex, in_pArrayVertex, sizeof( AkPathVertex ) * in_ulNumVertices );
	m_Params.m_ulNumVertices = in_ulNumVertices;
	m_Params.m_pArrayPlaylist = (AkPathListItem *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkPathListItem ) * in_ulNumPlaylistItem );
	if( !m_Params.m_pArrayPlaylist )
	{
		UpdateTransitionTimeInVertex();
		return AK_InsufficientMemory;
	}
	m_Params.m_ulNumPlaylistItem = in_ulNumPlaylistItem;
	for( AkUInt32 i = 0; i < in_ulNumPlaylistItem; ++i )
	{
		m_Params.m_pArrayPlaylist[ i ].iNumVertices = in_pArrayPlaylist[ i ].iNumVertices;
		const AkUInt32 ulVerticesOffset = in_pArrayPlaylist[ i ].ulVerticesOffset;
		if( ulVerticesOffset >= in_ulNumVertices )
		{
			UpdateTransitionTimeInVertex();
			return AK_Fail;
		}
		m_Params.m_pArrayPlaylist[ i ].pVertices = &m_Params.m_pArrayVertex[ ulVerticesOffset ];
		m_Params.m_pArrayPlaylist[ i ].fRangeX = 0.f;
		m_Params.m_pArrayPlaylist[ i ].fRangeY = 0.f;
	}
	UpdateTransitionTimeInVertex();
	return eResult;
}

/** 2012 0x954e00. */
void CAkGen3DParams::Term()
{
	if( m_Params.m_pArrayVertex )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_Params.m_pArrayVertex );
		m_Params.m_pArrayVertex = NULL;
	}
	if( m_Params.m_pArrayPlaylist )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_Params.m_pArrayPlaylist );
		m_Params.m_pArrayPlaylist = NULL;
	}
	m_Params.m_ulNumPlaylistItem = 0;
	m_Params.m_ulNumVertices = 0;
}

/** 2012 0x954da0. */
void CAkGen3DParamsEx::FreePathInfo()
{
	if( m_PathState.pbPlayed )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_PathState.pbPlayed );
		m_PathState.pbPlayed = NULL;
	}
}

// ------------------------------------------------------------------------------------------------------------
// AkStateGroupChunk (akparameternodebase.cpp)
// ------------------------------------------------------------------------------------------------------------

AkStateGroupChunk::AkStateGroupChunk( CAkParameterNodeBase * in_pOwner, AkStateGroupID in_ulStateGroup )
	: m_pNextInGroup( NULL )
	, m_pNextInNode( NULL )
	, m_pOwner( in_pOwner )
	, m_ulStateGroup( in_ulStateGroup )
	, m_ulActualState( 0 )
	, m_eStateSyncType( 0 )
{
}

AkStateGroupChunk::~AkStateGroupChunk()
{
	for( MapStruct<AkStateID, AkStateLink> * p = m_mapStates.m_pItems; p != m_mapStates.m_pItemsEnd; ++p )
	{
		if( p->item.pState )
		{
			p->item.pState->TermNotificationSystem();
			p->item.pState->Release();
		}
	}
	m_mapStates.Term();
	m_values.RemoveAll();
}

/** 2012 0x945930: the custom state comes out of the custom states index (g_pIndex + 800). */
AKRESULT AkStateGroupChunk::AddState( AkUniqueID in_ulStateInstanceID, AkStateID in_ulStateID, bool in_bNotify )
{
	CAkState * pState = g_pIndex->m_idxCustomStates.GetPtrAndAddRef( in_ulStateInstanceID );
	AkStateLink * pLink = m_mapStates.Exists( in_ulStateID );
	if( pLink )
	{
		if( pState == pLink->pState )
		{
			if( pState )
			{
				pState->Release();
			}
			return AK_Success;
		}
		pLink->pState->TermNotificationSystem();
		pLink->pState->Release();
		m_mapStates.Unset( in_ulStateID );
	}
	if( !pState )
	{
		return AK_IDNotFound;
	}
	AkStateLink link;
	link.pState = pState;
	link.ulStateID = in_ulStateInstanceID;
	if( !m_mapStates.Set( in_ulStateID, link ) )
	{
		pState->Release();
		return AK_InsufficientMemory;
	}
	pState->SetParentToNotify( m_pOwner );
	if( in_bNotify )
	{
		m_pOwner->NotifyStateParametersModified();
	}
	return AK_Success;
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNodeBase (akparameternodebase.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x942ad0. */
CAkParameterNodeBase::FXChunk::FXChunk()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		aFX[ i ].bRendered = false;
		aFX[ i ].bShareSet = false;
		aFX[ i ].id = 0;
	}
	bitsMainFXBypass = 0;
}

/** 2012 0x944180: byte +46 starts at 0x90 (m_bIsGlobalLimit | m_bUseState). */
CAkParameterNodeBase::CAkParameterNodeBase( AkUniqueID in_ulID )
	: CAkPBIAware( in_ulID )
	, m_pGlobalSIS( NULL )
	, m_pFXChunk( NULL )
	, m_pActivityChunk( NULL )
	, m_pParentNode( NULL )
	, m_pBusOutputNode( NULL )
	, m_u16MaxNumInstance( 0 )
	, m_bKillNewest( 0 )
	, m_bUseVirtualBehavior( 0 )
	, m_bIsVVoicesOptOverrideParent( 0 )
	, m_bIsMaxNumInstOverrideParent( 0 )
	, m_bIsGlobalLimit( 1 )
	, m_bPriorityApplyDistFactor( 0 )
	, m_bPriorityOverrideParent( 0 )
	, m_bUseState( 1 )
	, m_bIsInDestructor( 0 )
	, m_bPositioningInfoOverrideParent( 0 )
	, m_bPositioningEnablePanner( 0 )
	, m_bIsFXOverrideParent( 0 )
	, m_bIsContinuousValidation( 0 )
	, m_pFeedbackInfo( NULL )
{
}

/** 2012 0x946390. */
CAkParameterNodeBase::~CAkParameterNodeBase()
{
	m_bIsInDestructor = 1;
	if( m_pFXChunk )
	{
		m_pFXChunk->~FXChunk();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pFXChunk );
		m_pFXChunk = NULL;
	}
	for( AkUInt32 i = 0; m_RTPCBitArrayMax32.m_i32BitArray; ++i )
	{
		if( m_RTPCBitArrayMax32.IsSet( i ) )
		{
			if( g_pRTPCMgr )
			{
				g_pRTPCMgr->UnSubscribeRTPC( this, (AkRTPC_ParameterID)i );
			}
			m_RTPCBitArrayMax32.UnsetBit( i );
		}
	}
	if( m_pGlobalSIS )
	{
		m_pGlobalSIS->~CAkSIS();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pGlobalSIS );
		m_pGlobalSIS = NULL;
	}
	FlushStateTransitions();
	RemoveStateGroups( true );
	if( m_pFeedbackInfo )
	{
		if( m_pFeedbackInfo->m_pFeedbackBus )
		{
			m_pFeedbackInfo->m_pFeedbackBus->RemoveChild( this );
		}
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pFeedbackInfo );
		m_pFeedbackInfo = NULL;
	}
	m_props.RemoveAll();
}

/** 2012 0x9437e0: under the audio node index lock. */
AkUInt32 CAkParameterNodeBase::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAudioNode.m_IndexLock );
	return ++m_lRef;
}

/** 2012 0x944a90. */
AkUInt32 CAkParameterNodeBase::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAudioNode.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		g_pIndex->m_idxAudioNode.RemoveID( key );
		OnPreRelease();
		if( m_pParentNode )
		{
			m_pParentNode->RemoveChild( this );
		}
		if( m_pBusOutputNode )
		{
			m_pBusOutputNode->RemoveChild( this );
		}
		this->~CAkParameterNodeBase();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x944a40. */
void CAkParameterNodeBase::AddToIndex()
{
	g_pIndex->m_idxAudioNode.SetIDToPtr( this );
}

/** 2012 0x942d00. */
AKRESULT CAkParameterNodeBase::SetFX( AkUInt32 in_uFXIndex, AkUniqueID in_uID, bool in_bShareSet )
{
	if( in_uFXIndex >= 4 )
	{
		return AK_InvalidParameter;
	}
	if( !m_pFXChunk )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( FXChunk ) );
		if( !pMem )
		{
			return AK_InsufficientMemory;
		}
		m_pFXChunk = AkPlacementNew( pMem ) FXChunk();
	}
	FXStruct & fx = m_pFXChunk->aFX[ in_uFXIndex ];
	if( fx.bShareSet != in_bShareSet || fx.id != in_uID )
	{
		fx.bShareSet = in_bShareSet;
		fx.id = in_uID;
		RecalcNotification();
		UpdateFx( in_uFXIndex );
	}
	return AK_Success;
}

/** 2012 0x942e00: a rendered effect is removed from the node (its sound already carries it). */
void CAkParameterNodeBase::RenderedFX( AkUInt32 in_uFXIndex, bool in_bRendered )
{
	if( !m_pFXChunk )
	{
		if( !in_bRendered )
		{
			return;
		}
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( FXChunk ) );
		if( !pMem )
		{
			return;
		}
		m_pFXChunk = AkPlacementNew( pMem ) FXChunk();
	}
	m_pFXChunk->aFX[ in_uFXIndex ].bRendered = in_bRendered;
	if( in_bRendered && m_pFXChunk->aFX[ in_uFXIndex ].id )
	{
		RemoveFX( in_uFXIndex );
	}
}

/** 2012 0x942e70: the bypass bits live in the FX chunk; the SIS and the playing voices follow. */
void CAkParameterNodeBase::MainBypassFX( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	if( !m_pFXChunk )
	{
		if( !in_bitsFXBypass )
		{
			return;
		}
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( FXChunk ) );
		if( !pMem )
		{
			return;
		}
		m_pFXChunk = AkPlacementNew( pMem ) FXChunk();
	}
	m_pFXChunk->bitsMainFXBypass = (AkUInt8)( in_bitsFXBypass | ( m_pFXChunk->bitsMainFXBypass & ~in_uTargetMask ) );
	ResetFXBypass( in_bitsFXBypass, in_uTargetMask );
	NotifyBypass( in_bitsFXBypass, in_uTargetMask, NULL, NULL );
}

/** 2012 0x945a50: a 44-byte chunk registered with the state manager, pushed on the node's list. */
AkStateGroupChunk * CAkParameterNodeBase::AddStateGroup( AkStateGroupID in_ulStateGroupID, bool in_bNotify )
{
	for( AkStateGroupChunk * pChunk = m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
	{
		if( pChunk->m_ulStateGroup == in_ulStateGroupID )
		{
			return pChunk;
		}
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkStateGroupChunk ) );
	if( !pMem )
	{
		return NULL;
	}
	AkStateGroupChunk * pChunk = AkPlacementNew( pMem ) AkStateGroupChunk( this, in_ulStateGroupID );
	if( g_pStateMgr->AddStateGroupMember( in_ulStateGroupID, pChunk ) != AK_Success )
	{
		pChunk->~AkStateGroupChunk();
		AK::MemoryMgr::Free( g_DefaultPoolId, pChunk );
		return NULL;
	}
	m_states.AddFirst( pChunk );
	pChunk->m_ulActualState = g_pStateMgr->GetState( in_ulStateGroupID );
	if( in_bNotify )
	{
		NotifyStateParametersModified();
	}
	return pChunk;
}

/** CAkParameterNodeBase::RemoveStateGroups: unregister and free every chunk. */
void CAkParameterNodeBase::RemoveStateGroups( bool in_bNotify )
{
	AkStateGroupChunk * pChunk = m_states.First();
	while( pChunk )
	{
		AkStateGroupChunk * pNext = pChunk->m_pNextInNode;
		if( g_pStateMgr )
		{
			g_pStateMgr->RemoveStateGroupMember( pChunk->m_ulStateGroup, pChunk );
		}
		pChunk->~AkStateGroupChunk();
		AK::MemoryMgr::Free( g_DefaultPoolId, pChunk );
		pChunk = pNext;
	}
	m_states.m_pFirst = NULL;
	if( in_bNotify )
	{
		NotifyStateParametersModified();
	}
}

/** 2012 0x945da0. */
AKRESULT CAkParameterNodeBase::SetNodeBaseParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly )
{
	AKRESULT eResult = SetInitialFxParams( io_rpData, io_rulDataSize, in_bPartialLoadOnly );
	if( eResult != AK_Success || in_bPartialLoadOnly )
	{
		return eResult;
	}

	const AkUniqueID OverrideBusId = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( OverrideBusId )
	{
		CAkParameterNodeBase * pBus = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( OverrideBusId );
		if( !pBus )
		{
			return AK_Fail;
		}
		eResult = pBus->AddChild( key );
		pBus->Release();
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}

	const AkUniqueID DirectParentID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( DirectParentID )
	{
		CAkParameterNodeBase * pParent = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( DirectParentID );
		if( pParent )
		{
			eResult = pParent->AddChild( key );
			pParent->Release();
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}

	const bool bPriorityOverrideParent = *io_rpData++ != 0;
	const bool bPriorityApplyDistFactor = *io_rpData++ != 0;
	if( ( m_bPriorityOverrideParent != 0 ) != bPriorityOverrideParent )
	{
		m_bPriorityOverrideParent = bPriorityOverrideParent;
		RecalcNotification();
	}
	if( ( m_bPriorityApplyDistFactor != 0 ) != bPriorityApplyDistFactor )
	{
		m_bPriorityApplyDistFactor = bPriorityApplyDistFactor;
		RecalcNotification();
	}

	eResult = SetInitialParams( io_rpData, io_rulDataSize );
	if( eResult == AK_Success )
	{
		eResult = SetPositioningParams( io_rpData, io_rulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = SetAdvSettingsParams( io_rpData, io_rulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = ReadStateChunk( io_rpData, io_rulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = SetInitialRTPC( io_rpData, io_rulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = ReadFeedbackInfo( io_rpData, io_rulDataSize );
	}
	return eResult;
}

/** 2012 0x943860: {rtpc id, param id, curve id, scaling, point count, points x 12}. */
AKRESULT CAkParameterNodeBase::SetInitialRTPC( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	const AkUInt16 uNumCurves = *(AkUInt16 *)io_rpData;
	io_rpData += 2;
	for( AkUInt32 i = 0; i < uNumCurves; ++i )
	{
		const AkRtpcID RTPC_ID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const AkUInt32 ParamID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const AkUniqueID RTPCCurveID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const AkCurveScaling eScaling = (AkCurveScaling)*io_rpData++;
		const AkUInt32 ulSize = *(AkUInt16 *)io_rpData;
		io_rpData += 2;
		AkRTPCGraphPoint * pCurve = (AkRTPCGraphPoint *)io_rpData;
		m_RTPCBitArrayMax32.SetBit( ParamID );
		if( g_pRTPCMgr )
		{
			g_pRTPCMgr->SubscribeRTPC( this, RTPC_ID, (AkRTPC_ParameterID)ParamID, RTPCCurveID, eScaling, pCurve, ulSize, NULL, GetRTPCSubscriberType() );
		}
		NotifyRTPCChanged();
		io_rpData += sizeof( AkRTPCGraphPoint ) * ulSize;
		io_rulDataSize -= sizeof( AkRTPCGraphPoint ) * ulSize;
	}
	return AK_Success;
}

/** 2012 0x945ce0: {group count, per group {group id, sync type, state count, {state id, instance id}}}. */
AKRESULT CAkParameterNodeBase::ReadStateChunk( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	const AkUInt32 ulNumStateGroups = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	for( AkUInt32 groupIdx = 0; groupIdx < ulNumStateGroups; ++groupIdx )
	{
		const AkStateGroupID ulStateGroupID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		AkStateGroupChunk * pChunk = AddStateGroup( ulStateGroupID, true );
		if( !pChunk )
		{
			return AK_Fail;
		}
		pChunk->m_eStateSyncType = *io_rpData++;
		const AkUInt16 ulNumStates = *(AkUInt16 *)io_rpData;
		io_rpData += 2;
		for( AkUInt32 i = 0; i < ulNumStates; ++i )
		{
			const AkStateID ulStateType = *(AkUInt32 *)io_rpData;
			io_rpData += 4;
			const AkUniqueID ulStateInstanceID = *(AkUInt32 *)io_rpData;
			io_rpData += 4;
			const AKRESULT eResult = pChunk->AddState( ulStateInstanceID, ulStateType, true );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x945820: only present when the bank header says bFeedbackInBank. */
AKRESULT CAkParameterNodeBase::ReadFeedbackInfo( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	if( !g_pBankManager->m_bFeedbackInBank )
	{
		return AK_Success;
	}
	const AkUniqueID FeedbackBusID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( !FeedbackBusID )
	{
		return AK_Success;
	}
	if( !m_pFeedbackInfo )
	{
		m_pFeedbackInfo = (AkFeedbackInfo *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkFeedbackInfo ) );
		if( !m_pFeedbackInfo )
		{
			return AK_InsufficientMemory;
		}
		m_pFeedbackInfo->m_pFeedbackBus = NULL;
	}
	CAkParameterNodeBase * pBus = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( FeedbackBusID );
	if( !pBus )
	{
		return AK_Fail;
	}
	const AKRESULT eResult = pBus->AddChild( key );
	pBus->Release();
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNode (akparameternode.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x938590. */
CAkParameterNode::CAkParameterNode( AkUniqueID in_ulID )
	: CAkParameterNodeBase( in_ulID )
	, m_pMapSIS( NULL )
	, m_p3DParameters( NULL )
	, m_eVirtualQueueBehavior( 0 )
	, m_eBelowThresholdBehavior( 0 )
	, m_pAssociatedLayers( NULL )
{
}

/** 2012 0x939060. */
CAkParameterNode::~CAkParameterNode()
{
	if( m_pMapSIS )
	{
		for( MapStruct<CAkRegisteredObj *, CAkSIS *> * p = m_pMapSIS->m_pItems; p != m_pMapSIS->m_pItemsEnd; ++p )
		{
			if( p->item )
			{
				p->item->~CAkSIS();
				AK::MemoryMgr::Free( g_DefaultPoolId, p->item );
			}
		}
		m_pMapSIS->Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pMapSIS );
		m_pMapSIS = NULL;
	}
	m_bPositioningInfoOverrideParent = 0;
	if( m_p3DParameters )
	{
		m_p3DParameters->Term();
		m_p3DParameters->~CAkGen3DParamsEx();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_p3DParameters );
		m_p3DParameters = NULL;
	}
	if( m_pAssociatedLayers )
	{
		m_pAssociatedLayers->Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pAssociatedLayers );
		m_pAssociatedLayers = NULL;
	}
	m_ranges.RemoveAll();
}

/** 2012 0x938b50. */
AKRESULT CAkParameterNode::SetInitialParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	if( reinterpret_cast< AkPropBundle<AkReal32> & >( m_props ).SetInitialParams( io_rpData, io_rulDataSize ) != AK_Success )
	{
		return AK_Fail;
	}
	if( m_ranges.SetInitialParams( io_rpData, io_rulDataSize ) != AK_Success )
	{
		return AK_Fail;
	}
	m_bUseState = 1;
	return AK_Success;
}

/** 2012 0x937e40: {override, count, [bypass bits, {index, id, is share set, rendered} x count]}. */
AKRESULT CAkParameterNode::SetInitialFxParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool in_bPartialLoadOnly )
{
	AKRESULT eResult = AK_Success;
	const bool bIsOverrideParentFX = *io_rpData++ != 0;
	if( !in_bPartialLoadOnly )
	{
		m_bIsFXOverrideParent = bIsOverrideParentFX;
	}
	const AkUInt32 uNumFx = *io_rpData++;
	if( !uNumFx )
	{
		return AK_Success;
	}
	const AkUInt32 bitsFXBypass = *io_rpData++;
	for( AkUInt32 uFX = 0; uFX < uNumFx; ++uFX )
	{
		const AkUInt32 uFXIndex = *io_rpData++;
		const AkUniqueID fxID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const bool bIsShareSet = *io_rpData++ != 0;
		const bool bIsRendered = *io_rpData++ != 0;
		RenderedFX( uFXIndex, bIsRendered );
		if( !bIsRendered && fxID && !in_bPartialLoadOnly )
		{
			eResult = SetFX( uFXIndex, fxID, bIsShareSet );
			if( eResult != AK_Success )
			{
				MainBypassFX( bitsFXBypass, 0xFFFFFFFF );
				return eResult;
			}
		}
	}
	if( !in_bPartialLoadOnly )
	{
		MainBypassFX( bitsFXBypass, 0xFFFFFFFF );
	}
	return eResult;
}

/** 2012 0x937f50. */
AKRESULT CAkParameterNode::Enable3DPosParams()
{
	if( m_p3DParameters )
	{
		return AK_Success;
	}
	m_bPositioningInfoOverrideParent = 1;
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkGen3DParamsEx ) );
	if( !pMem )
	{
		return AK_InsufficientMemory;
	}
	m_p3DParameters = AkPlacementNew( pMem ) CAkGen3DParamsEx();
	m_p3DParameters->m_Params.m_ID = key;
	return AK_Success;
}

/** 2012 0x937fc0. */
AKRESULT CAkParameterNode::SetPositioningParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_bPositioningInfoOverrideParent = *io_rpData++ != 0;
	if( !m_bPositioningInfoOverrideParent )
	{
		return AK_Success;
	}
	const bool bIs3D = *io_rpData++ != 0;
	if( !bIs3D )
	{
		m_bPositioningEnablePanner = *io_rpData++ != 0;
		return AK_Success;
	}
	AKRESULT eResult = Enable3DPosParams();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	Gen3DParams & params = m_p3DParameters->m_Params;
	params.m_eType = (AkPositioningType)*(AkUInt32 *)io_rpData;
	io_rpData += 4;
	params.m_uAttenuationID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	params.m_bIsSpatialized = *io_rpData++ != 0;
	if( params.m_eType == Ak3DGameDef )
	{
		params.m_bIsDynamic = *io_rpData++ != 0;
		return AK_Success;
	}
	if( params.m_eType != Ak3DUserDef )
	{
		return AK_Success;
	}
	params.m_ePathMode = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	params.m_bIsLooping = *io_rpData++ != AkStepSequence;
	params.m_TransitionTime = *(AkInt32 *)io_rpData;
	io_rpData += 4;
	params.m_bFollowOrientation = *io_rpData++ != 0;

	AkPathVertex * pVertices = NULL;
	const AkUInt32 ulNumVertices = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( ulNumVertices )
	{
		pVertices = (AkPathVertex *)io_rpData;
		io_rpData += sizeof( AkPathVertex ) * ulNumVertices;
	}
	const AkUInt32 ulNumPlayListItem = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( ulNumPlayListItem )
	{
		AkPathListItemOffset * pPlayListItems = (AkPathListItemOffset *)io_rpData;
		io_rpData += sizeof( AkPathListItemOffset ) * ulNumPlayListItem;
		if( ulNumVertices )
		{
			eResult = m_p3DParameters ? m_p3DParameters->SetPath( pVertices, ulNumVertices, pPlayListItems, ulNumPlayListItem ) : AK_Fail;
		}
	}
	for( AkUInt32 i = 0; i < ulNumPlayListItem; ++i )
	{
		const AkReal32 fRangeX = ( (AkReal32 *)io_rpData )[ 0 ];
		const AkReal32 fRangeY = ( (AkReal32 *)io_rpData )[ 1 ];
		io_rpData += 8;
		if( m_p3DParameters && i < m_p3DParameters->m_Params.m_ulNumPlaylistItem )
		{
			m_p3DParameters->m_Params.m_pArrayPlaylist[ i ].fRangeX = fRangeX;
			m_p3DParameters->m_Params.m_pArrayPlaylist[ i ].fRangeY = fRangeY;
		}
	}
	return eResult;
}

/** 2012 0x938280: {virtual queue, max reached, over limit, max instances u16, global limit, below threshold, max inst override, vvoices override}. */
AKRESULT CAkParameterNode::SetAdvSettingsParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	const AkUInt8 eVirtualQueueBehavior = *io_rpData++;
	const bool bKillNewest = *io_rpData++ != 0;
	const bool bUseVirtualBehavior = *io_rpData++ != 0;
	m_u16MaxNumInstance = *(AkUInt16 *)io_rpData;
	io_rpData += 2;
	m_bIsGlobalLimit = *io_rpData++ != 0;
	const AkUInt8 eBelowThresholdBehavior = *io_rpData++;
	const bool bIsMaxNumInstOverrideParent = *io_rpData++ != 0;
	const bool bIsVVoiceOptOverrideParent = *io_rpData++ != 0;
	m_eVirtualQueueBehavior = eVirtualQueueBehavior;
	SetMaxReachedBehavior( bKillNewest );
	SetOverLimitBehavior( bUseVirtualBehavior );
	m_eBelowThresholdBehavior = eBelowThresholdBehavior;
	SetMaxNumInstOverrideParent( bIsMaxNumInstOverrideParent );
	SetVVoicesOptOverrideParent( bIsVVoiceOptOverrideParent );
	return AK_Success;
}

/** 2012 0x939870: walk up to the node that owns positioning, then read its attenuation's curves. */
AKRESULT CAkParameterNode::GetStatic3DParams( AkPositioningInfo & out_rPosInfo )
{
	memset( &out_rPosInfo, 0, sizeof( out_rPosInfo ) );
	CAkParameterNode * pNode = this;
	for( ;; )
	{
		CAkParameterNode * pParent = static_cast<CAkParameterNode *>( pNode->m_pParentNode );
		if( pNode->m_p3DParameters )
		{
			break;
		}
		if( pNode->m_bPositioningInfoOverrideParent || !pParent )
		{
			break;
		}
		pNode = pParent;
	}
	if( !pNode->m_p3DParameters )
	{
		out_rPosInfo.positioningType = Ak2DPositioning;
	}
	else
	{
		const Gen3DParams & params = pNode->m_p3DParameters->m_Params;
		out_rPosInfo.positioningType = params.m_eType;
		out_rPosInfo.bUpdateEachFrame = params.m_bIsDynamic != 0;
		out_rPosInfo.bUseSpatialization = params.m_bIsSpatialized != 0;
		CAkAttenuation * pAttenuation = g_pIndex->m_idxAttenuations.GetPtrAndAddRef( params.m_uAttenuationID );
		if( pAttenuation )
		{
			out_rPosInfo.bUseAttenuation = true;
			out_rPosInfo.bUseConeAttenuation = pAttenuation->m_bIsConeEnabled != 0;
			if( pAttenuation->m_bIsConeEnabled )
			{
				out_rPosInfo.fInnerAngle = pAttenuation->m_ConeParams.fInsideAngle;
				out_rPosInfo.fOuterAngle = pAttenuation->m_ConeParams.fOutsideAngle;
				out_rPosInfo.fConeMaxAttenuation = pAttenuation->m_ConeParams.fOutsideVolume;
				out_rPosInfo.LPFCone = pAttenuation->m_ConeParams.LoPass;
			}
			CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeDry = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeDry );
			out_rPosInfo.fMaxDistance = pVolumeDry->m_pArrayGraphPoints[ pVolumeDry->m_ulArraySize - 1 ].From;
			out_rPosInfo.fVolDryAtMaxDist = pVolumeDry->m_pArrayGraphPoints[ pVolumeDry->m_ulArraySize - 1 ].To;
			CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeWet = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeWet );
			if( pVolumeWet )
			{
				out_rPosInfo.fVolWetAtMaxDist = pVolumeWet->m_pArrayGraphPoints[ pVolumeWet->m_ulArraySize - 1 ].To;
			}
			CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pLPF = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_LowPassFilter );
			if( pLPF )
			{
				out_rPosInfo.LPFValueAtMaxDist = pLPF->m_pArrayGraphPoints[ pLPF->m_ulArraySize - 1 ].To;
			}
			pAttenuation->Release();
		}
	}
	AkPropValue * pCenterPct = pNode->m_props.FindProp( AkPropID_CenterPCT );
	const AkReal32 fCenterPct = pCenterPct ? pCenterPct->fValue : 0.f;
	out_rPosInfo.fCenterPct = fCenterPct * 0.0099999998f;
	return AK_Success;
}

/** 2012 0x939a10. */
bool CAkParameterNode::GetMaxRadius( AkReal32 & out_fRadius )
{
	out_fRadius = 0.f;
	CAkParameterNode * pNode = this;
	while( !pNode->m_p3DParameters )
	{
		if( pNode->m_bPositioningInfoOverrideParent || !pNode->m_pParentNode )
		{
			return false;
		}
		pNode = static_cast<CAkParameterNode *>( pNode->m_pParentNode );
	}
	CAkAttenuation * pAttenuation = g_pIndex->m_idxAttenuations.GetPtrAndAddRef( pNode->m_p3DParameters->m_Params.m_uAttenuationID );
	if( !pAttenuation )
	{
		return false;
	}
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> * pVolumeDry = pAttenuation->GetCurve( CAkAttenuation::AttenuationCurveID_VolumeDry );
	if( !pVolumeDry )
	{
		pAttenuation->Release();
		return false;
	}
	out_fRadius = pVolumeDry->m_pArrayGraphPoints[ pVolumeDry->m_ulArraySize - 1 ].From;
	pAttenuation->Release();
	return true;
}

// ------------------------------------------------------------------------------------------------------------
// CAkParentNode<T> (akparentnode.h)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94ed60 (CAkParameterNode) / 0x93cd80 (CAkParameterNodeBase). */
template <class T>
AKRESULT CAkParentNode<T>::AddChild( AkUniqueID in_ulID )
{
	if( !in_ulID )
	{
		return AK_InvalidID;
	}
	CAkParameterNodeBase * pAudioNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_ulID );
	if( !pAudioNode )
	{
		return AK_IDNotFound;
	}
	AKRESULT eResult = this->CanAddChild( pAudioNode );
	if( eResult == AK_Success )
	{
		CAkParameterNodeBase ** ppNode = m_mapChildId.AddNoSetKey( in_ulID );
		if( !ppNode )
		{
			pAudioNode->Release();
			return AK_Fail;
		}
		*ppNode = pAudioNode;
		pAudioNode->Parent( this );
		this->AddRef();
	}
	pAudioNode->Release();
	return eResult;
}

/** 2012 0x94e570 / 0x93c640. */
template <class T>
void CAkParentNode<T>::RemoveChild( CAkParameterNodeBase * in_pChild )
{
	if( in_pChild->m_pParentNode != this )
	{
		return;
	}
	in_pChild->Parent( NULL );
	m_mapChildId.Unset( in_pChild->key );
	this->Release();
}

/** 2012 0x94e320. */
template <class T>
void CAkParentNode<T>::GetChildren( AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos, AkUInt32 & index_out, AkUInt32 iDepth )
{
	for( CAkParameterNodeBase ** it = m_mapChildId.m_pItems; it != m_mapChildId.m_pItemsEnd; ++it )
	{
		if( index_out < io_ruNumItems )
		{
			out_aObjectInfos[ index_out ].objID = ( *it )->key;
			out_aObjectInfos[ index_out ].parentID = ( *it )->m_pParentNode->key;
			out_aObjectInfos[ index_out ].iDepth = iDepth;
		}
		if( ++index_out == io_ruNumItems )
		{
			break;
		}
		( *it )->GetChildren( io_ruNumItems, out_aObjectInfos, index_out, iDepth + 1 );
		if( index_out == io_ruNumItems )
		{
			break;
		}
	}
}

/** 2012 0x94dc40: reserve exactly the child count, then AddChild each. */
template <class T>
AKRESULT CAkParentNode<T>::SetChildren( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	AKRESULT eResult = AK_Success;
	const AkUInt32 ulNumChilds = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( ulNumChilds )
	{
		if( m_mapChildId.Reserve( ulNumChilds ) != AK_Success )
		{
			return AK_InsufficientMemory;
		}
		for( AkUInt32 i = 0; i < ulNumChilds; ++i )
		{
			const AkUniqueID ulChildID = *(AkUInt32 *)io_rpData;
			io_rpData += 4;
			eResult = AddChild( ulChildID );
			if( eResult != AK_Success )
			{
				break;
			}
		}
	}
	return eResult;
}

template class CAkParentNode<CAkParameterNode>;
template class CAkParentNode<CAkParameterNodeBase>;

// ------------------------------------------------------------------------------------------------------------
// CAkSound (aksound.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94a670. */
CAkSound * CAkSound::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSound ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSound * pSound = AkPlacementNew( pMem ) CAkSound( in_ulID );
	pSound->AddToIndex();
	return pSound;
}

/** 2012 0x94a6f0: the source first (CAkBankMgr::LoadSource), then the node params. */
AKRESULT CAkSound::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize, CAkUsageSlot *, bool in_bIsPartialLoadOnly )
{
	in_pData += 4;
	AkBankSourceData oSourceInfo;
	AKRESULT eResult = CAkBankMgr::LoadSource( in_pData, in_ulDataSize, oSourceInfo );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( oSourceInfo.m_pParam )
	{
		m_Source.SetSource( oSourceInfo.m_MediaInfo.sourceID );
	}
	else
	{
		m_Source.SetSource( oSourceInfo.m_PluginID, oSourceInfo.m_MediaInfo );
	}
	eResult = SetNodeBaseParams( in_pData, in_ulDataSize, in_bIsPartialLoadOnly );
	g_pHircParseEnd = in_pData;
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkActorMixer (akactormixer.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x951a30. */
CAkActorMixer * CAkActorMixer::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkActorMixer ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkActorMixer * pActorMixer = AkPlacementNew( pMem ) CAkActorMixer( in_ulID );
	pActorMixer->AddToIndex();
	return pActorMixer;
}

/** 2012 0x951980. */
AKRESULT CAkActorMixer::CanAddChild( CAkParameterNodeBase * in_pAudioNode )
{
	if( in_pAudioNode->NodeCategory() == AkNodeCategory_Bus )
	{
		return AK_NotCompatible;
	}
	if( in_pAudioNode->m_pParentNode )
	{
		return AK_ChildAlreadyHasAParent;
	}
	if( m_mapChildId.Exists( in_pAudioNode->key ) )
	{
		return AK_AlreadyConnected;
	}
	if( key == in_pAudioNode->key )
	{
		return AK_CannotAddItseflAsAChild;
	}
	return AK_Success;
}

/** 2012 0x9519f0. */
AKRESULT CAkActorMixer::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 4;
	AKRESULT eResult = SetNodeBaseParams( in_pData, in_ulDataSize, false );
	if( eResult == AK_Success )
	{
		eResult = SetChildren( in_pData, in_ulDataSize );
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}

/** 2012 0x95e410 (agent IH3 fork C: retail's rule - sounds and containers only, at most 0xFFFF children). */
AKRESULT CAkContainerBase::CanAddChild( CAkParameterNodeBase * in_pAudioNode )
{
	const AkNodeCategory eCategory = in_pAudioNode->NodeCategory();
	if( m_mapChildId.Length() == 0xFFFF )
	{
		return AK_MaxReached;
	}
	if( eCategory != AkNodeCategory_Sound && eCategory != AkNodeCategory_FeedbackNode && eCategory != AkNodeCategory_RanSeqCntr
		&& eCategory != AkNodeCategory_SwitchCntr && eCategory != AkNodeCategory_LayerCntr )
	{
		return AK_NotCompatible;
	}
	if( in_pAudioNode->m_pParentNode )
	{
		return AK_ChildAlreadyHasAParent;
	}
	if( m_mapChildId.Exists( in_pAudioNode->key ) )
	{
		return AK_AlreadyConnected;
	}
	if( key == in_pAudioNode->key )
	{
		return AK_CannotAddItseflAsAChild;
	}
	return AK_Success;
}

// ------------------------------------------------------------------------------------------------------------
// CAkRanSeqCntr (akranseqcntr.cpp; the playlists and the playback half are AkRanSeqCntr.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94ddd0. */
CAkRanSeqCntr::CAkRanSeqCntr( AkUniqueID in_ulID, AkContainerMode in_ContainerMode )
	: CAkContainerBase( in_ulID )
	, m_pPlayList( NULL )
	, m_pGlobalContainerInfo( NULL )
	, m_Loop( 1 )
	, m_bContainerBeenPlayed( false )
{
	m_RanSeqParams.m_TransitionTime.m_base = 0.f;
	m_RanSeqParams.m_TransitionTime.m_mod.m_min = 0.f;
	m_RanSeqParams.m_TransitionTime.m_mod.m_max = 0.f;
	m_RanSeqParams.m_wAvoidRepeatCount = 0;
	m_RanSeqParams.m_eTransitionMode = 0;
	m_RanSeqParams.m_eRandomMode = 0;
	m_RanSeqParams.m_eMode = in_ContainerMode;
	m_RanSeqParams.m_bIsUsingWeight = 0;
	m_RanSeqParams.m_bResetPlayListAtEachPlay = 1;
	m_RanSeqParams.m_bIsRestartBackward = 0;
	m_RanSeqParams.m_bIsContinuous = 0;
	m_RanSeqParams.m_bIsGlobal = 1;
}

/** 2012 inlined in the deleting destructor 0x94f420: DestroySpecificInfo, then the playlist's Destroy. */
CAkRanSeqCntr::~CAkRanSeqCntr()
{
	DestroySpecificInfo();
	if( m_pPlayList )
	{
		m_pPlayList->Destroy();
		m_pPlayList = NULL;
	}
}

CAkPlayList * CAkRanSeqCntr::NewPlayList()
{
	if( m_RanSeqParams.m_eMode == ContainerMode_Sequence )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkPlayListSequence ) );
		return pMem ? AkPlacementNew( pMem ) CAkPlayListSequence() : NULL;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkPlayListRandom ) );
	return pMem ? AkPlacementNew( pMem ) CAkPlayListRandom() : NULL;
}

/** 2012 0x94e3b0. */
AKRESULT CAkRanSeqCntr::Init()
{
	AddToIndex();
	m_pPlayList = NewPlayList();
	return m_pPlayList ? AK_Success : AK_Fail;
}

/** 2012 0x94e420. */
CAkRanSeqCntr * CAkRanSeqCntr::Create( AkUniqueID in_ulID, AkContainerMode in_ContainerMode )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRanSeqCntr ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkRanSeqCntr * pCntr = AkPlacementNew( pMem ) CAkRanSeqCntr( in_ulID, in_ContainerMode );
	if( pCntr->Init() != AK_Success )
	{
		pCntr->Release();
		return NULL;
	}
	return pCntr;
}

/** 2012 0x94f180: swap the playlist kind when the mode changes (the selection state is reset first). */
AKRESULT CAkRanSeqCntr::Mode( AkContainerMode in_eMode )
{
	if( in_eMode == (AkContainerMode)m_RanSeqParams.m_eMode )
	{
		return AK_Success;
	}
	m_RanSeqParams.m_eMode = in_eMode;
	if( m_pPlayList )
	{
		m_pPlayList->Destroy();
	}
	m_pPlayList = NewPlayList();
	AKRESULT eResult = AK_Fail;
	if( m_pPlayList )
	{
		eResult = m_pPlayList->Init();
		if( eResult != AK_Success )
		{
			m_pPlayList->Destroy();
			m_pPlayList = NULL;
		}
	}
	ResetSpecificInfo();
	return eResult;
}

/** 2012 0x94f480: {count u16, {id, weight} x count}; weight != 50000 turns on m_bIsUsingWeight. */
AKRESULT CAkRanSeqCntr::SetPlaylistWithoutCheck( AkUInt8 *& io_pData, AkUInt32 & io_ulDataSize )
{
	AKRESULT eResult = AK_Success;
	const AkUInt32 ulPlayListItem = *(AkUInt16 *)io_pData;
	io_pData += 2;
	for( AkUInt32 i = 0; i < ulPlayListItem; ++i )
	{
		const AkUniqueID ulPlayID = *(AkUInt32 *)io_pData;
		io_pData += 4;
		const AkUInt32 weight = *(AkUInt32 *)io_pData;
		io_pData += 4;
		if( weight != 50000 )
		{
			m_RanSeqParams.m_bIsUsingWeight = 1;
		}
		if( m_RanSeqParams.m_eMode == ContainerMode_Sequence || !m_pPlayList->Exists( ulPlayID ) )
		{
			eResult = m_pPlayList->Add( ulPlayID, weight );
		}
		else
		{
			eResult = AK_ElementAlreadyInList;
		}
		if( eResult == AK_IDNotFound )
		{
			eResult = AK_Success;
		}
		else if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	return eResult;
}

/** 2012 0x94f580. */
AKRESULT CAkRanSeqCntr::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 4;
	AKRESULT eResult = SetNodeBaseParams( in_pData, in_ulDataSize, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkInt16 sLoopCount = *(AkInt16 *)in_pData;
	in_pData += 2;
	m_Loop = sLoopCount ? sLoopCount : 0;
	RANGED_PARAMETER<AkReal32> transitionTime;
	transitionTime.m_base = *(AkReal32 *)in_pData;
	in_pData += 4;
	transitionTime.m_mod.m_min = *(AkReal32 *)in_pData;
	in_pData += 4;
	transitionTime.m_mod.m_max = *(AkReal32 *)in_pData;
	in_pData += 4;
	const AkUInt16 wAvoidRepeatCount = *(AkUInt16 *)in_pData;
	in_pData += 2;
	const AkUInt8 eTransitionMode = *in_pData++;
	const AkUInt8 eRandomMode = *in_pData++;
	const AkUInt8 eMode = *in_pData++;
	const AkUInt8 bIsUsingWeight = *in_pData++;
	const AkUInt8 bResetPlayListAtEachPlay = *in_pData++;
	const AkUInt8 bIsRestartBackward = *in_pData++;
	const AkUInt8 bIsContinuous = *in_pData++;
	const AkUInt8 bIsGlobal = *in_pData++;

	eResult = Mode( (AkContainerMode)eMode );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_RanSeqParams.m_TransitionTime = transitionTime;
	m_RanSeqParams.m_wAvoidRepeatCount = wAvoidRepeatCount;
	m_RanSeqParams.m_eTransitionMode = eTransitionMode;
	m_RanSeqParams.m_eRandomMode = eRandomMode;
	m_RanSeqParams.m_bIsUsingWeight = bIsUsingWeight != 0;
	m_RanSeqParams.m_bResetPlayListAtEachPlay = bResetPlayListAtEachPlay != 0;
	m_RanSeqParams.m_bIsRestartBackward = bIsRestartBackward != 0;
	m_RanSeqParams.m_bIsContinuous = bIsContinuous != 0;
	m_RanSeqParams.m_bIsGlobal = bIsGlobal != 0;

	eResult = SetChildren( in_pData, in_ulDataSize );
	if( eResult == AK_Success )
	{
		eResult = SetPlaylistWithoutCheck( in_pData, in_ulDataSize );
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkSwitchCntr (akswitchcntr.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94fac0. */
CAkSwitchCntr::CAkSwitchCntr( AkUniqueID in_ulID )
	: CAkMultiPlayNode( in_ulID )
	, m_eGroupType( AkGroupType_Switch )
	, m_ulGroupID( 0 )
	, m_ulDefaultSwitch( 0 )
{
}

/** 2012 0x951480. */
CAkSwitchCntr::~CAkSwitchCntr()
{
	UnsubscribeSwitches();
	ClearSwitches();
	m_listSwitchContPlayback.Term();
	m_listParameters.Term();
	m_SwitchList.Term();
	Term();
}

/** 2012 0x950d50. */
AKRESULT CAkSwitchCntr::Init()
{
	const AKRESULT eResult = CAkMultiPlayNode::Init();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_SwitchList.InitUnbounded();
	m_listParameters.InitUnbounded();
	m_listSwitchContPlayback.InitUnbounded();
	return AK_Success;
}

/** 2012 0x950de0. */
CAkSwitchCntr * CAkSwitchCntr::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSwitchCntr ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSwitchCntr * pCntr = AkPlacementNew( pMem ) CAkSwitchCntr( in_ulID );
	if( pCntr->Init() != AK_Success )
	{
		pCntr->Release();
		return NULL;
	}
	return pCntr;
}

/** 2012 0x950170. */
AKRESULT CAkSwitchCntr::AddNodeInSwitch( CAkSwitchPackage * in_pSwitchPack, AkUniqueID in_NodeID )
{
	if( in_pSwitchPack->m_list.Exists( in_NodeID ) )
	{
		return AK_Success;
	}
	return in_pSwitchPack->m_list.AddLast( in_NodeID ) ? AK_Success : AK_Fail;
}

/** 2012 0x951020: SetNodeBaseParams' result is ignored, as retail does. */
AKRESULT CAkSwitchCntr::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 4;
	SetNodeBaseParams( in_pData, in_ulDataSize, false );
	const AkGroupType eGroupType = (AkGroupType)*(AkUInt32 *)in_pData;
	in_pData += 4;
	const AkUInt32 ulGroupID = *(AkUInt32 *)in_pData;
	in_pData += 4;
	const AkUInt32 ulDefaultSwitch = *(AkUInt32 *)in_pData;
	in_pData += 4;
	const bool bIsContinuousValidation = *in_pData++ != 0;
	AKRESULT eResult = AK_Success;
	if( ulGroupID != m_ulGroupID || eGroupType != m_eGroupType )
	{
		m_ulGroupID = ulGroupID;
		m_eGroupType = eGroupType;
		eResult = SubscribeSwitch( ulGroupID, eGroupType );
	}
	if( eResult != AK_Success )
	{
		g_pHircParseEnd = in_pData;
		return eResult;
	}
	m_bIsContinuousValidation = bIsContinuousValidation;
	m_ulDefaultSwitch = ulDefaultSwitch;

	eResult = SetChildren( in_pData, in_ulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}

	const AkUInt32 ulNumSwitchGroups = *(AkUInt32 *)in_pData;
	in_pData += 4;
	m_SwitchList.Reserve( ulNumSwitchGroups );
	for( AkUInt32 i = 0; i < ulNumSwitchGroups; ++i )
	{
		const AkUInt32 ulSwitchID = *(AkUInt32 *)in_pData;
		in_pData += 4;
		CAkSwitchPackage * pSwitchNodeList = m_SwitchList.Set( ulSwitchID );
		if( !pSwitchNodeList )
		{
			return AK_Fail;
		}
		const AkUInt32 ulNumItems = *(AkUInt32 *)in_pData;
		in_pData += 4;
		if( ulNumItems )
		{
			pSwitchNodeList->m_list.Reserve( ulNumItems );
		}
		for( AkUInt32 j = 0; j < ulNumItems; ++j )
		{
			const AkUniqueID ulNodeID = *(AkUInt32 *)in_pData;
			in_pData += 4;
			eResult = AddNodeInSwitch( pSwitchNodeList, ulNodeID );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}

	const AkUInt32 ulNumSwitchParams = *(AkUInt32 *)in_pData;
	in_pData += 4;
	m_listParameters.Reserve( ulNumSwitchParams );
	for( AkUInt32 i = 0; i < ulNumSwitchParams; ++i )
	{
		const AkUniqueID ulNodeID = *(AkUInt32 *)in_pData;
		in_pData += 4;
		AkSwitchNodeParams params;
		params.bIsFirstOnly = *in_pData++ != 0;
		params.bContinuePlayback = *in_pData++ != 0;
		params.eOnSwitchMode = *(AkUInt32 *)in_pData & 7;
		in_pData += 4;
		params.FadeOutTime = *(AkInt32 *)in_pData;
		in_pData += 4;
		params.FadeInTime = *(AkInt32 *)in_pData;
		in_pData += 4;
		if( !m_listParameters.Set( ulNodeID, params ) )
		{
			return AK_Fail;
		}
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkLayer / CAkLayerCntr (aklayer.cpp, aklayercntr.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x954a50 (+ Init 0x9547e0: the layers index, g_pIndex + 3200). */
CAkLayer * CAkLayer::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkLayer ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkLayer * pLayer = AkPlacementNew( pMem ) CAkLayer( in_ulID );
	g_pIndex->m_idxLayers.SetIDToPtr( pLayer );
	return pLayer;
}

AkUInt32 CAkLayer::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxLayers.m_IndexLock );
	return ++m_lRef;
}

AkUInt32 CAkLayer::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxLayers.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		g_pIndex->m_idxLayers.RemoveID( key );
		this->~CAkLayer();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x954150: the layer's own RTPC chunk, same shape as a node's, subscriber type CAkLayer. */
AKRESULT CAkLayer::SetInitialRTPC( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	const AkUInt16 uNumCurves = *(AkUInt16 *)io_rpData;
	io_rpData += 2;
	for( AkUInt32 i = 0; i < uNumCurves; ++i )
	{
		const AkRtpcID RTPC_ID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const AkUInt32 ParamID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const AkUniqueID RTPCCurveID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		const AkCurveScaling eScaling = (AkCurveScaling)*io_rpData++;
		const AkUInt32 ulSize = *(AkUInt16 *)io_rpData;
		io_rpData += 2;
		m_RTPCBitArrayMax32.SetBit( ParamID );
		g_pRTPCMgr->SubscribeRTPC( this, RTPC_ID, (AkRTPC_ParameterID)ParamID, RTPCCurveID, eScaling, (AkRTPCGraphPoint *)io_rpData, ulSize, NULL, CAkRTPCMgr::SubscriberType_CAkLayer );
		io_rpData += sizeof( AkRTPCGraphPoint ) * ulSize;
		io_rulDataSize -= sizeof( AkRTPCGraphPoint ) * ulSize;
	}
	return AK_Success;
}

/** 2012 0x954200. */
AKRESULT CAkLayer::SetCrossfadingRTPC( AkRtpcID in_rtpcID )
{
	AKRESULT eResult = AK_Success;
	if( m_crossfadingRTPCID != in_rtpcID )
	{
		if( m_crossfadingRTPCID )
		{
			g_pRTPCMgr->UnregisterLayer( this, m_crossfadingRTPCID );
		}
		m_crossfadingRTPCID = in_rtpcID;
		if( in_rtpcID )
		{
			eResult = g_pRTPCMgr->RegisterLayer( this, in_rtpcID );
			if( eResult != AK_Success )
			{
				m_crossfadingRTPCID = 0;
			}
		}
	}
	return eResult;
}

/** 2012 0x954c10: skips the layer id (the caller created the layer with it). */
AKRESULT CAkLayer::SetInitialValues( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	io_rpData += 4;
	AKRESULT eResult = SetInitialRTPC( io_rpData, io_rulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkRtpcID rtpcID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	eResult = SetCrossfadingRTPC( rtpcID );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkUInt32 ulNumAssoc = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( ulNumAssoc )
	{
		eResult = m_assocs.Reserve( ulNumAssoc );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		for( AkUInt32 i = 0; i < ulNumAssoc; ++i )
		{
			const AkUniqueID ulAssociatedChildID = *(AkUInt32 *)io_rpData;
			io_rpData += 4;
			const AkUInt32 ulCurveSize = *(AkUInt32 *)io_rpData;
			io_rpData += 4;
			eResult = SetChildAssoc( ulAssociatedChildID, (AkRTPCGraphPoint *)io_rpData, ulCurveSize );
			if( eResult != AK_Success )
			{
				break;
			}
			io_rpData += sizeof( AkRTPCGraphPoint ) * ulCurveSize;
			io_rulDataSize -= sizeof( AkRTPCGraphPoint ) * ulCurveSize;
		}
	}
	return eResult;
}

/** 2012 0x951500. */
CAkLayerCntr * CAkLayerCntr::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkLayerCntr ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkLayerCntr * pCntr = AkPlacementNew( pMem ) CAkLayerCntr( in_ulID );
	pCntr->AddToIndex();
	return pCntr;
}

/** 2012 0x951560. */
CAkLayerCntr::~CAkLayerCntr()
{
	for( CAkLayer ** it = m_layers.m_pItems; it != m_layers.m_pItemsEnd; ++it )
	{
		( *it )->SetOwner( NULL );
		( *it )->Release();
	}
	m_layers.Term();
	Term();
}

/** 2012 0x9517a0. */
AKRESULT CAkLayerCntr::AddChild( AkUniqueID in_ulID )
{
	const AKRESULT eResult = CAkParentNode<CAkParameterNode>::AddChild( in_ulID );
	if( eResult == AK_Success )
	{
		for( CAkLayer ** it = m_layers.m_pItems; it != m_layers.m_pItemsEnd; ++it )
		{
			( *it )->UpdateChildPtr( in_ulID );
		}
	}
	return eResult;
}

/** 2012 0x9515f0. */
AKRESULT CAkLayerCntr::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 4;
	AKRESULT eResult = SetNodeBaseParams( in_pData, in_ulDataSize, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	eResult = SetChildren( in_pData, in_ulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkUInt32 ulNumLayers = *(AkUInt32 *)in_pData;
	in_pData += 4;
	for( AkUInt32 i = 0; i < ulNumLayers; ++i )
	{
		CAkLayer * pLayer = CAkLayer::Create( *(AkUInt32 *)in_pData );
		if( !pLayer )
		{
			return AK_Fail;
		}
		pLayer->SetOwner( this );
		eResult = pLayer->SetInitialValues( in_pData, in_ulDataSize );
		if( eResult != AK_Success )
		{
			pLayer->Release();
			return eResult;
		}
		if( !m_layers.AddLast( pLayer ) )
		{
			pLayer->Release();
			return AK_Fail;
		}
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkBus (akbus.cpp)
// ------------------------------------------------------------------------------------------------------------

CAkBus * CAkBus::s_pMasterBus = NULL;

/** 2012 0x93cea0. */
CAkBus::CAkBus( AkUniqueID in_ulID )
	: CAkParentNode<CAkParameterNodeBase>( in_ulID )
	, m_RecoveryTime( 0 )
	, m_uChannelConfig( 63 )
	, m_fMaxDuckVolume( -96.300003f )
	, m_eDuckingState( 0 )
	, m_bIsEnvironmental( 0 )
{
}

CAkBus::~CAkBus()
{
	for( CAkKeyList<AkUniqueID, CAkDuckItem, 1, ArrayPoolDefault>::ListItem * p = m_DuckedVolumeList.m_pFirst; p; p = p->pNextListItem )
	{
		p->Item.item.Term();
	}
	m_DuckedVolumeList.Term();
	m_ToDuckList.Term();
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAudioNode.m_IndexLock );
	if( s_pMasterBus == this )
	{
		s_pMasterBus = NULL;
		g_MasterBusCtx.m_pBus = NULL;
	}
}

/** 2012 0x93cf50 (+ Init 0x93cf10): the first bus created becomes the master bus. */
CAkBus * CAkBus::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkBus ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkBus * pBus = AkPlacementNew( pMem ) CAkBus( in_ulID );
	pBus->AddToIndex();
	pBus->m_ToDuckList.Init( 0, 100 );
	pBus->m_DuckedVolumeList.Init( 0, 100 );
	if( !s_pMasterBus )
	{
		s_pMasterBus = pBus;
		g_MasterBusCtx.m_pBus = pBus;
	}
	return pBus;
}

/** 2012 0x93c150. */
void CAkBus::ClearMasterBus()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAudioNode.m_IndexLock );
	s_pMasterBus = NULL;
}

/** 2012 0x93d320. */
AKRESULT CAkBus::AddChild( AkUniqueID in_ulID )
{
	if( !in_ulID )
	{
		return AK_InvalidID;
	}
	CAkParameterNodeBase * pAudioNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_ulID );
	if( !pAudioNode )
	{
		return AK_IDNotFound;
	}
	AKRESULT eResult = CanAddChild( pAudioNode );
	if( eResult == AK_Success )
	{
		CAkParameterNodeBase ** ppNode = m_mapChildId.AddNoSetKey( in_ulID );
		if( !ppNode )
		{
			pAudioNode->Release();
			return AK_Fail;
		}
		*ppNode = pAudioNode;
		pAudioNode->ParentBus( this );
		AddRef();
	}
	pAudioNode->Release();
	return eResult;
}

/** 2012 0x93c240. */
void CAkBus::RemoveChild( CAkParameterNodeBase * in_pChild )
{
	if( in_pChild->m_pBusOutputNode != this )
	{
		return;
	}
	in_pChild->ParentBus( NULL );
	m_mapChildId.Unset( in_pChild->key );
	Release();
}

/** 2012 0x93c1e0. */
AKRESULT CAkBus::CanAddChild( CAkParameterNodeBase * in_pAudioNode )
{
	if( in_pAudioNode->m_pBusOutputNode )
	{
		return AK_ChildAlreadyHasAParent;
	}
	if( m_mapChildId.Exists( in_pAudioNode->key ) )
	{
		return AK_AlreadyConnected;
	}
	if( key == in_pAudioNode->key )
	{
		return AK_CannotAddItseflAsAChild;
	}
	return AK_Success;
}

/** 2012 0x93c380: {props, kill newest, use virtual, max instances, max inst override, channel config u16, (2 skipped), environmental}. */
AKRESULT CAkBus::SetInitialParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	AKRESULT eResult = reinterpret_cast< AkPropBundle<AkReal32> & >( m_props ).SetInitialParams( io_rpData, io_rulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const bool bKillNewest = *io_rpData++ != 0;
	const bool bUseVirtualBehavior = *io_rpData++ != 0;
	SetMaxReachedBehavior( bKillNewest );
	SetOverLimitBehavior( bUseVirtualBehavior );
	m_u16MaxNumInstance = *(AkUInt16 *)io_rpData;
	io_rpData += 2;
	m_bIsMaxNumInstOverrideParent = *io_rpData++ & 1;
	const AkUInt16 uChannelConfig = *(AkUInt16 *)io_rpData;
	io_rpData += 2;
	m_bUseState = 1;
	m_uChannelConfig = uChannelConfig;
	io_rpData += 2;
	m_bIsEnvironmental = *io_rpData++ != 0;
	return AK_Success;
}

/** 2012 0x93c050: {count, [bypass bits, {index, id, is share set, (1 skipped)} x count]}. */
AKRESULT CAkBus::SetInitialFxParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize, bool )
{
	AKRESULT eResult = AK_Success;
	const AkUInt32 uNumFx = *io_rpData++;
	if( uNumFx || m_bIsEnvironmental )
	{
		const AkUInt32 bitsFXBypass = *io_rpData++;
		if( m_bIsEnvironmental )
		{
			g_pEnvironmentMgr->m_bEnvironmentalBypass = ( bitsFXBypass & 0x11 ) != 0;
		}
		for( AkUInt32 uFX = 0; uFX < uNumFx; ++uFX )
		{
			const AkUInt32 uFXIndex = *io_rpData++;
			const AkUniqueID fxID = *(AkUInt32 *)io_rpData;
			io_rpData += 4;
			const bool bIsShareSet = *io_rpData != 0;
			io_rpData += 2;
			if( fxID )
			{
				eResult = SetFX( uFXIndex, fxID, bIsShareSet );
				if( eResult != AK_Success )
				{
					break;
				}
			}
		}
		MainBypassFX( bitsFXBypass, 0xFFFFFFFF );
	}
	return eResult;
}

/** CAkBus::AddDuck: one entry of the to-duck list. */
AKRESULT CAkBus::AddDuck( AkUniqueID in_BusID, AkReal32 in_DuckVolume, AkTimeMs in_FadeOutTime, AkTimeMs in_FadeInTime, AkCurveInterpolation in_eFadeCurve )
{
	AkDuckInfo info;
	info.DuckVolume = in_DuckVolume;
	info.FadeOutTime = in_FadeOutTime;
	info.FadeInTime = in_FadeInTime;
	info.FadeCurve = in_eFadeCurve;
	return m_ToDuckList.Set( in_BusID, info ) ? AK_Success : AK_Fail;
}

/** 2012 0x93d520: parent bus first, then params, recovery time, max duck volume, ducks, FX, RTPC, states, feedback. */
AKRESULT CAkBus::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 4;
	const AkUniqueID ParentBusID = *(AkUInt32 *)in_pData;
	in_pData += 4;
	if( ParentBusID )
	{
		CAkParameterNodeBase * pParentBus = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( ParentBusID );
		if( !pParentBus )
		{
			return AK_Fail;
		}
		const AKRESULT eResult = pParentBus->AddChild( key );
		pParentBus->Release();
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	AKRESULT eResult = SetInitialParams( in_pData, in_ulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_RecoveryTime = *(AkUInt32 *)in_pData * ( AkAudioLibSettings::g_pipelineCoreFrequency / 1000 );
	in_pData += 4;
	m_fMaxDuckVolume = *(AkReal32 *)in_pData;
	in_pData += 4;
	const AkUInt32 ulDucks = *(AkUInt32 *)in_pData;
	in_pData += 4;
	for( AkUInt32 i = 0; i < ulDucks; ++i )
	{
		const AkUniqueID BusID = *(AkUInt32 *)in_pData;
		in_pData += 4;
		const AkReal32 DuckVolume = *(AkReal32 *)in_pData;
		in_pData += 4;
		const AkTimeMs FadeOutTime = *(AkInt32 *)in_pData;
		in_pData += 4;
		const AkTimeMs FadeInTime = *(AkInt32 *)in_pData;
		in_pData += 4;
		const AkCurveInterpolation eFadeCurve = (AkCurveInterpolation)*in_pData++;
		eResult = AddDuck( BusID, DuckVolume, FadeOutTime, FadeInTime, eFadeCurve );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	eResult = SetInitialFxParams( in_pData, in_ulDataSize, false );
	if( eResult == AK_Success )
	{
		eResult = SetInitialRTPC( in_pData, in_ulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = ReadStateChunk( in_pData, in_ulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = ReadFeedbackInfo( in_pData, in_ulDataSize );
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkState (akstate.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x952f30 (+ Init 0x952ee0: the custom states index, g_pIndex + 800). */
CAkState * CAkState::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkState ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkState * pState = AkPlacementNew( pMem ) CAkState( in_ulID );
	g_pIndex->m_idxCustomStates.SetIDToPtr( pState );
	return pState;
}

AkUInt32 CAkState::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxCustomStates.m_IndexLock );
	return ++m_lRef;
}

AkUInt32 CAkState::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxCustomStates.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		g_pIndex->m_idxCustomStates.RemoveID( key );
		this->~CAkState();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x952de0. */
AKRESULT CAkState::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 4;
	const AKRESULT eResult = m_props.SetInitialParams( in_pData, in_ulDataSize );
	g_pHircParseEnd = in_pData;
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkAttenuation (akattenuationmgr.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x951b70 + Init 0x952020 (curveToUse all 0xFF). */
CAkAttenuation::CAkAttenuation( AkUniqueID in_ulID )
	: CAkIndexable( in_ulID )
	, m_bIsConeEnabled( 0 )
{
	memset( &m_ConeParams, 0, sizeof( m_ConeParams ) );
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_curveToUse[ i ] = AttenuationCurveID_None;
	}
}

/** 2012 0x951ef0. */
CAkAttenuation::~CAkAttenuation()
{
	ClearRTPCs();
	m_rtpcsubs.Term();
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_curves[ i ].Unset();
	}
}

/** 2012 0x952090. */
CAkAttenuation * CAkAttenuation::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkAttenuation ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkAttenuation * pAttenuation = AkPlacementNew( pMem ) CAkAttenuation( in_ulID );
	g_pIndex->m_idxAttenuations.SetIDToPtr( pAttenuation );
	return pAttenuation;
}

/** 2012 0x951bc0. */
AkUInt32 CAkAttenuation::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAttenuations.m_IndexLock );
	return ++m_lRef;
}

/** 2012 0x951f70. */
AkUInt32 CAkAttenuation::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxAttenuations.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		g_pIndex->m_idxAttenuations.RemoveID( key );
		this->~CAkAttenuation();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x934c90 (agent IH3): the curve m_curveToUse names for an attenuation parameter (AttenuationCurveID_*);
    0xFF is "none". */
CAkConversionTable<AkRTPCGraphPoint, AkReal32> * CAkAttenuation::GetCurve( AkUInt8 in_parameterIndex )
{
	const AkUInt8 uCurve = m_curveToUse[ in_parameterIndex ];
	return uCurve == AttenuationCurveID_None ? NULL : &m_curves[ uCurve ];
}

/** 2012 0x951c90 (agent IH3). */
void CAkAttenuation::ClearRTPCs()
{
	for( RTPCSubs * p = m_rtpcsubs.m_pItems; p != m_rtpcsubs.m_pItemsEnd; ++p )
	{
		p->ConversionTable.Unset();
	}
	m_rtpcsubs.m_pItemsEnd = m_rtpcsubs.m_pItems;
}

/** 2012 0x951bf0. */
void CAkAttenuation::UnsetRTPC( AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID )
{
	for( RTPCSubs * p = m_rtpcsubs.m_pItems; p != m_rtpcsubs.m_pItemsEnd; ++p )
	{
		if( p->ParamID == in_ParamID && p->RTPCCurveID == in_RTPCCurveID )
		{
			p->ConversionTable.Unset();
			m_rtpcsubs.Erase( (AkUInt32)( p - m_rtpcsubs.m_pItems ) );
			return;
		}
	}
}

/** 2012 0x951ce0: {cone on, [cone x4 (angles in degrees, halved to radians)], curveToUse x4, curves, RTPCs}. */
AKRESULT CAkAttenuation::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	AkUInt8 * pData = in_pData + 4;
	m_bIsConeEnabled = *pData++ != 0;
	if( m_bIsConeEnabled )
	{
		const AkReal32 kDegToRad = 3.1415927f / 180.f;
		m_ConeParams.fInsideAngle = ( *(AkReal32 *)pData * kDegToRad ) * 0.5f;
		pData += 4;
		m_ConeParams.fOutsideAngle = ( *(AkReal32 *)pData * kDegToRad ) * 0.5f;
		pData += 4;
		m_ConeParams.fOutsideVolume = *(AkReal32 *)pData;
		pData += 4;
		m_ConeParams.LoPass = *(AkReal32 *)pData;
		pData += 4;
	}
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_curveToUse[ i ] = *pData++;
	}
	const AkUInt32 NumCurves = *pData++;
	AKRESULT eResult = AK_Fail;
	for( AkUInt32 i = 0; i < NumCurves; ++i )
	{
		const AkCurveScaling eScaling = (AkCurveScaling)*pData++;
		const AkUInt32 ulSize = *(AkUInt16 *)pData;
		pData += 2;
		eResult = m_curves[ i ].Set( (AkRTPCGraphPoint *)pData, ulSize, eScaling );
		if( eResult != AK_Success )
		{
			break;
		}
		pData += sizeof( AkRTPCGraphPoint ) * ulSize;
	}
	const AkUInt32 ulNumRTPC = *(AkUInt16 *)pData;
	pData += 2;
	for( AkUInt32 i = 0; i < ulNumRTPC; ++i )
	{
		const AkRtpcID RTPCID = *(AkUInt32 *)pData;
		pData += 4;
		const AkUInt32 ParamID = *(AkUInt32 *)pData;
		pData += 4;
		const AkUniqueID RTPCCurveID = *(AkUInt32 *)pData;
		pData += 4;
		const AkCurveScaling eScaling = (AkCurveScaling)*pData++;
		const AkUInt32 ulSize = *(AkUInt16 *)pData;
		pData += 2;
		AkRTPCGraphPoint * pPoints = (AkRTPCGraphPoint *)pData;
		UnsetRTPC( ParamID, RTPCCurveID );
		eResult = AK_Success;
		RTPCSubs * pSubs = m_rtpcsubs.AddLast();
		if( pSubs )
		{
			AkPlacementNew( &pSubs->ConversionTable ) CAkConversionTable<AkRTPCGraphPoint, AkReal32>();
			pSubs->ParamID = ParamID;
			pSubs->RTPCCurveID = RTPCCurveID;
			pSubs->RTPCID = RTPCID;
			if( ulSize )
			{
				eResult = pSubs->ConversionTable.Set( pPoints, ulSize, eScaling );
			}
		}
		else
		{
			eResult = AK_Fail;
		}
		if( eResult != AK_Success )
		{
			return eResult;
		}
		pData += sizeof( AkRTPCGraphPoint ) * ulSize;
	}
	g_pHircParseEnd = pData;
	return ulNumRTPC ? AK_Success : eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkFxBase / CAkFxShareSet / CAkFxCustom (akfxbase.cpp)
// ------------------------------------------------------------------------------------------------------------

CAkFxBase::~CAkFxBase()
{
	m_media.Term();
	for( RTPCSubs * p = m_rtpcsubs.m_pItems; p != m_rtpcsubs.m_pItemsEnd; ++p )
	{
		p->ConversionTable.Unset();
	}
	m_rtpcsubs.Term();
}

AkUInt32 CAkFxBase::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( Index().m_IndexLock );
	return ++m_lRef;
}

AkUInt32 CAkFxBase::Release()
{
	CAkIndexItemBase & index = Index();
	AkAutoLock<CAkLock> IndexLock( index.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		index.RemoveID( key );
		this->~CAkFxBase();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x94ab90: the plug-in's param object comes from CAkEffectsMgr::AllocParams; see the header. */
AKRESULT CAkFxBase::SetFX( AkPluginID in_FXID, void * in_pParamBlock, AkUInt32 in_uParamBlockSize )
{
	return AK_Success;
}

/** 2012 0x94ac40. */
void CAkFxBase::UnsetRTPC( AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID )
{
	for( RTPCSubs * p = m_rtpcsubs.m_pItems; p != m_rtpcsubs.m_pItemsEnd; ++p )
	{
		if( p->ParamID == in_ParamID && p->RTPCCurveID == in_RTPCCurveID )
		{
			p->ConversionTable.Unset();
			m_rtpcsubs.Erase( (AkUInt32)( p - m_rtpcsubs.m_pItems ) );
			return;
		}
	}
}

/** 2012 0x94aef0: {plug-in id, block size, block, media count, {index u8, media id} x n, RTPC count u16, RTPCs}. */
AKRESULT CAkFxBase::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	const AkPluginID fxID = *(AkUInt32 *)( in_pData + 4 );
	const AkUInt32 uSize = *(AkUInt32 *)( in_pData + 8 );
	AkUInt8 * pData = in_pData + 12;
	AKRESULT eResult = AK_Success;
	if( fxID != AK_INVALID_PLUGINID )
	{
		eResult = SetFX( fxID, pData, uSize );
	}
	pData += uSize;
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkUInt32 uNumBankData = *pData++;
	if( uNumBankData )
	{
		m_media.Reserve( uNumBankData );
		for( AkUInt32 i = 0; i < uNumBankData; ++i )
		{
			const AkUInt32 uIndex = *pData++;
			const AkUniqueID mediaID = *(AkUInt32 *)pData;
			pData += 4;
			m_media.Set( uIndex, mediaID );
		}
	}
	const AkUInt32 ulNumRTPC = *(AkUInt16 *)pData;
	pData += 2;
	for( AkUInt32 i = 0; i < ulNumRTPC; ++i )
	{
		const AkRtpcID RTPCID = *(AkUInt32 *)pData;
		pData += 4;
		const AkUInt32 ParamID = *(AkUInt32 *)pData;
		pData += 4;
		const AkUniqueID RTPCCurveID = *(AkUInt32 *)pData;
		pData += 4;
		const AkCurveScaling eScaling = (AkCurveScaling)*pData++;
		const AkUInt32 ulSize = *(AkUInt16 *)pData;
		pData += 2;
		AkRTPCGraphPoint * pPoints = (AkRTPCGraphPoint *)pData;
		UnsetRTPC( ParamID, RTPCCurveID );
		RTPCSubs * pSubs = m_rtpcsubs.AddLast();
		if( pSubs )
		{
			AkPlacementNew( &pSubs->ConversionTable ) CAkConversionTable<AkRTPCGraphPoint, AkReal32>();
			pSubs->RTPCCurveID = RTPCCurveID;
			pSubs->RTPCID = RTPCID;
			pSubs->ParamID = ParamID;
			if( ulSize )
			{
				pSubs->ConversionTable.Set( pPoints, ulSize, eScaling );
			}
		}
		pData += sizeof( AkRTPCGraphPoint ) * ulSize;
	}
	g_pHircParseEnd = pData;
	return AK_Success;
}

/** 2012 0x94b120. */
CAkFxShareSet::CAkFxShareSet( AkUniqueID in_ulID )
	: CAkFxBase( in_ulID )
	, m_bBypassed( false )
{
	m_sVolumes.fFrontLeft = 1.f;
	m_sVolumes.fFrontRight = 1.f;
	m_sVolumes.fCenter = 1.f;
	m_sVolumes.fRearLeft = 1.f;
	m_sVolumes.fRearRight = 1.f;
	m_sVolumes.fLfe = 1.f;
}

CAkIndexItemBase & CAkFxShareSet::Index()
{
	return g_pIndex->m_idxFxShareSets;
}

/** 2012 0x94b240 (g_pIndex + 6400). */
CAkFxShareSet * CAkFxShareSet::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkFxShareSet ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkFxShareSet * pFx = AkPlacementNew( pMem ) CAkFxShareSet( in_ulID );
	g_pIndex->m_idxFxShareSets.SetIDToPtr( pFx );
	return pFx;
}

CAkIndexItemBase & CAkFxCustom::Index()
{
	return g_pIndex->m_idxFxCustom;
}

/** 2012 0x94b2d0 (g_pIndex + 7200). */
CAkFxCustom * CAkFxCustom::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkFxCustom ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkFxCustom * pFx = AkPlacementNew( pMem ) CAkFxCustom( in_ulID );
	g_pIndex->m_idxFxCustom.SetIDToPtr( pFx );
	return pFx;
}

// ------------------------------------------------------------------------------------------------------------
// CAkAction and subclasses (akaction*.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x949ea0: m_eFadeCurve 4 (linear), m_bWasLoadedFromBank 0. */
CAkAction::CAkAction( AkUInt32 in_eActionType, AkUniqueID in_ulID )
	: CAkIndexable( in_ulID )
	, pNextLightItem( NULL )
	, m_ulElementID( 0 )
	, m_eActionType( (AkUInt16)in_eActionType )
	, m_eFadeCurve( 4 )
	, m_bWasLoadedFromBank( 0 )
{
}

CAkAction::~CAkAction()
{
	m_props.RemoveAll();
	m_ranges.RemoveAll();
}

AkUInt32 CAkAction::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxActions.m_IndexLock );
	return ++m_lRef;
}

AkUInt32 CAkAction::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxActions.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		if( key )
		{
			g_pIndex->m_idxActions.RemoveID( key );
		}
		this->~CAkAction();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x94a0f0 (g_pIndex + 2400). */
void CAkAction::AddToIndex()
{
	if( key )
	{
		g_pIndex->m_idxActions.SetIDToPtr( this );
	}
}

template <class T>
static CAkAction * NewAction( AkUInt32 in_eActionType, AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( T ) );
	if( !pMem )
	{
		return NULL;
	}
	T * pAction = AkPlacementNew( pMem ) T( in_eActionType, in_ulID );
	pAction->AddToIndex();
	return pAction;
}

static CAkAction * NewSetAkProp( AkUInt32 in_eActionType, AkUniqueID in_ulID, AkPropID in_ePropID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkActionSetAkProp ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkActionSetAkProp * pAction = AkPlacementNew( pMem ) CAkActionSetAkProp( in_eActionType, in_ulID, in_ePropID );
	pAction->AddToIndex();
	return pAction;
}

/** 2012 0x949ce0: dispatch on the high byte of the action type. */
CAkAction * CAkAction::Create( AkUInt32 in_eActionType, AkUniqueID in_ulID )
{
	switch( in_eActionType & 0xFF00 )
	{
	case 0x0100: return NewAction<CAkActionStop>( in_eActionType, in_ulID );
	case 0x0200: return NewAction<CAkActionPause>( in_eActionType, in_ulID );
	case 0x0300: return NewAction<CAkActionResume>( in_eActionType, in_ulID );
	case 0x0400: return NewAction<CAkActionPlay>( in_eActionType, in_ulID );
	case 0x0600:
	case 0x0700: return NewAction<CAkActionMute>( in_eActionType, in_ulID );
	case 0x0800:
	case 0x0900: return NewSetAkProp( in_eActionType, in_ulID, AkPropID_Pitch );
	case 0x0A00:
	case 0x0B00: return NewSetAkProp( in_eActionType, in_ulID, AkPropID_Volume );
	case 0x0E00:
	case 0x0F00: return NewSetAkProp( in_eActionType, in_ulID, AkPropID_LPF );
	case 0x1000:
	case 0x1100: return NewAction<CAkActionUseState>( in_eActionType, in_ulID );
	case 0x1200: return NewAction<CAkActionSetState>( in_eActionType, in_ulID );
	case 0x1300:
	case 0x1400: return NewAction<CAkActionSetGameParameter>( in_eActionType, in_ulID );
	case 0x1900: return NewAction<CAkActionSetSwitch>( in_eActionType, in_ulID );
	case 0x1A00:
	case 0x1B00: return NewAction<CAkActionBypassFX>( in_eActionType, in_ulID );
	case 0x1C00: return NewAction<CAkActionBreak>( in_eActionType, in_ulID );
	case 0x1D00: return NewAction<CAkActionTrigger>( in_eActionType, in_ulID );
	case 0x1E00: return NewAction<CAkActionSeek>( in_eActionType, in_ulID );
	default: return NULL;
	}
}

/** 2012 0x949f30: {type u16 (read by the caller), element id, props, ranged props, type params}; delay ms -> samples. */
AKRESULT CAkAction::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	in_pData += 6;
	const AkUniqueID ulElementID = *(AkUInt32 *)in_pData;
	in_pData += 4;
	SetElementID( ulElementID );
	m_props.RemoveAll();
	m_ranges.RemoveAll();
	AKRESULT eResult = reinterpret_cast< AkPropBundle<AkReal32> & >( m_props ).SetInitialParams( in_pData, in_ulDataSize );
	if( eResult == AK_Success )
	{
		eResult = m_ranges.SetInitialParams( in_pData, in_ulDataSize );
	}
	if( eResult == AK_Success )
	{
		eResult = SetActionParams( in_pData, in_ulDataSize );
	}
	g_pHircParseEnd = in_pData;
	const AkUInt32 uSamplesPerMs = AkAudioLibSettings::g_pipelineCoreFrequency / 1000;
	AkPropValue * pDelay = m_props.FindProp( AkPropID_DelayTime );
	if( pDelay )
	{
		pDelay->iValue *= uSamplesPerMs;
	}
	RANGED_MODIFIERS<AkPropValue> * pDelayRange = m_ranges.FindProp( AkPropID_DelayTime );
	if( pDelayRange )
	{
		pDelayRange->m_min.iValue *= uSamplesPerMs;
		pDelayRange->m_max.iValue *= uSamplesPerMs;
	}
	return eResult;
}

/** 2012 0x95c240: {fade curve, bank id}. */
AKRESULT CAkActionPlay::SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_eFadeCurve = *io_rpData++ & 0x1F;
	m_fileID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	m_bWasLoadedFromBank = 1;
	return AK_Success;
}

/** 2012 0x95ddc0: {count, ids}. */
AKRESULT CAkActionExcept::SetExceptParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	const AkUInt32 ulExceptionListSize = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	if( !ulExceptionListSize )
	{
		return AK_Success;
	}
	if( m_listElementException.Reserve( ulExceptionListSize ) != AK_Success )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = 0; i < ulExceptionListSize; ++i )
	{
		const AkUniqueID ulID = *(AkUInt32 *)io_rpData;
		io_rpData += 4;
		m_listElementException.AddLast( ulID );
	}
	return AK_Success;
}

/** 2012 0x95fdb0: {fade curve, specific params, exceptions}. */
AKRESULT CAkActionActive::SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_eFadeCurve = *io_rpData++ & 0x1F;
	const AKRESULT eResult = SetActionSpecificParams( io_rpData, io_rulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return SetExceptParams( io_rpData, io_rulDataSize );
}

AKRESULT CAkActionSetValue::SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_eFadeCurve = *io_rpData++ & 0x1F;
	const AKRESULT eResult = SetActionSpecificParams( io_rpData, io_rulDataSize );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return SetExceptParams( io_rpData, io_rulDataSize );
}

/** 2012 0x95d9d0. */
AKRESULT CAkActionPause::SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_bPausePendingResume = *io_rpData++ != 0;
	return AK_Success;
}

/** 2012 0x95d850. */
AKRESULT CAkActionResume::SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_bIsMasterResume = *io_rpData++ != 0;
	return AK_Success;
}

/** 2012 0x95d4b0: {value meaning, base, min, max}. */
AKRESULT CAkActionSetAkProp::SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_eValueMeaning = *io_rpData;
	m_TargetValue.m_base = *(AkReal32 *)( io_rpData + 1 );
	m_TargetValue.m_mod.m_min = *(AkReal32 *)( io_rpData + 5 );
	m_TargetValue.m_mod.m_max = *(AkReal32 *)( io_rpData + 9 );
	io_rpData += 13;
	return AK_Success;
}

/** 2012 0x95d040. */
AKRESULT CAkActionSetGameParameter::SetActionSpecificParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_eValueMeaning = *io_rpData;
	m_TargetValue.m_base = *(AkReal32 *)( io_rpData + 1 );
	m_TargetValue.m_mod.m_min = *(AkReal32 *)( io_rpData + 5 );
	m_TargetValue.m_mod.m_max = *(AkReal32 *)( io_rpData + 9 );
	io_rpData += 13;
	return AK_Success;
}

/** 2012 0x95d120 (also CAkActionSetState's slot). */
AKRESULT CAkActionSetSwitch::SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_ulSwitchGroupID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	m_ulSwitchStateID = *(AkUInt32 *)io_rpData;
	io_rpData += 4;
	return AK_Success;
}

/** 2012 0x95ca00: {bypass, target mask, exceptions}. */
AKRESULT CAkActionBypassFX::SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_bIsBypass = *io_rpData++ != 0;
	m_uTargetMask = *io_rpData++;
	return SetExceptParams( io_rpData, io_rulDataSize );
}

/** 2012 0x95c530: {relative to duration, base, min, max, snap to marker, exceptions}. */
AKRESULT CAkActionSeek::SetActionParams( AkUInt8 *& io_rpData, AkUInt32 & io_rulDataSize )
{
	m_bIsSeekRelativeToDuration = *io_rpData++ != 0;
	m_position.m_base = ( (AkReal32 *)io_rpData )[ 0 ];
	m_position.m_mod.m_min = ( (AkReal32 *)io_rpData )[ 1 ];
	m_position.m_mod.m_max = ( (AkReal32 *)io_rpData )[ 2 ];
	io_rpData += 12;
	m_bSnapToNearestMarker = *io_rpData++ != 0;
	return SetExceptParams( io_rpData, io_rulDataSize );
}

// ------------------------------------------------------------------------------------------------------------
// CAkEvent (akevent.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x937310. */
CAkEvent * CAkEvent::CreateNoIndex( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkEvent ) );
	if( !pMem )
	{
		return NULL;
	}
	return AkPlacementNew( pMem ) CAkEvent( in_ulID );
}

/** 2012 0x937350. */
AkUInt32 CAkEvent::AddRef()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxEvents.m_IndexLock );
	return ++m_lRef;
}

/** 2012 0x937410. */
AkUInt32 CAkEvent::Release()
{
	AkAutoLock<CAkLock> IndexLock( g_pIndex->m_idxEvents.m_IndexLock );
	const AkInt32 lRef = --m_lRef;
	if( !lRef )
	{
		g_pIndex->m_idxEvents.RemoveID( key );
		this->~CAkEvent();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
	return lRef;
}

/** 2012 0x9373c0. */
void CAkEvent::AddToIndex()
{
	g_pIndex->m_idxEvents.SetIDToPtr( this );
}

/** 2012 0x937380. */
void CAkEvent::Clear()
{
	while( CAkAction * pAction = m_actions.m_pFirst )
	{
		m_actions.m_pFirst = pAction->pNextLightItem;
		pAction->Release();
	}
}

/** 2012 0x9374e0: {action count, action ids}; each action is AddRef'd onto the list in bank order. */
AKRESULT CAkEvent::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	const AkUInt32 ulNumActions = *(AkUInt32 *)( in_pData + 4 );
	AkUInt32 * pIDs = (AkUInt32 *)( in_pData + 8 );
	CAkAction * pLast = NULL;
	for( AkUInt32 i = 0; i < ulNumActions; ++i )
	{
		const AkUniqueID ulActionID = *pIDs++;
		if( !ulActionID )
		{
			return AK_InvalidID;
		}
		CAkAction * pAction = g_pIndex->m_idxActions.GetPtrAndAddRef( ulActionID );
		if( !pAction )
		{
			return AK_Fail;
		}
		pAction->pNextLightItem = NULL;
		if( pLast )
		{
			pLast->pNextLightItem = pAction;
		}
		else
		{
			m_actions.m_pFirst = pAction;
		}
		pLast = pAction;
	}
	g_pHircParseEnd = (const AkUInt8 *)pIDs;
	return AK_Success;
}

/** 2012 0x937570: the play actions' targets and their whole subtree. */
AKRESULT CAkEvent::QueryAudioObjectIDs( AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos )
{
	AkUInt32 uIndex_Out = 0;
	for( CAkAction * pAction = m_actions.m_pFirst; pAction; pAction = pAction->pNextLightItem )
	{
		if( pAction->m_eActionType != AkActionType_Play )
		{
			continue;
		}
		const AkUniqueID ulElementID = pAction->m_ulElementID;
		CAkParameterNodeBase * pObj = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( ulElementID );
		if( !pObj )
		{
			continue;
		}
		if( io_ruNumItems )
		{
			out_aObjectInfos[ uIndex_Out ].objID = ulElementID;
			out_aObjectInfos[ uIndex_Out ].parentID = pObj->m_pParentNode ? pObj->m_pParentNode->key : 0;
			out_aObjectInfos[ uIndex_Out ].iDepth = 0;
			++uIndex_Out;
			if( uIndex_Out == io_ruNumItems )
			{
				pObj->Release();
				break;
			}
		}
		else
		{
			++uIndex_Out;
		}
		pObj->GetChildren( io_ruNumItems, out_aObjectInfos, uIndex_Out, 1 );
		pObj->Release();
		if( uIndex_Out == io_ruNumItems )
		{
			break;
		}
	}
	const bool bWasCount = io_ruNumItems == 0;
	io_ruNumItems = uIndex_Out;
	return bWasCount ? AK_PartialSuccess : AK_Success;
}

/** DISHONORED(port, IH2): retail's ~CAkGen3DParams (inlined in ~CAkGen3DParamsEx 0x954ff0) only releases the attenuation
    the PBI copy took (CAkPBI::Init); the path arrays belong to the node and are freed by Term in ~CAkParameterNode. */
CAkGen3DParams::~CAkGen3DParams()
{
	if( m_Params.m_pAttenuation )
	{
		static_cast<CAkAttenuation *>( m_Params.m_pAttenuation )->Release();
	}
}
