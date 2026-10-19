/*=============================================================================
	AkActionsSetValue.cpp - the value-setting actions an event runs: SetValue (Mute, SetAkProp, SetGameParameter),
	SetState, SetSwitch, UseState, Trigger, BypassFX, Seek, Duck, the exception list and CAkAction::SetAkProp
	(akaction.cpp, akactionexcept.cpp, akactionsetvalue.cpp, akactionmute.cpp, akactionsetakprop.cpp,
	akactionsetgameparameter.cpp, akactionsetstate.cpp, akactionsetswitch.cpp, akactionusestate.cpp, akactiontrigger.cpp,
	akactionbypassfx.cpp, akactionseek.cpp, akactionduck.cpp).

	DISHONORED(port): agent IH3 fork N (wave 26); 2012 rva on each definition, 2013 from ih1_inventory.csv. Each action
	type's Create is IH1's CAkAction::Create dispatch (AkHierarchy.cpp).
	Deviations (named):
	  * The BypassFX action sets the bypass bits on the nodes, their SIS and the voices; an effect instance that would
	    honour them is IH4's.
	  * -akdiag logs every SetValue / SetState / SetSwitch action executed.
=============================================================================*/
#include "AkAudioMgr.h"
#include "AkPBI.h"
#include "AkRegistryMgr.h"
#include <stdlib.h>

/** RandomizerModifier::GetModValue<float> as the actions inline it: the base plus a draw in [min, max]. */
static AkReal32 GetModValue( const RANGED_PARAMETER<AkReal32> & in_rParam )
{
	const AkReal32 fRange = in_rParam.m_mod.m_max - in_rParam.m_mod.m_min;
	const AkReal32 fRand = fRange != 0.f ? (AkReal32)( (double)rand() * ( 1.0 / 32767.0 ) * fRange ) : 0.f;
	return fRand + in_rParam.m_mod.m_min + in_rParam.m_base;
}

/** 2012 0x94a1c0 (2013 0x946a10): a property and its range; an unchanged value with an empty range is left as it is. */
AKRESULT CAkAction::SetAkProp( AkPropID in_eProp, AkReal32 in_fValue, AkReal32 in_fMin, AkReal32 in_fMax )
{
	const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
	const AkReal32 fCurrent = pProp ? pProp->fValue : g_AkPropDefault[ in_eProp ].fValue;
	if( fCurrent != in_fValue )
	{
		const AKRESULT eResult = m_props.SetAkProp( (AkUInt8)in_eProp, AkPropValue( in_fValue ) );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	if( in_fMin == 0.f && in_fMax == 0.f && !m_ranges.FindProp( (AkUInt8)in_eProp ) )
	{
		return AK_Success;
	}
	RANGED_MODIFIERS<AkPropValue> range;
	range.m_min = AkPropValue( in_fMin );
	range.m_max = AkPropValue( in_fMax );
	return m_ranges.SetAkProp( (AkUInt8)in_eProp, range );
}

/** 2012 0x94a2b0 (2013 0x946b00). */
AKRESULT CAkAction::SetAkProp( AkPropID in_eProp, AkInt32 in_iValue, AkInt32 in_iMin, AkInt32 in_iMax )
{
	const AkPropValue * pProp = m_props.FindProp( (AkUInt8)in_eProp );
	const AkInt32 iCurrent = pProp ? pProp->iValue : g_AkPropDefault[ in_eProp ].iValue;
	if( iCurrent != in_iValue )
	{
		AkPropValue value;
		value.iValue = in_iValue;
		const AKRESULT eResult = m_props.SetAkProp( (AkUInt8)in_eProp, value );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	if( !in_iMin && !in_iMax && !m_ranges.FindProp( (AkUInt8)in_eProp ) )
	{
		return AK_Success;
	}
	RANGED_MODIFIERS<AkPropValue> range;
	range.m_min.iValue = in_iMin;
	range.m_max.iValue = in_iMax;
	return m_ranges.SetAkProp( (AkUInt8)in_eProp, range );
}

/** 2012 0x95dd30 (2013 0x95a600). */
AKRESULT CAkActionExcept::AddException( AkUniqueID in_ulElementID )
{
	if( m_listElementException.Exists( in_ulElementID ) )
	{
		return AK_Success;
	}
	return m_listElementException.AddLast( in_ulElementID ) ? AK_Success : AK_Fail;
}

/** 2012 0x95ddb0 (2013 0x95a680). */
void CAkActionExcept::RemoveException( AkUniqueID in_ulElementID )
{
	m_listElementException.Remove( in_ulElementID );
}

// ------------------------------------------------------------------------------------------------------------
// CAkActionSetValue and its three kinds
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95fdf0 (2013 0x95c5e0). */
void CAkActionSetValue::ResetAllHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID )
{
	if( !in_pListID )
	{
		return;
	}
	for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = in_pListID->m_pFirst; p; p = p->pNextListItem )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
		if( pNode )
		{
			ExecResetValueAll( pNode );
			pNode->Release();
		}
	}
}

/** 2012 0x95fe40 (2013 0x95c630). */
void CAkActionSetValue::ResetAEHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID )
{
	if( !in_pListID )
	{
		return;
	}
	for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = in_pListID->m_pFirst; p; p = p->pNextListItem )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
		if( pNode )
		{
			ExecResetValueExcept( pNode );
			pNode->Release();
		}
	}
}

/** 2012 0x95fe90 (2013 0x95c680): the low byte is the scope - 2 the element, 3 the element on the game object, 4 every
    node any SetValue touched, 5 those of the game object, 8 / 9 the same minus the exceptions. The set types (mute,
    pitch, volume, LPF) run scopes 2 / 3, the reset types (unmute, the resets) all six; the game parameter (0x13 / 0x14)
    has no node. */
AKRESULT CAkActionSetValue::Execute( AkPendingAction * in_pAction )
{
	const AkUInt32 eScope = m_eActionType & 0xFF;
	CAkRegisteredObj * pGameObj = in_pAction->pGameObj;
	AKDIAG( L"akdiag: set-value action 0x%08x type 0x%04x on element 0x%08x, game object %u", key, m_eActionType, m_ulElementID, pGameObj ? (AkUInt32)pGameObj->m_GameObjID : 0 );
	switch( m_eActionType & 0xFF00 )
	{
	case 0x1300:
		if( eScope == 2 || eScope == 3 )
		{
			ExecSetValue( NULL, pGameObj );
		}
		return AK_Success;
	case 0x1400:
		if( eScope == 2 || eScope == 3 )
		{
			ExecResetValue( NULL, pGameObj );
		}
		return AK_Success;
	case 0x0600:
	case 0x0800:
	case 0x0A00:
	case 0x0E00:
	{
		if( eScope != 2 && eScope != 3 )
		{
			return AK_Success;
		}
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
		if( pNode )
		{
			if( eScope == 2 )
			{
				ExecSetValue( pNode );
			}
			else
			{
				ExecSetValue( pNode, pGameObj );
			}
			pNode->Release();
		}
		return AK_Success;
	}
	case 0x0700:
	case 0x0900:
	case 0x0B00:
	case 0x0F00:
		break;
	default:
		return AK_Success;
	}
	switch( eScope )
	{
	case 2:
	case 3:
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
		if( pNode )
		{
			if( eScope == 2 )
			{
				ExecResetValue( pNode );
			}
			else
			{
				ExecResetValue( pNode, pGameObj );
			}
			pNode->Release();
		}
		return AK_Success;
	}
	case 4:
	case 8:
		if( eScope == 4 )
		{
			ResetAllHelper( &g_pRegistryMgr->m_listModifiedNodes );
		}
		else
		{
			ResetAEHelper( &g_pRegistryMgr->m_listModifiedNodes );
		}
		for( AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::Iterator it = g_pRegistryMgr->m_mapRegisteredObj.Begin(); it != g_pRegistryMgr->m_mapRegisteredObj.End(); ++it )
		{
			if( eScope == 4 )
			{
				ResetAllHelper( ( *it ).item->m_pListModifiedNodes );
			}
			else
			{
				ResetAEHelper( ( *it ).item->m_pListModifiedNodes );
			}
		}
		return AK_Success;
	case 5:
	case 9:
		if( !pGameObj->m_pListModifiedNodes )
		{
			return AK_Success;
		}
		for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = pGameObj->m_pListModifiedNodes->m_pFirst; p; p = p->pNextListItem )
		{
			CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
			if( !pNode )
			{
				continue;
			}
			if( eScope == 5 )
			{
				ExecResetValue( pNode, pGameObj );
			}
			else
			{
				ExecResetValueExcept( pNode, pGameObj );
			}
			pNode->Release();
		}
		return AK_Success;
	default:
		return AK_Success;
	}
}

/** 2012 0x95d4f0 (2013 0x959dc0). */
void CAkActionMute::ExecSetValue( CAkParameterNodeBase * in_pNode )
{
	in_pNode->Mute( NULL, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d520 (2013 0x959df0). */
void CAkActionMute::ExecSetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj )
{
	in_pNode->Mute( in_pGameObj, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d550 (2013 0x959e20). */
void CAkActionMute::ExecResetValue( CAkParameterNodeBase * in_pNode )
{
	in_pNode->Unmute( NULL, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d580 (2013 0x959e50). */
void CAkActionMute::ExecResetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj )
{
	in_pNode->Unmute( in_pGameObj, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d5b0 (2013 0x959e80). */
void CAkActionMute::ExecResetValueAll( CAkParameterNodeBase * in_pNode )
{
	in_pNode->UnmuteAll( (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d650 (2013 0x959f20). */
void CAkActionMute::ExecResetValueExcept( CAkParameterNodeBase * in_pNode )
{
	if( !m_listElementException.Exists( in_pNode->key ) )
	{
		in_pNode->UnmuteAll( (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
	}
}

/** 2012 0x95d6a0 (2013 0x959f70). */
void CAkActionMute::ExecResetValueExcept( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj )
{
	if( !m_listElementException.Exists( in_pNode->key ) )
	{
		in_pNode->Unmute( in_pGameObj, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
	}
}

/** 2012 0x95d270 (2013 0x959b40): the target value drawn in its range. */
void CAkActionSetAkProp::ExecSetValue( CAkParameterNodeBase * in_pNode )
{
	const AkReal32 fValue = GetModValue( m_TargetValue );
	AKDIAG( L"akdiag: set-value prop %u = %.2f (meaning %u) on node 0x%08x, global", m_ePropID, fValue, m_eValueMeaning, in_pNode->key );
	in_pNode->SetAkProp( (AkPropID)m_ePropID, (CAkRegisteredObj *)NULL, (AkValueMeaning)m_eValueMeaning, fValue, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d310 (2013 0x959be0). */
void CAkActionSetAkProp::ExecSetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj )
{
	const AkReal32 fValue = GetModValue( m_TargetValue );
	AKDIAG( L"akdiag: set-value prop %u = %.2f (meaning %u) on node 0x%08x, game object %u", m_ePropID, fValue, m_eValueMeaning, in_pNode->key, in_pGameObj ? (AkUInt32)in_pGameObj->m_GameObjID : 0 );
	in_pNode->SetAkProp( (AkPropID)m_ePropID, in_pGameObj, (AkValueMeaning)m_eValueMeaning, fValue, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d210 (2013 0x959ae0). */
void CAkActionSetAkProp::ExecResetValue( CAkParameterNodeBase * in_pNode )
{
	in_pNode->ResetAkProp( (AkPropID)m_ePropID, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d3b0 (2013 0x959c80). */
void CAkActionSetAkProp::ExecResetValue( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj )
{
	in_pNode->SetAkProp( (AkPropID)m_ePropID, in_pGameObj, AkValueMeaning_Default, 0.f, (AkCurveInterpolation)m_eFadeCurve, GetTransitionTime() );
}

/** 2012 0x95d3f0 (2013 0x959cc0). */
void CAkActionSetAkProp::ExecResetValueExcept( CAkParameterNodeBase * in_pNode )
{
	if( !m_listElementException.Exists( in_pNode->key ) )
	{
		ExecResetValue( in_pNode );
	}
}

/** 2012 0x95d420 (2013 0x959cf0). */
void CAkActionSetAkProp::ExecResetValueExcept( CAkParameterNodeBase * in_pNode, CAkRegisteredObj * in_pGameObj )
{
	if( !m_listElementException.Exists( in_pNode->key ) )
	{
		ExecResetValue( in_pNode, in_pGameObj );
	}
}

/** 2012 0x95cf20 (2013 0x959820): the element is the game parameter. */
void CAkActionSetGameParameter::ExecSetValue( CAkParameterNodeBase * )
{
	TransParams transParams;
	transParams.TransitionTime = GetTransitionTime();
	transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	g_pRTPCMgr->SetRTPCInternal( m_ulElementID, GetModValue( m_TargetValue ), NULL, transParams, (AkValueMeaning)m_eValueMeaning );
}

/** 2012 0x95cfb0 (2013 0x9598b0). */
void CAkActionSetGameParameter::ExecSetValue( CAkParameterNodeBase *, CAkRegisteredObj * in_pGameObj )
{
	TransParams transParams;
	transParams.TransitionTime = GetTransitionTime();
	transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	g_pRTPCMgr->SetRTPCInternal( m_ulElementID, GetModValue( m_TargetValue ), in_pGameObj, transParams, (AkValueMeaning)m_eValueMeaning );
}

/** 2012 0x95ce10 (2013 0x959710). */
void CAkActionSetGameParameter::ExecResetValue( CAkParameterNodeBase * )
{
	TransParams transParams;
	transParams.TransitionTime = GetTransitionTime();
	transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	g_pRTPCMgr->ResetRTPCValue( m_ulElementID, NULL, transParams );
}

/** 2012 0x95ce50 (2013 0x959750). */
void CAkActionSetGameParameter::ExecResetValue( CAkParameterNodeBase *, CAkRegisteredObj * in_pGameObj )
{
	TransParams transParams;
	transParams.TransitionTime = GetTransitionTime();
	transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	g_pRTPCMgr->ResetRTPCValue( m_ulElementID, in_pGameObj, transParams );
}

// ------------------------------------------------------------------------------------------------------------
// Game syncs, triggers, ducking
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95cd60 (2013 0x959630). */
AKRESULT CAkActionSetSwitch::Execute( AkPendingAction * in_pAction )
{
	AKDIAG( L"akdiag: set-switch action 0x%08x: group 0x%08x = 0x%08x", key, m_ulSwitchGroupID, m_ulSwitchStateID );
	g_pRTPCMgr->SetSwitchInternal( m_ulSwitchGroupID, m_ulSwitchStateID, in_pAction->pGameObj );
	return AK_Success;
}

/** 2012 0x95d080 (2013 0x959980). */
AKRESULT CAkActionSetState::Execute( AkPendingAction * )
{
	AKDIAG( L"akdiag: set-state action 0x%08x: group 0x%08x = 0x%08x", key, m_ulSwitchGroupID, m_ulSwitchStateID );
	g_pStateMgr->SetStateInternal( m_ulSwitchGroupID, m_ulSwitchStateID, false, false );
	return AK_Success;
}

/** 2012 0x95d1c0 (2013 0x959a90): 0x1002 uses the states, 0x1102 ignores them. */
AKRESULT CAkActionUseState::Execute( AkPendingAction * )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
	if( pNode )
	{
		pNode->UseState( m_eActionType == 0x1002 );
		pNode->Release();
	}
	return AK_Success;
}

/** 2012 0x95c410 (2013 0x958ce0). */
AKRESULT CAkActionTrigger::Execute( AkPendingAction * in_pAction )
{
	g_pStateMgr->Trigger( m_ulElementID, in_pAction->pGameObj );
	return AK_Success;
}

/** 2012 0x955800 (2013 0x9520a0). */
AKRESULT CAkActionDuck::Execute( AkPendingAction * )
{
	CAkBus * pBus = static_cast<CAkBus *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID ) );
	if( pBus )
	{
		pBus->DuckNotif();
		pBus->Release();
	}
	return AK_Success;
}

// ------------------------------------------------------------------------------------------------------------
// CAkActionBypassFX
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95ca30 (2013 0x959300). */
void CAkActionBypassFX::ResetBypassFxAllHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID )
{
	if( !in_pListID )
	{
		return;
	}
	for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = in_pListID->m_pFirst; p; p = p->pNextListItem )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
		if( pNode )
		{
			pNode->ResetBypassFX( m_uTargetMask, NULL );
			pNode->Release();
		}
	}
}

/** 2012 0x95ca80 (2013 0x959350). */
void CAkActionBypassFX::ResetBypassFXAEHelper( const CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault> * in_pListID )
{
	if( !in_pListID )
	{
		return;
	}
	for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = in_pListID->m_pFirst; p; p = p->pNextListItem )
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
		if( !pNode )
		{
			continue;
		}
		if( !m_listElementException.Exists( pNode->key ) )
		{
			pNode->ResetBypassFX( m_uTargetMask, NULL );
		}
		pNode->Release();
	}
}

/** 2012 0x95caf0 (2013 0x9593c0): 0x1A02 / 03 bypass (or un-bypass) the element, 0x1B02 / 03 reset it, 0x1B04 / 05
    reset every modified node (of the object), 0x1B08 / 09 the same minus the exceptions (retail's 0x1B09 resets the
    object's nodes globally). */
AKRESULT CAkActionBypassFX::Execute( AkPendingAction * in_pAction )
{
	CAkRegisteredObj * pGameObj = in_pAction->pGameObj;
	switch( m_eActionType )
	{
	case 0x1A02:
	case 0x1A03:
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
		if( pNode )
		{
			pNode->BypassFX( m_bIsBypass ? m_uTargetMask : 0, m_uTargetMask, pGameObj, false );
			pNode->Release();
		}
		return AK_Success;
	}
	case 0x1B02:
	case 0x1B03:
	{
		CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
		if( pNode )
		{
			pNode->ResetBypassFX( m_uTargetMask, pGameObj );
			pNode->Release();
		}
		return AK_Success;
	}
	case 0x1B04:
	case 0x1B08:
		if( m_eActionType == 0x1B04 )
		{
			ResetBypassFxAllHelper( &g_pRegistryMgr->m_listModifiedNodes );
		}
		else
		{
			ResetBypassFXAEHelper( &g_pRegistryMgr->m_listModifiedNodes );
		}
		for( AkHashList<AkGameObjectID, CAkRegisteredObj *, 193>::Iterator it = g_pRegistryMgr->m_mapRegisteredObj.Begin(); it != g_pRegistryMgr->m_mapRegisteredObj.End(); ++it )
		{
			if( m_eActionType == 0x1B04 )
			{
				ResetBypassFxAllHelper( ( *it ).item->m_pListModifiedNodes );
			}
			else
			{
				ResetBypassFXAEHelper( ( *it ).item->m_pListModifiedNodes );
			}
		}
		return AK_Success;
	case 0x1B05:
	case 0x1B09:
		if( !pGameObj->m_pListModifiedNodes )
		{
			return AK_Success;
		}
		for( CAkList2<AkUniqueID, AkUniqueID, 1, ArrayPoolDefault>::ListItem * p = pGameObj->m_pListModifiedNodes->m_pFirst; p; p = p->pNextListItem )
		{
			CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( p->Item );
			if( !pNode )
			{
				continue;
			}
			if( m_eActionType == 0x1B05 )
			{
				pNode->ResetBypassFX( m_uTargetMask, pGameObj );
			}
			else if( !m_listElementException.Exists( pNode->key ) )
			{
				pNode->ResetBypassFX( m_uTargetMask, NULL );
			}
			pNode->Release();
		}
		return AK_Success;
	default:
		return AK_Success;
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkActionSeek
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x95c710 (2013 0x958fe0): the seek position drawn in its range, as absolute ms or a fraction of the duration. */
AKRESULT CAkActionSeek::Exec( CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
	if( !pNode )
	{
		return AK_IDNotFound;
	}
	SeekActionParams params;
	params.eType = ActionParamType_Seek;
	params.pGameObj = in_pGameObj;
	params.playingID = in_TargetPlayingID;
	params.transParams.TransitionTime = 0;
	params.transParams.eFadeCurve = AkCurveInterpolation_Linear;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = false;
	params.targetNodeID = 0;
	params.bIsSeekRelativeToDuration = m_bIsSeekRelativeToDuration;
	if( m_bIsSeekRelativeToDuration )
	{
		params.fSeekPercent = GetModValue( m_position );
	}
	else
	{
		params.iSeekTime = (AkTimeMs)GetModValue( m_position );
	}
	params.bSnapToNearestMarker = m_bSnapToNearestMarker;
	const AKRESULT eResult = pNode->ExecuteAction( params );
	pNode->Release();
	return eResult;
}

/** 2012 0x95c580 (2013 0x958e50). */
void CAkActionSeek::AllExec( CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID )
{
	CAkBus * pMasterBus = CAkBus::GetMasterBusAndAddRef();
	if( !pMasterBus )
	{
		return;
	}
	SeekActionParams params;
	params.eType = ActionParamType_Seek;
	params.pGameObj = in_pGameObj;
	params.playingID = in_TargetPlayingID;
	params.transParams.TransitionTime = 0;
	params.transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = in_pGameObj == NULL;
	params.bIsMasterResume = false;
	params.targetNodeID = 0;
	params.bIsSeekRelativeToDuration = m_bIsSeekRelativeToDuration;
	if( m_bIsSeekRelativeToDuration )
	{
		params.fSeekPercent = GetModValue( m_position );
	}
	else
	{
		params.iSeekTime = (AkTimeMs)GetModValue( m_position );
	}
	params.bSnapToNearestMarker = m_bSnapToNearestMarker;
	pMasterBus->ExecuteAction( params );
	pMasterBus->Release();
}

/** 2012 0x95c650 (2013 0x958f20). */
void CAkActionSeek::AllExecExcept( CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID )
{
	CAkBus * pMasterBus = CAkBus::GetMasterBusAndAddRef();
	if( !pMasterBus )
	{
		return;
	}
	SeekActionParamsExcept params;
	params.eType = ActionParamType_Seek;
	params.pExeceptionList = &m_listElementException;
	params.pGameObj = in_pGameObj;
	params.playingID = in_TargetPlayingID;
	params.transParams.TransitionTime = 0;
	params.transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterResume = false;
	params.bIsSeekRelativeToDuration = m_bIsSeekRelativeToDuration;
	if( m_bIsSeekRelativeToDuration )
	{
		params.fSeekPercent = GetModValue( m_position );
	}
	else
	{
		params.iSeekTime = (AkTimeMs)GetModValue( m_position );
	}
	params.bSnapToNearestMarker = m_bSnapToNearestMarker;
	pMasterBus->ExecuteActionExcept( params );
	pMasterBus->Release();
}

/** 2012 0x95c7f0 (2013 0x9590c0). */
AKRESULT CAkActionSeek::Execute( AkPendingAction * in_pAction )
{
	switch( m_eActionType )
	{
	case 0x1E02:
	case 0x1E03:
		return Exec( in_pAction->pGameObj, in_pAction->TargetPlayingID );
	case 0x1E04:
	case 0x1E05:
		AllExec( in_pAction->pGameObj, in_pAction->TargetPlayingID );
		return AK_Success;
	case 0x1E08:
	case 0x1E09:
		AllExecExcept( in_pAction->pGameObj, in_pAction->TargetPlayingID );
		return AK_Success;
	default:
		return AK_Success;
	}
}
