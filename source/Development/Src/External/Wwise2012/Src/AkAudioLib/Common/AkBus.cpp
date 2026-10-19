/*=============================================================================
	AkBus.cpp - the runtime half of CAkBus (akbus.cpp / akbus.h) and CAkDuckItem (akduckitem.cpp): the bus SIS (SetValue /
	Mute on a bus), the ducking state machine, the bus's notifications - a volume change on a bus mixed by its own mix
	bus node (an effect or an environmental bus) goes to the lower engine, every other one down to the voices.

	DISHONORED(port): agent IH3 fork N (wave 26); 2012 rva on each definition, 2013 from ih1_inventory.csv.
	Deviations (named):
	  * UpdateFx: CAkLEngine::UpdateMasterBusFX / UpdateMixBusFX (the bus insert-effect swap) are IH4's; the bus's voices
	    are not touched by an effect change before then.
	  * GetFX / GetFXDataID (and AK::CAkBusCtx::GetFX) wait for IH4's insert effects.
=============================================================================*/
#include "AkHierarchy.h"
#include "AkActivityChunk.h"
#include "AkAudioMgr.h"
#include "AkRegistryMgr.h"
#include "AkLEngine.h"
#include "AkBusCtx.h"

// ------------------------------------------------------------------------------------------------------------
// CAkDuckItem
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x955860 (2013 0x952100). */
AKRESULT CAkDuckItem::Init( CAkBus * in_pBusNode )
{
	m_pBusNode = in_pBusNode;
	m_pvVolumeTransition = NULL;
	m_EffectiveVolumeOffset = 0.f;
	return AK_Success;
}

/** 2012 0x955830 (2013 0x9520d0). */
AKRESULT CAkDuckItem::Term()
{
	if( m_pvVolumeTransition )
	{
		g_pTransitionManager->RemoveTransitionUser( m_pvVolumeTransition, this );
		m_pvVolumeTransition = NULL;
	}
	return AK_Success;
}

/** 2012 0x955880 (2013 0x952120): the bus's ducked volume moves; the delta reaches the voices (or the mix bus node). */
void CAkDuckItem::TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated )
{
	const AkReal32 fPreviousVolume = m_pBusNode->GetDuckedVolume();
	m_EffectiveVolumeOffset = in_fValue;
	const AkReal32 fNotified = m_pBusNode->GetDuckedVolume() - fPreviousVolume;
	if( in_bIsTerminated )
	{
		m_pBusNode->CheckDuck();
		m_pvVolumeTransition = NULL;
	}
	if( fNotified != 0.f )
	{
		m_pBusNode->Notification( RTPC_Volume, fNotified, NULL, NULL );
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkBus
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x93c520 (2013 0x938ec0): a bus has the global SIS only. */
CAkSIS * CAkBus::GetSIS( CAkRegisteredObj * )
{
	g_pRegistryMgr->SetNodeIDAsModified( this );
	if( !m_pGlobalSIS )
	{
		AkUInt8 bitsFXBypass = 0;
		for( AkUInt32 i = 0; i < 4; ++i )
		{
			bitsFXBypass |= (AkUInt8)( GetBypassFX( i ) << i );
		}
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSIS ) );
		m_pGlobalSIS = pMem ? AkPlacementNew( pMem ) CAkSIS( this, NULL, bitsFXBypass ) : NULL;
	}
	return m_pGlobalSIS;
}

/** 2012 0x93bbd0 (2013 0x938570). */
void CAkBus::SetAkProp( AkPropID in_eProp, CAkRegisteredObj * in_pGameObj, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	CAkSIS * pSIS = GetSIS( NULL );
	if( pSIS )
	{
		StartSISTransition( pSIS, in_eProp, in_fTargetValue, in_eValueMeaning, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x93c9f0 (2013 0x939390): the authoring path; volume / LFE / pitch / LPF notify the delta. */
void CAkBus::SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax )
{
	if( (AkUInt32)in_eProp > AkPropID_LPF )
	{
		CAkParameterNodeBase::SetAkProp( in_eProp, in_fValue, in_fMin, in_fMax );
		return;
	}
	const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
	const AkReal32 fDelta = in_fValue - ( pProp ? pProp->fValue : 0.f );
	if( fDelta != 0.f )
	{
		Notification( g_AkPropRTPCID[ in_eProp ], fDelta, NULL, NULL );
		m_props.SetAkProp( (AkUInt8)in_eProp, AkPropValue( in_fValue ) );
	}
}

/** 2012 0x93c440 (2013 0x938de0). */
void CAkBus::ResetAkProp( AkPropID in_eProp, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	if( !m_pGlobalSIS )
	{
		return;
	}
	const AkSISValue * pValue = m_pGlobalSIS->m_values.FindProp( (AkUInt8)in_eProp );
	if( pValue && pValue->fValue != 0.f )
	{
		SetAkProp( in_eProp, (CAkRegisteredObj *)NULL, AkValueMeaning_Default, 0.f, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x93bc10 (2013 0x9385b0): a bus mutes globally only. */
void CAkBus::Mute( CAkRegisteredObj * in_pGameObj, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	if( in_pGameObj )
	{
		return;
	}
	CAkSIS * pSIS = GetSIS( NULL );
	if( pSIS )
	{
		StartSisMuteTransitions( pSIS, 0.f, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x93c4c0 (2013 0x938e60). */
void CAkBus::Unmute( CAkRegisteredObj * in_pGameObj, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	if( in_pGameObj || !m_pGlobalSIS )
	{
		return;
	}
	const AkSISValue * pValue = m_pGlobalSIS->m_values.FindProp( AkPropID_MuteRatio );
	if( pValue && pValue->fValue != 1.f )
	{
		g_pRegistryMgr->SetNodeIDAsModified( this );
		StartSisMuteTransitions( m_pGlobalSIS, 1.f, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x93bc50 (2013 0x9385f0). */
void CAkBus::UnmuteAll( AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	Unmute( NULL, in_eFadeCurve, in_lTransitionTime );
}

/** 2012 0x93bbb0 (2013 0x938550). */
void CAkBus::ResetFXBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	if( m_pGlobalSIS )
	{
		m_pGlobalSIS->m_bitsFXBypass = (AkUInt8)( in_bitsFXBypass | ( m_pGlobalSIS->m_bitsFXBypass & ~in_uTargetMask ) );
	}
}

/** 2012 0x93bdf0 (2013 0x938790): a volume change of a bus with its own mix bus node (effect or environmental) is a
    volume offset on that node; everything else goes to the children that play or are active. */
void CAkBus::ParamNotification( NotifParams & in_rParams )
{
	in_rParams.bIsFromBus = true;
	if( in_rParams.eType == RTPC_Volume && HasEffect() )
	{
		if( this == s_pMasterBus )
		{
			CAkLEngine::SetMasterBusVolume( in_rParams.fValue );
		}
		else
		{
			CAkLEngine::SetBusVolume( key, in_rParams.fValue );
		}
		return;
	}
	if( !m_pActivityChunk || ( !m_pActivityChunk->m_PlayCount && !m_pActivityChunk->m_uActivityCount ) )
	{
		return;
	}
	for( CAkParameterNodeBase ** pp = m_mapChildId.m_pItems; pp != m_mapChildId.m_pItemsEnd; ++pp )
	{
		const AkActivityChunk * pChunk = ( *pp )->m_pActivityChunk;
		if( pChunk && ( pChunk->m_PlayCount || pChunk->m_uActivityCount ) )
		{
			( *pp )->ParamNotification( in_rParams );
		}
	}
}

/** 2012 0x93bec0 (2013 0x938860). */
void CAkBus::MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus )
{
	for( CAkParameterNodeBase ** pp = m_mapChildId.m_pItems; pp != m_mapChildId.m_pItemsEnd; ++pp )
	{
		if( ( *pp )->IsPlaying() )
		{
			( *pp )->MuteNotification( in_fMuteRatio, in_rMutedItem, true );
		}
	}
}

/** 2012 0x93bf10 (2013 0x9388b0): the bus's effects are the lower engine's; an environmental bus's bypass is global. */
void CAkBus::NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	if( this == s_pMasterBus )
	{
		CAkLEngine::BypassMasterBusFx( in_bitsFXBypass, in_uTargetMask );
		return;
	}
	CAkLEngine::BypassBusFx( key, in_bitsFXBypass, in_uTargetMask );
	if( m_bIsEnvironmental )
	{
		g_pEnvironmentMgr->m_bEnvironmentalBypass = ( in_bitsFXBypass & 0x11 ) != 0;
	}
}

/** 2012 0x93cac0 (2013 0x939460): a bus with its own mix bus node gets its volume recomputed there. */
void CAkBus::RecalcNotification()
{
	if( HasEffect() )
	{
		const AkReal32 fVolume = GetBusEffectiveVolume( BusVolumeType_ToNextBusWithEffect, AkPropID_Volume );
		if( this == s_pMasterBus )
		{
			CAkLEngine::ForceMasterBusVolume( fVolume );
		}
		else
		{
			CAkLEngine::ForceBusVolume( key, fVolume );
		}
	}
	for( CAkParameterNodeBase ** pp = m_mapChildId.m_pItems; pp != m_mapChildId.m_pItemsEnd; ++pp )
	{
		const AkActivityChunk * pChunk = ( *pp )->m_pActivityChunk;
		if( pChunk && ( pChunk->m_PlayCount || pChunk->m_uActivityCount ) )
		{
			( *pp )->RecalcNotification();
		}
	}
}

/** 2012 0x93c020 (2013 0x9389c0). See the banner. */
void CAkBus::UpdateFx( AkUInt32 in_uFXIndex )
{
}

/** 2012 0x93b7a0 (2013 0x938140): a bus has the global limit only. */
void CAkBus::ApplyMaxNumInstances( AkUInt16 in_u16MaxNumInstance, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray, bool in_bFromRTPC )
{
	if( in_pGameObj || ( in_bFromRTPC && !m_u16MaxNumInstance ) )
	{
		return;
	}
	if( m_pActivityChunk )
	{
		m_pActivityChunk->m_Limiter.m_u16LimiterMax = in_u16MaxNumInstance;
	}
	if( !in_bFromRTPC )
	{
		m_u16MaxNumInstance = in_u16MaxNumInstance;
	}
}

/** 2012 0x93b7e0 (2013 0x938180). */
void CAkBus::IncrementVirtualCount( CounterParameters & io_params )
{
	if( m_bIsMaxNumInstOverrideParent || !m_pBusOutputNode )
	{
		if( !io_params.bMaxConsidered && m_pActivityChunk )
		{
			++m_pActivityChunk->m_iVirtualCountValid;
		}
		io_params.bMaxConsidered = true;
	}
	if( m_pBusOutputNode )
	{
		m_pBusOutputNode->IncrementVirtualCount( io_params );
	}
	if( m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		m_pFeedbackInfo->m_pFeedbackBus->IncrementVirtualCount( io_params );
	}
}

/** 2012 0x93b840 (2013 0x9381e0). */
void CAkBus::DecrementVirtualCount( CounterParameters & io_params )
{
	if( m_bIsMaxNumInstOverrideParent || !m_pBusOutputNode )
	{
		if( !io_params.bMaxConsidered )
		{
			DecrementVirtualCountGlobal( io_params.ui16NumKicked, io_params.bAllowKick );
			io_params.bMaxConsidered = true;
		}
	}
	if( m_pBusOutputNode )
	{
		m_pBusOutputNode->DecrementVirtualCount( io_params );
	}
	if( m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		m_pFeedbackInfo->m_pFeedbackBus->DecrementVirtualCount( io_params );
	}
}

/** 2012 0x93ba40 (2013 0x9383e0). */
CAkBus * CAkBus::GetLimitingBus()
{
	return m_bIsMaxNumInstOverrideParent ? this : CAkParameterNodeBase::GetLimitingBus();
}

/** 2012 0x93b980 (2013 0x938320). */
bool CAkBus::IsOrIsChildOf( AkUniqueID in_NodeIDToTest )
{
	for( CAkParameterNodeBase * pBus = this; pBus; pBus = pBus->m_pBusOutputNode )
	{
		if( in_NodeIDToTest == pBus->key )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x93ba90 (2013 0x938430). */
bool CAkBus::GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes )
{
	for( CAkParameterNodeBase * pBus = this; pBus; pBus = pBus->m_pBusOutputNode )
	{
		if( pBus->CheckSyncTypes( in_stateGroupID, io_pSyncTypes ) )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x93bad0 (2013 0x938470). */
bool CAkBus::GetBypassFX( AkUInt32 in_uFXIndex )
{
	if( !m_pFXChunk )
	{
		return false;
	}
	if( m_pFXChunk->aFX[ in_uFXIndex ].id && m_RTPCBitArrayMax32.IsSet( RTPC_BypassFX0 + in_uFXIndex ) )
	{
		return g_pRTPCMgr->GetRTPCConvertedValue( this, RTPC_BypassFX0 + in_uFXIndex, NULL ) != 0.f;
	}
	if( m_pGlobalSIS )
	{
		return ( m_pGlobalSIS->m_bitsFXBypass >> in_uFXIndex ) & 1;
	}
	return ( m_pFXChunk->bitsMainFXBypass >> in_uFXIndex ) & 1;
}

/** 2012 0x93bf70 (2013 0x938910): retail scans for a duck still below -0.01 dB and does nothing with the answer. */
void CAkBus::CheckDuck()
{
	for( CAkKeyList<AkUniqueID, CAkDuckItem, 1, ArrayPoolDefault>::ListItem * p = m_DuckedVolumeList.m_pFirst; p; p = p->pNextListItem )
	{
		if( p->Item.item.m_EffectiveVolumeOffset < -0.0099999998f )
		{
			break;
		}
	}
}

/** 2012 0x93bf90 (2013 0x938930): the sum of the ducks, no lower than the bus's maximum duck volume. */
AkReal32 CAkBus::GetDuckedVolume()
{
	AkReal32 fDuckedVolume = 0.f;
	for( CAkKeyList<AkUniqueID, CAkDuckItem, 1, ArrayPoolDefault>::ListItem * p = m_DuckedVolumeList.m_pFirst; p; p = p->pNextListItem )
	{
		fDuckedVolume += p->Item.item.m_EffectiveVolumeOffset;
	}
	return m_fMaxDuckVolume > fDuckedVolume ? m_fMaxDuckVolume : fDuckedVolume;
}

/** 2012 0x93b6b0 (2013 0x938050): a running duck fade is retargeted, else one starts (or the value is set at once). */
void CAkBus::StartDuckTransitions( CAkDuckItem * in_pDuckItem, AkReal32 in_fTargetValue, AkValueMeaning in_eValueMeaning, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	if( in_pDuckItem->m_pvVolumeTransition )
	{
		g_pTransitionManager->ChangeParameter( in_pDuckItem->m_pvVolumeTransition, TransTarget_Volume, in_fTargetValue, in_lTransitionTime, in_eFadeCurve, in_eValueMeaning );
		return;
	}
	const AkReal32 fStartValue = in_pDuckItem->m_EffectiveVolumeOffset;
	const AkReal32 fTargetValue = ( in_eValueMeaning == AkValueMeaning_Independent || in_eValueMeaning == AkValueMeaning_Offset ) ? in_fTargetValue : 0.f;
	if( fStartValue == fTargetValue || !in_lTransitionTime )
	{
		in_pDuckItem->TransUpdateValue( TransTarget_Volume, fTargetValue, true );
		return;
	}
	TransitionParameters params;
	params.pUser = in_pDuckItem;
	params.eTarget = TransTarget_Volume;
	params.fStartValue = fStartValue;
	params.fTargetValue = fTargetValue;
	params.lDuration = in_lTransitionTime;
	params.eFadeCurve = in_eFadeCurve;
	params.bdBs = true;
	params.bUseReciprocalCurve = true;
	in_pDuckItem->m_pvVolumeTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
}

/** 2012 0x93d420 (2013 0x939dc0): bus in_BusID ducks this bus. */
AKRESULT CAkBus::Duck( AkUniqueID in_BusID, AkReal32 in_DuckVolume, AkTimeMs in_FadeOutTime, AkCurveInterpolation in_eFadeCurve )
{
	CAkDuckItem * pDuckItem = m_DuckedVolumeList.Exists( in_BusID );
	if( !pDuckItem )
	{
		pDuckItem = m_DuckedVolumeList.Set( in_BusID );
		if( !pDuckItem )
		{
			return AK_Fail;
		}
		pDuckItem->Init( this );
	}
	StartDuckTransitions( pDuckItem, in_DuckVolume, AkValueMeaning_Offset, in_eFadeCurve, in_FadeOutTime );
	return AK_Success;
}

/** 2012 0x93c2b0 (2013 0x938c50). */
AKRESULT CAkBus::Unduck( AkUniqueID in_BusID, AkTimeMs in_FadeInTime, AkCurveInterpolation in_eFadeCurve )
{
	CAkDuckItem * pDuckItem = m_DuckedVolumeList.Exists( in_BusID );
	if( pDuckItem )
	{
		StartDuckTransitions( pDuckItem, 0.f, AkValueMeaning_Default, in_eFadeCurve, in_FadeInTime );
	}
	return AK_Success;
}

/** 2012 0x93c300 (2013 0x938ca0): the duck holds where it is while the recovery time runs. */
AKRESULT CAkBus::PauseDuck( AkUniqueID in_BusID )
{
	CAkDuckItem * pDuckItem = m_DuckedVolumeList.Exists( in_BusID );
	if( !pDuckItem )
	{
		return AK_Success;
	}
	if( pDuckItem->m_pvVolumeTransition )
	{
		g_pTransitionManager->ChangeParameter( pDuckItem->m_pvVolumeTransition, TransTarget_Volume, pDuckItem->m_EffectiveVolumeOffset, 0, AkCurveInterpolation_Linear, AkValueMeaning_Independent );
	}
	else
	{
		pDuckItem->TransUpdateValue( TransTarget_Volume, pDuckItem->m_EffectiveVolumeOffset, true );
	}
	return AK_Success;
}

/** 2012 0x93d490 (2013 0x939e30): every bus this bus ducks follows its ducking state. */
void CAkBus::UpdateDuckedBus()
{
	for( CAkKeyList<AkUniqueID, AkDuckInfo, 1, ArrayPoolDefault>::ListItem * p = m_ToDuckList.m_pFirst; p; p = p->pNextListItem )
	{
		CAkBus * pDuckedBus = static_cast<CAkBus *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item.key ) );
		if( !pDuckedBus )
		{
			continue;
		}
		const AkDuckInfo & rInfo = p->Item.item;
		switch( m_eDuckingState )
		{
		case DuckState_OFF:
			pDuckedBus->Unduck( key, rInfo.FadeInTime, rInfo.FadeCurve );
			break;
		case DuckState_ON:
			pDuckedBus->Duck( key, rInfo.DuckVolume, rInfo.FadeOutTime, rInfo.FadeCurve );
			break;
		case DuckState_PENDING:
			pDuckedBus->PauseDuck( key );
			break;
		}
		pDuckedBus->Release();
	}
}

/** 2012 0x93b9a0 (2013 0x938340): a duck action delayed by the recovery time ends the pending state. */
AKRESULT CAkBus::RequestDuckNotif()
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkActionDuck ) );
	if( !pMem )
	{
		return AK_Fail;
	}
	CAkActionDuck * pAction = AkPlacementNew( pMem ) CAkActionDuck( AkActionType_Duck, 0 );
	AKRESULT eResult = AK_Fail;
	if( pAction->SetAkProp( AkPropID_DelayTime, (AkInt32)m_RecoveryTime, 0, 0 ) == AK_Success )
	{
		pAction->SetElementID( key );
		void * pPendingMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkPendingAction ) );
		if( pPendingMem )
		{
			AkPendingAction * pPending = AkPlacementNew( pPendingMem ) AkPendingAction( NULL );
			pPending->pAction = pAction;
			g_pAudioMgr->EnqueueOrExecuteAction( pPending );
			eResult = AK_Success;
		}
	}
	pAction->Release();
	return eResult;
}

/** 2012 0x93da90 (2013 0x939e70): the recovery time is over. */
void CAkBus::DuckNotif()
{
	if( m_eDuckingState == DuckState_PENDING )
	{
		m_eDuckingState = DuckState_OFF;
		UpdateDuckedBus();
	}
}
