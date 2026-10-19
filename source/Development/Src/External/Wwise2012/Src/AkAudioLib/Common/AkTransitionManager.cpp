/*=============================================================================
	AkTransitionManager.cpp - CAkTransition (aktransition.cpp, 2012 0x953d10..0x953f00) and CAkTransitionManager
	(aktransitionmanager.cpp, 2012 0x92fbe0..0x9301f0); 2013 matched by bytes.

	DISHONORED(port): agent IH2 (wave 26). No deviation; the curve math is CAkInterpolation's (g_Interpol).
=============================================================================*/
#include "AkTransitionManager.h"
#include "AkAudioMgr.h"
#include "AkInterpolation.h"

CAkTransitionManager * g_pTransitionManager = NULL;

namespace AkAudioLibSettings
{
	AkInt32 g_msPerBufferTick = 21;
}

/** 2012 0x953d40. */
CAkTransition::CAkTransition()
	: m_eFadeCurve( AkCurveInterpolation_Linear )
	, m_fStartValue( 0.f )
	, m_fTargetValue( 0.f )
	, m_fCurrentValue( 0.f )
	, m_eTarget( TransTarget_UndefinedTarget )
	, m_fStartTimeInBufferTick( 0.f )
	, m_fDurationInBufferTick( 0.f )
	, m_fTimeRatio( 0.f )
	, m_uLastBufferTickUpdated( 0 )
	, m_eState( NotRunning )
	, m_bdBs( 0 )
	, m_bCurrentValueSet( 0 )
	, m_iNumUsers( 0 )
{
}

/** 2012 0x953d10. */
void CAkTransition::ConvertdBs( AkReal32 & io_fStart, AkReal32 & io_fTarget )
{
	io_fStart = g_Interpol.m_tableDBtoReal.Get( io_fStart );
	io_fTarget = g_Interpol.m_tableDBtoReal.Get( io_fTarget );
}

/** 2012 0x953d70: true when the transition reached its target this tick. */
bool CAkTransition::ComputeTransition( AkUInt32 in_CurrentBufferTick )
{
	bool bDone;
	if( m_fDurationInBufferTick == 0.f )
	{
		bDone = true;
	}
	else
	{
		m_fTimeRatio = ( (AkReal32)in_CurrentBufferTick - m_fStartTimeInBufferTick ) / m_fDurationInBufferTick;
		bDone = m_fTimeRatio >= 1.f;
	}
	AkReal32 fResult;
	if( bDone )
	{
		fResult = m_bdBs ? g_Interpol.RealTodB( m_fTargetValue ) : m_fTargetValue;
	}
	else
	{
		if( m_fTimeRatio < 0.f )
		{
			m_fTimeRatio = 0.f;
		}
		fResult = m_bdBs ? g_Interpol.InterpolateDBNoCheck( m_fTimeRatio, m_fStartValue, m_fTargetValue, m_eFadeCurve )
			: g_Interpol.InterpolateNoCheck( m_fTimeRatio, m_fStartValue, m_fTargetValue, m_eFadeCurve );
	}
	m_bCurrentValueSet = 1;
	m_fCurrentValue = fResult;
	for( ITransitionable ** ppUser = m_UsersList.m_pItems; ppUser != m_UsersList.m_pItemsEnd; ++ppUser )
	{
		( *ppUser )->TransUpdateValue( m_eTarget, fResult, bDone );
	}
	return bDone;
}

/** 2012 0x953ea0. */
void CAkTransition::Reset()
{
	m_UsersList.m_pItemsEnd = m_UsersList.m_pItems;
	m_bdBs = 0;
	m_bCurrentValueSet = 0;
	m_iNumUsers = 0;
	m_eTarget = TransTarget_UndefinedTarget;
	m_eState = NotRunning;
	m_fTimeRatio = 0.f;
}

/** 2012 0x953ec0. */
void CAkTransition::Term()
{
	m_UsersList.Term();
}

/** 2012 0x953f00. */
AKRESULT CAkTransition::InitParameters( const TransitionParameters & in_Params, AkUInt32 in_CurrentBufferTick )
{
	m_eTarget = in_Params.eTarget;
	m_bdBs = in_Params.bdBs;
	AkReal32 fStartValue = in_Params.fStartValue;
	AkReal32 fTargetValue = in_Params.fTargetValue;
	if( in_Params.bdBs )
	{
		fStartValue = g_Interpol.m_tableDBtoReal.Get( fStartValue );
		fTargetValue = g_Interpol.m_tableDBtoReal.Get( fTargetValue );
	}
	m_fStartValue = fStartValue;
	m_fCurrentValue = fStartValue;
	m_fTargetValue = fTargetValue;
	if( !in_Params.bUseReciprocalCurve || fTargetValue > fStartValue || in_Params.eFadeCurve == AkCurveInterpolation_InvSCurve || in_Params.eFadeCurve == AkCurveInterpolation_SCurve )
	{
		m_eFadeCurve = in_Params.eFadeCurve;
	}
	else
	{
		m_eFadeCurve = (AkCurveInterpolation)( AkCurveInterpolation_LastFadeCurve - in_Params.eFadeCurve );
	}
	m_fStartTimeInBufferTick = (AkReal32)in_CurrentBufferTick;
	m_fDurationInBufferTick = (AkReal32)(AkUInt32)( ( in_Params.lDuration + AkAudioLibSettings::g_msPerBufferTick - 1 ) / AkAudioLibSettings::g_msPerBufferTick );
	m_fTimeRatio = 0.f;
	if( m_UsersList.Exists( in_Params.pUser ) )
	{
		return AK_Success;
	}
	if( !m_UsersList.AddLast( in_Params.pUser ) )
	{
		return AK_Fail;
	}
	++m_iNumUsers;
	return AK_Success;
}

/** 2012 0x92fdc0. */
CAkTransitionManager::CAkTransitionManager() : m_uMaxNumTransitions( 0 )
{
}

/** 2012 0x92fde0: both lists reserved to the maximum (255 when the settings say 0), never grown. */
AKRESULT CAkTransitionManager::Init( AkUInt32 in_uMaxNumTransitions )
{
	m_uMaxNumTransitions = in_uMaxNumTransitions ? in_uMaxNumTransitions : 255;
	if( m_ActiveTransitionsList_Fade.Reserve( m_uMaxNumTransitions ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	if( m_ActiveTransitionsList_State.Reserve( m_uMaxNumTransitions ) != AK_Success )
	{
		return AK_InsufficientMemory;
	}
	return AK_Success;
}

/** 2012 0x92fe60: every transition jumps to its end value before it is freed. */
void CAkTransitionManager::TermList( AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0> & in_rTransitionList )
{
	for( CAkTransition ** pp = in_rTransitionList.m_pItems; pp != in_rTransitionList.m_pItemsEnd; ++pp )
	{
		CAkTransition * pTransition = *pp;
		pTransition->m_fDurationInBufferTick = 0.f;
		pTransition->ComputeTransition( 0 );
		pTransition->Term();
		pTransition->~CAkTransition();
		AK::MemoryMgr::Free( g_DefaultPoolId, pTransition );
	}
	in_rTransitionList.Term();
}

/** 2012 0x930040. */
void CAkTransitionManager::Term()
{
	TermList( m_ActiveTransitionsList_Fade );
	TermList( m_ActiveTransitionsList_State );
}

/** 2012 0x930060: when the list is full the most advanced transition is finished early and recycled. */
CAkTransition * CAkTransitionManager::AddTransitionToList( const TransitionParameters & in_Params, bool in_bStart, AkTransitionCategory in_eTransitionCategory )
{
	AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0> & rList = in_eTransitionCategory == TC_State ? m_ActiveTransitionsList_State : m_ActiveTransitionsList_Fade;
	CAkTransition * pTransition = NULL;
	if( rList.Length() < m_uMaxNumTransitions )
	{
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkTransition ) );
		if( pMem )
		{
			pTransition = AkPlacementNew( pMem ) CAkTransition();
		}
	}
	if( !pTransition )
	{
		AkReal32 fMaxTimeRatio = 0.f;
		for( CAkTransition ** pp = rList.m_pItems; pp != rList.m_pItemsEnd; ++pp )
		{
			if( ( *pp )->m_fTimeRatio > fMaxTimeRatio )
			{
				pTransition = *pp;
				fMaxTimeRatio = ( *pp )->m_fTimeRatio;
			}
		}
		if( !pTransition )
		{
			return NULL;
		}
		pTransition->m_fDurationInBufferTick = 0.f;
		pTransition->ComputeTransition( 0 );
		pTransition->Reset();
		rList.RemoveSwap( pTransition );
	}
	if( pTransition->InitParameters( in_Params, g_pAudioMgr->GetBufferTick() ) == AK_Fail || !rList.AddLast( pTransition ) )
	{
		pTransition->Term();
		pTransition->~CAkTransition();
		AK::MemoryMgr::Free( g_DefaultPoolId, pTransition );
		return NULL;
	}
	if( in_bStart )
	{
		pTransition->m_eState = CAkTransition::Running;
	}
	return pTransition;
}

/** 2012 0x930180. */
AKRESULT CAkTransitionManager::AddTransitionUser( CAkTransition * in_pTransition, ITransitionable * in_pUser )
{
	if( in_pTransition->m_UsersList.Exists( in_pUser ) )
	{
		return AK_UserAlreadyInList;
	}
	if( in_pTransition->m_iNumUsers == 0xFF || !in_pTransition->m_UsersList.AddLast( in_pUser ) )
	{
		return AK_UsersListFull;
	}
	++in_pTransition->m_iNumUsers;
	return AK_Success;
}

/** 2012 0x92fee0: the last user leaving marks the transition for removal. */
AKRESULT CAkTransitionManager::RemoveTransitionUser( CAkTransition * in_pTransition, ITransitionable * in_pUser )
{
	if( in_pTransition->m_UsersList.RemoveSwap( in_pUser ) != AK_Success )
	{
		return AK_UserNotInList;
	}
	if( in_pTransition->m_iNumUsers-- == 1 )
	{
		in_pTransition->m_eState = CAkTransition::ToRemove;
	}
	return AK_Success;
}

/** 2012 0x92fc50. */
void CAkTransitionManager::ChangeParameter( CAkTransition * in_pTransition, TransitionTargets in_eTransitionType, AkReal32 in_NewTarget, AkTimeMs in_NewDuration, AkCurveInterpolation in_eCurveType, AkValueMeaning in_eValueMeaning )
{
	const TransitionTargets eOldTarget = in_pTransition->m_eTarget;
	in_pTransition->m_eTarget = in_eTransitionType;
	AkReal32 fNewTarget = in_NewTarget;
	if( in_pTransition->m_bdBs )
	{
		if( in_pTransition->m_bCurrentValueSet )
		{
			in_pTransition->m_fStartValue = in_pTransition->m_fCurrentValue;
			CAkTransition::ConvertdBs( in_pTransition->m_fStartValue, fNewTarget );
		}
		else
		{
			fNewTarget = g_Interpol.m_tableDBtoReal.Get( fNewTarget );
		}
		if( in_eValueMeaning == AkValueMeaning_Offset )
		{
			fNewTarget = in_pTransition->m_fTargetValue * fNewTarget;
		}
	}
	else
	{
		if( in_pTransition->m_bCurrentValueSet )
		{
			in_pTransition->m_fStartValue = in_pTransition->m_fCurrentValue;
		}
		if( in_eValueMeaning == AkValueMeaning_Offset )
		{
			fNewTarget = in_pTransition->m_fTargetValue + fNewTarget;
		}
	}
	in_pTransition->m_fTargetValue = fNewTarget;
	if( fNewTarget > in_pTransition->m_fStartValue || in_eCurveType == AkCurveInterpolation_InvSCurve || in_eCurveType == AkCurveInterpolation_SCurve )
	{
		in_pTransition->m_eFadeCurve = in_eCurveType;
	}
	else
	{
		in_pTransition->m_eFadeCurve = (AkCurveInterpolation)( AkCurveInterpolation_LastFadeCurve - in_eCurveType );
	}
	const AkUInt32 uBufferTick = g_pAudioMgr->GetBufferTick();
	const AkReal32 fNewDuration = (AkReal32)(AkUInt32)( ( AkAudioLibSettings::g_msPerBufferTick + in_NewDuration - 1 ) / AkAudioLibSettings::g_msPerBufferTick );
	if( eOldTarget == in_pTransition->m_eTarget && ( in_eTransitionType & ( TransTarget_Play | TransTarget_Stop | TransTarget_Pause | TransTarget_Resume ) ) )
	{
		const AkReal32 fRemaining = in_pTransition->m_fDurationInBufferTick - ( (AkReal32)uBufferTick - in_pTransition->m_fStartTimeInBufferTick );
		in_pTransition->m_fDurationInBufferTick = fRemaining <= fNewDuration ? fRemaining : fNewDuration;
	}
	else
	{
		in_pTransition->m_fDurationInBufferTick = fNewDuration;
	}
	in_pTransition->m_fStartTimeInBufferTick = (AkReal32)uBufferTick;
	in_pTransition->m_uLastBufferTickUpdated = uBufferTick;
}

/** 2012 0x92fbf0. */
void CAkTransitionManager::Pause( CAkTransition * in_pTransition )
{
	if( in_pTransition->m_eState == CAkTransition::Running )
	{
		in_pTransition->m_eState = CAkTransition::ToPause;
	}
	else if( in_pTransition->m_eState == CAkTransition::ToResume )
	{
		in_pTransition->m_eState = CAkTransition::Paused;
	}
}

/** 2012 0x92fc20. */
void CAkTransitionManager::Resume( CAkTransition * in_pTransition )
{
	if( in_pTransition->m_eState == CAkTransition::Paused )
	{
		in_pTransition->m_eState = CAkTransition::ToResume;
	}
	else if( in_pTransition->m_eState == CAkTransition::ToPause )
	{
		in_pTransition->m_eState = CAkTransition::Running;
	}
}

/** 2012 0x92fbe0. */
bool CAkTransitionManager::IsTerminated( CAkTransition * in_pTransition )
{
	return in_pTransition->m_eState == CAkTransition::Done;
}

/** 2012 0x92ff40. */
void CAkTransitionManager::ProcessTransitionsList( AkUInt32 in_CurrentBufferTick, AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0> & in_rTransitionList )
{
	CAkTransition ** pp = in_rTransitionList.m_pItems;
	while( pp != in_rTransitionList.m_pItemsEnd )
	{
		CAkTransition * pTransition = *pp;
		if( pTransition->m_eState == CAkTransition::ToRemove )
		{
			pTransition->Term();
			pTransition->~CAkTransition();
			AK::MemoryMgr::Free( g_DefaultPoolId, pTransition );
			if( in_rTransitionList.Length() > 1 )
			{
				*pp = *( in_rTransitionList.m_pItemsEnd - 1 );
			}
			--in_rTransitionList.m_pItemsEnd;
			continue;
		}
		if( pTransition->m_eState == CAkTransition::ToPause )
		{
			pTransition->m_uLastBufferTickUpdated = in_CurrentBufferTick;
			pTransition->m_eState = CAkTransition::Paused;
		}
		else if( pTransition->m_eState == CAkTransition::ToResume )
		{
			pTransition->m_fStartTimeInBufferTick += (AkReal32)( in_CurrentBufferTick - pTransition->m_uLastBufferTickUpdated );
			pTransition->m_eState = CAkTransition::Running;
		}
		if( pTransition->m_eState == CAkTransition::Running && pTransition->ComputeTransition( in_CurrentBufferTick ) )
		{
			pTransition->Term();
			if( in_rTransitionList.Length() > 1 )
			{
				*pp = *( in_rTransitionList.m_pItemsEnd - 1 );
			}
			--in_rTransitionList.m_pItemsEnd;
			pTransition->~CAkTransition();
			AK::MemoryMgr::Free( g_DefaultPoolId, pTransition );
		}
		else
		{
			++pp;
		}
	}
}

/** 2012 0x9301f0. */
void CAkTransitionManager::ProcessTransitionsList( AkUInt32 in_CurrentBufferTick )
{
	ProcessTransitionsList( in_CurrentBufferTick, m_ActiveTransitionsList_Fade );
	ProcessTransitionsList( in_CurrentBufferTick, m_ActiveTransitionsList_State );
}
