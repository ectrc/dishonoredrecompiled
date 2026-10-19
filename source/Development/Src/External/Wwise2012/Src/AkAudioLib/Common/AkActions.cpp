/*=============================================================================
	AkActions.cpp - the execution half of the actions an event runs: play, stop, pause, resume, break (akaction.cpp,
	akactionplay.cpp, akactionactive.cpp, akactionstop.cpp, akactionpause.cpp, akactionresume.cpp, akactionbreak.cpp).

	DISHONORED(port): agent IH2 (wave 26), pulled from IH3's list (the brief puts play / stop / pause / resume in IH2).
	Deviations (named):
	  * CAkAction::Execute: retail's base slot is pure; every action type now overrides it (the value-setting ones in
	    AkActionsSetValue.cpp, agent IH3), so this default only answers a type no class handles, after an -akdiag line.
	  * CAkActionActive::AllExec's dynamic-sequence pass and the master motion bus (CAkFeedbackBus) are not run:
	    no dynamic sequence is ever opened by the glue and Dishonored has no feedback bus (wwise_bank_dump.py --all).
	  * The monitoring calls (GetHistArray + MONITOR_*) are compiled out as in a release build of the SDK.
=============================================================================*/
#include "AkAudioMgr.h"
#include "AkPBI.h"
#include <stdlib.h>

/** DISHONORED: default for the action types whose Execute belongs to IH3 / IH4 (see the banner). */
AKRESULT CAkAction::Execute( AkPendingAction * in_pAction )
{
	AKDIAG( L"akdiag: action 0x%08x type 0x%04x on element 0x%08x not executed (IH3/IH4 action)", key, m_eActionType, m_ulElementID );
	return AK_Success;
}

/** Shared body of GetTransitionTime 0x94a360 / GetDelayTime 0x94a420: the property (or g_AkPropDefault) plus its range. */
static AkInt32 GetIntPropWithRange( const AkPropBundle<AkPropValue> & in_props, const AkPropBundle< RANGED_MODIFIERS<AkPropValue> > & in_ranges, AkPropID in_eProp )
{
	const AkPropValue * pValue = in_props.FindProp( (AkUInt8)in_eProp );
	AkInt32 iValue = pValue ? pValue->iValue : 0;
	const RANGED_MODIFIERS<AkPropValue> * pRange = in_ranges.FindProp( (AkUInt8)in_eProp );
	if( pRange )
	{
		const AkInt32 iRange = pRange->m_max.iValue - pRange->m_min.iValue;
		AkInt32 iRand = 0;
		if( iRange )
		{
			iRand = (AkInt32)( (double)rand() * ( 1.0 / 32767.0 ) * (double)iRange + 0.5 );
		}
		iValue += iRand + pRange->m_min.iValue;
	}
	return iValue;
}

/** 2012 0x94a360: milliseconds. */
AkTimeMs CAkAction::GetTransitionTime()
{
	return GetIntPropWithRange( m_props, m_ranges, AkPropID_TransitionTime );
}

/** 2012 0x94a420: samples at the pipeline rate (SetInitialValues converted the bank's milliseconds). */
AkInt32 CAkAction::GetDelayTime()
{
	return GetIntPropWithRange( m_props, m_ranges, AkPropID_DelayTime );
}

/** 2012 0x95c270: the probability roll, then the element's Play with the pending action's user params and frame offset. */
AKRESULT CAkActionPlay::Execute( AkPendingAction * in_pAction )
{
	const AkPropValue * pProbability = m_props.FindProp( AkPropID_Probability );
	if( pProbability )
	{
		if( pProbability->fValue == 0.f )
		{
			return AK_Success;
		}
		const AkInt32 iRand = ( rand() << 15 ) + rand();
		if( (double)iRand * 9.313225754828403e-10 * 100.0 > pProbability->fValue )
		{
			return AK_Success;
		}
	}
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
	if( !pNode )
	{
		AKDIAG( L"akdiag: play action 0x%08x: element 0x%08x is not loaded", key, m_ulElementID );
		return AK_IDNotFound;
	}
	TransParams Tparameters;
	Tparameters.TransitionTime = GetTransitionTime();
	Tparameters.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;

	AkPBIParams pbiParams;
	pbiParams.eType = AkPBIParams::PBI;
	pbiParams.pInstigator = pNode;
	pbiParams.pGameObj = in_pAction->pGameObj;
	pbiParams.pTransitionParameters = &Tparameters;
	pbiParams.userParams = in_pAction->UserParam;
	pbiParams.playHistory.HistArray.uiArraySize = 0;
	pbiParams.playHistory.arrayIsContinuous.m_i32BitArray = 0;
	pbiParams.ePlaybackState = PB_Playing;
	pbiParams.uFrameOffset = in_pAction->LaunchFrameOffset;
	pbiParams.pContinuousParams = NULL;
	pbiParams.sequenceID = 0;
	pbiParams.bTargetFeedback = false;
	pbiParams.bIsFirst = true;
	const AKRESULT eResult = pNode->Play( pbiParams );
	pNode->Release();
	return eResult;
}

/** 2012 0x95dc20: the action's own element. */
AKRESULT CAkActionActive::Exec( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
	if( !pNode )
	{
		return AK_IDNotFound;
	}
	ActionParams params;
	params.eType = in_eType;
	params.pGameObj = in_pGameObj;
	params.playingID = in_TargetPlayingID;
	params.transParams.TransitionTime = GetTransitionTime();
	params.transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = m_bIsMasterResume;
	params.targetNodeID = 0;
	const AKRESULT eResult = pNode->ExecuteAction( params );
	pNode->Release();
	return eResult;
}

/** 2012 0x95db00: everything, from the master bus down (a master call when no game object is given). */
void CAkActionActive::AllExec( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID )
{
	CAkBus * pMasterBus = CAkBus::GetMasterBusAndAddRef();
	if( !pMasterBus )
	{
		return;
	}
	ActionParams params;
	params.eType = in_eType;
	params.pGameObj = in_pGameObj;
	params.playingID = in_TargetPlayingID;
	params.transParams.TransitionTime = GetTransitionTime();
	params.transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterCall = in_pGameObj == NULL;
	params.bIsMasterResume = m_bIsMasterResume;
	params.targetNodeID = 0;
	pMasterBus->ExecuteAction( params );
	pMasterBus->Release();
}

/** 2012 0x95da30. */
void CAkActionActive::AllExecExcept( ActionParamType in_eType, CAkRegisteredObj * in_pGameObj, AkPlayingID in_TargetPlayingID )
{
	CAkBus * pMasterBus = CAkBus::GetMasterBusAndAddRef();
	if( !pMasterBus )
	{
		return;
	}
	ActionParamsExcept params;
	params.eType = in_eType;
	params.pExeceptionList = &m_listElementException;
	params.pGameObj = in_pGameObj;
	params.playingID = in_TargetPlayingID;
	params.transParams.TransitionTime = GetTransitionTime();
	params.transParams.eFadeCurve = (AkCurveInterpolation)m_eFadeCurve;
	params.bIsFromBus = false;
	params.bIsMasterResume = m_bIsMasterResume;
	pMasterBus->ExecuteActionExcept( params );
	pMasterBus->Release();
}

/** 2012 0x94a520: Stop_E(_O) 0x102/0x103, Stop_ALL(_O) 0x104/0x105, Stop_AE(_O) 0x108/0x109. */
AKRESULT CAkActionStop::Execute( AkPendingAction * in_pAction )
{
	CAkRegisteredObj * pGameObj = in_pAction->pGameObj;
	switch( m_eActionType )
	{
	case 0x102:
	case 0x103:
	{
		const AKRESULT eResult = Exec( ActionParamType_Stop, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->StopPendingAction( m_ulElementID, pGameObj, in_pAction->TargetPlayingID );
		return eResult;
	}
	case 0x104:
	case 0x105:
		AllExec( ActionParamType_Stop, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->StopPendingAction( m_ulElementID, pGameObj, in_pAction->TargetPlayingID );
		return AK_Success;
	case 0x108:
	case 0x109:
		AllExecExcept( ActionParamType_Stop, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->StopPendingActionAllExcept( pGameObj, &m_listElementException, in_pAction->TargetPlayingID );
		return AK_Success;
	default:
		return AK_Success;
	}
}

/** 2012 0x95d8c0. */
AKRESULT CAkActionPause::Execute( AkPendingAction * in_pAction )
{
	CAkRegisteredObj * pGameObj = in_pAction->pGameObj;
	switch( m_eActionType )
	{
	case 0x202:
	case 0x203:
	{
		const AKRESULT eResult = Exec( ActionParamType_Pause, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->PausePendingAction( m_ulElementID, pGameObj, m_bPausePendingResume, in_pAction->TargetPlayingID );
		return eResult;
	}
	case 0x204:
	case 0x205:
		AllExec( ActionParamType_Pause, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->PausePendingAction( m_ulElementID, pGameObj, m_bPausePendingResume, in_pAction->TargetPlayingID );
		return AK_Success;
	case 0x208:
	case 0x209:
		AllExecExcept( ActionParamType_Pause, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->PausePendingActionAllExcept( pGameObj, &m_listElementException, m_bPausePendingResume, in_pAction->TargetPlayingID );
		return AK_Success;
	default:
		return AK_Success;
	}
}

/** 2012 0x95d730. */
AKRESULT CAkActionResume::Execute( AkPendingAction * in_pAction )
{
	CAkRegisteredObj * pGameObj = in_pAction->pGameObj;
	switch( m_eActionType )
	{
	case 0x302:
	case 0x303:
	{
		const AKRESULT eResult = Exec( ActionParamType_Resume, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->ResumePausedPendingAction( m_ulElementID, pGameObj, m_bIsMasterResume, in_pAction->TargetPlayingID );
		return eResult;
	}
	case 0x304:
	case 0x305:
		AllExec( ActionParamType_Resume, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->ResumePausedPendingAction( m_ulElementID, pGameObj, m_bIsMasterResume, in_pAction->TargetPlayingID );
		return AK_Success;
	case 0x308:
	case 0x309:
		AllExecExcept( ActionParamType_Resume, pGameObj, in_pAction->TargetPlayingID );
		g_pAudioMgr->ResumePausedPendingActionAllExcept( pGameObj, &m_listElementException, m_bIsMasterResume, in_pAction->TargetPlayingID );
		return AK_Success;
	default:
		return AK_Success;
	}
}

/** 2012 0x95c8e0. */
AKRESULT CAkActionBreak::Execute( AkPendingAction * in_pAction )
{
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( m_ulElementID );
	if( !pNode )
	{
		return AK_IDNotFound;
	}
	ActionParams params;
	params.eType = ActionParamType_Break;
	params.pGameObj = in_pAction->pGameObj;
	params.playingID = in_pAction->TargetPlayingID;
	params.transParams.TransitionTime = 0;
	params.transParams.eFadeCurve = AkCurveInterpolation_Linear;
	params.bIsFromBus = false;
	params.bIsMasterCall = false;
	params.bIsMasterResume = false;
	params.targetNodeID = m_ulElementID;
	AKRESULT eResult = pNode->ExecuteAction( params );
	if( eResult == AK_Success )
	{
		eResult = g_pAudioMgr->BreakPendingAction( m_ulElementID, in_pAction->pGameObj, in_pAction->TargetPlayingID );
	}
	pNode->Release();
	return eResult;
}
