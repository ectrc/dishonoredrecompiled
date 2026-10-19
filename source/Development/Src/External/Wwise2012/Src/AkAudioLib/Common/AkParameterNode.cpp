/*=============================================================================
	AkParameterNode.cpp - the runtime half of CAkParameterNode (akparameternode.cpp), CAkSoundBase (aksoundbase.cpp),
	CAkSound's seek (aksound.cpp), CAkActiveParent<T>'s notification forwarding (akactiveparent.h) and the parameter
	half of CAkPBI (akpbi.cpp): SetValue / Mute through the SIS, the virtual voice counts, layer association, and the
	value notifications from a node down to its playing PBIs.

	DISHONORED(port): agent IH3 fork N (wave 26); 2012 rva on each definition, 2013 from ih1_inventory.csv.
	Deviations (named):
	  * CAkPBI::UpdateFx refreshes the voice's bypass bits only: swapping an insert effect (CAkVPLSrcCbxNode::UpdateFx)
	    is IH4's, as is every effect instance (AddPipeline leaves the filter slots empty).
	  * CAkParameterNode::PlayAndContinueAlternate, AssociateLayer / DissociateLayer (AkLayer.cpp) and GetPathState,
	    CAkSoundBase / CAkActiveParent PositioningChangeNotification (AkPositioning.cpp) are the containers' and the
	    positioning's (IH3 forks C, P); GetFX / GetFXDataID wait for IH4's insert effects (nothing consumes an AkFXDesc
	    before them).
=============================================================================*/
#include "AkHierarchy.h"
#include "AkActivityChunk.h"
#include "AkRegistryMgr.h"
#include "AkVPLSrcCbxNode.h"
#include "AkBusCtx.h"

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNode
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9393b0 (2013 0x935d40): one SIS per game object (registered as modified on the object), or the global one. */
CAkSIS * CAkParameterNode::GetSIS( CAkRegisteredObj * in_GameObjPtr )
{
	const AkUInt8 bitsFXBypass = m_pFXChunk ? m_pFXChunk->bitsMainFXBypass : 0;
	if( !in_GameObjPtr )
	{
		g_pRegistryMgr->SetNodeIDAsModified( this );
		if( !m_pGlobalSIS )
		{
			void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSIS ) );
			m_pGlobalSIS = pMem ? AkPlacementNew( pMem ) CAkSIS( this, NULL, bitsFXBypass ) : NULL;
		}
		return m_pGlobalSIS;
	}
	if( !m_pMapSIS )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkKeyArray<CAkRegisteredObj *, CAkSIS *, 8> ) );
		if( !pMem )
		{
			return NULL;
		}
		m_pMapSIS = AkPlacementNew( pMem ) CAkKeyArray<CAkRegisteredObj *, CAkSIS *, 8>();
	}
	CAkSIS ** ppSIS = m_pMapSIS->Exists( in_GameObjPtr );
	if( ppSIS )
	{
		return *ppSIS;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSIS ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSIS * pSIS = AkPlacementNew( pMem ) CAkSIS( this, in_GameObjPtr, bitsFXBypass );
	if( m_pMapSIS->Set( in_GameObjPtr, pSIS ) )
	{
		if( in_GameObjPtr->SetNodeAsModified( this ) == AK_Success )
		{
			return pSIS;
		}
		m_pMapSIS->Unset( in_GameObjPtr );
	}
	pSIS->~CAkSIS();
	AK::MemoryMgr::Free( g_DefaultPoolId, pSIS );
	return NULL;
}

/** 2012 0x9376f0 (2013 0x933fb0). */
void CAkParameterNode::SetAkProp( AkPropID in_eProp, CAkRegisteredObj * in_GameObjPtr, AkValueMeaning in_eValueMeaning, AkReal32 in_fTargetValue, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	CAkSIS * pSIS = GetSIS( in_GameObjPtr );
	if( pSIS )
	{
		StartSISTransition( pSIS, in_eProp, in_fTargetValue, in_eValueMeaning, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x9391a0 (2013 0x935b30): the authoring path; volume / pitch / LPF / LFE and the feedback values notify the playing
    voices with the delta, the panning values through the positioning path. */
void CAkParameterNode::SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax )
{
	if( in_eProp < AkPropID_Priority || (AkUInt32)( in_eProp - AkPropID_FeedbackVolume ) <= 1 )
	{
		const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
		const AkReal32 fDelta = in_fValue - ( pProp ? pProp->fValue : 0.f );
		if( fDelta != 0.f )
		{
			Notification( g_AkPropRTPCID[ in_eProp ], fDelta, NULL, NULL );
			m_props.SetAkProp( (AkUInt8)in_eProp, AkPropValue( in_fValue ) );
		}
	}
	else if( (AkUInt32)( in_eProp - AkPropID_PAN_LR ) <= 2 )
	{
		const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
		if( in_fValue - ( pProp ? pProp->fValue : 0.f ) != 0.f )
		{
			PositioningChangeNotification( in_fValue, g_AkPropRTPCID[ in_eProp ], NULL, NULL );
			m_props.SetAkProp( (AkUInt8)in_eProp, AkPropValue( in_fValue ) );
		}
	}
	else
	{
		CAkParameterNodeBase::SetAkProp( in_eProp, in_fValue, in_fMin, in_fMax );
	}
	if( in_fMin != 0.f || in_fMax != 0.f || m_ranges.FindProp( (AkUInt8)in_eProp ) )
	{
		RANGED_MODIFIERS<AkPropValue> range;
		range.m_min = AkPropValue( in_fMin );
		range.m_max = AkPropValue( in_fMax );
		m_ranges.SetAkProp( (AkUInt8)in_eProp, range );
	}
}

/** 2012 0x939360 (2013 0x935cf0). */
void CAkParameterNode::SetAkProp( AkPropID in_eProp, AkInt32 in_iValue, AkInt32 in_iMin, AkInt32 in_iMax )
{
	CAkParameterNodeBase::SetAkProp( in_eProp, in_iValue, in_iMin, in_iMax );
	if( in_iMin || in_iMax || m_ranges.FindProp( (AkUInt8)in_eProp ) )
	{
		RANGED_MODIFIERS<AkPropValue> range;
		range.m_min.iValue = in_iMin;
		range.m_max.iValue = in_iMax;
		m_ranges.SetAkProp( (AkUInt8)in_eProp, range );
	}
}

/** 2012 0x938770 (2013 0x935070): every per-object value snaps back at once (curve 4, no fade), the global one fades. */
void CAkParameterNode::ResetAkProp( AkPropID in_eProp, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	if( m_pMapSIS )
	{
		for( MapStruct<CAkRegisteredObj *, CAkSIS *> * p = m_pMapSIS->m_pItems; p != m_pMapSIS->m_pItemsEnd; ++p )
		{
			const AkSISValue * pValue = p->item->m_values.FindProp( (AkUInt8)in_eProp );
			if( pValue && pValue->fValue != 0.f )
			{
				SetAkProp( in_eProp, p->item->m_pGameObj, AkValueMeaning_Default, 0.f, AkCurveInterpolation_Linear, 0 );
			}
		}
	}
	if( m_pGlobalSIS )
	{
		const AkSISValue * pValue = m_pGlobalSIS->m_values.FindProp( (AkUInt8)in_eProp );
		if( pValue && pValue->fValue != 0.f )
		{
			SetAkProp( in_eProp, (CAkRegisteredObj *)NULL, AkValueMeaning_Default, 0.f, in_eFadeCurve, in_lTransitionTime );
		}
	}
}

/** 2012 0x937730 (2013 0x933ff0). */
void CAkParameterNode::Mute( CAkRegisteredObj * in_GameObjPtr, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	CAkSIS * pSIS = GetSIS( in_GameObjPtr );
	if( pSIS )
	{
		StartSisMuteTransitions( pSIS, 0.f, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x9385e0 (2013 0x934ee0): only an existing SIS that is muted fades back. */
void CAkParameterNode::Unmute( CAkRegisteredObj * in_GameObjPtr, AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	CAkSIS * pSIS = NULL;
	if( in_GameObjPtr )
	{
		CAkSIS ** ppSIS = m_pMapSIS ? m_pMapSIS->Exists( in_GameObjPtr ) : NULL;
		pSIS = ppSIS ? *ppSIS : NULL;
	}
	else if( m_pGlobalSIS )
	{
		const AkSISValue * pValue = m_pGlobalSIS->m_values.FindProp( AkPropID_MuteRatio );
		if( pValue && pValue->fValue != 1.f )
		{
			g_pRegistryMgr->SetNodeIDAsModified( this );
			pSIS = m_pGlobalSIS;
		}
	}
	if( pSIS )
	{
		StartSisMuteTransitions( pSIS, 1.f, in_eFadeCurve, in_lTransitionTime );
	}
}

/** 2012 0x937770 (2013 0x934030). */
void CAkParameterNode::UnmuteAll( AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	Unmute( NULL, in_eFadeCurve, in_lTransitionTime );
	UnmuteAllObj( in_eFadeCurve, in_lTransitionTime );
}

/** 2012 0x938670 (2013 0x934f70). */
void CAkParameterNode::UnmuteAllObj( AkCurveInterpolation in_eFadeCurve, AkTimeMs in_lTransitionTime )
{
	if( !m_pMapSIS )
	{
		return;
	}
	for( MapStruct<CAkRegisteredObj *, CAkSIS *> * p = m_pMapSIS->m_pItems; p != m_pMapSIS->m_pItemsEnd; ++p )
	{
		const AkSISValue * pValue = p->item->m_values.FindProp( AkPropID_MuteRatio );
		if( pValue && pValue->fValue != 1.f )
		{
			Unmute( p->item->m_pGameObj, in_eFadeCurve, in_lTransitionTime );
		}
	}
}

/** 2012 0x9386f0 (2013 0x934ff0): an unregistered game object's SIS goes. */
void CAkParameterNode::Unregister( CAkRegisteredObj * in_GameObjPtr )
{
	if( !m_pMapSIS )
	{
		return;
	}
	MapStruct<CAkRegisteredObj *, CAkSIS *> * p = m_pMapSIS->m_pItems;
	while( p != m_pMapSIS->m_pItemsEnd )
	{
		if( p->key != in_GameObjPtr )
		{
			++p;
			continue;
		}
		if( p->item )
		{
			p->item->~CAkSIS();
			AK::MemoryMgr::Free( g_DefaultPoolId, p->item );
		}
		m_pMapSIS->Erase( (AkUInt32)( p - m_pMapSIS->m_pItems ) );
	}
}

/** 2012 0x938350 (2013 0x934b60). */
void CAkParameterNode::ResetFXBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask )
{
	if( m_pGlobalSIS )
	{
		m_pGlobalSIS->m_bitsFXBypass = (AkUInt8)( in_bitsFXBypass | ( m_pGlobalSIS->m_bitsFXBypass & ~in_uTargetMask ) );
	}
	if( m_pMapSIS )
	{
		for( MapStruct<CAkRegisteredObj *, CAkSIS *> * p = m_pMapSIS->m_pItems; p != m_pMapSIS->m_pItemsEnd; ++p )
		{
			p->item->m_bitsFXBypass = (AkUInt8)( in_bitsFXBypass | ( p->item->m_bitsFXBypass & ~in_uTargetMask ) );
		}
	}
}

/** 2012 0x9377a0 (2013 0x934060): a node under an excepted parent is skipped. */
AKRESULT CAkParameterNode::ExecuteActionExceptParentCheck( ActionParamsExcept & in_rAction )
{
	for( CAkParameterNodeBase * pParent = m_pParentNode; pParent; pParent = pParent->m_pParentNode )
	{
		if( IsException( pParent->key, *in_rAction.pExeceptionList ) )
		{
			return AK_Success;
		}
	}
	return ExecuteActionExcept( in_rAction );
}

/** 2012 0x937ab0 (2013 0x934370). */
void CAkParameterNode::IncrementVirtualCount( CounterParameters & io_params )
{
	if( ( m_bIsMaxNumInstOverrideParent || !m_pParentNode ) && !io_params.bMaxConsidered )
	{
		if( m_pActivityChunk )
		{
			if( m_pActivityChunk->m_bIsGlobalLimit )
			{
				++m_pActivityChunk->m_iVirtualCountValid;
			}
			else
			{
				IncrementVirtualCountGameObject( io_params.pGameObj );
			}
		}
		io_params.bMaxConsidered = true;
	}
	const bool bMaxConsideredBackup = io_params.bMaxConsidered;
	if( ( io_params.uiFlagForwardToBus & 1 ) && m_pBusOutputNode )
	{
		io_params.uiFlagForwardToBus &= ~1;
		io_params.bMaxConsidered = false;
		m_pBusOutputNode->IncrementVirtualCount( io_params );
	}
	if( ( io_params.uiFlagForwardToBus & 2 ) && m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		io_params.uiFlagForwardToBus &= ~2;
		io_params.bMaxConsidered = false;
		m_pFeedbackInfo->m_pFeedbackBus->IncrementVirtualCount( io_params );
	}
	if( m_pParentNode )
	{
		io_params.bMaxConsidered = bMaxConsideredBackup;
		m_pParentNode->IncrementVirtualCount( io_params );
	}
}

/** 2012 0x937b80 (2013 0x934440). */
void CAkParameterNode::DecrementVirtualCount( CounterParameters & io_params )
{
	if( m_bIsMaxNumInstOverrideParent || !m_pParentNode )
	{
		if( !io_params.bMaxConsidered && m_pActivityChunk )
		{
			if( m_pActivityChunk->m_bIsGlobalLimit )
			{
				DecrementVirtualCountGlobal( io_params.ui16NumKicked, io_params.bAllowKick );
			}
			else
			{
				DecrementVirtualCountGameObject( io_params.ui16NumKicked, io_params.bAllowKick, io_params.pGameObj );
			}
		}
		io_params.bMaxConsidered = true;
	}
	const bool bMaxConsideredBackup = io_params.bMaxConsidered;
	if( ( io_params.uiFlagForwardToBus & 1 ) && m_pBusOutputNode )
	{
		io_params.uiFlagForwardToBus &= ~1;
		io_params.bMaxConsidered = false;
		m_pBusOutputNode->DecrementVirtualCount( io_params );
	}
	if( ( io_params.uiFlagForwardToBus & 2 ) && m_pFeedbackInfo && m_pFeedbackInfo->m_pFeedbackBus )
	{
		io_params.uiFlagForwardToBus &= ~2;
		io_params.bMaxConsidered = false;
		m_pFeedbackInfo->m_pFeedbackBus->DecrementVirtualCount( io_params );
	}
	if( m_pParentNode )
	{
		io_params.bMaxConsidered = bMaxConsideredBackup;
		m_pParentNode->DecrementVirtualCount( io_params );
	}
}

/** 2012 0x939660 (2013 0x935ff0): an RTPC-driven limit changes the limiters only (one object, every object but the
    excepted ones, or the global one); the authored limit also becomes the node's. */
void CAkParameterNode::ApplyMaxNumInstances( AkUInt16 in_u16MaxNumInstance, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray, bool in_bFromRTPC )
{
	if( !in_bFromRTPC )
	{
		if( m_pActivityChunk )
		{
			if( m_pActivityChunk->m_bIsGlobalLimit )
			{
				m_pActivityChunk->m_Limiter.m_u16LimiterMax = in_u16MaxNumInstance;
				m_u16MaxNumInstance = in_u16MaxNumInstance;
				return;
			}
			for( MapStruct<CAkRegisteredObj *, StructMaxInst> * p = m_pActivityChunk->m_ListPlayCountPerObj.m_pItems; p != m_pActivityChunk->m_ListPlayCountPerObj.m_pItemsEnd; ++p )
			{
				if( p->item.m_pLimiter )
				{
					p->item.m_pLimiter->m_u16LimiterMax = in_u16MaxNumInstance;
				}
			}
		}
		m_u16MaxNumInstance = in_u16MaxNumInstance;
		return;
	}
	if( !m_pActivityChunk )
	{
		return;
	}
	if( in_pGameObj )
	{
		if( !m_pActivityChunk->m_bIsGlobalLimit && ( m_bIsMaxNumInstOverrideParent || !m_pParentNode ) )
		{
			StructMaxInst * pPerObj = m_pActivityChunk->m_ListPlayCountPerObj.Exists( in_pGameObj );
			if( pPerObj && pPerObj->m_pLimiter )
			{
				pPerObj->m_pLimiter->m_u16LimiterMax = in_u16MaxNumInstance;
			}
		}
		return;
	}
	if( m_pActivityChunk->m_bIsGlobalLimit )
	{
		m_pActivityChunk->m_Limiter.m_u16LimiterMax = in_u16MaxNumInstance;
		return;
	}
	const GameObjExceptArray * pExceptArray = (const GameObjExceptArray *)in_pExceptArray;
	for( MapStruct<CAkRegisteredObj *, StructMaxInst> * p = m_pActivityChunk->m_ListPlayCountPerObj.m_pItems; p != m_pActivityChunk->m_ListPlayCountPerObj.m_pItemsEnd; ++p )
	{
		if( pExceptArray && pExceptArray->Exists( p->key ) )
		{
			continue;
		}
		if( p->item.m_pLimiter )
		{
			p->item.m_pLimiter->m_u16LimiterMax = in_u16MaxNumInstance;
		}
	}
}

/** 2012 0x939510 (2013 0x935ea0): an unset panning RTPC returns the voices to the authored pan. */
void CAkParameterNode::UnsetRTPC( AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID )
{
	CAkParameterNodeBase::UnsetRTPC( in_ParamID, in_RTPCCurveID );
	if( !m_p3DParameters )
	{
		return;
	}
	AkPropID ePropID;
	if( in_ParamID == RTPC_Position_PAN_RL )
	{
		ePropID = AkPropID_PAN_LR;
	}
	else if( in_ParamID == RTPC_Position_PAN_FR )
	{
		ePropID = AkPropID_PAN_FR;
	}
	else
	{
		return;
	}
	const AkPropValue * pProp = m_props.FindProp( (AkUInt8)ePropID );
	PositioningChangeNotification( pProp ? pProp->fValue : g_AkPropDefault[ ePropID ].fValue, in_ParamID, NULL, NULL );
}

/** 2012 0x937850 (2013 0x934110): the panner follows RTPC when either pan has a curve. */
void CAkParameterNode::NotifyRTPCChanged()
{
	if( m_p3DParameters )
	{
		m_p3DParameters->m_Params.m_bIsPanningFromRTPC = m_RTPCBitArrayMax32.IsSet( RTPC_Position_PAN_RL ) || m_RTPCBitArrayMax32.IsSet( RTPC_Position_PAN_FR );
	}
}

/** 2012 0x937cb0 (2013 0x934570): this node, a parent, or (once) the first output bus chain. */
bool CAkParameterNode::IsOrIsChildOf( AkUniqueID in_NodeIDToTest, bool in_bIsBusChecked )
{
	bool bResult = false;
	bool bBusChecked = false;
	CAkParameterNodeBase * pNode = this;
	while( pNode )
	{
		bResult = in_NodeIDToTest == pNode->key;
		if( !bResult && !bBusChecked && pNode->m_pBusOutputNode )
		{
			bResult = static_cast<CAkBus *>( pNode->m_pBusOutputNode )->IsOrIsChildOf( in_NodeIDToTest );
			bBusChecked = true;
		}
		if( bResult )
		{
			break;
		}
		pNode = pNode->m_pParentNode;
	}
	return bResult;
}

/** 2012 0x938b90 (2013 0x935490): an effect with a bypass RTPC follows it, else the object's SIS, the global SIS, the node. */
bool CAkParameterNode::GetBypassFX( AkUInt32 in_uFXIndex, CAkRegisteredObj * in_GameObjPtr )
{
	if( !m_pFXChunk )
	{
		return false;
	}
	if( m_pFXChunk->aFX[ in_uFXIndex ].id && m_RTPCBitArrayMax32.IsSet( RTPC_BypassFX0 + in_uFXIndex ) )
	{
		return g_pRTPCMgr->GetRTPCConvertedValue( this, RTPC_BypassFX0 + in_uFXIndex, in_GameObjPtr ) != 0.f;
	}
	CAkSIS ** ppSIS = m_pMapSIS ? m_pMapSIS->Exists( in_GameObjPtr ) : NULL;
	if( ppSIS )
	{
		return ( ( *ppSIS )->m_bitsFXBypass >> in_uFXIndex ) & 1;
	}
	const AkUInt8 bits = m_pGlobalSIS ? m_pGlobalSIS->m_bitsFXBypass : m_pFXChunk->bitsMainFXBypass;
	return ( bits >> in_uFXIndex ) & 1;
}

// ------------------------------------------------------------------------------------------------------------
// CAkSoundBase: the PBIs of the sound
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x946940 (2013 0x9431d0): to every PBI of the game object, or (no object) of every object but the excepted ones. */
void CAkSoundBase::ParamNotification( NotifParams & in_rParams )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	const GameObjExceptArray * pExceptArray = (const GameObjExceptArray *)in_rParams.pExceptObjects;
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( in_rParams.pGameObj ? pPBI->GetGameObjectPtr() == in_rParams.pGameObj : !( pExceptArray && pExceptArray->Exists( pPBI->GetGameObjectPtr() ) ) )
		{
			pPBI->ParamNotification( in_rParams );
		}
	}
}

/** 2012 0x9469a0 (2013 0x943230). */
void CAkSoundBase::MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		pPBI->MuteNotification( in_fMuteRatio, in_rMutedItem, false );
	}
}

/** 2012 0x9469e0 (2013 0x943270). */
void CAkSoundBase::MuteNotification( AkReal32 in_fMuteRatio, CAkRegisteredObj * in_pGameObj, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( !in_pGameObj || pPBI->GetGameObjectPtr() == in_pGameObj )
		{
			pPBI->MuteNotification( in_fMuteRatio, in_rMutedItem, in_bPrioritizeGameObjectSpecificItems );
		}
	}
}

/** 2012 0x946a30 (2013 0x9432c0). */
void CAkSoundBase::ForAllPBI( void ( *in_funcForAll )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj * in_pGameObj, void * in_pCookie )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( !in_pGameObj || in_pGameObj == pPBI->GetGameObjectPtr() )
		{
			in_funcForAll( pPBI, in_pGameObj, in_pCookie );
		}
	}
}

/** 2012 0x946af0 (2013 0x943380). */
void CAkSoundBase::RecalcNotification()
{
	if( !m_pActivityChunk )
	{
		return;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		pPBI->RecalcNotification();
	}
}

/** 2012 0x946b10 (2013 0x9433a0). */
void CAkSoundBase::NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	const GameObjExceptArray * pExceptArray = (const GameObjExceptArray *)in_pExceptArray;
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( in_pGameObj ? pPBI->GetGameObjectPtr() == in_pGameObj : !( pExceptArray && pExceptArray->Exists( pPBI->GetGameObjectPtr() ) ) )
		{
			pPBI->NotifyBypass( in_bitsFXBypass, in_uTargetMask );
		}
	}
}

/** 2012 0x946b80 (2013 0x943410). */
void CAkSoundBase::UpdateFx( AkUInt32 in_uFXIndex )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		pPBI->UpdateFx( in_uFXIndex );
	}
}

/** 2012 0x94a7b0 (2013 0x946fe0): the matching PBIs seek to a clamped percentage or a non-negative time. */
void CAkSound::SeekSound( CAkRegisteredObj * in_pGameObj, const SeekActionParams & in_rSeekParams )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	const bool bSnap = in_rSeekParams.bSnapToNearestMarker;
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( ( in_pGameObj && in_pGameObj != pPBI->GetGameObjectPtr() ) || ( in_rSeekParams.playingID && in_rSeekParams.playingID != pPBI->GetPlayingID() ) )
		{
			continue;
		}
		if( in_rSeekParams.bIsSeekRelativeToDuration )
		{
			pPBI->SeekPercent( in_rSeekParams.fSeekPercent < 0.f ? 0.f : ( in_rSeekParams.fSeekPercent > 1.f ? 1.f : in_rSeekParams.fSeekPercent ), bSnap );
		}
		else
		{
			pPBI->SeekTimeAbsolute( in_rSeekParams.iSeekTime < 0 ? 0 : in_rSeekParams.iSeekTime, bSnap );
		}
	}
}

/** 2012 0x94a8a0 (2013 0x9470d0). */
void CAkSound::SeekSound( CAkRegisteredObj * in_pGameObj, const SeekActionParamsExcept & in_rSeekParams )
{
	if( !m_pActivityChunk )
	{
		return;
	}
	const bool bSnap = in_rSeekParams.bSnapToNearestMarker;
	for( CAkPBI * pPBI = m_pActivityChunk->m_listPBI.First(); pPBI; pPBI = pPBI->pNextLightItem )
	{
		if( in_pGameObj && pPBI->GetGameObjectPtr() != in_pGameObj )
		{
			continue;
		}
		if( in_rSeekParams.bIsSeekRelativeToDuration )
		{
			pPBI->SeekPercent( in_rSeekParams.fSeekPercent < 0.f ? 0.f : ( in_rSeekParams.fSeekPercent > 1.f ? 1.f : in_rSeekParams.fSeekPercent ), bSnap );
		}
		else
		{
			pPBI->SeekTimeAbsolute( in_rSeekParams.iSeekTime < 0 ? 0 : in_rSeekParams.iSeekTime, bSnap );
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkActiveParent<T>: forwarding to the playing children
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94dfb0 (2013 0x94a880): from a bus, a child routed to its own output bus is that bus's business. */
template <class T>
void CAkParentNode<T>::ParamNotification( NotifParams & in_rParams )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		CAkParameterNodeBase * pChild = *pp;
		if( ( !in_rParams.bIsFromBus || !pChild->m_pBusOutputNode ) && pChild->IsPlaying() )
		{
			pChild->ParamNotification( in_rParams );
		}
	}
}

/** 2012 0x94e000 / 0x93cbb0 (2013 0x94a8d0 / 0x939550): children overriding the priority keep their own. */
template <class T>
void CAkParentNode<T>::PriorityNotification( NotifParams & in_rParams )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		CAkParameterNodeBase * pChild = *pp;
		if( !pChild->m_bPriorityOverrideParent && pChild->IsPlaying() )
		{
			pChild->PriorityNotification( in_rParams );
		}
	}
}

/** 2012 0x94e040 (2013 0x94a910). */
template <class T>
void CAkParentNode<T>::MuteNotification( AkReal32 in_fMuteRatio, AkMutedMapItem & in_rMutedItem, bool in_bIsFromBus )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		CAkParameterNodeBase * pChild = *pp;
		if( ( !in_bIsFromBus || !pChild->m_pBusOutputNode ) && pChild->IsPlaying() )
		{
			pChild->MuteNotification( in_fMuteRatio, in_rMutedItem, in_bIsFromBus );
		}
	}
}

/** 2012 0x94e0a0 / 0x93cbf0 (2013 0x94a970 / 0x939590). */
template <class T>
void CAkParentNode<T>::MuteNotification( AkReal32 in_fMuteRatio, CAkRegisteredObj * in_pGameObj, AkMutedMapItem & in_rMutedItem, bool in_bPrioritizeGameObjectSpecificItems )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		if( ( *pp )->IsPlaying() )
		{
			( *pp )->MuteNotification( in_fMuteRatio, in_pGameObj, in_rMutedItem, in_bPrioritizeGameObjectSpecificItems );
		}
	}
}

/** 2012 0x94e0f0 / 0x93cc40 (2013 0x94a9c0 / 0x9395e0). */
template <class T>
void CAkParentNode<T>::ForAllPBI( void ( *in_funcForAll )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj * in_pGameObj, void * in_pCookie )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		if( ( *pp )->IsPlaying() )
		{
			( *pp )->ForAllPBI( in_funcForAll, in_pGameObj, in_pCookie );
		}
	}
}

/** 2012 0x94e1a0 (2013 0x94aa70). */
template <class T>
void CAkParentNode<T>::RecalcNotification()
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		if( ( *pp )->IsPlaying() )
		{
			( *pp )->RecalcNotification();
		}
	}
}

/** 2012 0x94e1e0 (2013 0x94aab0): children overriding the effects keep their own bypass. */
template <class T>
void CAkParentNode<T>::NotifyBypass( AkUInt32 in_bitsFXBypass, AkUInt32 in_uTargetMask, CAkRegisteredObj * in_pGameObj, void * in_pExceptArray )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		CAkParameterNodeBase * pChild = *pp;
		if( pChild->IsPlaying() && !pChild->m_bIsFXOverrideParent )
		{
			pChild->NotifyBypass( in_bitsFXBypass, in_uTargetMask, in_pGameObj, in_pExceptArray );
		}
	}
}

/** 2012 0x94e2d0 (2013 0x94aba0). */
template <class T>
void CAkParentNode<T>::UpdateFx( AkUInt32 in_uFXIndex )
{
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		if( ( *pp )->IsPlaying() )
		{
			( *pp )->UpdateFx( in_uFXIndex );
		}
	}
}

/** 2012 0x94df60 / 0x93cb60 (2013 0x94a830 / 0x939500): every child, until one fails. */
template <class T>
AKRESULT CAkParentNode<T>::PlayToEnd( CAkRegisteredObj * in_pGameObj, AkUniqueID in_NodeID, AkPlayingID in_PlayingID )
{
	AKRESULT eResult = AK_Success;
	for( CAkParameterNodeBase ** pp = this->m_mapChildId.m_pItems; pp != this->m_mapChildId.m_pItemsEnd; ++pp )
	{
		eResult = ( *pp )->PlayToEnd( in_pGameObj, in_NodeID, in_PlayingID );
		if( eResult != AK_Success )
		{
			break;
		}
	}
	return eResult;
}

template void CAkParentNode<CAkParameterNode>::ParamNotification( NotifParams & );
template void CAkParentNode<CAkParameterNode>::PriorityNotification( NotifParams & );
template void CAkParentNode<CAkParameterNode>::MuteNotification( AkReal32, AkMutedMapItem &, bool );
template void CAkParentNode<CAkParameterNode>::MuteNotification( AkReal32, CAkRegisteredObj *, AkMutedMapItem &, bool );
template void CAkParentNode<CAkParameterNode>::ForAllPBI( void ( * )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj *, void * );
template void CAkParentNode<CAkParameterNode>::RecalcNotification();
template void CAkParentNode<CAkParameterNode>::NotifyBypass( AkUInt32, AkUInt32, CAkRegisteredObj *, void * );
template void CAkParentNode<CAkParameterNode>::UpdateFx( AkUInt32 );
template AKRESULT CAkParentNode<CAkParameterNode>::PlayToEnd( CAkRegisteredObj *, AkUniqueID, AkPlayingID );
template void CAkParentNode<CAkParameterNodeBase>::ParamNotification( NotifParams & );
template void CAkParentNode<CAkParameterNodeBase>::PriorityNotification( NotifParams & );
template void CAkParentNode<CAkParameterNodeBase>::MuteNotification( AkReal32, AkMutedMapItem &, bool );
template void CAkParentNode<CAkParameterNodeBase>::MuteNotification( AkReal32, CAkRegisteredObj *, AkMutedMapItem &, bool );
template void CAkParentNode<CAkParameterNodeBase>::ForAllPBI( void ( * )( CAkPBI *, CAkRegisteredObj *, void * ), CAkRegisteredObj *, void * );
template void CAkParentNode<CAkParameterNodeBase>::RecalcNotification();
template void CAkParentNode<CAkParameterNodeBase>::NotifyBypass( AkUInt32, AkUInt32, CAkRegisteredObj *, void * );
template void CAkParentNode<CAkParameterNodeBase>::UpdateFx( AkUInt32 );
template AKRESULT CAkParentNode<CAkParameterNodeBase>::PlayToEnd( CAkRegisteredObj *, AkUniqueID, AkPlayingID );

// ------------------------------------------------------------------------------------------------------------
// CAkPBI: the parameter half
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9402e0 (2013 0x93cc60): a node's value changed by a delta - volume (a bus volume is ignored by a voice that
    already mixes it through its own bus), LPF, pitch, priority (clamped 0..100); feedback values need a feedback voice. */
void CAkPBI::ParamNotification( NotifParams & in_rParams )
{
	switch( in_rParams.eType )
	{
	case RTPC_Volume:
		if( !m_bTargetIsFeedback || !in_rParams.bIsFromBus )
		{
			m_Volume += in_rParams.fValue;
			CalculateMutedEffectiveVolume();
		}
		return;
	case RTPC_Pitch:
		m_EffectiveParams.Pitch += in_rParams.fValue;
		return;
	case RTPC_LPF:
		m_LPF += in_rParams.fValue;
		m_EffectiveParams.LPF = m_LPFAutomationOffset + m_LPF;
		return;
	case RTPC_Priority:
	{
		AkReal32 fNewCurrent = m_PriorityInfoCurrent.priorityInfo.priority + in_rParams.fValue - m_PriorityInfoCurrent.currentPriority.priority;
		if( fNewCurrent < 0.f )
		{
			fNewCurrent = 0.f;
		}
		else if( fNewCurrent >= 100.f )
		{
			fNewCurrent = 100.f;
		}
		UpdatePriority( fNewCurrent );
		m_PriorityInfoCurrent.priorityInfo.priority = in_rParams.fValue;
		return;
	}
	case RTPC_FeedbackPitch:
		if( m_bTargetIsFeedback )
		{
			m_EffectiveParams.Pitch += in_rParams.fValue;
		}
		return;
	default:
		return;
	}
}

/** 2012 0x93f500 (2013 0x93be80). See the banner: the effect swap itself is IH4's. */
void CAkPBI::UpdateFx( AkUInt32 in_uFXIndex )
{
	if( m_pCbx )
	{
		m_pCbx->RefreshBypassFx();
	}
}

/** DISHONORED(IH3 -akdiag, see AkPBI.h): the node chain's authored volume, state values, RTPC and SIS, and the whole bus
    tree's effective volume (whatever part of it the voice or the mix bus nodes apply). */
void AkDiagVoiceParameters( CAkPBI * in_pPBI, bool in_bFirstRefresh )
{
	static AkUInt32 s_uLastTick = 0;
	const AkUInt32 uTick = g_pAudioMgr ? g_pAudioMgr->GetBufferTick() : 0;
	if( !in_bFirstRefresh && uTick - s_uLastTick < 47 )
	{
		return;
	}
	s_uLastTick = uTick;
	CAkRegisteredObj * pGameObj = in_pPBI->GetGameObjectPtr();
	AkReal32 fAuthored = 0.f;
	AkReal32 fStates = 0.f;
	AkReal32 fRTPC = 0.f;
	AkReal32 fSIS = 0.f;
	CAkParameterNodeBase * pBus = NULL;
	for( CAkParameterNodeBase * pNode = in_pPBI->GetSound(); pNode; pNode = pNode->m_pParentNode )
	{
		const AkPropValue * pProp = pNode->m_props.FindProp( AkPropID_Volume );
		fAuthored += pProp ? pProp->fValue : 0.f;
		if( pNode->m_bUseState )
		{
			for( AkStateGroupChunk * pChunk = pNode->m_states.First(); pChunk; pChunk = pChunk->m_pNextInNode )
			{
				const AkStateValue * pValue = pChunk->m_values.FindProp( AkPropID_Volume );
				fStates += pValue ? pValue->fValue : 0.f;
			}
		}
		if( pNode->m_RTPCBitArrayMax32.IsSet( RTPC_Volume ) )
		{
			fRTPC += g_pRTPCMgr->GetRTPCConvertedValue( pNode, RTPC_Volume, pGameObj );
		}
		if( pNode->m_pGlobalSIS )
		{
			const AkSISValue * pValue = pNode->m_pGlobalSIS->m_values.FindProp( AkPropID_Volume );
			fSIS += pValue ? pValue->fValue : 0.f;
		}
		CAkKeyArray<CAkRegisteredObj *, CAkSIS *, 8> * pMapSIS = static_cast<CAkParameterNode *>( pNode )->m_pMapSIS;
		CAkSIS ** ppSIS = pMapSIS ? pMapSIS->Exists( pGameObj ) : NULL;
		if( ppSIS )
		{
			const AkSISValue * pValue = ( *ppSIS )->m_values.FindProp( AkPropID_Volume );
			fSIS += pValue ? pValue->fValue : 0.f;
		}
		if( !pBus && pNode->m_pBusOutputNode )
		{
			pBus = pNode->m_pBusOutputNode;
		}
	}
	const AkReal32 fBusTree = pBus ? static_cast<CAkBus *>( pBus )->GetBusEffectiveVolume( BusVolumeType_IncludeEntireBusTree, AkPropID_Volume ) : 0.f;
	const AkSoundParams & rParams = in_pPBI->GetEffectiveParams();
	AKDIAG( L"akdiag: voice sound 0x%08x playing id %u: volume %.2f dB (effective %.2f) = node chain %.2f + states %.2f + RTPC %.2f + SIS %.2f (+ bus part); bus 0x%08x tree %.2f dB; pitch %.0f, LPF %.1f",
		in_pPBI->GetSoundID(), in_pPBI->GetPlayingID(), in_pPBI->m_Volume, rParams.Volume, fAuthored, fStates, fRTPC, fSIS, pBus ? pBus->key : 0, fBusTree, rParams.Pitch, rParams.LPF );
}
