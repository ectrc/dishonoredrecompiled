/*=============================================================================
	AkRTPCMgr.cpp - CAkRTPCMgr (akrtpcmgr.cpp, 2012 0x930ce0..0x933ea0; 2013 0x92d480..0x9307a0, matched by bytes).

	DISHONORED(port): agent IH3 (wave 26), fork R. Rva on each definition.
	Deviations (named):
	  * The hash / sorted-array / list containers are AkPrivateTypes.h's: retail inlines the same walks (AkHashListBare
	    Exists / Unset / Erase, AkSortedKeyArray BinarySearch / Unset, CAkList2 AddLast / RemoveItem), which is what
	    these bodies call by name.
	  * -akdiag: SetRTPCInternal logs each RTPC id at most once per second of audio-thread buffer ticks (id, value, game
	    object, subscriptions on the entry); SetSwitchInternal logs every call.
=============================================================================*/
#include "AkHierarchy.h"
#include "AkSwitchAware.h"
#include "AkPBI.h"
#include "AkContainers.h"
#include "AkRegistryMgr.h"
#include "AkAudioMgr.h"

/** 2012 0x9312c0. */
CAkRTPCMgr::CAkRTPCMgr()
{
}

/** 2012 0x933da0: the switch subscriber list pre-allocates 16 items, the RTPC-switch list none. */
AKRESULT CAkRTPCMgr::Init()
{
	m_SwitchEntries.Init( g_DefaultPoolId );
	m_RTPCEntries.Init();
	m_RTPCSubscribers.Init();
	AKRESULT eResult = m_listSwitchSubscribers.Init( 16, (AkUInt32)-1 );
	if( eResult != AK_Success )
	{
		Term();
		return eResult;
	}
	m_listRTPCSwitch.InitUnbounded();
	return AK_Success;
}

void CAkRTPCMgr::DestroySubscription( AkRTPCSubscription * in_pSubscription )
{
	in_pSubscription->Curves.Term();
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pSubscription );
}

/** 2012 0x9334f0. */
AKRESULT CAkRTPCMgr::Term()
{
	AkHashListBare<AkRtpcID, AkRTPCEntry, 193>::IteratorEx itEntry = m_RTPCEntries.BeginEx();
	while( itEntry.pItem )
	{
		AkRTPCEntry * pEntry = itEntry.pItem;
		itEntry = m_RTPCEntries.Erase( itEntry );
		pEntry->RemoveGameObject( NULL );
		pEntry->~AkRTPCEntry();
		AK::MemoryMgr::Free( g_DefaultPoolId, pEntry );
	}
	AkHashListBare<AkRTPCSubscriptionKey, AkRTPCSubscription, 193>::IteratorEx itSubs = m_RTPCSubscribers.BeginEx();
	while( itSubs.pItem )
	{
		AkRTPCSubscription * pSubscription = itSubs.pItem;
		itSubs = m_RTPCSubscribers.Erase( itSubs );
		RemoveReferencesToSubscription( pSubscription );
		DestroySubscription( pSubscription );
	}
	if( m_listRTPCSwitch.m_ulMaxNumListItems )
	{
		for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pItem = m_listRTPCSwitch.m_pFirst; pItem; pItem = pItem->pNextListItem )
		{
			pItem->Item.listRTPCSwitchSubscribers.Term();
			pItem->Item.ConversionTable.Unset();
		}
	}
	m_SwitchEntries.Term();
	m_listSwitchSubscribers.Term();
	m_listRTPCSwitch.Term();
	return AK_Success;
}

// ------------------------------------------------------------------------------------------------------------
// CAkRTPCTransition
// ------------------------------------------------------------------------------------------------------------

CAkRTPCMgr::CAkRTPCTransition::~CAkRTPCTransition()
{
	if( m_pTransition )
	{
		g_pTransitionManager->RemoveTransitionUser( m_pTransition, this );
	}
}

/** 2012 0x930ce0: a fade-category transition from the current to the target value. */
AKRESULT CAkRTPCMgr::CAkRTPCTransition::Start( AkReal32 in_fStartValue, AkReal32 in_fTargetValue, TransParams & in_transParams, bool in_bRemoveEntryWhenDone )
{
	TransitionParameters params;
	params.pUser = this;
	params.eTarget = TransTarget_RTPC;
	params.fStartValue = in_fStartValue;
	params.fTargetValue = in_fTargetValue;
	params.lDuration = in_transParams.TransitionTime;
	params.eFadeCurve = in_transParams.eFadeCurve;
	params.bdBs = false;
	params.bUseReciprocalCurve = false;
	m_pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
	m_bRemoveEntryWhenDone = in_bRemoveEntryWhenDone;
	return m_pTransition ? AK_Success : AK_Fail;
}

/** 2012 0x932c20: each step applies the value; the last one removes the transition (and the entry when asked). */
void CAkRTPCMgr::CAkRTPCTransition::TransUpdateValue( TransitionTargets, AkReal32 in_unionValue, bool in_bIsTerminated )
{
	const bool bRemoveEntry = m_bRemoveEntryWhenDone && in_bIsTerminated;
	AkRTPCEntry * pOwner = m_pOwner;
	CAkRegisteredObj * pGameObject = m_pGameObject;
	pOwner->ApplyRTPCValue( pOwner->ValueExists( pGameObject ), in_unionValue, pGameObject, bRemoveEntry );
	if( in_bIsTerminated )
	{
		pOwner->transitions.Remove( this );
		m_pTransition = NULL;
		this->~CAkRTPCTransition();
		AK::MemoryMgr::Free( g_DefaultPoolId, this );
	}
}

// ------------------------------------------------------------------------------------------------------------
// AkRTPCEntry
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x931880: the game object's value, else the global one, else the default. */
AkReal32 CAkRTPCMgr::AkRTPCEntry::GetCurrentValue( AkRTPCValue * in_pValueEntry )
{
	if( in_pValueEntry )
	{
		return in_pValueEntry->fValue;
	}
	AkRTPCValue * pGlobal = ValueExists( NULL );
	return pGlobal ? pGlobal->fValue : fDefaultValue;
}

/** 2012 0x931d00: where a running transition of this game object is heading, else the current value. */
AkReal32 CAkRTPCMgr::AkRTPCEntry::GetCurrentTargetValue( AkRTPCValue * in_pValueEntry )
{
	if( !in_pValueEntry )
	{
		AkRTPCValue * pGlobal = ValueExists( NULL );
		return pGlobal ? pGlobal->fValue : fDefaultValue;
	}
	for( CAkRTPCTransition * pTrans = transitions.First(); pTrans; pTrans = pTrans->pNextLightItem )
	{
		if( pTrans->m_pGameObject == in_pValueEntry->pGameObj )
		{
			return pTrans->m_pTransition->m_fTargetValue;
		}
	}
	return in_pValueEntry->fValue;
}

/** 2012 0x931b30. */
void CAkRTPCMgr::AkRTPCEntry::DestroyTransition( CAkRTPCTransition * in_pPrevItem, CAkRTPCTransition * in_pItem )
{
	if( in_pItem == transitions.m_pFirst )
	{
		transitions.m_pFirst = in_pItem->pNextLightItem;
	}
	else
	{
		in_pPrevItem->pNextLightItem = in_pItem->pNextLightItem;
	}
	in_pItem->~CAkRTPCTransition();
	AK::MemoryMgr::Free( g_DefaultPoolId, in_pItem );
}

/** 2012 0x931ba0: one game object's transition and value, or (NULL) every transition and value. */
void CAkRTPCMgr::AkRTPCEntry::RemoveGameObject( CAkRegisteredObj * in_pGameObj )
{
	if( in_pGameObj )
	{
		CAkRTPCTransition * pPrev = NULL;
		for( CAkRTPCTransition * pTrans = transitions.First(); pTrans; pTrans = pTrans->pNextLightItem )
		{
			if( pTrans->m_pGameObject == in_pGameObj )
			{
				DestroyTransition( pPrev, pTrans );
				break;
			}
			pPrev = pTrans;
		}
		values.Unset( in_pGameObj );
		return;
	}
	while( CAkRTPCTransition * pTrans = transitions.First() )
	{
		DestroyTransition( NULL, pTrans );
	}
	values.RemoveAll();
}

/** 2012 0x931c70: every game object with its own value (they keep it when the global value changes). */
void CAkRTPCMgr::AkRTPCEntry::GetRTPCExceptions( GameObjExceptArray & io_ExceptArrayObj )
{
	const AkUInt32 uNumValues = values.Length();
	if( uNumValues )
	{
		io_ExceptArrayObj.Reserve( uNumValues );
	}
	for( AkRTPCValue * pValue = values.m_pItems; pValue != values.m_pItemsEnd; ++pValue )
	{
		if( pValue->pGameObj )
		{
			io_ExceptArrayObj.AddLast( pValue->pGameObj );
		}
	}
}

/** 2012 0x931da0: retarget this game object's transition, or start one; true when a transition now runs. */
bool CAkRTPCMgr::AkRTPCEntry::CreateOrModifyTransition( CAkRegisteredObj * in_pGameObj, AkReal32 in_fStartValue, AkReal32 in_fTargetValue, TransParams & in_transParams, bool in_bRemoveEntryWhenDone )
{
	CAkRTPCTransition * pPrev = NULL;
	for( CAkRTPCTransition * pTrans = transitions.First(); pTrans; pTrans = pTrans->pNextLightItem )
	{
		if( pTrans->m_pGameObject == in_pGameObj )
		{
			if( in_fStartValue == in_fTargetValue )
			{
				DestroyTransition( pPrev, pTrans );
				return false;
			}
			g_pTransitionManager->ChangeParameter( pTrans->m_pTransition, TransTarget_RTPC, in_fTargetValue, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
			pTrans->m_bRemoveEntryWhenDone = in_bRemoveEntryWhenDone;
			return true;
		}
		pPrev = pTrans;
	}
	if( in_fStartValue == in_fTargetValue )
	{
		return false;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkRTPCTransition ) );
	if( !pMem )
	{
		return false;
	}
	CAkRTPCTransition * pTrans = AkPlacementNew( pMem ) CAkRTPCTransition( this, in_pGameObj );
	if( pTrans->Start( in_fStartValue, in_fTargetValue, in_transParams, in_bRemoveEntryWhenDone ) == AK_Success )
	{
		transitions.AddFirst( pTrans );
		return true;
	}
	pTrans->~CAkRTPCTransition();
	AK::MemoryMgr::Free( g_DefaultPoolId, pTrans );
	return false;
}

/** 2012 0x932e20: a timed change starts / retargets a transition; an immediate one cancels it and applies the value. */
AKRESULT CAkRTPCMgr::AkRTPCEntry::SetRTPC( AkRTPCValue * in_pValueEntry, AkReal32 in_Value, CAkRegisteredObj * in_GameObj, TransParams & in_transParams, bool in_bUnsetWhenDone )
{
	const bool bRequiresUpdate = !in_pValueEntry || in_pValueEntry->fValue != in_Value;
	if( in_transParams.TransitionTime > 0 && bRequiresUpdate )
	{
		if( CreateOrModifyTransition( in_GameObj, GetCurrentValue( in_pValueEntry ), in_Value, in_transParams, in_bUnsetWhenDone ) )
		{
			return AK_Success;
		}
	}
	else
	{
		CAkRTPCTransition * pPrev = NULL;
		for( CAkRTPCTransition * pTrans = transitions.First(); pTrans; pTrans = pTrans->pNextLightItem )
		{
			if( pTrans->m_pGameObject == in_GameObj )
			{
				DestroyTransition( pPrev, pTrans );
				break;
			}
			pPrev = pTrans;
		}
		if( !bRequiresUpdate && !in_bUnsetWhenDone )
		{
			return AK_Success;
		}
	}
	return ApplyRTPCValue( in_pValueEntry, in_Value, in_GameObj, in_bUnsetWhenDone );
}

/** 2012 0x932740: notify, then store (or drop) the value. */
AKRESULT CAkRTPCMgr::AkRTPCEntry::ApplyRTPCValue( AkRTPCValue * in_pValueEntry, AkReal32 in_NewValue, CAkRegisteredObj * in_GameObj, bool in_bUnsetValue )
{
	NotifyRTPCChange( in_pValueEntry, in_NewValue, in_GameObj );
	if( in_bUnsetValue )
	{
		if( in_pValueEntry )
		{
			values.Unset( in_GameObj );
		}
		return AK_Success;
	}
	if( in_pValueEntry )
	{
		in_pValueEntry->fValue = in_NewValue;
		return AK_Success;
	}
	AkRTPCValue * pNewValue = values.AddNoSetKey( in_GameObj );
	if( !pNewValue )
	{
		return AK_Fail;
	}
	pNewValue->pGameObj = in_GameObj;
	pNewValue->fValue = in_NewValue;
	return AK_Success;
}

/**
 * 2012 0x931f10: every subscription to this RTPC that targets the game object (or all game objects) gets the sum of
 * its curves for this RTPC at the new value. Nodes are told only while they play (a bus also for its volume and
 * bypass parameters), layers only while their container and the layer play, PBIs and effects directly; a global
 * change skips the game objects that keep their own value (the exception array).
 */
void CAkRTPCMgr::AkRTPCEntry::NotifyRTPCChange( AkRTPCValue * in_pValueEntry, AkReal32 in_NewValue, CAkRegisteredObj * in_GameObj )
{
	const AkRtpcID idRTPC = key;
	GameObjExceptArray l_ExceptArray;
	bool l_bCheckedExcept = false;
	AkRTPCSubscription * pPrevSubs = NULL;
	for( AkRTPCSubscription ** it = subscriptions.m_pItems; it != subscriptions.m_pItemsEnd; ++it )
	{
		AkRTPCSubscription * pSubscription = *it;
		if( pSubscription == pPrevSubs )
		{
			continue;
		}
		pPrevSubs = pSubscription;
		CAkRegisteredObj * pTarget = pSubscription->TargetGameObject;
		if( in_GameObj != pTarget && in_GameObj && pTarget )
		{
			continue;
		}
		AkReal32 fValue = 0.f;
		switch( pSubscription->eType )
		{
		case SubscriberType_CAkParameterNodeBase:
		case SubscriberType_CAkBus:
		{
			if( in_GameObj && pSubscription->eType == SubscriberType_CAkBus )
			{
				break;
			}
			CAkParameterNodeBase * pNode = (CAkParameterNodeBase *)pSubscription->key.pSubscriber;
			const AkRTPC_ParameterID eParam = pSubscription->key.ParamID;
			if( !pNode->IsPlaying() && !( pSubscription->eType == SubscriberType_CAkBus && ( ( eParam >= RTPC_BypassFX0 && eParam <= RTPC_BypassAllFX ) || eParam == RTPC_Volume ) ) )
			{
				break;
			}
			for( RTPCCurve * pCurve = pSubscription->Curves.m_pItems; pCurve != pSubscription->Curves.m_pItemsEnd; ++pCurve )
			{
				if( pCurve->RTPC_ID == idRTPC )
				{
					AkUInt32 uIndex = 0;
					fValue += pCurve->ConversionTable.ConvertInternal( in_NewValue, uIndex );
				}
			}
			GameObjExceptArray * pExceptArray = NULL;
			if( !in_GameObj && pSubscription->eType != SubscriberType_CAkBus )
			{
				if( !l_bCheckedExcept )
				{
					GetRTPCExceptions( l_ExceptArray );
					l_bCheckedExcept = true;
				}
				pExceptArray = &l_ExceptArray;
			}
			pNode->SetParamComplexFromRTPCManager( pSubscription, eParam, idRTPC, fValue, in_GameObj, pExceptArray );
			break;
		}
		case SubscriberType_PBI:
			if( in_GameObj == pTarget || ( !in_GameObj && !values.Exists( pTarget ) ) )
			{
				for( RTPCCurve * pCurve = pSubscription->Curves.m_pItems; pCurve != pSubscription->Curves.m_pItemsEnd; ++pCurve )
				{
					if( pCurve->RTPC_ID == idRTPC )
					{
						AkUInt32 uIndex = 0;
						fValue += pCurve->ConversionTable.ConvertInternal( in_NewValue, uIndex );
					}
				}
				( (CAkPBI *)pSubscription->key.pSubscriber )->SetParam( (AkInt16)pSubscription->key.ParamID, &fValue, sizeof( fValue ) );
			}
			break;
		case SubscriberType_CAkLayer:
		{
			CAkLayer * pLayer = (CAkLayer *)pSubscription->key.pSubscriber;
			CAkParameterNodeBase * pOwner = (CAkParameterNodeBase *)pLayer->m_pOwner;
			if( !pOwner || !pOwner->IsPlaying() || !pLayer->IsPlaying() )
			{
				break;
			}
			for( RTPCCurve * pCurve = pSubscription->Curves.m_pItems; pCurve != pSubscription->Curves.m_pItemsEnd; ++pCurve )
			{
				if( pCurve->RTPC_ID == idRTPC )
				{
					AkUInt32 uIndex = 0;
					fValue += pCurve->ConversionTable.ConvertInternal( in_NewValue, uIndex );
				}
			}
			if( !in_GameObj && !l_bCheckedExcept )
			{
				GetRTPCExceptions( l_ExceptArray );
				l_bCheckedExcept = true;
			}
			if( pSubscription->key.ParamID == RTPC_MaxNumRTPC )
			{
				pLayer->OnRTPCChanged( in_GameObj, in_NewValue );
			}
			else
			{
				pLayer->SetParamComplexFromRTPCManager( pSubscription, pSubscription->key.ParamID, idRTPC, fValue, in_GameObj, &l_ExceptArray );
			}
			break;
		}
		default:
			if( in_GameObj == pTarget || ( !in_GameObj && !values.Exists( pTarget ) ) )
			{
				AkUInt32 uIndex = 0;
				fValue = pSubscription->Curves.m_pItems->ConversionTable.ConvertInternal( in_NewValue, uIndex );
				( (AK::IAkRTPCSubscriber *)pSubscription->key.pSubscriber )->SetParam( (AkUInt16)pSubscription->key.ParamID, &fValue, sizeof( fValue ) );
			}
			break;
		}
	}
	l_ExceptArray.Term();
	g_pRTPCMgr->NotifyClientRTPCChange( idRTPC, in_pValueEntry, in_NewValue, in_GameObj );
}

// ------------------------------------------------------------------------------------------------------------
// CAkRTPCMgr
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x932510: the entry of an RTPC id, created on first use. */
CAkRTPCMgr::AkRTPCEntry * CAkRTPCMgr::GetRTPCEntry( AkRtpcID in_RTPCid )
{
	AkRTPCEntry * pEntry = m_RTPCEntries.Exists( in_RTPCid );
	if( pEntry )
	{
		return pEntry;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkRTPCEntry ) );
	if( !pMem )
	{
		return NULL;
	}
	pEntry = AkPlacementNew( pMem ) AkRTPCEntry( in_RTPCid );
	m_RTPCEntries.Set( pEntry );
	return pEntry;
}

/** 2012 0x9318e0: an RTPC-driven switch group re-evaluates its curve for the game object; a changed switch is pushed. */
void CAkRTPCMgr::NotifyClientRTPCChange( AkRtpcID in_idRTPC, AkRTPCValue * in_pValueEntry, AkReal32 in_NewValue, CAkRegisteredObj * in_GameObj )
{
	if( !in_GameObj )
	{
		return;
	}
	for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pItem = m_listRTPCSwitch.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.RTPC_ID != in_idRTPC )
		{
			continue;
		}
		AkUInt32 uIndex = 0;
		const AkSwitchStateID newSwitch = pItem->Item.ConversionTable.ConvertInternal( in_NewValue, uIndex );
		uIndex = 0;
		if( in_pValueEntry && pItem->Item.ConversionTable.ConvertInternal( in_pValueEntry->fValue, uIndex ) == newSwitch )
		{
			continue;
		}
		for( CAkList2<CAkSwitchAware *, CAkSwitchAware *, 2>::ListItem * pSub = pItem->Item.listRTPCSwitchSubscribers.m_pFirst; pSub; pSub = pSub->pNextListItem )
		{
			pSub->Item->SetSwitch( newSwitch, in_GameObj );
		}
	}
}

/** 2012 0x931970: store the switch for the game object, then tell every container subscribed to the group. */
AKRESULT CAkRTPCMgr::SetSwitchInternal( AkSwitchGroupID in_SwitchGroup, AkSwitchStateID in_SwitchState, CAkRegisteredObj * in_GameObj )
{
	AkSwitchKey switchKey;
	switchKey.m_SwitchGroup = in_SwitchGroup;
	switchKey.m_pGameObj = in_GameObj;
	AkSwitchStateID * pSwitchState = m_SwitchEntries.Set( switchKey );
	AKRESULT eResult = AK_Fail;
	if( pSwitchState )
	{
		*pSwitchState = in_SwitchState;
		eResult = AK_Success;
	}
	AkUInt32 uNumSubscribers = 0;
	for( CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>::ListItem * pItem = m_listSwitchSubscribers.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.switchGroup == in_SwitchGroup )
		{
			pItem->Item.pSwitch->SetSwitch( in_SwitchState, in_GameObj );
			++uNumSubscribers;
		}
	}
	AKDIAG( L"akdiag: SetSwitch group 0x%08x = 0x%08x on game object %u: %u container(s)", in_SwitchGroup, in_SwitchState,
		in_GameObj ? (AkUInt32)in_GameObj->m_GameObjID : 0xFFFFFFFF, uNumSubscribers );
	return eResult;
}

/** 2012 0x932260: unhook the subscription from every RTPC entry it is listed in and drop its curves. */
void CAkRTPCMgr::RemoveReferencesToSubscription( AkRTPCSubscription * in_pSubscription )
{
	bool bFoundEntry = false;
	for( RTPCCurve * pCurve = in_pSubscription->Curves.m_pItems; pCurve != in_pSubscription->Curves.m_pItemsEnd; ++pCurve )
	{
		AkRTPCEntry * pEntry = m_RTPCEntries.Exists( pCurve->RTPC_ID );
		if( pEntry )
		{
			pEntry->subscriptions.Unset( in_pSubscription );
			bFoundEntry = true;
		}
		pCurve->ConversionTable.Unset();
	}
	in_pSubscription->Curves.RemoveAll();
	if( bFoundEntry )
	{
		return;
	}
	for( AkHashListBare<AkRtpcID, AkRTPCEntry, 193>::Iterator it = m_RTPCEntries.Begin(); it.pItem; ++it )
	{
		it.pItem->subscriptions.Unset( in_pSubscription );
	}
}

/** 2012 0x9323c0: the game object's value, else (for a game object) the global value. */
bool CAkRTPCMgr::GetRTPCValue( AkRtpcID in_RTPC_ID, CAkRegisteredObj * in_GameObj, AkReal32 & out_value, bool & out_bGameObjectSpecificValue )
{
	AkRTPCEntry * pEntry = m_RTPCEntries.Exists( in_RTPC_ID );
	if( !pEntry )
	{
		return false;
	}
	AkRTPCValue * pValue = pEntry->ValueExists( in_GameObj );
	if( pValue )
	{
		out_value = pValue->fValue;
		out_bGameObjectSpecificValue = in_GameObj != NULL;
		return true;
	}
	if( !in_GameObj )
	{
		return false;
	}
	out_bGameObjectSpecificValue = false;
	pValue = pEntry->ValueExists( NULL );
	if( !pValue )
	{
		return false;
	}
	out_value = pValue->fValue;
	return true;
}

/** 2012 0x932460: from the plain switch subscribers, else from the RTPC-driven group it sits in. */
void CAkRTPCMgr::UnSubscribeSwitch( CAkSwitchAware * in_pSwitch )
{
	CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>::ListItem * pPrev = NULL;
	for( CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>::ListItem * pItem = m_listSwitchSubscribers.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.pSwitch == in_pSwitch )
		{
			m_listSwitchSubscribers.RemoveItem( pItem, pPrev );
			return;
		}
		pPrev = pItem;
	}
	for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pAssoc = m_listRTPCSwitch.m_pFirst; pAssoc; pAssoc = pAssoc->pNextListItem )
	{
		CAkList2<CAkSwitchAware *, CAkSwitchAware *, 2> & rList = pAssoc->Item.listRTPCSwitchSubscribers;
		CAkList2<CAkSwitchAware *, CAkSwitchAware *, 2>::ListItem * pPrevSub = NULL;
		for( CAkList2<CAkSwitchAware *, CAkSwitchAware *, 2>::ListItem * pSub = rList.m_pFirst; pSub; pSub = pSub->pNextListItem )
		{
			if( pSub->Item == in_pSwitch )
			{
				rList.RemoveItem( pSub, pPrevSub );
				return;
			}
			pPrevSub = pSub;
		}
	}
}

/** 2012 0x9325b0. */
void CAkRTPCMgr::SetDefaultParamValue( AkRtpcID in_RTPCid, AkReal32 in_fValue )
{
	AkRTPCEntry * pEntry = GetRTPCEntry( in_RTPCid );
	if( pEntry )
	{
		pEntry->fDefaultValue = in_fValue;
	}
}

/** 2012 0x9325d0. */
AkReal32 CAkRTPCMgr::GetDefaultValue( AkRtpcID in_RTPCid, bool * out_pbWasFound )
{
	AkRTPCEntry * pEntry = m_RTPCEntries.Exists( in_RTPCid );
	if( out_pbWasFound )
	{
		*out_pbWasFound = pEntry != NULL;
	}
	return pEntry ? pEntry->fDefaultValue : 0.f;
}

/** 2012 0x9327d0: an RTPC-driven group converts the RTPC's value (else its default); otherwise the game object's
    switch, else the global one, else 0. */
AkSwitchStateID CAkRTPCMgr::GetSwitch( AkSwitchGroupID in_SwitchGroup, CAkRegisteredObj * in_GameObj )
{
	for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pAssoc = m_listRTPCSwitch.m_pFirst; pAssoc; pAssoc = pAssoc->pNextListItem )
	{
		if( pAssoc->Item.switchGroup != in_SwitchGroup )
		{
			continue;
		}
		AkReal32 fValue;
		bool bGameObjectSpecific;
		if( !GetRTPCValue( pAssoc->Item.RTPC_ID, in_GameObj, fValue, bGameObjectSpecific ) )
		{
			AkRTPCEntry * pEntry = m_RTPCEntries.Exists( pAssoc->Item.RTPC_ID );
			fValue = pEntry ? pEntry->fDefaultValue : 0.f;
		}
		AkUInt32 uIndex = 0;
		return pAssoc->Item.ConversionTable.ConvertInternal( fValue, uIndex );
	}
	AkSwitchKey switchKey;
	switchKey.m_SwitchGroup = in_SwitchGroup;
	switchKey.m_pGameObj = in_GameObj;
	AkSwitchStateID * pSwitchState = m_SwitchEntries.Exists( switchKey );
	if( pSwitchState )
	{
		return *pSwitchState;
	}
	if( in_GameObj )
	{
		switchKey.m_pGameObj = NULL;
		pSwitchState = m_SwitchEntries.Exists( switchKey );
		if( pSwitchState )
		{
			return *pSwitchState;
		}
	}
	return 0;
}

/** 2012 0x9328e0: the sum over the subscription's curves (one RTPC, or all when in_RTPCid is 0) of the curve at the
    game object's RTPC value (else the global value, else the RTPC's default). */
AkReal32 CAkRTPCMgr::GetRTPCConvertedValue( void * in_pToken, CAkRegisteredObj * in_GameObj, AkRtpcID in_RTPCid )
{
	AkRTPCSubscription * pSubscription = (AkRTPCSubscription *)in_pToken;
	AkReal32 rtpcValue = 0.f;
	bool bGameObjectSpecificValue;
	const bool bGotValue = in_RTPCid && GetRTPCValue( in_RTPCid, in_GameObj, rtpcValue, bGameObjectSpecificValue );
	AkReal32 fResult = 0.f;
	for( RTPCCurve * pCurve = pSubscription->Curves.m_pItems; pCurve != pSubscription->Curves.m_pItemsEnd; ++pCurve )
	{
		if( in_RTPCid && pCurve->RTPC_ID != in_RTPCid )
		{
			continue;
		}
		if( !bGotValue )
		{
			if( in_RTPCid || !GetRTPCValue( pCurve->RTPC_ID, in_GameObj, rtpcValue, bGameObjectSpecificValue ) )
			{
				AkRTPCEntry * pEntry = m_RTPCEntries.Exists( pCurve->RTPC_ID );
				rtpcValue = pEntry ? pEntry->fDefaultValue : 0.f;
			}
		}
		AkUInt32 uIndex = 0;
		fResult += pCurve->ConversionTable.ConvertInternal( rtpcValue, uIndex );
	}
	return fResult;
}

/** 2012 0x932a40: the (subscriber, parameter) subscription's converted value; 0 without one. */
AkReal32 CAkRTPCMgr::GetRTPCConvertedValue( void * in_pSubscriber, AkUInt32 in_ParamID, CAkRegisteredObj * in_GameObj )
{
	AkRTPCSubscriptionKey subscriptionKey;
	subscriptionKey.pSubscriber = in_pSubscriber;
	subscriptionKey.ParamID = (AkRTPC_ParameterID)in_ParamID;
	AkRTPCSubscription * pSubscription = m_RTPCSubscribers.Exists( subscriptionKey );
	return pSubscription ? GetRTPCConvertedValue( pSubscription, in_GameObj, 0 ) : 0.f;
}

/** 2012 0x932a80: into the RTPC-driven group's list when the group has one (and RTPCs are not refused), else into
    the plain switch subscribers. */
AKRESULT CAkRTPCMgr::SubscribeSwitch( CAkSwitchAware * in_pSwitch, AkSwitchGroupID in_switchGroup, bool in_bForceNoRTPC )
{
	if( !in_bForceNoRTPC )
	{
		for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pAssoc = m_listRTPCSwitch.m_pFirst; pAssoc; pAssoc = pAssoc->pNextListItem )
		{
			if( pAssoc->Item.switchGroup == in_switchGroup )
			{
				return pAssoc->Item.listRTPCSwitchSubscribers.AddLast( in_pSwitch ) ? AK_Success : AK_Fail;
			}
		}
	}
	if( !in_pSwitch )
	{
		return AK_InvalidParameter;
	}
	UnSubscribeSwitch( in_pSwitch );
	AkSwitchSubscription subscription;
	subscription.pSwitch = in_pSwitch;
	subscription.switchGroup = in_switchGroup;
	return m_listSwitchSubscribers.AddLast( subscription ) ? AK_Success : AK_Fail;
}

/** 2012 0x932b10: push the current converted value of a new / changed subscription to its subscriber. */
void CAkRTPCMgr::UpdateSubscription( AkRTPCSubscription & in_rSubscription )
{
	switch( in_rSubscription.eType )
	{
	case SubscriberType_CAkParameterNodeBase:
	case SubscriberType_CAkBus:
		( (CAkParameterNodeBase *)in_rSubscription.key.pSubscriber )->RecalcNotification();
		break;
	case SubscriberType_PBI:
	{
		AkReal32 fValue = GetRTPCConvertedValue( &in_rSubscription, in_rSubscription.TargetGameObject, 0 );
		( (CAkPBI *)in_rSubscription.key.pSubscriber )->SetParam( (AkInt16)in_rSubscription.key.ParamID, &fValue, sizeof( fValue ) );
		break;
	}
	case SubscriberType_CAkLayer:
		( (CAkLayer *)in_rSubscription.key.pSubscriber )->RecalcNotification();
		break;
	default:
	{
		AkReal32 fValue = GetRTPCConvertedValue( &in_rSubscription, in_rSubscription.TargetGameObject, 0 );
		( (AK::IAkRTPCSubscriber *)in_rSubscription.key.pSubscriber )->SetParam( (AkUInt16)in_rSubscription.key.ParamID, &fValue, sizeof( fValue ) );
		break;
	}
	}
}

/** 2012 0x932cb0: the group's subscribers go back to the plain switch list; the association is removed. */
void CAkRTPCMgr::RemoveSwitchRTPC( AkSwitchGroupID in_switchGroup )
{
	CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pPrev = NULL;
	for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pAssoc = m_listRTPCSwitch.m_pFirst; pAssoc; pAssoc = pAssoc->pNextListItem )
	{
		if( pAssoc->Item.switchGroup != in_switchGroup )
		{
			pPrev = pAssoc;
			continue;
		}
		pAssoc->Item.ConversionTable.Unset();
		for( CAkList2<CAkSwitchAware *, CAkSwitchAware *, 2>::ListItem * pSub = pAssoc->Item.listRTPCSwitchSubscribers.m_pFirst; pSub; pSub = pSub->pNextListItem )
		{
			CAkSwitchAware * pSwitch = pSub->Item;
			if( pSwitch )
			{
				UnSubscribeSwitch( pSwitch );
				AkSwitchSubscription subscription;
				subscription.pSwitch = pSwitch;
				subscription.switchGroup = in_switchGroup;
				m_listSwitchSubscribers.AddLast( subscription );
			}
		}
		pAssoc->Item.listRTPCSwitchSubscribers.Term();
		m_listRTPCSwitch.RemoveItem( pAssoc, pPrev );
		return;
	}
}

/** 2012 0x932f00: drop one curve of a subscription; the subscription goes with its last curve. */
void CAkRTPCMgr::UnSubscribeRTPC( void * in_pSubscriber, AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID, bool * out_bMoreCurvesRemaining )
{
	AkRTPCSubscriptionKey subscriptionKey;
	subscriptionKey.pSubscriber = in_pSubscriber;
	subscriptionKey.ParamID = in_ParamID;
	AkRTPCSubscription * pSubscription = m_RTPCSubscribers.Exists( subscriptionKey );
	if( !pSubscription )
	{
		if( out_bMoreCurvesRemaining )
		{
			*out_bMoreCurvesRemaining = false;
		}
		return;
	}
	if( out_bMoreCurvesRemaining )
	{
		*out_bMoreCurvesRemaining = !pSubscription->Curves.IsEmpty();
	}
	for( RTPCCurve * pCurve = pSubscription->Curves.m_pItems; pCurve != pSubscription->Curves.m_pItemsEnd; ++pCurve )
	{
		if( pCurve->RTPCCurveID != in_RTPCCurveID )
		{
			continue;
		}
		AkRTPCEntry * pEntry = m_RTPCEntries.Exists( pCurve->RTPC_ID );
		if( pEntry )
		{
			pEntry->subscriptions.Unset( pSubscription );
		}
		pCurve->ConversionTable.Unset();
		pSubscription->Curves.Erase( (AkUInt32)( pCurve - pSubscription->Curves.m_pItems ) );
		if( pSubscription->Curves.IsEmpty() )
		{
			if( out_bMoreCurvesRemaining )
			{
				*out_bMoreCurvesRemaining = false;
			}
			m_RTPCSubscribers.Unset( subscriptionKey );
			DestroySubscription( pSubscription );
		}
		return;
	}
}

/** 2012 0x9330b0: the whole (subscriber, parameter) subscription. */
void CAkRTPCMgr::UnSubscribeRTPC( void * in_pSubscriber, AkRTPC_ParameterID in_ParamID )
{
	AkRTPCSubscriptionKey subscriptionKey;
	subscriptionKey.pSubscriber = in_pSubscriber;
	subscriptionKey.ParamID = in_ParamID;
	AkRTPCSubscription * pSubscription = m_RTPCSubscribers.Unset( subscriptionKey );
	if( pSubscription )
	{
		RemoveReferencesToSubscription( pSubscription );
		DestroySubscription( pSubscription );
	}
}

/** 2012 0x933140: every subscription of a subscriber. */
void CAkRTPCMgr::UnSubscribeRTPC( void * in_pSubscriber )
{
	AkHashListBare<AkRTPCSubscriptionKey, AkRTPCSubscription, 193>::IteratorEx it = m_RTPCSubscribers.BeginEx();
	while( it.pItem )
	{
		AkRTPCSubscription * pSubscription = it.pItem;
		if( pSubscription->key.pSubscriber != in_pSubscriber )
		{
			++it;
			continue;
		}
		it = m_RTPCSubscribers.Erase( it );
		RemoveReferencesToSubscription( pSubscription );
		DestroySubscription( pSubscription );
	}
}

/** 2012 0x933240: the game object's subscriptions, switches, values and transitions. */
void CAkRTPCMgr::UnregisterGameObject( CAkRegisteredObj * in_GameObj )
{
	if( !in_GameObj )
	{
		return;
	}
	AkHashListBare<AkRTPCSubscriptionKey, AkRTPCSubscription, 193>::IteratorEx itSubs = m_RTPCSubscribers.BeginEx();
	while( itSubs.pItem )
	{
		AkRTPCSubscription * pSubscription = itSubs.pItem;
		if( pSubscription->TargetGameObject != in_GameObj )
		{
			++itSubs;
			continue;
		}
		itSubs = m_RTPCSubscribers.Erase( itSubs );
		RemoveReferencesToSubscription( pSubscription );
		DestroySubscription( pSubscription );
	}
	AkHashList<AkSwitchKey, AkSwitchStateID, 193>::IteratorEx itSwitch;
	itSwitch.pTable = m_SwitchEntries.m_table;
	itSwitch.uiTable = 0;
	itSwitch.pPrevItem = NULL;
	itSwitch.pItem = m_SwitchEntries.m_table[ 0 ];
	while( !itSwitch.pItem && ++itSwitch.uiTable < 193 )
	{
		itSwitch.pItem = m_SwitchEntries.m_table[ itSwitch.uiTable ];
	}
	while( itSwitch.pItem )
	{
		if( itSwitch.pItem->Assoc.key.m_pGameObj == in_GameObj )
		{
			itSwitch = m_SwitchEntries.Erase( itSwitch );
			continue;
		}
		itSwitch.pPrevItem = itSwitch.pItem;
		itSwitch.pItem = itSwitch.pItem->pNextItem;
		while( !itSwitch.pItem && ++itSwitch.uiTable < 193 )
		{
			itSwitch.pPrevItem = NULL;
			itSwitch.pItem = m_SwitchEntries.m_table[ itSwitch.uiTable ];
		}
	}
	for( AkHashListBare<AkRtpcID, AkRTPCEntry, 193>::Iterator it = m_RTPCEntries.Begin(); it.pItem; ++it )
	{
		it.pItem->RemoveGameObject( in_GameObj );
	}
}

/** 2012 0x9333c0: refresh every subscription of a subscriber. */
AKRESULT CAkRTPCMgr::UpdateRTPCSubscriberInfo( void * in_pSubscriber )
{
	for( AkHashListBare<AkRTPCSubscriptionKey, AkRTPCSubscription, 193>::Iterator it = m_RTPCSubscribers.Begin(); it.pItem; ++it )
	{
		if( it.pItem->key.pSubscriber == in_pSubscriber )
		{
			UpdateSubscription( *it.pItem );
		}
	}
	return AK_Success;
}

/** 2012 0x933430: back to the global value (for a game object) or to the default (global), then the entry goes. */
void CAkRTPCMgr::ResetRTPCValue( AkRtpcID in_RTPCid, CAkRegisteredObj * in_GameObj, TransParams & in_transParams )
{
	AkRTPCEntry * pEntry = m_RTPCEntries.Exists( in_RTPCid );
	if( !pEntry )
	{
		return;
	}
	AkRTPCValue * pGlobal = pEntry->ValueExists( NULL );
	AkRTPCValue * pValue = pGlobal;
	AkReal32 fResetValue;
	if( in_GameObj )
	{
		pValue = pEntry->ValueExists( in_GameObj );
		if( !pValue )
		{
			return;
		}
		fResetValue = pGlobal ? pGlobal->fValue : pEntry->fDefaultValue;
	}
	else
	{
		if( !pGlobal )
		{
			return;
		}
		fResetValue = pEntry->fDefaultValue;
	}
	pEntry->SetRTPC( pValue, fResetValue, in_GameObj, in_transParams, true );
}

/** 2012 0x9336d0: a switch group driven by an RTPC; plain subscribers of the group move to it. */
AKRESULT CAkRTPCMgr::AddSwitchRTPC( AkSwitchGroupID in_switchGroup, AkRtpcID in_RTPC_ID, AkRTPCGraphPointBase<AkUInt32> * in_pArrayConversion, AkUInt32 in_ulConversionArraySize )
{
	for( CAkList2<AkRTPCSwitchAssociation, const AkRTPCSwitchAssociation &, 2>::ListItem * pAssoc = m_listRTPCSwitch.m_pFirst; pAssoc; pAssoc = pAssoc->pNextListItem )
	{
		if( pAssoc->Item.switchGroup == in_switchGroup )
		{
			pAssoc->Item.RTPC_ID = in_RTPC_ID;
			pAssoc->Item.ConversionTable.Unset();
			return pAssoc->Item.ConversionTable.Set( in_pArrayConversion, in_ulConversionArraySize, AkCurveScaling_None );
		}
	}
	AKRESULT eResult = AK_Fail;
	AkRTPCSwitchAssociation * pNewAssoc = m_listRTPCSwitch.AddLast();
	if( pNewAssoc )
	{
		pNewAssoc->RTPC_ID = in_RTPC_ID;
		pNewAssoc->switchGroup = in_switchGroup;
		eResult = pNewAssoc->ConversionTable.Set( in_pArrayConversion, in_ulConversionArraySize, AkCurveScaling_None );
		if( eResult == AK_Success )
		{
			pNewAssoc->listRTPCSwitchSubscribers.InitUnbounded();
		}
		else
		{
			RemoveSwitchRTPC( in_switchGroup );
		}
	}
	CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>::ListItem * pPrev = NULL;
	CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>::ListItem * pItem = m_listSwitchSubscribers.m_pFirst;
	while( pItem )
	{
		CAkList2<AkSwitchSubscription, const AkSwitchSubscription &, 2>::ListItem * pNext = pItem->pNextListItem;
		if( pItem->Item.switchGroup != in_switchGroup )
		{
			pPrev = pItem;
			pItem = pNext;
			continue;
		}
		CAkSwitchAware * pSwitch = pItem->Item.pSwitch;
		m_listSwitchSubscribers.RemoveItem( pItem, pPrev );
		SubscribeSwitch( pSwitch, in_switchGroup );
		pItem = pNext;
	}
	return eResult;
}

/** 2012 0x9339e0. */
void CAkRTPCMgr::UnregisterLayer( CAkLayer * in_pLayer, AkRtpcID )
{
	UnSubscribeRTPC( in_pLayer, RTPC_MaxNumRTPC );
}

/** 2012 0x9339f0: an offset is added to the value the entry is heading for. */
AKRESULT CAkRTPCMgr::SetRTPCInternal( AkRtpcID in_RTPCid, AkReal32 in_Value, CAkRegisteredObj * in_GameObj, TransParams & in_transParams, AkValueMeaning in_eValueMeaning )
{
	AkRTPCEntry * pEntry = m_RTPCEntries.Exists( in_RTPCid );
	AkRTPCValue * pValue = NULL;
	if( pEntry )
	{
		pValue = pEntry->ValueExists( in_GameObj );
	}
	else
	{
		pEntry = GetRTPCEntry( in_RTPCid );
		if( !pEntry )
		{
			return AK_Fail;
		}
	}
	const AkReal32 fValue = in_eValueMeaning == AkValueMeaning_Offset ? pEntry->GetCurrentTargetValue( pValue ) + in_Value : in_Value;
	if( DishonoredWwise::IsDiagnostics() )
	{
		static struct { AkRtpcID id; AkUInt32 uTick; } s_lastLog[ 64 ];
		const AkUInt32 uTick = g_pAudioMgr ? g_pAudioMgr->GetBufferTick() : 0;
		AkUInt32 uSlot = in_RTPCid % 64;
		if( s_lastLog[ uSlot ].id != in_RTPCid || uTick - s_lastLog[ uSlot ].uTick >= 1000 / (AkUInt32)AkAudioLibSettings::g_msPerBufferTick )
		{
			s_lastLog[ uSlot ].id = in_RTPCid;
			s_lastLog[ uSlot ].uTick = uTick;
			AkSilent::Logf( L"akdiag: SetRTPCValue 0x%08x = %.3f on game object %d (fade %d ms): %u subscription(s)", in_RTPCid, fValue,
				in_GameObj ? (AkInt32)in_GameObj->m_GameObjID : -1, in_transParams.TransitionTime, pEntry->subscriptions.Length() );
		}
	}
	return pEntry->SetRTPC( pValue, fValue, in_GameObj, in_transParams, false );
}

/** 2012 0x933af0: a subscription keyed (subscriber, parameter) holds one curve per curve id; adding a curve replaces
    the one with the same id. A layer's crossfade registration (RTPC_MaxNumRTPC) has no curve. */
AKRESULT CAkRTPCMgr::SubscribeRTPC( void * in_pSubscriber, AkRtpcID in_RTPC_ID, AkRTPC_ParameterID in_ParamID, AkUniqueID in_RTPCCurveID, AkCurveScaling in_eScaling, AkRTPCGraphPoint * in_pArrayConversion, AkUInt32 in_ulConversionArraySize, CAkRegisteredObj * in_TargetGameObject, SubscriberType in_eType )
{
	if( !in_pSubscriber )
	{
		return AK_InvalidParameter;
	}
	AkRTPCSubscriptionKey subscriptionKey;
	subscriptionKey.pSubscriber = in_pSubscriber;
	subscriptionKey.ParamID = in_ParamID;
	AkRTPCSubscription * pSubscription = m_RTPCSubscribers.Exists( subscriptionKey );
	AKRESULT eResult = AK_InvalidParameter;
	if( pSubscription )
	{
		for( RTPCCurve * pCurve = pSubscription->Curves.m_pItems; pCurve != pSubscription->Curves.m_pItemsEnd; ++pCurve )
		{
			if( pCurve->RTPCCurveID == in_RTPCCurveID )
			{
				AkRTPCEntry * pEntry = m_RTPCEntries.Exists( pCurve->RTPC_ID );
				if( pEntry )
				{
					pEntry->subscriptions.Unset( pSubscription );
				}
				pCurve->ConversionTable.Unset();
				pSubscription->Curves.Erase( (AkUInt32)( pCurve - pSubscription->Curves.m_pItems ) );
				break;
			}
		}
	}
	else
	{
		pSubscription = (AkRTPCSubscription *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkRTPCSubscription ) );
		if( !pSubscription )
		{
			return AK_InsufficientMemory;
		}
		AkPlacementNew( &pSubscription->Curves ) AkArray<RTPCCurve, const RTPCCurve &, ArrayPoolDefault, 1>();
		pSubscription->key = subscriptionKey;
		pSubscription->eType = in_eType;
		pSubscription->TargetGameObject = in_TargetGameObject;
		m_RTPCSubscribers.Set( pSubscription );
	}
	if( in_pArrayConversion && in_ulConversionArraySize )
	{
		RTPCCurve * pNewCurve = pSubscription->Curves.AddLast();
		if( !pNewCurve )
		{
			eResult = AK_InsufficientMemory;
		}
		else
		{
			AkPlacementNew( &pNewCurve->ConversionTable ) CAkConversionTable<AkRTPCGraphPoint, AkReal32>();
			pNewCurve->RTPC_ID = in_RTPC_ID;
			pNewCurve->RTPCCurveID = in_RTPCCurveID;
			eResult = pNewCurve->ConversionTable.Set( in_pArrayConversion, in_ulConversionArraySize, in_eScaling );
			if( eResult != AK_Success )
			{
				pSubscription->Curves.Erase( pSubscription->Curves.Length() - 1 );
			}
		}
	}
	else if( in_eType == SubscriberType_CAkLayer && in_ParamID == RTPC_MaxNumRTPC )
	{
		eResult = AK_Success;
	}
	if( eResult == AK_Success )
	{
		AkRTPCEntry * pEntry = GetRTPCEntry( in_RTPC_ID );
		if( pEntry )
		{
			AkRTPCSubscription ** ppSlot = pEntry->subscriptions.AddNoSetKey( pSubscription );
			if( ppSlot )
			{
				*ppSlot = pSubscription;
			}
			return UpdateRTPCSubscriberInfo( in_pSubscriber );
		}
		eResult = AK_InsufficientMemory;
	}
	if( pSubscription->Curves.IsEmpty() )
	{
		m_RTPCSubscribers.Unset( subscriptionKey );
		RemoveReferencesToSubscription( pSubscription );
		DestroySubscription( pSubscription );
	}
	return eResult;
}

/** 2012 0x933ea0: a layer's crossfading RTPC is a curve-less subscription on RTPC_MaxNumRTPC. */
AKRESULT CAkRTPCMgr::RegisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID )
{
	return SubscribeRTPC( in_pLayer, in_rtpcID, RTPC_MaxNumRTPC, 0, AkCurveScaling_None, NULL, 0, NULL, SubscriberType_CAkLayer );
}
