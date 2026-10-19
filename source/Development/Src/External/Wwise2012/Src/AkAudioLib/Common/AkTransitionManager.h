/*=============================================================================
	AkTransitionManager.h - CAkTransition / CAkTransitionManager / ITransitionable (aktransition.cpp,
	aktransitionmanager.cpp): the fades a PBI, a pending action or a node runs, advanced once per audio buffer.

	DISHONORED(port): agent IH2 (wave 26), pulled from IH3's list because every play / stop / pause with a fade time
	needs it. Layouts are the 2012 PDB's (CAkTransition 0x38 bytes).
=============================================================================*/
#pragma once

#include "AkAudioLibGlobals.h"

enum TransitionTargets
{
	TransTarget_UndefinedTarget	= 0,
	TransTarget_Volume			= 0x10000,
	TransTarget_Pitch			= 0x20000,
	TransTarget_LPF				= 0x40000,
	TransTarget_Lfe				= 0x80000,
	TransTarget_Mute			= 0x800000,
	TransTarget_Play			= 0x1000000,
	TransTarget_Stop			= 0x2000000,
	TransTarget_Pause			= 0x4000000,
	TransTarget_Resume			= 0x8000000,
	TransTarget_RTPC			= 0x10000000,
	TransTarget_FeedbackVolume	= 0x20000000,
	TransTarget_TargetMask		= 0xFFFF0000
};

enum AkValueMeaning
{
	AkValueMeaning_Default		= 0,
	AkValueMeaning_Independent	= 1,
	AkValueMeaning_Offset		= 2
};

class ITransitionable
{
public:
	virtual void TransUpdateValue( TransitionTargets in_eTargetType, AkReal32 in_fNewValue, bool in_bLastUpdate ) = 0;
};

struct TransitionParameters
{
	TransitionParameters() : pUser( NULL ), eTarget( TransTarget_UndefinedTarget ), fStartValue( 0.f ), fTargetValue( 0.f ), lDuration( 0 ), eFadeCurve( AkCurveInterpolation_Linear ), bdBs( false ), bUseReciprocalCurve( true ) {}

	ITransitionable *		pUser;
	TransitionTargets		eTarget;
	AkReal32				fStartValue;
	AkReal32				fTargetValue;
	AkTimeMs				lDuration;
	AkCurveInterpolation	eFadeCurve;
	bool					bdBs;
	bool					bUseReciprocalCurve;
};

class CAkTransition
{
public:
	enum State
	{
		NotRunning	= 0,
		Running		= 1,
		ToPause		= 2,
		Paused		= 3,
		ToResume	= 4,
		Done		= 5,
		ToRemove	= 6
	};

	CAkTransition();

	static void ConvertdBs( AkReal32 & io_fStart, AkReal32 & io_fTarget );
	bool ComputeTransition( AkUInt32 in_CurrentBufferTick );
	void Reset();
	void Term();
	AKRESULT InitParameters( const TransitionParameters & in_Params, AkUInt32 in_CurrentBufferTick );

	AkCurveInterpolation	m_eFadeCurve;
	AkReal32				m_fStartValue;
	AkReal32				m_fTargetValue;
	AkReal32				m_fCurrentValue;
	TransitionTargets		m_eTarget;
	AkReal32				m_fStartTimeInBufferTick;
	AkReal32				m_fDurationInBufferTick;
	AkReal32				m_fTimeRatio;
	AkUInt32				m_uLastBufferTickUpdated;
	AkArray<ITransitionable *, ITransitionable *, ArrayPoolDefault, 8>	m_UsersList;
	State					m_eState;
	AkUInt8					m_bdBs : 1;
	AkUInt8					m_bCurrentValueSet : 1;
	AkUInt8					m_iNumUsers;
};

class CAkTransitionManager
{
public:
	enum AkTransitionCategory
	{
		TC_State	= 0,
		TC_Fade		= 1
	};

	CAkTransitionManager();

	AKRESULT Init( AkUInt32 in_uMaxNumTransitions );
	void Term();
	CAkTransition * AddTransitionToList( const TransitionParameters & in_Params, bool in_bStart, AkTransitionCategory in_eTransitionCategory );
	AKRESULT AddTransitionUser( CAkTransition * in_pTransition, ITransitionable * in_pUser );
	AKRESULT RemoveTransitionUser( CAkTransition * in_pTransition, ITransitionable * in_pUser );
	void ChangeParameter( CAkTransition * in_pTransition, TransitionTargets in_eTransitionType, AkReal32 in_NewTarget, AkTimeMs in_NewDuration, AkCurveInterpolation in_eCurveType, AkValueMeaning in_eValueMeaning );
	void Pause( CAkTransition * in_pTransition );
	void Resume( CAkTransition * in_pTransition );
	bool IsTerminated( CAkTransition * in_pTransition );
	void ProcessTransitionsList( AkUInt32 in_CurrentBufferTick );

	AkUInt32	m_uMaxNumTransitions;
	AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0>	m_ActiveTransitionsList_Fade;
	AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0>	m_ActiveTransitionsList_State;

private:
	void TermList( AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0> & in_rTransitionList );
	void ProcessTransitionsList( AkUInt32 in_CurrentBufferTick, AkArray<CAkTransition *, CAkTransition *, ArrayPoolDefault, 0> & in_rTransitionList );
};

extern CAkTransitionManager * g_pTransitionManager;

namespace AkAudioLibSettings
{
	/** 2012 .data 0xe3703c: 1024 frames at 48 kHz, truncated to 21 ms. */
	extern AkInt32 g_msPerBufferTick;
}
