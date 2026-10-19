/*=============================================================================
	AkLayer.cpp - the playback half of CAkLayer (aklayer.cpp, 2012 0x954060..0x954b40): the child associations a layer
	keeps (CAssociatedChildData, CAkParameterNode::AssociateLayer / DissociateLayer 0x9395a0 / 0x938cf0), the layer's own
	RTPCs and the crossfade curve evaluated into a child's mute map; 2013 matched by bytes (ih1_inventory.csv).

	DISHONORED(port): agent IH3, fork C (wave 26). Replaces IH1's association stand-in (UpdateAssociatedChild, which bound
	the owner's child without telling the child) with retail's CAssociatedChildData::UpdateChildPtr / Term.
	Deviations (named): the destructor skips the RTPC manager when it is already gone (sound engine Term order; IH1's
	guard kept); LayerMuteFunc's cookie is the struct the decompile reads (AkMutedMapItem, curve, RTPC value,
	mute ratio, computed flag), declared here as AkLayerMuteCookie (retail's local type has no PDB name).
=============================================================================*/
#include "AkContainers.h"
#include "AkActivityChunk.h"

struct AkLayerMuteCookie
{
	AkMutedMapItem										item;
	CAkConversionTable<AkRTPCGraphPoint, AkReal32> *	pCurve;
	AkReal32											fValue;
	AkReal32											fMuteRatio;
	bool												bMuteCalculated;
};

/** 2012 0x9543a0: the crossfade curve (dB, -96.3..0) as a mute ratio, computed once per OnRTPCChanged. */
static void LayerMuteFunc( CAkPBI * in_pPBI, CAkRegisteredObj *, void * in_pCookie )
{
	AkLayerMuteCookie * pCookie = (AkLayerMuteCookie *)in_pCookie;
	if( !pCookie->bMuteCalculated )
	{
		AkUInt32 uIndex = 0;
		pCookie->fMuteRatio = ( pCookie->pCurve->ConvertInternal( pCookie->fValue, uIndex ) - -96.3f ) * 0.010384216f;
		pCookie->bMuteCalculated = true;
	}
	in_pPBI->MuteNotification( pCookie->fMuteRatio, pCookie->item, true );
}

// ------------------------------------------------------------------------------------------------------------
// CAkParameterNode's layer list
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9395a0. */
AKRESULT CAkParameterNode::AssociateLayer( CAkLayer * in_pLayer )
{
	if( !m_pAssociatedLayers )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( *m_pAssociatedLayers ) );
		m_pAssociatedLayers = pMem ? AkPlacementNew( pMem ) AkArray<CAkLayer *, CAkLayer *, ArrayPoolDefault, 8>() : NULL;
	}
	if( m_pAssociatedLayers && m_pAssociatedLayers->AddLast( in_pLayer ) )
	{
		RecalcNotification();
		return AK_Success;
	}
	if( m_pAssociatedLayers && m_pAssociatedLayers->IsEmpty() )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_pAssociatedLayers );
		m_pAssociatedLayers = NULL;
	}
	return AK_InsufficientMemory;
}

/** 2012 0x938cf0. */
AKRESULT CAkParameterNode::DissociateLayer( CAkLayer * in_pLayer )
{
	if( !m_pAssociatedLayers )
	{
		return AK_Fail;
	}
	const AKRESULT eResult = m_pAssociatedLayers->Remove( in_pLayer );
	if( eResult == AK_Success )
	{
		RecalcNotification();
	}
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkLayer::CAssociatedChildData
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9549b0: binds to the child only when it is already the owner's child (AK_Fail when it has another parent). */
AKRESULT CAkLayer::CAssociatedChildData::UpdateChildPtr( CAkLayer * in_pLayer )
{
	if( m_pChild )
	{
		return AK_Success;
	}
	AKRESULT eResult = AK_Success;
	CAkParameterNode * pChild = static_cast<CAkParameterNode *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulChildID ) );
	if( pChild )
	{
		if( in_pLayer->m_pOwner && pChild->m_pParentNode )
		{
			eResult = pChild->m_pParentNode == in_pLayer->m_pOwner ? AK_Success : AK_Fail;
			if( eResult == AK_Success )
			{
				m_pChild = pChild;
				eResult = pChild->AssociateLayer( in_pLayer );
				if( eResult != AK_Success )
				{
					m_pChild = NULL;
				}
			}
		}
		pChild->Release();
	}
	return eResult;
}

/** 2012 0x954260. */
AKRESULT CAkLayer::CAssociatedChildData::Term( CAkLayer * in_pLayer )
{
	AKRESULT eResult = AK_Success;
	if( m_pChild )
	{
		eResult = m_pChild->DissociateLayer( in_pLayer );
		m_pChild = NULL;
	}
	m_ulChildID = 0;
	m_fadeCurve.Unset();
	return eResult;
}

// ------------------------------------------------------------------------------------------------------------
// CAkLayer
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9544e0. */
CAkLayer::~CAkLayer()
{
	for( AkUInt32 i = 0; m_RTPCBitArrayMax32.m_i32BitArray; ++i )
	{
		if( m_RTPCBitArrayMax32.IsSet( i ) )
		{
			g_pRTPCMgr->UnSubscribeRTPC( this, (AkRTPC_ParameterID)i );
			m_RTPCBitArrayMax32.UnsetBit( i );
		}
	}
	if( m_crossfadingRTPCID && g_pRTPCMgr )
	{
		g_pRTPCMgr->UnregisterLayer( this, m_crossfadingRTPCID );
	}
	for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
	{
		p->item.Term( this );
	}
	m_assocs.Term();
}

/** 2012 0x954060: the associated children that are playing recompute their parameters. */
void CAkLayer::RecalcNotification()
{
	for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
	{
		CAkParameterNode * pChild = p->item.m_pChild;
		if( pChild && pChild->m_pActivityChunk && pChild->m_pActivityChunk->m_PlayCount )
		{
			pChild->RecalcNotification();
		}
	}
}

/** 2012 0x9540d0. */
void CAkLayer::ParamNotification( NotifParams & in_rParams )
{
	for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
	{
		CAkParameterNode * pChild = p->item.m_pChild;
		if( pChild && pChild->m_pActivityChunk && pChild->m_pActivityChunk->m_PlayCount )
		{
			pChild->ParamNotification( in_rParams );
		}
	}
}

/** 2012 0x954110. */
bool CAkLayer::IsPlaying() const
{
	for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
	{
		CAkParameterNode * pChild = p->item.m_pChild;
		if( pChild && pChild->m_pActivityChunk && pChild->m_pActivityChunk->m_PlayCount )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x9543f0: the crossfade RTPC moved - every playing PBI of every associated child takes its new mute ratio. */
void CAkLayer::OnRTPCChanged( CAkRegisteredObj * in_GameObjPtr, AkReal32 in_fValue )
{
	if( m_assocs.IsEmpty() )
	{
		return;
	}
	AkLayerMuteCookie muteStruct;
	muteStruct.item.m_Identifier = this;
	muteStruct.item.m_bIsGlobal = in_GameObjPtr == NULL;
	muteStruct.item.m_bIsPersistent = 0;
	muteStruct.fValue = in_fValue;
	for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
	{
		CAkParameterNode * pChild = p->item.m_pChild;
		if( pChild && pChild->m_pActivityChunk && pChild->m_pActivityChunk->m_PlayCount )
		{
			muteStruct.pCurve = &p->item.m_fadeCurve;
			muteStruct.bMuteCalculated = false;
			pChild->ForAllPBI( LayerMuteFunc, in_GameObjPtr, &muteStruct );
		}
	}
}

/** 2012 0x9545b0: a layer RTPC (volume / pitch / LPF) reaches the children as a delta. */
AKRESULT CAkLayer::SetParamComplexFromRTPCManager( void * in_pToken, AkRTPC_ParameterID in_Param_id, AkRtpcID in_RTPCid, AkReal32 in_value, CAkRegisteredObj * in_GameObj, void * in_pGameObjExceptArray )
{
	NotifParams params;
	params.eType = in_Param_id;
	params.bIsFromBus = false;
	params.pGameObj = in_GameObj;
	params.fValue = in_value - g_pRTPCMgr->GetRTPCConvertedValue( in_pToken, in_GameObj, in_RTPCid );
	params.pExceptObjects = in_pGameObjExceptArray;
	ParamNotification( params );
	return AK_Success;
}

/** 2012 0x9546a0. */
AKRESULT CAkLayer::UnsetChildAssoc( AkUniqueID in_ChildID )
{
	for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
	{
		if( p->key == in_ChildID )
		{
			const AKRESULT eResult = p->item.Term( this );
			m_assocs.Erase( (AkUInt32)( p - m_assocs.m_pItems ) );
			return eResult;
		}
	}
	return AK_InvalidInstanceID;
}

/** 2012 0x954830: the layer's RTPC offsets and the crossfade position's mute ratio (an entry of the mute map) for one
    associated child. */
AKRESULT CAkLayer::GetAudioParameters( CAkParameterNode * in_pAssociatedChild, AkSoundParams & io_Parameters, AkUInt32 in_ulParamSelect, CAkKeyArray<AkMutedMapItem, AkReal32, 5> & io_rMutedMap, CAkRegisteredObj * in_GameObjPtr )
{
	MapStruct<AkUniqueID, CAssociatedChildData> * pAssoc = m_assocs.m_pItems;
	while( pAssoc != m_assocs.m_pItemsEnd && pAssoc->key != in_pAssociatedChild->key )
	{
		++pAssoc;
	}
	if( ( in_ulParamSelect & 1 ) && m_RTPCBitArrayMax32.IsSet( RTPC_Volume ) )
	{
		io_Parameters.Volume += g_pRTPCMgr->GetRTPCConvertedValue( this, RTPC_Volume, in_GameObjPtr );
	}
	if( ( in_ulParamSelect & 2 ) && m_RTPCBitArrayMax32.IsSet( RTPC_Pitch ) )
	{
		io_Parameters.Pitch += g_pRTPCMgr->GetRTPCConvertedValue( this, RTPC_Pitch, in_GameObjPtr );
	}
	if( ( in_ulParamSelect & 4 ) && m_RTPCBitArrayMax32.IsSet( RTPC_LPF ) )
	{
		io_Parameters.LPF += g_pRTPCMgr->GetRTPCConvertedValue( this, RTPC_LPF, in_GameObjPtr );
	}
	if( !m_crossfadingRTPCID || !pAssoc->item.m_fadeCurve.m_pArrayGraphPoints )
	{
		return AK_Success;
	}
	AkReal32 fValue;
	bool bGameObjectSpecificValue = true;
	if( !g_pRTPCMgr->GetRTPCValue( m_crossfadingRTPCID, in_GameObjPtr, fValue, bGameObjectSpecificValue ) )
	{
		bGameObjectSpecificValue = false;
		fValue = g_pRTPCMgr->GetDefaultValue( m_crossfadingRTPCID );
	}
	AkUInt32 uIndex = 0;
	const AkReal32 fMuteRatio = ( pAssoc->item.m_fadeCurve.ConvertInternal( fValue, uIndex ) - -96.3f ) * 0.010384216f;
	if( fMuteRatio == 1.f && !bGameObjectSpecificValue )
	{
		return AK_Success;
	}
	AkMutedMapItem item;
	item.m_Identifier = this;
	item.m_bIsGlobal = !bGameObjectSpecificValue;
	item.m_bIsPersistent = 0;
	io_rMutedMap.Set( item, fMuteRatio );
	return AK_Success;
}

/** 2012 0x954b10. */
void CAkLayer::UpdateChildPtr( AkUniqueID in_ChildID )
{
	CAssociatedChildData * pData = m_assocs.Exists( in_ChildID );
	if( pData )
	{
		pData->UpdateChildPtr( this );
	}
}

/** 2012 0x954ab0. */
void CAkLayer::SetOwner( CAkLayerCntr * in_pOwner )
{
	if( m_pOwner )
	{
		for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
		{
			if( p->item.m_pChild )
			{
				p->item.m_pChild->DissociateLayer( this );
				p->item.m_pChild = NULL;
			}
		}
	}
	m_pOwner = in_pOwner;
	if( in_pOwner )
	{
		for( MapStruct<AkUniqueID, CAssociatedChildData> * p = m_assocs.m_pItems; p != m_assocs.m_pItemsEnd; ++p )
		{
			p->item.UpdateChildPtr( this );
		}
	}
}

/** 2012 0x954b40. */
AKRESULT CAkLayer::SetChildAssoc( AkUniqueID in_ChildID, AkRTPCGraphPoint * in_pCrossfadingCurve, AkUInt32 in_ulCrossfadingCurveSize )
{
	AKRESULT eResult = AK_Success;
	CAssociatedChildData * pData = m_assocs.Exists( in_ChildID );
	if( !pData )
	{
		pData = m_assocs.Set( in_ChildID );
		if( !pData )
		{
			return AK_InsufficientMemory;
		}
		AkPlacementNew( pData ) CAssociatedChildData();
		pData->m_ulChildID = in_ChildID;
		eResult = pData->UpdateChildPtr( this );
		if( eResult != AK_Success )
		{
			m_assocs.Unset( in_ChildID );
			return eResult;
		}
	}
	if( in_ulCrossfadingCurveSize )
	{
		eResult = pData->m_fadeCurve.Set( in_pCrossfadingCurve, in_ulCrossfadingCurveSize, AkCurveScaling_dB_96_3_NoCheck );
	}
	else
	{
		pData->m_fadeCurve.Unset();
	}
	if( pData->m_pChild )
	{
		pData->m_pChild->RecalcNotification();
	}
	return eResult;
}
