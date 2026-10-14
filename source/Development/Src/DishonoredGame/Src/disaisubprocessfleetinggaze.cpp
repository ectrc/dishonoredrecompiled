// DishonoredGame/src/disaisubprocessfleetinggaze.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (12): the six below plus the six class-registration helpers of the two classes.
//
// ---- agent GI ports (PHASE20 GI): UDisAISubProcessFleetingGaze, the glance at whoever the brain attends most ----
// Resolved by hand in both databases (build/agentGI/idb; decompiles in build/agentGI/decomp2012; every 2013 address from the
// match table at ratio 1.000 "bytes" or "callee"). This is the mechanism behind a non-hostile NPC turning his head to the
// player: the brain's attention process elects the player as the top attention target (TopAttnProxyReplaced), and once that
// proxy is Busted this sub-process - if the behaviour's tweaks list it - looks at it for a random duration every random
// interval, head only, while the proxy is within the gaze radius and angle. The brain's own head-track look-at (agent GE's
// ChangeTopAttnTargetAttentionLevel) fires only for an ENEMY at HeadTrack (2013 0x725890, the disassembly at 0xb25910..33:
// ebx = (relationship == Enemy) only when level == 1), so a friendly guard's glance is this unit.

#include "DishonoredGame.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdelegate.h"

void DishonoredLinkUDisAISubProcessFleetingGaze()
{
}

static const UDisTweaks_AISubProcess_FleetingGaze* DisFleetingGazeTweaksOf( const UDisAISubProcessFleetingGaze* _pProcess )
{
	const UDisTweaks_AISubProcess_FleetingGaze* Tweaks = Cast<UDisTweaks_AISubProcess_FleetingGaze>( _pProcess->GetTweaks() );
	return Tweaks ? Tweaks : UDisTweaks_AISubProcess_FleetingGaze::StaticClass()->GetDefaultObject<UDisTweaks_AISubProcess_FleetingGaze>();
}

// 2012 rva 0x827e40 (DisCloseEnough, 94 B) and 0x8277c0 (DisPawnYawIsFacing, 197 B): within the radius; within the half-angle of
// the pawn's yaw in the plane (a target at the pawn's own location faces).
static UBOOL DisCloseEnough( const FVector& _rA, const FVector& _rB, FLOAT _fThreshold )
{
	return ( _rB - _rA ).SizeSquared() < _fThreshold * _fThreshold;
}

static UBOOL DisPawnYawIsFacing( const APawn* _pPawn, const FVector& _rTargetPosition, FLOAT _fAngleInDeg )
{
	const FVector Delta = _rTargetPosition - _pPawn->Location;
	if( Abs( Delta.X ) < 0.0001f && Abs( Delta.Y ) < 0.0001f && Abs( Delta.Z ) < 0.0001f )
	{
		return TRUE;
	}
	const FVector SelfDir = _pPawn->Rotation.Vector();
	FVector TestDir( Delta.X, Delta.Y, 0.f );
	TestDir.Normalize();
	return ( TestDir | SelfDir ) >= appCos( _fAngleInDeg * PI / 180.f );
}

// The proxy's actor is watched for termination for as long as the proxy has one (retail registers UDisAISubState's handler,
// identical-code folded with this class's; the body compares the event's instigator with the noted proxy's actor).
void UDisAISubProcessFleetingGaze::SetTopNotedProxy( const FDisAttentionProxy& _rProxy )
{
	const INT EventType = GDisAIEvent_OtherActorTerminated;
	if( m_TopNotedProxy.HasActorReference() )
	{
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubProcessFleetingGaze::OnTopNotedActorTerminatedEvent );
	}
	m_TopNotedProxy = _rProxy;
	if( m_TopNotedProxy.HasActorReference() )
	{
		FArkGameEventDispatcher::GetInstance()->RegisterToEvent( EventType, this, &UDisAISubProcessFleetingGaze::OnTopNotedActorTerminatedEvent );
	}
}

void UDisAISubProcessFleetingGaze::OnTopNotedActorTerminatedEvent( const FArkGameEvent& _rEvent )
{
	if( _rEvent.m_pInstigator == m_TopNotedProxy.m_pCachedTargetActor )
	{
		SetTopNotedProxy( FDisAttentionProxy( EC_EventParm ) );
	}
}

// DISHONORED(port): 2012 rva 0x789a00 / 2013 0x73d8e0 (416 B): the interval starts, the noted proxy is the first target the brain
// has any attention on (Unaware and up, alive or dead, any relationship, any proxy status).
void UDisAISubProcessFleetingGaze::BeginSubProcess_Derived()
{
	const UDisTweaks_AISubProcess_FleetingGaze* Tweaks = DisFleetingGazeTweaksOf( this );
	m_fGazeIntervalTimer = Tweaks->m_GazeInterval.GetRandomFloatValue();
	m_fGazeDurationTimer = 0.f;
	SetTopNotedProxy( FDisAttentionProxy( EC_EventParm ) );
	if( !m_pOwningBrain || !m_pOwningBrain->GetAttentionProcess() )
	{
		return;
	}
	FMemMark Mark( GMainThreadMemStack );
	TArray< IDisAttentionTargetInterface*, TMemStackAllocator< GMainThreadMemStack > > AttnTargets;
	m_pOwningBrain->GetAttentionProcess()->GetTargetsAtAttentionLevel( DAL_Unaware, AttnTargets, DLSF_AliveOrDead, Relationship_MAX, DAPS_Indeterminate );
	if( AttnTargets.Num() > 0 )
	{
		SetTopNotedProxy( FDisAttentionProxy( m_pOwningBrain, AttnTargets(0) ) );
	}
	Mark.Pop();
}

// DISHONORED(port): 2012 rva 0x789ba0 / 2013 0x739a00 (299 B)
void UDisAISubProcessFleetingGaze::RefreshSubProcess_Derived( const FLOAT _fTimeSinceLastThought )
{
	if( m_fGazeDurationTimer > 0.f )
	{
		m_fGazeDurationTimer -= _fTimeSinceLastThought;
		return;
	}
	m_fGazeIntervalTimer -= _fTimeSinceLastThought;
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->GetOwningPawn() : NULL;
	if( m_fGazeIntervalTimer < 0.f && Pawn && m_TopNotedProxy.IsValid() && m_TopNotedProxy.IsBusted() )
	{
		const FVector AllyLocation = m_TopNotedProxy.GetProxyLocation();
		const UDisTweaks_AISubProcess_FleetingGaze* Tweaks = DisFleetingGazeTweaksOf( this );
		if( DisCloseEnough( Pawn->Location, AllyLocation, Tweaks->m_fGazeRadius ) && DisPawnYawIsFacing( Pawn, AllyLocation, Tweaks->m_fGazeAngleDegrees ) )
		{
			m_fGazeIntervalTimer = Tweaks->m_GazeInterval.GetRandomFloatValue();
			m_fGazeDurationTimer = Tweaks->m_GazeDuration.GetRandomFloatValue();
			SetLookAtProxyDesire( m_TopNotedProxy, FDisLookAtInfluence::Head, -1.f );
			return;
		}
	}
	ClearLookAtDesire();
}

// DISHONORED(port): 2012 rva 0x778100 / 2013 0x73da80 (135 B): the brain's new top proxy becomes the noted one; not consumed.
UBOOL UDisAISubProcessFleetingGaze::FilterTopAttnProxyReplaced( const FAIStimStruct_TopAttnProxyReplaced& _rStim )
{
	SetTopNotedProxy( _rStim.m_NewTopAttnProxy );
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x765f40 / 2013 0x728600 (31 B): one id (102 in 2012 = TopAttnProxyReplaced, 105 here).
const BYTE* UDisAISubProcessFleetingGaze::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_TopAttnProxyReplaced };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x780830 / 2013 0x6fe680 (39 B)
FDisStimPredicateDelegate UDisAISubProcessFleetingGaze::GetFilterStimDelegate_SubProcess( BYTE _StimID )
{
	if( _StimID == EAIStimID_TopAttnProxyReplaced )
	{
		return DIS_BIND_STIM_PREDICATE( UDisAISubProcessFleetingGaze, FAIStimStruct_TopAttnProxyReplaced, FilterTopAttnProxyReplaced );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2012 rva 0x7764f0 / 2013 0x739b80 (109 B): both watched proxies are released before the object goes.
void UDisAISubProcessFleetingGaze::BeginDestroy()
{
	SetTopNotedProxy( FDisAttentionProxy( EC_EventParm ) );
	UDisAISubProcess::BeginDestroy();
}
