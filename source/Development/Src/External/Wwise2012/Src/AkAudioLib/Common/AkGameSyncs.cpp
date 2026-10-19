/*=============================================================================
	AkGameSyncs.cpp - the registration side of CAkStateMgr / CAkRTPCMgr / CAkEnvironmentsMgr (see AkGameSyncs.h).

	DISHONORED(port): agent IH1 (wave 26). Data stores only; IH3 ports the managers proper.
=============================================================================*/
#include "AkGameSyncs.h"
#include "AkHierarchy.h"

// ------------------------------------------------------------------------------------------------------------
// CAkStateMgr
// ------------------------------------------------------------------------------------------------------------

AKRESULT CAkStateMgr::Init()
{
	m_StateGroups.Init( g_DefaultPoolId );
	return AK_Success;
}

void CAkStateMgr::Term()
{
	RemoveAllStateGroups( false );
	m_StateGroups.Term();
}

AkStateGroupInfo * CAkStateMgr::AddStateGroup( AkStateGroupID in_ulStateGroupID )
{
	return m_StateGroups.Set( in_ulStateGroupID );
}

AKRESULT CAkStateMgr::SetdefaultTransitionTime( AkStateGroupID in_ulStateGroupID, AkTimeMs in_lTransitionTime )
{
	AkStateGroupInfo * pGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !pGroup )
	{
		return AK_IDNotFound;
	}
	pGroup->lDefaultTransitionTime = in_lTransitionTime;
	return AK_Success;
}

AKRESULT CAkStateMgr::AddStateTransition( AkStateGroupID in_ulStateGroupID, AkStateID in_ulStateID1, AkStateID in_ulStateID2, AkTimeMs in_lTransitionTime, bool in_bIsShared )
{
	AkStateGroupInfo * pGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( !pGroup )
	{
		return AK_IDNotFound;
	}
	for( AkStateGroupInfo::Transition * p = pGroup->mapTransitions.m_pItems; p != pGroup->mapTransitions.m_pItemsEnd; ++p )
	{
		if( p->StateFrom == in_ulStateID1 && p->StateTo == in_ulStateID2 )
		{
			p->TransitionTime = in_lTransitionTime;
			return AK_Success;
		}
	}
	AkStateGroupInfo::Transition transition;
	transition.StateFrom = in_ulStateID1;
	transition.StateTo = in_ulStateID2;
	transition.TransitionTime = in_lTransitionTime;
	return pGroup->mapTransitions.AddLast( transition ) ? AK_Success : AK_InsufficientMemory;
}

/** A member registers its group, creating it on first use (retail AddStateGroupMember creates the group). */
AKRESULT CAkStateMgr::AddStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember )
{
	AkStateGroupInfo * pGroup = m_StateGroups.Set( in_ulStateGroupID );
	if( !pGroup )
	{
		return AK_InsufficientMemory;
	}
	return pGroup->members.AddLast( in_pMember ) ? AK_Success : AK_InsufficientMemory;
}

void CAkStateMgr::RemoveStateGroupMember( AkStateGroupID in_ulStateGroupID, AkStateGroupChunk * in_pMember )
{
	AkStateGroupInfo * pGroup = m_StateGroups.Exists( in_ulStateGroupID );
	if( pGroup )
	{
		pGroup->members.RemoveSwap( in_pMember );
	}
}

AkStateID CAkStateMgr::GetState( AkStateGroupID in_ulStateGroupID )
{
	AkStateGroupInfo * pGroup = m_StateGroups.Exists( in_ulStateGroupID );
	return pGroup ? pGroup->ActualState : 0;
}

/** CAkBankMgr::SignalLastBankUnloaded / Term: groups without members go; ClearBanks keeps member groups. */
void CAkStateMgr::RemoveAllStateGroups( bool in_bIsFromClearBanks )
{
	for( AkUInt32 i = 0; i < 31; ++i )
	{
		AkHashList<AkStateGroupID, AkStateGroupInfo, 31>::Item * pItem = m_StateGroups.m_table[ i ];
		while( pItem )
		{
			AkHashList<AkStateGroupID, AkStateGroupInfo, 31>::Item * pNext = pItem->pNextItem;
			if( !in_bIsFromClearBanks || !pItem->Assoc.item.members.Length() )
			{
				m_StateGroups.Unset( pItem->Assoc.key );
			}
			pItem = pNext;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkRTPCMgr
// ------------------------------------------------------------------------------------------------------------

AKRESULT CAkRTPCMgr::Init()
{
	m_Subscribers.Init( g_DefaultPoolId );
	m_uNumSubscriptions = 0;
	m_SwitchRTPC.Init( g_DefaultPoolId );
	m_DefaultParamValues.Init( g_DefaultPoolId );
	return AK_Success;
}

void CAkRTPCMgr::Term()
{
	m_Subscribers.Term();
	m_uNumSubscriptions = 0;
	for( AkHashList<AkSwitchGroupID, AkSwitchRTPC, 31>::Iterator it = m_SwitchRTPC.Begin(); it != m_SwitchRTPC.End(); ++it )
	{
		( *it ).item.Curve.Unset();
	}
	m_SwitchRTPC.Term();
	m_DefaultParamValues.Term();
	m_Layers.Term();
}

/** CAkRTPCMgr::SubscribeRTPC: one subscription per (subscriber, parameter), the curve copied. */
AKRESULT CAkRTPCMgr::SubscribeRTPC( void * in_pSubscriber, AkRtpcID in_RTPC_ID, AkUInt32 in_ParamID, AkUniqueID in_RTPCCurveID, AkCurveScaling in_eScaling, AkRTPCGraphPoint * in_pArrayConversion, AkUInt32 in_ulConversionArraySize, void *, SubscriberType in_eType )
{
	UnSubscribeRTPC( in_pSubscriber, in_ParamID );
	AkRTPCSubscriberEntry * pEntry = m_Subscribers.Set( (AkUIntPtr)in_pSubscriber );
	if( !pEntry )
	{
		return AK_InsufficientMemory;
	}
	AkRTPCSubscription * pSubscription = pEntry->subscriptions.AddLast();
	if( !pSubscription )
	{
		return AK_InsufficientMemory;
	}
	AkPlacementNew( &pSubscription->Curve ) CAkConversionTable<AkRTPCGraphPoint, AkReal32>();
	pSubscription->RTPCID = in_RTPC_ID;
	pSubscription->ParamID = in_ParamID;
	pSubscription->RTPCCurveID = in_RTPCCurveID;
	pSubscription->eType = in_eType;
	if( in_ulConversionArraySize )
	{
		pSubscription->Curve.Set( in_pArrayConversion, in_ulConversionArraySize, in_eScaling );
	}
	++m_uNumSubscriptions;
	return AK_Success;
}

void CAkRTPCMgr::UnSubscribeRTPC( void * in_pSubscriber, AkUInt32 in_ParamID )
{
	AkRTPCSubscriberEntry * pEntry = m_Subscribers.Exists( (AkUIntPtr)in_pSubscriber );
	if( !pEntry )
	{
		return;
	}
	for( AkRTPCSubscription * p = pEntry->subscriptions.m_pItems; p != pEntry->subscriptions.m_pItemsEnd; ++p )
	{
		if( p->ParamID == in_ParamID )
		{
			p->Curve.Unset();
			pEntry->subscriptions.Erase( (AkUInt32)( p - pEntry->subscriptions.m_pItems ) );
			--m_uNumSubscriptions;
			break;
		}
	}
	if( pEntry->subscriptions.IsEmpty() )
	{
		m_Subscribers.Unset( (AkUIntPtr)in_pSubscriber );
	}
}

AKRESULT CAkRTPCMgr::AddSwitchRTPC( AkSwitchGroupID in_switchGroup, AkRtpcID in_rtpcID, AkRTPCGraphPointBase<AkUInt32> * in_pArrayConversion, AkUInt32 in_ulConversionArraySize )
{
	AkSwitchRTPC * pEntry = m_SwitchRTPC.Set( in_switchGroup );
	if( !pEntry )
	{
		return AK_InsufficientMemory;
	}
	pEntry->SwitchGroup = in_switchGroup;
	pEntry->RTPCID = in_rtpcID;
	return pEntry->Curve.Set( in_pArrayConversion, in_ulConversionArraySize, AkCurveScaling_None );
}

void CAkRTPCMgr::SetDefaultParamValue( AkRtpcID in_RTPCid, AkReal32 in_fValue )
{
	AkReal32 * pValue = m_DefaultParamValues.Set( in_RTPCid );
	if( pValue )
	{
		*pValue = in_fValue;
	}
}

AKRESULT CAkRTPCMgr::RegisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID )
{
	AkLayerRegistration registration;
	registration.pLayer = in_pLayer;
	registration.RTPCID = in_rtpcID;
	return m_Layers.AddLast( registration ) ? AK_Success : AK_InsufficientMemory;
}

void CAkRTPCMgr::UnregisterLayer( CAkLayer * in_pLayer, AkRtpcID in_rtpcID )
{
	for( AkLayerRegistration * p = m_Layers.m_pItems; p != m_Layers.m_pItemsEnd; ++p )
	{
		if( p->pLayer == in_pLayer && p->RTPCID == in_rtpcID )
		{
			m_Layers.Erase( (AkUInt32)( p - m_Layers.m_pItems ) );
			return;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkEnvironmentsMgr
// ------------------------------------------------------------------------------------------------------------

AKRESULT CAkEnvironmentsMgr::Init()
{
	return AK_Success;
}

void CAkEnvironmentsMgr::Term()
{
	for( AkUInt32 i = 0; i < MAX_CURVE_X_TYPES; ++i )
	{
		for( AkUInt32 j = 0; j < MAX_CURVE_Y_TYPES; ++j )
		{
			m_ObsOccCurves[ i ][ j ].Unset();
		}
	}
}

AKRESULT CAkEnvironmentsMgr::SetObsOccCurve( eCurveXType in_x, eCurveYType in_y, AkUInt32 in_ulNbPoints, AkRTPCGraphPoint * in_paPoints, AkCurveScaling in_eScaling )
{
	return m_ObsOccCurves[ in_x ][ in_y ].Set( in_paPoints, in_ulNbPoints, in_eScaling );
}
