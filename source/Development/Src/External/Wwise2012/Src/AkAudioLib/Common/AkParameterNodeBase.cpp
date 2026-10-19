/*=============================================================================
	AkParameterNodeBase.cpp - the runtime half of CAkParameterNodeBase (akparameternodebase.cpp/.h): state values and
	their transitions (AkStateGroupChunk), the SIS a SetValue / Mute action writes (CAkSIS, aksis.cpp), the RTPC
	manager's node callback, the notifications that carry a changed value down to the PBIs, the virtual voice counts and
	the FX bookkeeping; the property tables of akparameters.cpp.

	DISHONORED(port): agent IH3 fork N (wave 26); 2012 rva on each definition, 2013 from ih1_inventory.csv.
	Deviations (named):
	  * The feedback (motion) half - GetFeedbackParameters, GetEffectiveFeedbackVolume / LPF, GetFeedbackParentBusOrDefault -
	    is not ported: no feedback device exists (the glue never creates CAkFeedbackDeviceMgr) and no bank holds a feedback
	    bus (wwise_bank_dump.py --all), so retail's own paths end at a NULL feedback bus. FeedbackParentBus get / set and
	    SetFeedbackVolume (a SIS value) are ported.
	  * CAkStateSyncArray holds AkUInt32 sync types (retail AkSyncType, same size).
=============================================================================*/
#include "AkHierarchy.h"
#include "AkActivityChunk.h"
#include "AkURenderer.h"
#include "AkRegistryMgr.h"

const AkRTPC_ParameterID g_AkPropRTPCID[ 17 ] =
{
	RTPC_Volume, RTPC_LFE, RTPC_Pitch, RTPC_LPF, RTPC_Priority, RTPC_MaxNumRTPC, RTPC_MaxNumRTPC, RTPC_FeedbackVolume,
	RTPC_FeedbackLowpass, RTPC_MaxNumRTPC, RTPC_Position_PAN_RL, RTPC_Position_PAN_FR, RTPC_Positioning_Divergence_Center_PCT,
	RTPC_MaxNumRTPC, RTPC_MaxNumRTPC, RTPC_MaxNumRTPC, RTPC_MaxNumRTPC
};

const bool g_AkPropDecibel[ 17 ] =
{
	true, true, false, false, false, false, false, true, false, true, false, false, false, false, false, false, false
};

/** AkPropID_Loop's default is the integer 1 (the float with bit pattern 1). */
const AkPropValue g_AkPropDefault[ 17 ] =
{
	AkPropValue( 0.f ), AkPropValue( 0.f ), AkPropValue( 0.f ), AkPropValue( 0.f ), AkPropValue( 50.f ), AkPropValue( -10.f ),
	AkPropValue( 1.401298464e-45f ), AkPropValue( 0.f ), AkPropValue( 0.f ), AkPropValue( 1.f ), AkPropValue( 0.f ), AkPropValue( 0.f ),
	AkPropValue( 0.f ), AkPropValue( 0.f ), AkPropValue( 0.f ), AkPropValue( 100.f ), AkPropValue( 0.f )
};

// ------------------------------------------------------------------------------------------------------------
// CAkSIS
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x955030 (2013 0x951830). */
CAkSIS::~CAkSIS()
{
	const AkUInt32 cProps = m_values.Count();
	for( AkUInt32 i = 0; i < cProps; ++i )
	{
		AkSISValue * pValue = m_values.ValueAt( i );
		if( pValue->pTransition )
		{
			g_pTransitionManager->RemoveTransitionUser( pValue->pTransition, this );
		}
	}
	m_values.RemoveAll();
}

/** 2012 0x9550b0 (2013 0x9518b0): the mute ratio goes to the muted map, every other value as a delta notification. */
void CAkSIS::TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated )
{
	AkSISValue * pValue = m_values.FindProp( (AkUInt8)in_eTarget );
	if( in_eTarget == AkPropID_MuteRatio )
	{
		in_fValue = ( in_fValue + 96.3f ) * 0.010384216f;
		AkMutedMapItem item;
		item.m_Identifier = m_pParamNode;
		item.m_bIsGlobal = m_pGameObj == NULL;
		item.m_bIsPersistent = 0;
		if( m_pGameObj )
		{
			m_pParamNode->MuteNotification( in_fValue, m_pGameObj, item, false );
		}
		else
		{
			m_pParamNode->MuteNotification( in_fValue, item, false );
		}
	}
	else
	{
		m_pParamNode->Notification( g_AkPropRTPCID[ in_eTarget ], in_fValue - pValue->fValue, m_pGameObj, NULL );
	}
	pValue->fValue = in_fValue;
	if( in_bIsTerminated )
	{
		pValue->pTransition = NULL;
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNodeBase
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x942b20 (2013 0x93f460). */
AKRESULT CAkParameterNodeBase::Stop( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	ActionParams params;
	params.eType = ActionParamType_Stop;
	params.pGameObj = in_pGameObj;
	params.playingID = in_PlayingID;
	params.transParams.TransitionTime = in_uTransitionDuration;
	params.transParams.eFadeCurve = in_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = false;
	params.targetNodeID = 0;
	return ExecuteAction( params );
}

/** 2012 0x942b70 (2013 0x93f4b0). */
AKRESULT CAkParameterNodeBase::Pause( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	ActionParams params;
	params.eType = ActionParamType_Pause;
	params.pGameObj = in_pGameObj;
	params.playingID = in_PlayingID;
	params.transParams.TransitionTime = in_uTransitionDuration;
	params.transParams.eFadeCurve = in_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = false;
	params.targetNodeID = 0;
	return ExecuteAction( params );
}

/** 2012 0x942bc0 (2013 0x93f500). */
AKRESULT CAkParameterNodeBase::Resume( CAkRegisteredObj * in_pGameObj, AkPlayingID in_PlayingID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve )
{
	ActionParams params;
	params.eType = ActionParamType_Resume;
	params.pGameObj = in_pGameObj;
	params.playingID = in_PlayingID;
	params.transParams.TransitionTime = in_uTransitionDuration;
	params.transParams.eFadeCurve = in_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = false;
	params.targetNodeID = 0;
	return ExecuteAction( params );
}

/** 2012 0x942c10 (2013 0x93f550). */
void CAkParameterNodeBase::Notification( AkRTPC_ParameterID in_ParamID, AkReal32 in_fValue, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	NotifParams params;
	params.eType = in_ParamID;
	params.pGameObj = in_pGameObj;
	params.bIsFromBus = false;
	params.pExceptObjects = in_pExceptArray;
	params.fValue = in_fValue;
	ParamNotification( params );
}

/** 2012 0x942c50 (2013 0x93f590). */
void CAkParameterNodeBase::PriorityNotification( AkReal32 in_Priority, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	NotifParams params;
	params.eType = RTPC_Priority;
	params.pGameObj = in_pGameObj;
	params.bIsFromBus = false;
	params.pExceptObjects = in_pExceptArray;
	params.fValue = in_Priority;
	PriorityNotification( params );
}

/** 2012 0x942c90 (2013 0x93f5d0). */
void CAkParameterNodeBase::PriorityNotification( NotifParams & in_rParams )
{
	ParamNotification( in_rParams );
}

/** 2012 0x942cd0 (2013 0x93f610). */
CAkBus * CAkParameterNodeBase::GetLimitingBus()
{
	if( m_pBusOutputNode )
	{
		return m_pBusOutputNode->GetLimitingBus();
	}
	if( m_pParentNode )
	{
		return m_pParentNode->GetLimitingBus();
	}
	return NULL;
}

/** 2012 0x930cb0 (akparameternodebase.h, 2013 0x92d450). */
bool CAkParameterNodeBase::IsPlaying() const
{
	return m_pActivityChunk && m_pActivityChunk->m_PlayCount != 0;
}

/** 2012 0x944b40 (2013 0x9413d0). */
bool CAkParameterNodeBase::IsException( AkUniqueID in_ID, AkArray<AkUniqueID, AkUniqueID, ArrayPoolDefault, 8> & in_rExceptionList )
{
	for( AkUniqueID * p = in_rExceptionList.m_pItems; p != in_rExceptionList.m_pItemsEnd; ++p )
	{
		if( *p == in_ID )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x942fb0 (2013 0x93f8f0). */
void CAkParameterNodeBase::UseState( bool in_bUseState )
{
	m_bUseState = in_bUseState;
	NotifyStateParametersModified();
}

/** 2012 0x942d90 (2013 0x93f6d0). */
AKRESULT CAkParameterNodeBase::RemoveFX( AkUInt32 in_uFXIndex )
{
	if( in_uFXIndex >= 4 )
	{
		return AK_InvalidParameter;
	}
	if( !m_pFXChunk )
	{
		return AK_Fail;
	}
	FXStruct & fx = m_pFXChunk->aFX[ in_uFXIndex ];
	if( fx.bShareSet || fx.id )
	{
		fx.bShareSet = false;
		fx.id = 0;
		RecalcNotification();
		UpdateFx( in_uFXIndex );
	}
	return AK_Success;
}

/** 2012 0x942f00 (2013 0x93f840). */
void CAkParameterNodeBase::BypassFX( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj, bool in_bIsFromReset )
{
	CAkSIS * pSIS = GetSIS( in_pGameObj );
	if( pSIS )
	{
		pSIS->m_bitsFXBypass = (AkUInt8)( in_bitsFXBypass | ( pSIS->m_bitsFXBypass & ~in_uTargetMask ) );
	}
	if( !in_pGameObj )
	{
		ResetFXBypass( in_bitsFXBypass, in_uTargetMask );
	}
	NotifyBypass( in_bitsFXBypass, in_uTargetMask, in_pGameObj, NULL );
}

/** 2012 0x942f70 (2013 0x93f8b0). */
void CAkParameterNodeBase::ResetBypassFX( AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj )
{
	BypassFX( m_pFXChunk ? m_pFXChunk->bitsMainFXBypass : 0, in_uTargetMask, in_pGameObj, true );
}

/** 2012 0x944b90 (2013 0x941420): a SIS value moves to its target (Independent: relative to the authored value) now, or
    through a fade transition; a running transition is retargeted. */
void CAkParameterNodeBase::StartSISTransition( CAkSIS * in_pSIS, AkPropID in_ePropID, AkReal32 in_fTargetValue, AkValueMeaning in_eValueMeaning, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	AkSISValue * pValue = in_pSIS->m_values.FindProp( (AkUInt8)in_ePropID );
	if( !pValue )
	{
		pValue = in_pSIS->m_values.AddAkProp( (AkUInt8)in_ePropID );
		if( !pValue )
		{
			return;
		}
		pValue->fValue = 0.f;
		pValue->pTransition = NULL;
	}
	if( pValue->pTransition )
	{
		if( in_eValueMeaning == AkValueMeaning_Independent )
		{
			const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_ePropID );
			in_fTargetValue -= pProp ? pProp->fValue : g_AkPropDefault[ in_ePropID ].fValue;
		}
		g_pTransitionManager->ChangeParameter( pValue->pTransition, (TransitionTargets)in_ePropID, in_fTargetValue, in_lTransitionTime, in_eFadeCurve, in_eValueMeaning );
		return;
	}
	const AkReal32 fStartValue = pValue->fValue;
	AkReal32 fTargetValue = 0.f;
	if( in_eValueMeaning == AkValueMeaning_Independent )
	{
		const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_ePropID );
		fTargetValue = in_fTargetValue - ( pProp ? pProp->fValue : g_AkPropDefault[ in_ePropID ].fValue );
	}
	else if( in_eValueMeaning == AkValueMeaning_Offset )
	{
		fTargetValue = fStartValue + in_fTargetValue;
	}
	if( fStartValue == fTargetValue || !in_lTransitionTime )
	{
		pValue->fValue = fTargetValue;
		Notification( g_AkPropRTPCID[ in_ePropID ], fTargetValue - fStartValue, in_pSIS->m_pGameObj, NULL );
		return;
	}
	TransitionParameters params;
	params.pUser = in_pSIS;
	params.eTarget = (TransitionTargets)in_ePropID;
	params.fStartValue = fStartValue;
	params.fTargetValue = fTargetValue;
	params.lDuration = in_lTransitionTime;
	params.eFadeCurve = in_eFadeCurve;
	params.bdBs = g_AkPropDecibel[ in_ePropID ];
	params.bUseReciprocalCurve = true;
	pValue->pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
}

/** 2012 0x944250 (2013 0x940bd0): the mute ratio fades in dB from its current value to -96.3 (mute) or 0 (unmute). */
void CAkParameterNodeBase::StartSisMuteTransitions( CAkSIS * in_pSIS, AkReal32 in_fTargetValue, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	AkSISValue * pValue = in_pSIS->m_values.FindProp( AkPropID_MuteRatio );
	if( !pValue )
	{
		pValue = in_pSIS->m_values.AddAkProp( AkPropID_MuteRatio );
		if( !pValue )
		{
			return;
		}
		pValue->fValue = 1.f;
		pValue->pTransition = NULL;
	}
	const AkReal32 fTarget = in_fTargetValue == 0.f ? -96.300003f : 0.f;
	if( pValue->pTransition )
	{
		g_pTransitionManager->ChangeParameter( pValue->pTransition, (TransitionTargets)AkPropID_MuteRatio, fTarget, in_lTransitionTime, in_eFadeCurve, AkValueMeaning_Default );
	}
	else if( in_lTransitionTime )
	{
		TransitionParameters params;
		params.pUser = in_pSIS;
		params.eTarget = (TransitionTargets)AkPropID_MuteRatio;
		params.fStartValue = pValue->fValue * 96.300003f - 96.300003f;
		params.fTargetValue = fTarget;
		params.lDuration = in_lTransitionTime;
		params.eFadeCurve = in_eFadeCurve;
		params.bdBs = true;
		params.bUseReciprocalCurve = true;
		pValue->pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
	}
	else
	{
		in_pSIS->TransUpdateValue( (TransitionTargets)AkPropID_MuteRatio, fTarget, true );
	}
}

/** 2012 0x944fa0 (2013 0x941830): true once a node with this state group decided the sync type ("immediate" ends the
    search with that type alone). */
bool CAkParameterNodeBase::CheckSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes )
{
	for( AkStateGroupChunk * pChunk = m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
	{
		if( pChunk->m_ulStateGroup != in_stateGroupID )
		{
			continue;
		}
		const AkUInt32 eSyncType = pChunk->m_eStateSyncType;
		if( !eSyncType )
		{
			io_pSyncTypes->m_StateSyncArray.RemoveAll();
			io_pSyncTypes->m_StateSyncArray.AddLast( eSyncType );
			return true;
		}
		if( !io_pSyncTypes->m_StateSyncArray.Exists( eSyncType ) )
		{
			io_pSyncTypes->m_StateSyncArray.AddLast( eSyncType );
		}
		return false;
	}
	return false;
}

/** 2012 0x9458d0 (2013 0x942160): up the parents, the first output bus chain once. */
bool CAkParameterNodeBase::GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes, bool in_bBusChecked )
{
	CAkParameterNodeBase * pNode = this;
	while( !pNode->CheckSyncTypes( in_stateGroupID, io_pSyncTypes ) )
	{
		if( !in_bBusChecked && pNode->m_pBusOutputNode )
		{
			in_bBusChecked = true;
			if( static_cast<CAkBus *>( pNode->m_pBusOutputNode )->GetStateSyncTypes( in_stateGroupID, io_pSyncTypes ) )
			{
				return true;
			}
		}
		pNode = pNode->m_pParentNode;
		if( !pNode )
		{
			return false;
		}
	}
	return true;
}

/** AkSortedKeyArray::ReSortArray<AkPriorityStruct_INC / _DEC> as SetMaxReachedBehavior inlines it: the limiter's kill
    order flips and its voices are re-sorted (priority descending, then PBIID ascending when killing the newest). */
static void FlipKillNewestAndReSort( CAkLimiter & io_rLimiter )
{
	io_rLimiter.m_bDoesKillNewest = !io_rLimiter.m_bDoesKillNewest;
	CAkPBI ** pItems = io_rLimiter.m_sortedPBIList.m_pItems;
	const AkInt32 cItems = (AkInt32)io_rLimiter.m_sortedPBIList.Length();
	for( AkInt32 i = 1; i < cItems; ++i )
	{
		CAkPBI * pPBI = pItems[ i ];
		const AkPriorityStruct & key = pPBI->m_PriorityInfoCurrent.currentPriority;
		AkInt32 j = i - 1;
		for( ; j >= 0; --j )
		{
			const AkPriorityStruct & other = pItems[ j ]->m_PriorityInfoCurrent.currentPriority;
			const bool bBefore = key.priority != other.priority ? key.priority > other.priority : ( io_rLimiter.m_bDoesKillNewest ? key.PBIID < other.PBIID : key.PBIID > other.PBIID );
			if( !bBefore )
			{
				break;
			}
			pItems[ j + 1 ] = pItems[ j ];
		}
		pItems[ j + 1 ] = pPBI;
	}
}

/** 2012 0x945790 (2013 0x942020): the limiters flip their kill order. */
void CAkParameterNodeBase::SetMaxReachedBehavior( bool in_bKillNewest )
{
	if( m_bKillNewest == (AkUInt8)in_bKillNewest )
	{
		return;
	}
	m_bKillNewest = in_bKillNewest;
	if( !m_pActivityChunk )
	{
		return;
	}
	FlipKillNewestAndReSort( m_pActivityChunk->m_Limiter );
	for( MapStruct<CAkRegisteredObj *, StructMaxInst> * p = m_pActivityChunk->m_ListPlayCountPerObj.m_pItems; p != m_pActivityChunk->m_ListPlayCountPerObj.m_pItemsEnd; ++p )
	{
		if( p->item.m_pLimiter )
		{
			FlipKillNewestAndReSort( *p->item.m_pLimiter );
		}
	}
}

/** 2012 0x943810 (2013 0x940150). */
void CAkParameterNodeBase::SetOverLimitBehavior( bool in_bUseVirtualBehavior )
{
	if( m_bUseVirtualBehavior == (AkUInt8)in_bUseVirtualBehavior )
	{
		return;
	}
	m_bUseVirtualBehavior = in_bUseVirtualBehavior;
	if( !m_pActivityChunk )
	{
		return;
	}
	m_pActivityChunk->m_Limiter.m_bAllowUseVirtualBehavior = in_bUseVirtualBehavior;
	for( MapStruct<CAkRegisteredObj *, StructMaxInst> * p = m_pActivityChunk->m_ListPlayCountPerObj.m_pItems; p != m_pActivityChunk->m_ListPlayCountPerObj.m_pItemsEnd; ++p )
	{
		if( p->item.m_pLimiter )
		{
			p->item.m_pLimiter->m_bAllowUseVirtualBehavior = in_bUseVirtualBehavior;
		}
	}
}

/** 2012 0x944eb0 (2013 0x941740): the authoring path's property change. */
void CAkParameterNodeBase::SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax )
{
	const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
	const AkReal32 fCurrent = pProp ? pProp->fValue : g_AkPropDefault[ in_eProp ].fValue;
	if( fCurrent != in_fValue )
	{
		m_props.SetAkProp( (AkUInt8)in_eProp, AkPropValue( in_fValue ) );
		RecalcNotification();
	}
}

/** 2012 0x944f30 (2013 0x9417c0). */
void CAkParameterNodeBase::SetAkProp( AkPropID in_eProp, AkInt32 in_iValue, AkInt32 in_iMin, AkInt32 in_iMax )
{
	const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
	const AkInt32 iCurrent = pProp ? pProp->iValue : g_AkPropDefault[ in_eProp ].iValue;
	if( iCurrent != in_iValue )
	{
		AkPropValue value;
		value.iValue = in_iValue;
		m_props.SetAkProp( (AkUInt8)in_eProp, value );
		RecalcNotification();
	}
}

/** 2012 0x945040 (2013 0x9418d0). */
void CAkParameterNodeBase::SetFeedbackVolume( CAkRegisteredObj * in_GameObjPtr, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	CAkSIS * pSIS = GetSIS( in_GameObjPtr );
	if( pSIS )
	{
		StartSISTransition( pSIS, AkPropID_FeedbackVolume, in_fTargetValue, in_eValueMeaning, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x943010 (2013 0x93f950): an RTPC value changed - volume / pitch / LPF / feedback as a delta from the value the
    curve gave before, priority, instance limit, positioning and bypass as the new value. */
AKRESULT CAkParameterNodeBase::SetParamComplexFromRTPCManager( void * in_pToken, AkUInt32 in_Param_id, AkRtpcID in_RTPCid, AkReal32 in_value, CAkRegisteredObj * in_GameObj, void * in_pGameObjExceptArray )
{
	switch( in_Param_id )
	{
	case RTPC_Volume:
	case RTPC_Pitch:
	case RTPC_LPF:
	case RTPC_FeedbackVolume:
	case RTPC_FeedbackLowpass:
		Notification( (AkRTPC_ParameterID)in_Param_id, in_value - g_pRTPCMgr->GetRTPCConvertedValue( in_pToken, in_GameObj, in_RTPCid ), in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_Priority:
		if( !m_bPriorityOverrideParent && m_pParentNode )
		{
			return AK_Success;
		}
		PriorityNotification( in_value, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_MaxNumInstances:
		if( !m_bIsMaxNumInstOverrideParent && m_pParentNode )
		{
			return AK_Success;
		}
		ApplyMaxNumInstances( (AkUInt16)(AkInt32)in_value, in_GameObj, in_pGameObjExceptArray, true );
		return AK_Success;
	case RTPC_Positioning_Divergence_Center_PCT:
	case RTPC_Positioning_Cone_Attenuation_ON_OFF:
	case RTPC_Positioning_Cone_Attenuation:
	case RTPC_Positioning_Cone_LPF:
	case RTPC_Position_PAN_RL:
	case RTPC_Position_PAN_FR:
	case 0x20:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
		PositioningChangeNotification( in_value, (AkRTPC_ParameterID)in_Param_id, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_BypassFX0:
		NotifyBypass( in_value != 0.f ? 1 : 0, 1, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_BypassFX1:
		NotifyBypass( in_value != 0.f ? 2 : 0, 2, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_BypassFX2:
		NotifyBypass( in_value != 0.f ? 4 : 0, 4, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_BypassFX3:
		NotifyBypass( in_value != 0.f ? 8 : 0, 8, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	case RTPC_BypassAllFX:
		NotifyBypass( in_value != 0.f ? 16 : 0, 16, in_GameObj, in_pGameObjExceptArray );
		return AK_Success;
	default:
		return AK_Fail;
	}
}

/** 2012 0x943280 (2013 0x93fbc0). */
void CAkParameterNodeBase::UnsetRTPC( AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID )
{
	bool bMoreCurvesRemaining = false;
	if( g_pRTPCMgr )
	{
		g_pRTPCMgr->UnSubscribeRTPC( this, in_ParamID, in_RTPCCurveID, &bMoreCurvesRemaining );
	}
	if( !bMoreCurvesRemaining )
	{
		m_RTPCBitArrayMax32.UnsetBit( in_ParamID );
	}
	RecalcNotification();
	NotifyRTPCChanged();
}

/** 2012 0x9432e0 (2013 0x93fc20). */
void CAkParameterNodeBase::FeedbackParentBus( CAkFeedbackBus * in_pParent )
{
	if( !in_pParent && !m_pFeedbackInfo )
	{
		return;
	}
	if( !m_pFeedbackInfo )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkFeedbackInfo ) );
		if( !pMem )
		{
			return;
		}
		m_pFeedbackInfo = (AkFeedbackInfo *)pMem;
		m_pFeedbackInfo->m_pFeedbackBus = NULL;
	}
	m_pFeedbackInfo->m_pFeedbackBus = (CAkParameterNodeBase *)in_pParent;
}

/** 2012 0x943330 (2013 0x93fc70). */
CAkFeedbackBus * CAkParameterNodeBase::FeedbackParentBus()
{
	return m_pFeedbackInfo ? (CAkFeedbackBus *)m_pFeedbackInfo->m_pFeedbackBus : NULL;
}

/** 2012 0x9441d0 (2013 0x940b50): the node's running state transitions pause / resume with it. */
void CAkParameterNodeBase::PauseTransitions( bool in_bPause )
{
	for( AkStateGroupChunk * pChunk = m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
	{
		const AkUInt32 cProps = pChunk->m_values.Count();
		for( AkUInt32 i = 0; i < cProps; ++i )
		{
			CAkTransition * pTransition = pChunk->m_values.ValueAt( i )->pTransition;
			if( !pTransition )
			{
				continue;
			}
			if( in_bPause )
			{
				g_pTransitionManager->Pause( pTransition );
			}
			else
			{
				g_pTransitionManager->Resume( pTransition );
			}
		}
	}
}

/** 2012 0x944380 (2013 0x940d00): every state group's current values, when the node uses states. */
void CAkParameterNodeBase::GetAudioStateParams( AkSoundParams & io_Parameters, AkUInt32 in_uParamSelect )
{
	if( !m_bUseState )
	{
		return;
	}
	for( AkStateGroupChunk * pChunk = m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
	{
		if( in_uParamSelect & 1 )
		{
			const AkStateValue * pValue = pChunk->m_values.FindProp( AkPropID_Volume );
			if( pValue )
			{
				io_Parameters.Volume += pValue->fValue;
			}
		}
		if( in_uParamSelect & 2 )
		{
			const AkStateValue * pValue = pChunk->m_values.FindProp( AkPropID_Pitch );
			if( pValue )
			{
				io_Parameters.Pitch += pValue->fValue;
			}
		}
		if( in_uParamSelect & 4 )
		{
			const AkStateValue * pValue = pChunk->m_values.FindProp( AkPropID_LPF );
			if( pValue )
			{
				io_Parameters.LPF += pValue->fValue;
			}
		}
	}
}

/** 2012 0x944b70 (2013 0x941400). */
void CAkParameterNodeBase::FlushStateTransitions()
{
	for( AkStateGroupChunk * pChunk = m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
	{
		pChunk->FlushStateTransitions();
	}
}

/** 2012 0x9455e0 (2013 0x941e70): each state group's values jump to the current state's (a running transition is
    retargeted with no duration); a group whose state carries no values returns to 0. */
void CAkParameterNodeBase::NotifyStateParametersModified()
{
	for( AkStateGroupChunk * pChunk = m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
	{
		CAkState * pState = pChunk->GetState( pChunk->m_ulActualState );
		if( !pState )
		{
			pChunk->FlushStateTransitions();
			const AkUInt32 cProps = pChunk->m_values.Count();
			for( AkUInt32 i = 0; i < cProps; ++i )
			{
				pChunk->m_values.ValueAt( i )->fValue = 0.f;
			}
			continue;
		}
		const AkUInt32 cStateProps = pState->m_props.Count();
		for( AkUInt32 i = 0; i < cStateProps; ++i )
		{
			const AkUInt8 ePropID = pState->m_props.IDAt( i );
			const AkReal32 fValue = *pState->m_props.ValueAt( i );
			AkStateValue * pValue = pChunk->m_values.FindProp( ePropID );
			if( !pValue )
			{
				pValue = pChunk->m_values.AddAkProp( ePropID );
				if( pValue )
				{
					pValue->pTransition = NULL;
					pValue->fValue = fValue;
				}
				continue;
			}
			if( pValue->pTransition )
			{
				g_pTransitionManager->ChangeParameter( pValue->pTransition, (TransitionTargets)ePropID, fValue, 0, AkCurveInterpolation_Linear, AkValueMeaning_Default );
			}
			else
			{
				pValue->fValue = fValue;
			}
		}
	}
	RecalcNotification();
}

/** 2012 0x945500 (2013 0x941d90). */
void CAkParameterNodeBase::IncrementVirtualCountGameObject( CAkRegisteredObj * in_pGameObj )
{
	StructMaxInst * pPerObj = m_pActivityChunk->m_ListPlayCountPerObj.Exists( in_pGameObj );
	if( pPerObj )
	{
		++pPerObj->u16CurrentVirtual;
	}
}

/** 2012 0x945fd0 (2013 0x942860): a voice leaving the virtual state can push the node over its limit again. */
void CAkParameterNodeBase::DecrementVirtualCountGlobal( AkUInt16 & io_ui16NumKickedOrRevived, bool in_bAllowKick )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	--m_pActivityChunk->m_iVirtualCountValid;
	if( in_bAllowKick )
	{
		const AkUInt16 u16MaxNumInstance = GetMaxNumInstances( NULL );
		if( u16MaxNumInstance )
		{
			const AkInt32 iPlayCountValid = m_pActivityChunk ? m_pActivityChunk->m_iPlayCountValid : 0;
			const AkInt32 iVirtualCountValid = m_pActivityChunk ? m_pActivityChunk->m_iVirtualCountValid : 0;
			if( iPlayCountValid - io_ui16NumKickedOrRevived - iVirtualCountValid > (AkInt32)u16MaxNumInstance )
			{
				CAkParameterNodeBase * pKicked = NULL;
				CAkURenderer::Kick( 101.f, NULL, key, m_bKillNewest, m_bUseVirtualBehavior, pKicked, KickFrom_OverNodeLimit );
				++io_ui16NumKickedOrRevived;
			}
		}
	}
	if( m_pActivityChunk->ChunkIsUseless() )
	{
		DeleteActivityChunk();
	}
}

/** 2012 0x946180 (2013 0x942a10). */
void CAkParameterNodeBase::DecrementVirtualCountGameObject( AkUInt16 & io_ui16NumKickedOrRevived, bool in_bAllowKick, CAkRegisteredObj * in_pGameObj )
{
	StructMaxInst * pPerObj = m_pActivityChunk->m_ListPlayCountPerObj.Exists( in_pGameObj );
	if( !pPerObj )
	{
		return;
	}
	--pPerObj->u16CurrentVirtual;
	if( in_bAllowKick && pPerObj->m_pLimiter )
	{
		const AkUInt16 u16Max = pPerObj->m_pLimiter->m_u16LimiterMax;
		if( u16Max && (AkInt32)pPerObj->u16Current - io_ui16NumKickedOrRevived - pPerObj->u16CurrentVirtual > (AkInt32)u16Max )
		{
			CAkParameterNodeBase * pKicked = NULL;
			CAkURenderer::Kick( 101.f, in_pGameObj, key, m_bKillNewest, m_bUseVirtualBehavior, pKicked, KickFrom_OverNodeLimit );
			if( !pKicked || pKicked->GetLimitingBus() == GetLimitingBus() )
			{
				++io_ui16NumKickedOrRevived;
			}
		}
	}
	if( !pPerObj->u16Current && !pPerObj->u16CurrentVirtual )
	{
		pPerObj->DisableLimiter();
		m_pActivityChunk->m_ListPlayCountPerObj.Unset( in_pGameObj );
		if( m_pActivityChunk->ChunkIsUseless() )
		{
			DeleteActivityChunk();
		}
	}
}
