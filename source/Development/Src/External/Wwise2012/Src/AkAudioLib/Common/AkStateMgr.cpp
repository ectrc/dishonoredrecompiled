/*=============================================================================
	AkStateMgr.cpp - CAkStateMgr (akstatemgr.cpp, 2012 0x92e760..0x92f6e0; 2013 0x92af60..0x92bd90, matched by bytes),
	the runtime half of AkStateGroupChunk (akparameternodebase.cpp 0x9444d0 / 0x944550 / 0x945530; 2013 0x940e50 /
	0x940ed0 / 0x941dc0) and CAkPreparationAware (akpreparationaware.cpp 0x95ea80..0x95eb60).

	DISHONORED(port): agent IH3 (wave 26), fork R. Rva on each definition.
	Deviations (named):
	  * SetStateInternal: a state value a chunk did not hold yet starts at the transition's start value (0); retail leaves
	    the new AkPropBundle slot's fValue unwritten until the first transition step.
	  * -akdiag: every SetStateInternal (group, old -> new, transition ms, member chunks, switch containers, whether the
	    external handler took it).
	  * The bank manager's PrepareGameSyncs item (CAkBankMgr::PrepareGameSync 2012 0x92d7c0) is IH1's file and still
	    completes without preparing: Dishonored never calls PrepareGameSyncs (0 in IH2b.log); PrepareGameSync here is
	    ported so the switch containers' preparation can subscribe.
=============================================================================*/
#include "AkHierarchy.h"
#include "AkStateMgr.h"
#include "AkSwitchAware.h"
#include "AkRegistryMgr.h"

AkExternalStateHandlerCallback g_pExternalStateHandler = NULL;

AkStateGroupChunk *& AkListBareLightNextInGroup::Get( AkStateGroupChunk * in_pItem )
{
	return in_pItem->m_pNextInGroup;
}

// ------------------------------------------------------------------------------------------------------------
// AkStateGroupChunk (runtime half)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x945530. */
CAkState * AkStateGroupChunk::GetState( AkStateID in_StateTypeID )
{
	AkStateLink * pLink = m_mapStates.Exists( in_StateTypeID );
	return pLink ? pLink->pState : NULL;
}

/** 2012 0x9444d0: every running state transition of the chunk is dropped (and its activity given back). */
void AkStateGroupChunk::FlushStateTransitions()
{
	const AkUInt32 cProps = m_values.Count();
	for( AkUInt32 i = 0; i < cProps; ++i )
	{
		AkStateValue * pValue = m_values.ValueAt( i );
		if( pValue->pTransition )
		{
			g_pTransitionManager->RemoveTransitionUser( pValue->pTransition, this );
			pValue->pTransition = NULL;
			m_pOwner->DecrementActivityCount( 3 );
		}
	}
}

/** 2012 0x944550: the target is the property id; the node recomputes its parameters on every step. */
void AkStateGroupChunk::TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated )
{
	AkStateValue * pValue = m_values.FindProp( (AkUInt8)in_eTarget );
	pValue->fValue = in_fValue;
	if( in_bIsTerminated )
	{
		m_pOwner->DecrementActivityCount( 3 );
		pValue->pTransition = NULL;
	}
	m_pOwner->RecalcNotification();
}

// ------------------------------------------------------------------------------------------------------------
// AkStateGroupInfo / CAkStateMgr
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x92ed70. */
void AkStateGroupInfo::Term()
{
	mapTransitions.Term();
	members.m_pFirst = NULL;
}

/** 2012 0x92e8b0. */
CAkStateMgr::CAkStateMgr()
{
}

/** 2012 0x92e7d0. */
CAkStateMgr::~CAkStateMgr()
{
}

/** 2012 0x92e900: 8 switch and 8 trigger registrations pre-allocated, no maximum. */
AKRESULT CAkStateMgr::Init()
{
	AKRESULT eResult = m_listRegisteredSwitch.Init( 8, (AkUInt32)-1 );
	if( eResult == AK_Success )
	{
		eResult = m_listRegisteredTrigger.Init( 8, (AkUInt32)-1 );
	}
	return eResult;
}

/** 2012 0x92f6e0. */
void CAkStateMgr::Term()
{
	RemoveAllStateGroups( false );
	m_StateGroups.Term();
	m_listRegisteredSwitch.Term();
	m_listRegisteredTrigger.Term();
	TermPreparationGroup( m_PreparationGroupsStates );
	TermPreparationGroup( m_PreparationGroupsSwitches );
}

/** 2012 0x92f4f0: STMG declares every group once; a second declaration keeps the first. */
AKRESULT CAkStateMgr::AddStateGroup( AkStateGroupID in_ulStateGroupID )
{
	if( m_StateGroups.Exists( in_ulStateGroupID ) )
	{
		return AK_Success;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkStateGroupInfo ) );
	if( !pMem )
	{
		return AK_Fail;
	}
	AkStateGroupInfo * pGroup = AkPlacementNew( pMem ) AkStateGroupInfo();
	if( m_StateGroups.Set( in_ulStateGroupID, pGroup ) )
	{
		return AK_Success;
	}
	pGroup->Term();
	AK::MemoryMgr::Free( g_DefaultPoolId, pGroup );
	return AK_Fail;
}

/** 2012 0x92e9c0: a member of an undeclared group is refused (AK_IDNotFound). */
AKRESULT CAkStateMgr::AddStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember )
{
	AkStateGroupInfo ** ppGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !ppGroup )
	{
		return AK_IDNotFound;
	}
	( *ppGroup )->members.AddFirst( in_pMember );
	return AK_Success;
}

/** 2012 0x92ea20. */
AKRESULT CAkStateMgr::SetdefaultTransitionTime( AkStateGroupID in_ulStateGroupID, AkTimeMs lTransitionTime )
{
	AkStateGroupInfo ** ppGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !ppGroup )
	{
		return AK_IDNotFound;
	}
	( *ppGroup )->lDefaultTransitionTime = lTransitionTime;
	return AK_Success;
}

/** 2012 0x92ea60. */
AkStateID CAkStateMgr::GetState( AkStateGroupID in_ulStateGroupID )
{
	AkStateGroupInfo ** ppGroup = m_StateGroups.Exists( in_ulStateGroupID );
	return ppGroup ? ( *ppGroup )->ActualState : 0;
}

/** 2012 0x92eda0. */
AKRESULT CAkStateMgr::RemoveStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember )
{
	AkStateGroupInfo ** ppGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !ppGroup )
	{
		return AK_IDNotFound;
	}
	( *ppGroup )->members.Remove( in_pMember );
	return AK_Success;
}

/** 2012 0x92edf0: a shared transition time also applies the other way. */
AKRESULT CAkStateMgr::AddStateTransition( AkStateGroupID in_ulStateGroupID, AkStateID in_ulStateID1, AkStateID in_ulStateID2, AkTimeMs lTransitionTime, bool in_bIsShared )
{
	AkStateGroupInfo ** ppGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !ppGroup )
	{
		return AK_IDNotFound;
	}
	AkStateTransition transition;
	transition.StateFrom = in_ulStateID1;
	transition.StateTo = in_ulStateID2;
	if( !( *ppGroup )->mapTransitions.Set( transition, lTransitionTime ) )
	{
		return AK_Fail;
	}
	if( in_bIsShared )
	{
		transition.StateFrom = in_ulStateID2;
		transition.StateTo = in_ulStateID1;
		if( !( *ppGroup )->mapTransitions.Set( transition, lTransitionTime ) )
		{
			return AK_Fail;
		}
	}
	return AK_Success;
}

/** 2012 0x92e7e0: a changed state is pushed to the switch containers following the group (no game object). */
AKRESULT CAkStateMgr::UpdateSwitches( AkStateGroupID in_ulStateGroup, AkStateID in_StateFrom, AkStateID in_StateTo )
{
	if( in_StateFrom == in_StateTo )
	{
		return AK_Success;
	}
	for( CAkList2<RegisteredSwitch, const RegisteredSwitch &, 1>::ListItem * pItem = m_listRegisteredSwitch.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		if( pItem->Item.ulStateGroup == in_ulStateGroup )
		{
			pItem->Item.pSwitch->SetSwitch( in_StateTo, NULL );
		}
	}
	return AK_Success;
}

/**
 * 2012 0x92ee70: the switch containers follow first; then, unless the external handler (the music engine) takes the
 * change, every member chunk moves its four state values (volume, LFE, pitch, LPF: the target is the property id)
 * to the new state's values over the group's transition time (the custom one for this pair, else the default) with
 * a linear state transition; a value without a transition is set at once and the node recomputes.
 */
void CAkStateMgr::SetStateInternal( AkStateGroupID in_ulStateGroupID, AkStateID in_ulStateID, bool in_bSkipTransitionTime, bool in_bSkipExtension )
{
	AkStateGroupInfo ** ppGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !ppGroup )
	{
		AKDIAG( L"akdiag: SetState group 0x%08x = 0x%08x: group not declared (no STMG entry), ignored", in_ulStateGroupID, in_ulStateID );
		return;
	}
	AkStateGroupInfo * pGroup = *ppGroup;
	if( !in_bSkipExtension )
	{
		UpdateSwitches( in_ulStateGroupID, pGroup->ActualState, in_ulStateID );
		if( g_pExternalStateHandler && !in_bSkipTransitionTime && g_pExternalStateHandler( in_ulStateGroupID, in_ulStateID ) )
		{
			AKDIAG( L"akdiag: SetState group 0x%08x 0x%08x -> 0x%08x: taken by the external state handler", in_ulStateGroupID, pGroup->ActualState, in_ulStateID );
			return;
		}
	}
	AkTimeMs Time = 0;
	if( !in_bSkipTransitionTime )
	{
		AkStateTransition transition;
		transition.StateFrom = pGroup->ActualState;
		transition.StateTo = in_ulStateID;
		AkTimeMs * pTime = pGroup->mapTransitions.Exists( transition );
		Time = pTime ? *pTime : pGroup->lDefaultTransitionTime;
	}
	AkUInt32 uNumMembers = 0;
	for( AkStateGroupChunk * pChunk = pGroup->members.First(); pChunk; pChunk = pChunk->m_pNextInGroup )
	{
		++uNumMembers;
		bool bRecalc = false;
		pChunk->m_ulActualState = in_ulStateID;
		CAkState * pState = pChunk->GetState( in_ulStateID );
		for( AkUInt8 iProp = 0; iProp < 4; ++iProp )
		{
			AkStateValue * pValue = pChunk->m_values.FindProp( iProp );
			const AkReal32 * pTarget = pState ? pState->m_props.FindProp( iProp ) : NULL;
			const AkReal32 fNewTarget = pTarget ? *pTarget : 0.f;
			if( pValue && pValue->pTransition )
			{
				g_pTransitionManager->ChangeParameter( pValue->pTransition, (TransitionTargets)iProp, fNewTarget, Time, AkCurveInterpolation_Linear, AkValueMeaning_Default );
				continue;
			}
			const AkReal32 fValue = pValue ? pValue->fValue : 0.f;
			if( fValue == fNewTarget )
			{
				continue;
			}
			TransitionParameters trans;
			trans.pUser = pChunk;
			trans.eTarget = (TransitionTargets)iProp;
			trans.fStartValue = fValue;
			trans.fTargetValue = fNewTarget;
			trans.lDuration = Time;
			trans.eFadeCurve = AkCurveInterpolation_Linear;
			trans.bdBs = g_AkPropDecibel[ iProp ];
			trans.bUseReciprocalCurve = true;
			CAkTransition * pTransition = g_pTransitionManager->AddTransitionToList( trans, true, CAkTransitionManager::TC_State );
			if( !pValue )
			{
				pValue = pChunk->m_values.AddAkProp( iProp );
				if( pValue )
				{
					pValue->fValue = fValue;
				}
			}
			if( pValue )
			{
				pValue->pTransition = pTransition;
				if( pTransition )
				{
					pChunk->m_pOwner->IncrementActivityCount( 3 );
				}
				else
				{
					pValue->fValue = fNewTarget;
					bRecalc = true;
				}
			}
			else if( pTransition )
			{
				g_pTransitionManager->RemoveTransitionUser( pTransition, pChunk );
			}
		}
		if( bRecalc )
		{
			pChunk->m_pOwner->RecalcNotification();
		}
	}
	AKDIAG( L"akdiag: SetState group 0x%08x 0x%08x -> 0x%08x: %u member chunk(s), transition %d ms", in_ulStateGroupID, pGroup->ActualState,
		in_ulStateID, uNumMembers, Time );
	pGroup->ActualState = in_ulStateID;
}

/** 2012 0x92f0f0. */
AKRESULT CAkStateMgr::RegisterSwitch( CAkSwitchAware * in_pSwitchCntr, AkStateGroupID in_ulStateGroup )
{
	if( !in_pSwitchCntr )
	{
		return AK_InvalidParameter;
	}
	RegisteredSwitch registration;
	registration.pSwitch = in_pSwitchCntr;
	registration.ulStateGroup = in_ulStateGroup;
	return m_listRegisteredSwitch.AddLast( registration ) ? AK_Success : AK_Fail;
}

/** 2012 0x92f140: every registration of the container. */
AKRESULT CAkStateMgr::UnregisterSwitch( CAkSwitchAware * in_pSwitchCntr )
{
	CAkList2<RegisteredSwitch, const RegisteredSwitch &, 1>::ListItem * pPrev = NULL;
	CAkList2<RegisteredSwitch, const RegisteredSwitch &, 1>::ListItem * pItem = m_listRegisteredSwitch.m_pFirst;
	while( pItem )
	{
		CAkList2<RegisteredSwitch, const RegisteredSwitch &, 1>::ListItem * pNext = pItem->pNextListItem;
		if( pItem->Item.pSwitch == in_pSwitchCntr )
		{
			m_listRegisteredSwitch.RemoveItem( pItem, pPrev );
		}
		else
		{
			pPrev = pItem;
		}
		pItem = pNext;
	}
	return AK_Success;
}

/** 2012 0x92f330: a trigger listener hears every trigger id (0) of its game object. */
AKRESULT CAkStateMgr::RegisterTrigger( IAkTriggerAware * in_pTrigerAware, CAkRegisteredObj * in_GameObj )
{
	RegisteredTrigger registration;
	registration.pTriggerAware = in_pTrigerAware;
	registration.triggerID = 0;
	registration.gameObj = in_GameObj;
	return m_listRegisteredTrigger.AddLast( registration ) ? AK_Success : AK_Fail;
}

/** 2012 0x92f370. */
AKRESULT CAkStateMgr::UnregisterTrigger( IAkTriggerAware * in_pTrigerAware )
{
	CAkList2<RegisteredTrigger, const RegisteredTrigger &, 1>::ListItem * pPrev = NULL;
	CAkList2<RegisteredTrigger, const RegisteredTrigger &, 1>::ListItem * pItem = m_listRegisteredTrigger.m_pFirst;
	while( pItem )
	{
		CAkList2<RegisteredTrigger, const RegisteredTrigger &, 1>::ListItem * pNext = pItem->pNextListItem;
		if( pItem->Item.pTriggerAware == in_pTrigerAware )
		{
			m_listRegisteredTrigger.RemoveItem( pItem, pPrev );
		}
		else
		{
			pPrev = pItem;
		}
		pItem = pNext;
	}
	return AK_Success;
}

/** 2012 0x92e830. */
void CAkStateMgr::Trigger( AkTriggerID in_Trigger, CAkRegisteredObj * in_GameObj )
{
	for( CAkList2<RegisteredTrigger, const RegisteredTrigger &, 1>::ListItem * pItem = m_listRegisteredTrigger.m_pFirst; pItem; pItem = pItem->pNextListItem )
	{
		const AkTriggerID triggerID = pItem->Item.triggerID;
		if( ( !triggerID || triggerID == in_Trigger ) && ( !in_GameObj || pItem->Item.gameObj == in_GameObj ) )
		{
			pItem->Item.pTriggerAware->Trigger( in_Trigger );
		}
	}
	AKDIAG( L"akdiag: PostTrigger 0x%08x on game object %d", in_Trigger, in_GameObj ? (AkInt32)in_GameObj->m_GameObjID : -1 );
}

/** 2012 0x92f5b0: ClearBanks keeps the groups that still have members. */
AKRESULT CAkStateMgr::RemoveAllStateGroups( bool in_bIsFromClearBanks )
{
	MapStruct<AkStateGroupID, AkStateGroupInfo *> * pItem = m_StateGroups.m_pItems;
	while( pItem != m_StateGroups.m_pItemsEnd )
	{
		AkStateGroupInfo * pGroup = pItem->item;
		if( in_bIsFromClearBanks && pGroup->members.First() )
		{
			++pItem;
			continue;
		}
		pGroup->Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, pGroup );
		if( m_StateGroups.Length() > 1 )
		{
			*pItem = *( m_StateGroups.m_pItemsEnd - 1 );
		}
		--m_StateGroups.m_pItemsEnd;
	}
	return AK_Success;
}

/** 2012 0x92f1c0: the preparation record of a state / switch group, created on first use. */
CAkStateMgr::PreparationStateItem * CAkStateMgr::GetPreparationItem( AkUInt32 in_ulGroup, AkGroupType in_eGroupType )
{
	AkAutoLock<CAkLock> gate( m_PrepareGameSyncLock );
	PreparationList & rList = in_eGroupType == AkGroupType_State ? m_PreparationGroupsStates : m_PreparationGroupsSwitches;
	for( PreparationStateItem * pItem = rList.m_pFirst; pItem; pItem = pItem->pNextItem )
	{
		if( pItem->m_GroupID == in_ulGroup )
		{
			return pItem;
		}
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( PreparationStateItem ) );
	if( !pMem )
	{
		return NULL;
	}
	PreparationStateItem * pItem = AkPlacementNew( pMem ) PreparationStateItem( in_ulGroup );
	rList.AddFirst( pItem );
	return pItem;
}

/** 2012 0x92e760: tell every preparation-aware object; a failed activation rolls back the ones already told. */
AKRESULT CAkStateMgr::PreparationStateItem::Notify( AkUInt32 in_GameSyncID, bool in_bIsActive )
{
	AKRESULT eResult = AK_Success;
	CAkPreparationAware * pFailed = NULL;
	for( CAkPreparationAware * pItem = m_PreparationList.m_pFirst; pItem; pItem = pItem->pNextItemPrepare )
	{
		eResult = pItem->ModifyActiveState( in_GameSyncID, in_bIsActive );
		if( eResult != AK_Success )
		{
			pFailed = pItem;
			break;
		}
	}
	if( pFailed && in_bIsActive )
	{
		for( CAkPreparationAware * pItem = m_PreparationList.m_pFirst; pItem != pFailed; pItem = pItem->pNextItemPrepare )
		{
			pItem->ModifyActiveState( in_GameSyncID, false );
		}
	}
	return eResult;
}

/** 2012 0x92f270: add / remove a game sync of a group from its prepared content and tell its listeners. */
AKRESULT CAkStateMgr::PrepareGameSync( AkGroupType in_eGroupType, AkUInt32 in_uGroupID, AkUInt32 in_uGameSyncID, bool in_bIsActive )
{
	PreparationStateItem * pItem = GetPreparationItem( in_uGroupID, in_eGroupType );
	if( !pItem )
	{
		return in_bIsActive ? AK_InsufficientMemory : AK_Success;
	}
	AkArray<AkUInt32, AkUInt32, ArrayPoolDefault, 4> & rContent = pItem->m_PreparedContent.m_PreparableContentList;
	AkUInt32 * pSync = rContent.Exists( in_uGameSyncID );
	if( pSync )
	{
		if( in_bIsActive )
		{
			return AK_Success;
		}
		rContent.RemoveSwap( in_uGameSyncID );
	}
	else
	{
		if( !in_bIsActive )
		{
			return AK_Success;
		}
		rContent.AddLast( in_uGameSyncID );
	}
	const AKRESULT eResult = pItem->Notify( in_uGameSyncID, in_bIsActive );
	if( eResult != AK_Success )
	{
		rContent.RemoveSwap( in_uGameSyncID );
	}
	return eResult;
}

/** 2012 0x92f640. */
void CAkStateMgr::TermPreparationGroup( PreparationList & in_rGroup )
{
	while( PreparationStateItem * pItem = in_rGroup.m_pFirst )
	{
		in_rGroup.Remove( pItem );
		pItem->m_PreparedContent.m_PreparableContentList.Term();
		AK::MemoryMgr::Free( g_DefaultPoolId, pItem );
	}
	in_rGroup.m_pFirst = NULL;
	in_rGroup.m_pLast = NULL;
}

// ------------------------------------------------------------------------------------------------------------
// CAkPreparationAware
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95ea80. */
CAkPreparedContent * CAkPreparationAware::GetPreparedContent( AkUInt32 in_ulGroup, AkGroupType in_eGroupType )
{
	CAkStateMgr::PreparationStateItem * pItem = g_pStateMgr->GetPreparationItem( in_ulGroup, in_eGroupType );
	return pItem ? &pItem->m_PreparedContent : NULL;
}

/** 2012 0x95eab0. */
AKRESULT CAkPreparationAware::SubscribePrepare( AkUInt32 in_ulGroup, AkGroupType in_eGroupType )
{
	CAkStateMgr::PreparationStateItem * pItem = g_pStateMgr->GetPreparationItem( in_ulGroup, in_eGroupType );
	if( !pItem )
	{
		return AK_Fail;
	}
	pItem->m_PreparationList.AddFirst( this );
	return AK_Success;
}

/** 2012 0x95eb60. */
void CAkPreparationAware::UnsubscribePrepare( AkUInt32 in_ulGroup, AkGroupType in_eGroupType )
{
	CAkStateMgr::PreparationStateItem * pItem = g_pStateMgr->GetPreparationItem( in_ulGroup, in_eGroupType );
	if( pItem )
	{
		pItem->m_PreparationList.Remove( this );
	}
}
