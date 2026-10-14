// DishonoredGame/src/disaibrainprocessattention.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (88): the 85 below plus the three class-registration helpers of the macro.
//
// ---- agent GI ports (PHASE20 GI): UDisAIBrainProcessAttention, the attention brain process ----
//
// Every address was resolved by hand in both databases (build/agentGI/idb/GI_shipping2012.i64, imagebase 0, and
// GI_retail2013.i64, imagebase 0x400000; 2013 rva = VA - 0x400000). Decompiles in build/agentGI/decomp2012/, the 2013 bodies
// read through the idalib MCP session. Where the two builds differ the 2013 shape is written and the 2012 one named.
//
// The process keeps m_AttentionInfos, one FDisAttentionSlot per target the brain has attended to: the player gets the
// brain's own UDisAttentionInfo_Complex (m_pPlayerAttentionInfo, a time-based meter: a leaky accumulator fed by sight,
// touch, noises, alarms, allies; its level is read against the pawn-attention tweaks' thresholds), everything else a pooled
// UDisAttentionInfo_Simple (Unaware or Busted). TickAttention ticks every slot, forwards the level changes to the brain
// (SendAttentionLevelChanged -> the Busted/Unbusted stims and the Kismet attention events), drops slots that have gone
// quiet, runs the tweaks' attention reactions, and HandleTopAttnTarget then elects the most obvious target and hands it to
// UDishonoredAIBrain::SetTopAttnTarget / SetTopEnemyAttnTarget - which is where a guard's head-track look-at (agent GE's
// ChangeTopAttnTargetAttentionLevel) and the FleetingGaze sub-process's glance come from.
//
// DISHONORED(bringup), stated once for the unit: the sense that feeds this process in retail is FDisComponentVisionNPC
// (discomponentvisionnpc.cpp, 14 functions + FDisComponentVision's 22, discomponentvision.cpp) together with
// FDisComponentObservable on the targets (discomponentobservable.cpp, 26) and ADishonoredNPCController::StartSeeingVisibleThing /
// StopSeeingVisibleThing (2012 0x7b08c0 / 0x7b0970, 2013 0x7591a0 / 0x759270), which raise TargetSighted / TargetUnsighted
// and supply GetVisibilityLevel / GetSpottedDistance to SetupTickParamsFromTarget. None of those is ported: an NPC cannot
// see, so the sight branch of the complex info never accrues. What reaches the process in this tree: the soiree minimum
// attention (UpdateSoireeAttention, the matinee co-actors), TargetTouched, IncomingDamage, the dialog / Kismet / psychic
// stims, HeardSomething (if a noise reaches the brain). `-gisight` (DisGISightStandIn, below) is a declared STAND-IN for
// the vision component, off by default, used only to prove the chain downstream of a sighting; it is not retail's vision.

#include "DishonoredGame.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdelegate.h"
#include "disattentioninfo.h"
#include "dishonoredutilities.h"

// DISHONORED(bringup): every function of this unit is a virtual or reached through the brain, so nothing references the object
// and MSVC would drop it out of the static library. Named from DishonoredGameRegistrants.cpp (the generator collects it).
void DishonoredLinkUDisAIBrainProcessAttention()
{
}

/*-----------------------------------------------------------------------------
	Diagnostics (-gidiag) and the vision stand-in (-gisight). Both free when absent.
-----------------------------------------------------------------------------*/

static INT GDisGIDiagLevel = 15;
INT DisGIDiagLevel()
{
	DisGIDiagEnabled();
	return GDisGIDiagLevel;
}

UBOOL DisGIDiagEnabled()
{
	static INT Enabled = -1;
	if( Enabled < 0 )
	{
		INT Level = 0;
		Enabled = ( ParseParam( appCmdLine(), TEXT("gidiag") ) || Parse( appCmdLine(), TEXT("gidiag="), Level ) ) ? 1 : 0;
		if( Level > 0 )
		{
			GDisGIDiagLevel = Level;
		}
	}
	return Enabled == 1;
}

// DISHONORED(written): agent GI. A STAND-IN for FDisComponentVisionNPC + FDisComponentObservable, declared as such: with
// -gisight[=range] each brain with an attention process tests the player's focal point against its pawn's facing (within
// DisGISightHalfAngleDeg of the pawn yaw, within the range) and a zero-extent world line check from the pawn's eye
// (ADishonoredPawn::GetLookAtPoint), and raises TargetSighted / TargetUnsighted on the transitions the way
// StartSeeingVisibleThing / StopSeeingVisibleThing would; SetupTickParamsFromTarget then reads a visibility level of 1 (lit)
// and the normalised distance as the spotted distance in place of the observable component. It fakes awareness and says so
// in every log line it writes; it is not retail's vision cone, LOD, lighting or time slicing.
static INT GDisGISightStandIn = -1;
static FLOAT GDisGISightRange = 1800.f;
static const FLOAT DisGISightHalfAngleDeg = 70.f;

UBOOL DisGISightStandInEnabled()
{
	if( GDisGISightStandIn < 0 )
	{
		FLOAT Range = 0.f;
		GDisGISightStandIn = ( ParseParam( appCmdLine(), TEXT("gisight") ) || Parse( appCmdLine(), TEXT("gisight="), Range ) ) ? 1 : 0;
		if( Range > 0.f )
		{
			GDisGISightRange = Range;
		}
		if( GDisGISightStandIn )
		{
			debugf( TEXT("gisight: STAND-IN for FDisComponentVisionNPC enabled (range %.0f, half angle %.0f deg) - this is not retail's vision"), GDisGISightRange, DisGISightHalfAngleDeg );
		}
	}
	return GDisGISightStandIn == 1;
}

FLOAT DisGISightStandInRange()
{
	return GDisGISightRange;
}

struct FDisGISightState
{
	const UDishonoredAIBrain* m_pBrain;
	UBOOL m_bSeesPlayer;
	FLOAT m_fNormalisedDistance;
	FLOAT m_fNextCheck;
};
static TArray<FDisGISightState> GDisGISightStates;

static FDisGISightState& DisGISightStateFor( const UDishonoredAIBrain* _pBrain )
{
	for( INT i = 0; i < GDisGISightStates.Num(); i++ )
	{
		if( GDisGISightStates(i).m_pBrain == _pBrain )
		{
			return GDisGISightStates(i);
		}
	}
	FDisGISightState& State = GDisGISightStates( GDisGISightStates.AddZeroed() );
	State.m_pBrain = _pBrain;
	State.m_fNormalisedDistance = 1.f;
	return State;
}

UBOOL DisGISightStandInSeesPlayer( const UDishonoredAIBrain* _pBrain, FLOAT& _rOutNormalisedDistance )
{
	const FDisGISightState& State = DisGISightStateFor( _pBrain );
	_rOutNormalisedDistance = State.m_fNormalisedDistance;
	return State.m_bSeesPlayer;
}

static void DisGISightStandInTick( UDishonoredAIBrain* _pBrain )
{
	ADishonoredNPCPawn* Pawn = _pBrain->GetOwningPawn();
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	if( !Pawn || !Player || !GWorld )
	{
		return;
	}
	FDisGISightState& State = DisGISightStateFor( _pBrain );
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < State.m_fNextCheck )
	{
		return;
	}
	State.m_fNextCheck = Now + 0.2f;
	const FVector Eye = Pawn->GetLookAtPoint();
	const FVector Focal = Player->GetAttnTargetFocalPoint();
	const FVector ToPlayer = Focal - Eye;
	const FLOAT Distance = ToPlayer.Size();
	UBOOL bSees = FALSE;
	if( Distance > 1.f && Distance < GDisGISightRange && Pawn->Health > 0 )
	{
		FVector Flat = ToPlayer;
		Flat.Z = 0.f;
		Flat.Normalize();
		const FVector Facing = Pawn->Rotation.Vector();
		if( ( Flat | Facing ) >= appCos( DisGISightHalfAngleDeg * PI / 180.f ) )
		{
			FCheckResult Hit( 1.f );
			bSees = GWorld->SingleLineCheck( Hit, Pawn, Focal, Eye, TRACE_World | TRACE_StopAtAnyHit );
		}
	}
	State.m_fNormalisedDistance = Clamp( Distance / GDisGISightRange, 0.f, 1.f );
	if( !bSees && Distance < GDisGISightRange && DisGIDiagEnabled() && ( GDisGIDiagLevel & 8 ) && appFloor( Now ) % 5 == 0 && appFloor( Now ) != appFloor( Now - 0.2f ) )
	{
		FVector Flat = ToPlayer;
		Flat.Z = 0.f;
		Flat.Normalize();
		debugf( TEXT("gisight STAND-IN %s: no sight at %.0f uu: health %d, cos to player %.2f (needs %.2f), eye (%.0f,%.0f,%.0f)"), *Pawn->GetName(), Distance, Pawn->Health,
			Flat | Pawn->Rotation.Vector(), appCos( DisGISightHalfAngleDeg * PI / 180.f ), Eye.X, Eye.Y, Eye.Z );
	}
	if( bSees != State.m_bSeesPlayer )
	{
		State.m_bSeesPlayer = bSees;
		debugf( TEXT("gisight STAND-IN %s: %s the player at %.0f uu (t=%.2f)"), *Pawn->GetName(), bSees ? TEXT("sights") : TEXT("unsights"), Distance, Now );
		if( bSees )
		{
			_pBrain->HandleTargetSighted( static_cast<IDisAttentionTargetInterface*>( Player ) );
		}
		else
		{
			_pBrain->HandleTargetUnsighted( static_cast<IDisAttentionTargetInterface*>( Player ) );
		}
	}
}

static const TCHAR* DisGILevelName( BYTE _Level )
{
	switch( _Level )
	{
	case DAL_Unaware: return TEXT("Unaware");
	case DAL_HeadTrack: return TEXT("HeadTrack");
	case DAL_TurnToFace: return TEXT("TurnToFace");
	case DAL_Investigate: return TEXT("Investigate");
	case DAL_Busted: return TEXT("Busted");
	default: return TEXT("?");
	}
}

static void DisGIDiagReport( const UDisAIBrainProcessAttention* _pProcess, FLOAT _fDeltaSeconds )
{
	static TMap<const UDisAIBrainProcessAttention*, FLOAT> NextReport;
	if( !GWorld )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	FLOAT* Next = NextReport.Find( _pProcess );
	if( Next && Now < *Next )
	{
		return;
	}
	NextReport.Set( _pProcess, Now + 1.f );
	const UDishonoredAIBrain* Brain = _pProcess->m_pOwningBrain;
	const ADishonoredNPCPawn* Pawn = _pProcess->m_pOwningPawn;
	if( !Brain || !Pawn )
	{
		return;
	}
	if( _pProcess->m_AttentionInfos.Num() == 0 )
	{
		NextReport.Set( _pProcess, Now + 5.f );
		const UDisTweaks_AIBrainProcess_Attention* ProcessTweaks = _pProcess->GetAttentionProcessTweaks();
		const UDisTweaks_PawnAttention* AllyTweaks = ProcessTweaks->m_pAttentionTweaks_Ally;
		const UDisTweaks_PawnAttention* NeutralTweaks = ProcessTweaks->m_pAttentionTweaks_Neutral;
		debugf( TEXT("gidiag t=%.2f %s tweaks process %s (%s) ally %s neutral %s enemy %s/%s | ally max %.2f leak %.3f thresholds %.2f/%.2f/%.2f/%.2f tuning points %d"), Now, *Pawn->GetName(),
			*ProcessTweaks->GetName(), ProcessTweaks == UDisTweaks_AIBrainProcess_Attention::StaticClass()->GetDefaultObject() ? TEXT("CLASS DEFAULT") : TEXT("asset"),
			AllyTweaks ? *AllyTweaks->GetName() : TEXT("NULL"), NeutralTweaks ? *NeutralTweaks->GetName() : TEXT("NULL"),
			ProcessTweaks->m_pAttentionTweaks_Enemy_Unsuspecting ? *ProcessTweaks->m_pAttentionTweaks_Enemy_Unsuspecting->GetName() : TEXT("NULL"),
			ProcessTweaks->m_pAttentionTweaks_Enemy_Suspecting ? *ProcessTweaks->m_pAttentionTweaks_Enemy_Suspecting->GetName() : TEXT("NULL"),
			AllyTweaks ? AllyTweaks->m_fMaxAttention : -1.f, AllyTweaks ? AllyTweaks->m_fAttentionLeakRate : -1.f,
			AllyTweaks ? AllyTweaks->m_AttentionThresholds[0].m_fAttentionLevelToStart : -1.f, AllyTweaks ? AllyTweaks->m_AttentionThresholds[1].m_fAttentionLevelToStart : -1.f,
			AllyTweaks ? AllyTweaks->m_AttentionThresholds[2].m_fAttentionLevelToStart : -1.f, AllyTweaks ? AllyTweaks->m_AttentionThresholds[3].m_fAttentionLevelToStart : -1.f,
			AllyTweaks ? AllyTweaks->m_DistanceValueTuning.Points().Num() : -1 );
		debugf( TEXT("gidiag t=%.2f %s slots 0 (process %s ticking, sense masks 0x%x/0x%x, lookat component %s) top %s lookat desired %d"), Now, *Pawn->GetName(), *_pProcess->GetName(),
			Brain->m_FinalSenseMask_Player, Brain->m_FinalSenseMask_NonPlayer, Brain->m_LookAtRequest.m_pLookAtComponent ? TEXT("set") : TEXT("NULL"),
			Brain->GetTopAttnProxy().m_pCachedTargetActor ? *Brain->GetTopAttnProxy().m_pCachedTargetActor->GetName() : TEXT("none"), (INT)Brain->m_LookAtRequest.m_bDesired );
		return;
	}
	for( INT i = 0; ( GDisGIDiagLevel & 2 ) && i < _pProcess->m_AttentionInfos.Num(); i++ )
	{
		const FDisAttentionSlot& Slot = _pProcess->m_AttentionInfos(i);
		const IDisAttentionTargetInterface* Target = Slot.m_pAttentionTarget.GetObject() ? (const IDisAttentionTargetInterface*)Slot.m_pAttentionTarget.GetInterface() : NULL;
		const UDisAttentionInfo_Base* Info = Slot.m_pAttentionInfo;
		if( !Target || !Info )
		{
			continue;
		}
		const UDisAttentionInfo_Complex* Complex = ConstCast<UDisAttentionInfo_Complex>( Info );
		debugf( TEXT("gidiag t=%.2f %s slot %d target %s level %s%s value %.3f rel %d dead %d prevrel %d proxy status %d tag %d loc (%.0f,%.0f,%.0f)%s"),
			Now, *Pawn->GetName(), i, *Slot.m_pAttentionTarget.GetObject()->GetName(), DisGILevelName( Info->GetAttnLevel_Derived() ),
			Complex ? TEXT(" complex") : TEXT(" simple"), Info->GetNormalizedAttnValue_Derived(),
			(INT)Target->GetAttnTargetIncomingRelationship( Pawn ), (INT)Target->IsAttnTargetDead(), (INT)Info->m_PreviousRelationship,
			(INT)Info->m_ProxyInfo.m_Status, Info->m_ProxyInfo.m_AttentionTag, Info->m_ProxyInfo.m_Loc.X, Info->m_ProxyInfo.m_Loc.Y, Info->m_ProxyInfo.m_Loc.Z,
			Complex ? *FString::Printf( TEXT(" acc %.3f/%.3f los %d sighted %d touched %d psychic %d visrate %.3f drain %.3f"),
				Complex->m_AttentionAccumulator.GetCurrentValue(), Complex->m_AttentionAccumulator.GetMaxValue(), (INT)Complex->m_bHasLOS, (INT)Complex->m_bIsSighted,
				(INT)Complex->m_bIsTouched, (INT)Complex->m_bIsPsychic, Complex->m_fDebug_AttentionAccumulationRate, Complex->m_fDebug_AttentionDrainRate ) : TEXT("") );
	}
	if( !( GDisGIDiagLevel & 4 ) )
	{
		return;
	}
	const FDisAttentionProxy& Top = Brain->GetTopAttnProxy();
	debugf( TEXT("gidiag t=%.2f %s top %s level %s rel %d dead %d enemy %s lookat desired %d prio %d"), Now, *Pawn->GetName(),
		Top.m_pCachedTargetActor ? *Top.m_pCachedTargetActor->GetName() : TEXT("none"), DisGILevelName( Brain->m_TopAttnTargetAttentionLevel ),
		(INT)Brain->m_TopAttnTargetRelationship, (INT)Brain->m_bTopAttnTargetDead,
		Brain->m_TopEnemyAttnTargetProxy.m_pCachedTargetActor ? *Brain->m_TopEnemyAttnTargetProxy.m_pCachedTargetActor->GetName() : TEXT("none"),
		(INT)Brain->m_LookAtRequest.m_bDesired, (INT)Brain->m_LookAtRequest.m_Priority );
}

/*-----------------------------------------------------------------------------
	The small structs of the unit.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rvas 0x745d90 / 0x745da0 / 0x745dc0 (2013 0x700c00 the one-argument form)
FDisAttentionChangeReason::FDisAttentionChangeReason( EDisAttentionChangeReasonType _ReasonType )
{
	m_ReasonType = _ReasonType;
	m_ReasonInfo.m_pActor = NULL;
	m_ReasonInfo.m_iParam1 = 0;
	m_ReasonInfo.m_iParam2 = 0;
}

FDisAttentionChangeReason::FDisAttentionChangeReason( EDisAttentionChangeReasonType _ReasonType, const FDisAttentionChangeReasonInfo& _rReasonInfo )
{
	m_ReasonType = _ReasonType;
	m_ReasonInfo = _rReasonInfo;
}

FDisAttentionChangeReason FDisAttentionChangeReason::Drain()
{
	return FDisAttentionChangeReason( DACRT_Drain );
}

// DISHONORED(port): 2012 rva 0x78fea0 / 2013 0x728e90 and 0x78ff00 / 0x728ef0
FDisAttentionIncreaseInfo::FDisAttentionIncreaseInfo( const FVector& _rLoc )
{
	m_Loc = _rLoc;
	m_Vel = FVector( 0.f, 0.f, 0.f );
	m_fHeight = 3.4028235e38f;
	m_AttentionTag = DisAttention::NewInformationAttentionTag;
}

FDisAttentionIncreaseInfo::FDisAttentionIncreaseInfo( const FDisAttentionProxyInfo& _rProxyInfo )
{
	m_Loc = _rProxyInfo.m_Loc;
	m_Vel = _rProxyInfo.m_Vel;
	m_fHeight = _rProxyInfo.m_fHeight;
	m_AttentionTag = _rProxyInfo.m_AttentionTag;
}

// DISHONORED(port): 2012 rva 0x78fe30 / 2013 0x728e60 (41 B)
FDisAttentionChangeReason DisAttention::MakeAttentionChangeReason( BYTE _ReasonType, AActor* const _pActor, INT _iParam1, INT _iParam2 )
{
	FDisAttentionChangeReason Reason( (EDisAttentionChangeReasonType)_ReasonType );
	Reason.m_ReasonInfo.m_pActor = _pActor;
	Reason.m_ReasonInfo.m_iParam1 = _iParam1;
	Reason.m_ReasonInfo.m_iParam2 = _iParam2;
	return Reason;
}

// DISHONORED(port): 2012 rva 0x78fe60 / 2013 0x72c9e0 (59 B): how far the focal point sits above the feet, never negative.
FLOAT DisAttention::FindAttentionHeight( const FVector& _rAttentionLoc, const FVector& _rAttentionExtent, const FVector& _rAttentionFocalPoint )
{
	return Max( 0.f, _rAttentionFocalPoint.Z - ( _rAttentionLoc.Z - _rAttentionExtent.Z ) );
}

// 2012 rva 0x78d9c0 (AttentionLevelEquals, a file static): does a level match a reaction context's EDisAttentionLevelForReactions.
static UBOOL DisAttentionLevelEquals( BYTE _DAL, BYTE _DALFR )
{
	switch( _DALFR )
	{
	case DALFR_Unaware: return _DAL == DAL_Unaware;
	case DALFR_HeadTrack: return _DAL == DAL_HeadTrack;
	case DALFR_HeadTrackOrHigher: return _DAL >= DAL_HeadTrack;
	case DALFR_TurnToFace: return _DAL == DAL_TurnToFace;
	case DALFR_TurnToFaceOrHigher: return _DAL >= DAL_TurnToFace;
	case DALFR_TurnToFaceOrLower: return _DAL <= DAL_TurnToFace;
	case DALFR_Investigate: return _DAL == DAL_Investigate;
	case DALFR_InvestigateOrHigher: return _DAL >= DAL_Investigate;
	case DALFR_InvestigateOrLower: return _DAL <= DAL_Investigate;
	case DALFR_Busted: return _DAL >= DAL_Busted;
	case DALFR_ANY: return TRUE;
	default: return FALSE;
	}
}

static IDisAttentionTargetInterface* DisAttnTargetOf( const TScriptInterface<IDisAttentionTargetInterface>& _rTarget )
{
	return _rTarget.GetObject() ? (IDisAttentionTargetInterface*)_rTarget.GetInterface() : NULL;
}

static IDisAttentionTargetInterface* DisAttnTargetOf( ADishonoredPawn* _pPawn )
{
	return _pPawn ? static_cast<IDisAttentionTargetInterface*>( _pPawn ) : NULL;
}

static UBOOL DisActorIsGone( const AActor* _pActor )
{
	return GIsGame && _pActor && _pActor->m_bHasTerminated && _pActor->m_bLegalToTerminate;
}

/*-----------------------------------------------------------------------------
	The tweaks, the categories and the infos.
-----------------------------------------------------------------------------*/

UDisTweaks_AIBrainProcess_Attention* UDisAIBrainProcessAttention::GetAttentionProcessTweaks() const
{
	UDisTweaks_AIBrainProcess_Attention* Tweaks = Cast<UDisTweaks_AIBrainProcess_Attention>( GetTweaks() );
	return Tweaks ? Tweaks : UDisTweaks_AIBrainProcess_Attention::StaticClass()->GetDefaultObject<UDisTweaks_AIBrainProcess_Attention>();
}

// DISHONORED(port): 2012 rva 0x7a6460 / 2013 0x7301d0 (159 B): the four categories, each a complex tweak, this process's
// stop-threshold row and a simple tweak (both enemy categories share the enemy simple tweak).
void UDisAIBrainProcessAttention::BuildAttnCategoryArray( FAttentionTweakCategoryInfo* const _TweakCategoryInfos ) const
{
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = GetAttentionProcessTweaks();
	_TweakCategoryInfos[DATC_Neutral].m_pComplexAttnTweaks = Tweaks->m_pAttentionTweaks_Neutral;
	_TweakCategoryInfos[DATC_Neutral].m_pComplexAttnStopThresholdArray = const_cast<FLOAT*>( m_AttnStopThresholds_Neutral );
	_TweakCategoryInfos[DATC_Neutral].m_pSimpleAttnTweaks = Tweaks->m_pSimpleAttnTweaks_Neutral;
	_TweakCategoryInfos[DATC_Ally].m_pComplexAttnTweaks = Tweaks->m_pAttentionTweaks_Ally;
	_TweakCategoryInfos[DATC_Ally].m_pComplexAttnStopThresholdArray = const_cast<FLOAT*>( m_AttnStopThresholds_Ally );
	_TweakCategoryInfos[DATC_Ally].m_pSimpleAttnTweaks = Tweaks->m_pSimpleAttnTweaks_Ally;
	_TweakCategoryInfos[DATC_Enemy_Unsuspecting].m_pComplexAttnTweaks = Tweaks->m_pAttentionTweaks_Enemy_Unsuspecting;
	_TweakCategoryInfos[DATC_Enemy_Unsuspecting].m_pComplexAttnStopThresholdArray = const_cast<FLOAT*>( m_AttnStopThresholds_Enemy_Unsuspecting );
	_TweakCategoryInfos[DATC_Enemy_Unsuspecting].m_pSimpleAttnTweaks = Tweaks->m_pSimpleAttnTweaks_Enemy;
	_TweakCategoryInfos[DATC_Enemy_Suspecting].m_pComplexAttnTweaks = Tweaks->m_pAttentionTweaks_Enemy_Suspecting;
	_TweakCategoryInfos[DATC_Enemy_Suspecting].m_pComplexAttnStopThresholdArray = const_cast<FLOAT*>( m_AttnStopThresholds_Enemy_Suspecting );
	_TweakCategoryInfos[DATC_Enemy_Suspecting].m_pSimpleAttnTweaks = Tweaks->m_pSimpleAttnTweaks_Enemy;
}

// DISHONORED(port): 2012 rva 0x78d830 / 2013 0x728fa0 (87 B)
INT UDisAIBrainProcessAttention::FindAttentionTweakCategory( BYTE _RelationshipToTarget ) const
{
	switch( _RelationshipToTarget )
	{
	case Relationship_Neutral: return DATC_Neutral;
	case Relationship_Ally: return DATC_Ally;
	case Relationship_Enemy:
		{
			const BYTE Suspicion = m_pOwningBrain ? m_pOwningBrain->GetSuspicionLevel() : (BYTE)DAISL_Unsuspecting;
			if( Suspicion == DAISL_Unsuspecting ) return DATC_Enemy_Unsuspecting;
			if( Suspicion == DAISL_Suspecting ) return DATC_Enemy_Suspecting;
		}
		break;
	}
	return DATC_MAX;
}

static BYTE DisRelationshipOfOwnerTo( const UDisAIBrainProcessAttention* _pProcess, const IDisAttentionTargetInterface* _pTarget )
{
	const IDisRelationshipInterface* Relator = _pProcess->m_pOwningPawn ? static_cast<const IDisRelationshipInterface*>( _pProcess->m_pOwningPawn ) : NULL;
	return _pTarget ? _pTarget->GetAttnTargetIncomingRelationship( Relator ) : (BYTE)Relationship_Neutral;
}

// DISHONORED(port): 2012 rva 0x7a6570 / 2013 0x730310 (103 / 77 B): the complex tweak of the target's category, the class
// default when the category is out of range or the tweak unset.
UDisTweaks_PawnAttention* UDisAIBrainProcessAttention::FindAttentionTweaks( const IDisAttentionTargetInterface* const _pTarget ) const
{
	const INT Category = FindAttentionTweakCategory( DisRelationshipOfOwnerTo( this, _pTarget ) );
	FAttentionTweakCategoryInfo Infos[DATC_MAX + 1];
	appMemzero( Infos, sizeof(Infos) );
	BuildAttnCategoryArray( Infos );
	UDisTweaks_PawnAttention* Tweaks = Infos[Category].m_pComplexAttnTweaks;
	return Tweaks ? Tweaks : UDisTweaks_PawnAttention::StaticClass()->GetDefaultObject<UDisTweaks_PawnAttention>();
}

// DISHONORED(port): 2012 rva 0x7a77a0 / 2013 0x7362f0 (103 B)
UDisTweaks_SimpleAttention* UDisAIBrainProcessAttention::FindSimpleAttentionTweaks( const IDisAttentionTargetInterface* const _pTarget ) const
{
	const INT Category = FindAttentionTweakCategory( DisRelationshipOfOwnerTo( this, _pTarget ) );
	FAttentionTweakCategoryInfo Infos[DATC_MAX + 1];
	appMemzero( Infos, sizeof(Infos) );
	BuildAttnCategoryArray( Infos );
	UDisTweaks_SimpleAttention* Tweaks = Infos[Category].m_pSimpleAttnTweaks;
	return Tweaks ? Tweaks : UDisTweaks_SimpleAttention::StaticClass()->GetDefaultObject<UDisTweaks_SimpleAttention>();
}

// DISHONORED(port): 2012 rva 0x793130 / 2013 0x730be0 (142 B)
UDisAttentionInfo_Base* UDisAIBrainProcessAttention::FindAttentionInfo( const IDisAttentionTargetInterface* _pTarget, INT* _pIndex ) const
{
	INT WhereAt = INDEX_NONE;
	UDisAttentionInfo_Base* Found = NULL;
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		if( DisAttnTargetOf( m_AttentionInfos(i).m_pAttentionTarget ) == _pTarget )
		{
			Found = m_AttentionInfos(i).m_pAttentionInfo;
			WhereAt = i;
			break;
		}
	}
	if( _pIndex )
	{
		*_pIndex = WhereAt;
	}
	return Found;
}

// DISHONORED(port): 2012 rva 0x7a7880 / 2013 0x73b460 (340 B): the slot of a target, made on first demand. The player takes
// the process's own complex info, re-initialised from the class default's template every time the player is re-noticed;
// anything else takes a pooled simple info re-initialised from the simple class default. The target is told it is attended.
// DISHONORED(bringup): the complex info is a default subobject of the class in retail (m_pPlayerAttentionInfo on the class
// default); when the cooked default did not instance one here it is constructed on first use and that is logged once.
UDisAttentionInfo_Base& UDisAIBrainProcessAttention::GetAttentionInfo( IDisAttentionTargetInterface* _pTarget )
{
	UDisAttentionInfo_Base* Existing = FindAttentionInfo( _pTarget, NULL );
	if( Existing )
	{
		return *Existing;
	}
	const BYTE RelationshipToTarget = DisRelationshipOfOwnerTo( this, _pTarget );
	FDisAttentionSlot& Slot = m_AttentionInfos( m_AttentionInfos.AddZeroed() );
	Slot.m_pAttentionTarget.SetObject( _pTarget->GetAttnTargetActor() );
	Slot.m_pAttentionTarget.SetInterface( _pTarget );
	if( _pTarget->IsAttnTargetPlayer() )
	{
		const UDisAIBrainProcessAttention* Default = GetClass()->GetDefaultObject<UDisAIBrainProcessAttention>();
		if( !m_pPlayerAttentionInfo )
		{
			static UBOOL bNotedOnce = FALSE;
			if( !bNotedOnce )
			{
				bNotedOnce = TRUE;
				debugf( TEXT("DISHONORED(bringup): UDisAIBrainProcessAttention::m_pPlayerAttentionInfo was not instanced from the class default on %s; constructing the UDisAttentionInfo_Complex here"), *GetPathName() );
			}
			m_pPlayerAttentionInfo = ConstructObject<UDisAttentionInfo_Complex>( UDisAttentionInfo_Complex::StaticClass(), this );
		}
		else if( Default )
		{
			m_pPlayerAttentionInfo->ResetFromTemplate( Default->m_pPlayerAttentionInfo );
		}
		Slot.m_pAttentionInfo = m_pPlayerAttentionInfo;
	}
	else
	{
		UDisGlobalAIAttentionManager* Manager = DisGetGlobalAIAttentionManager();
		UDisAttentionInfo_Simple* Simple = Manager ? Manager->GetSimpleAttnInfoFromPool() : ConstructObject<UDisAttentionInfo_Simple>( UDisAttentionInfo_Simple::StaticClass(), UObject::GetTransientPackage() );
		Simple->ResetFromTemplate( UDisAttentionInfo_Simple::StaticClass()->GetDefaultObject() );
		Slot.m_pAttentionInfo = Simple;
	}
	Slot.m_pAttentionInfo->InitAttnInfo( this, _pTarget, RelationshipToTarget );
	UDisAttentionInfo_Base* Info = Slot.m_pAttentionInfo;
	_pTarget->OnAttentionReceived( m_pOwningBrain );
	return *Info;
}

// DISHONORED(port): 2012 rva 0x7a0730 / 2013 0x73b5c0 (300 / 282 B): the level change to Unaware is announced (unless the
// actor is gone), the target is told, the slot is swapped out and a simple info goes back to the pool.
void UDisAIBrainProcessAttention::RemoveAttentionInfoIdx( INT _Index )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( m_AttentionInfos(_Index).m_pAttentionTarget );
	UDisAttentionInfo_Base* Info = m_AttentionInfos(_Index).m_pAttentionInfo;
	AActor* TargetActor = Target ? Target->GetAttnTargetActor() : NULL;
	if( Target && Info && !DisActorIsGone( TargetActor ) )
	{
		const FDisAttentionProxyInfo ProxyInfo = Info->m_ProxyInfo;
		m_pOwningBrain->SendAttentionLevelChanged( TargetActor, Info->GetAttnLevel_Derived(), DAL_Unaware, FDisAttentionChangeReason::Drain(), ProxyInfo.m_Loc );
	}
	if( Target )
	{
		Target->OnAttentionLost( m_pOwningBrain );
	}
	m_AttentionInfos.RemoveSwap( _Index );
	UDisAttentionInfo_Simple* Simple = Cast<UDisAttentionInfo_Simple>( Info );
	if( Simple )
	{
		UDisGlobalAIAttentionManager* Manager = DisGetGlobalAIAttentionManager();
		if( Manager )
		{
			Manager->ReleaseSimpleAttnInfoToPool( Simple );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7932a0 / 2013 0x730d40 (44 / 39 B)
BYTE UDisAIBrainProcessAttention::GetAttentionLevel( const IDisAttentionTargetInterface* _pForTarget ) const
{
	const UDisAttentionInfo_Base* Info = FindAttentionInfo( _pForTarget, NULL );
	return Info ? Info->GetAttnLevel_Derived() : (BYTE)DAL_Unaware;
}

// DISHONORED(port): 2012 rva 0x798ad0 / 2013 0x730d70 (95 / 93 B): the proxy info of the slot, or the target's focal point
// when there is no slot (the rest of the result is left as handed in).
void UDisAIBrainProcessAttention::GetAttentionProxyInfo( const IDisAttentionTargetInterface* _pForTarget, FDisAttentionProxyInfo& _rResult ) const
{
	const UDisAttentionInfo_Base* Info = FindAttentionInfo( _pForTarget, NULL );
	if( Info )
	{
		_rResult = Info->m_ProxyInfo;
	}
	else if( _pForTarget )
	{
		_rResult.m_Loc = _pForTarget->GetAttnTargetFocalPoint();
	}
}

// DISHONORED(port): 2012 rva 0x7932d0 (44 B)
UBOOL UDisAIBrainProcessAttention::ShouldKeepAlarmRinging( const IDisAttentionTargetInterface* _pForTarget ) const
{
	const UDisAttentionInfo_Base* Info = FindAttentionInfo( _pForTarget, NULL );
	return Info ? Info->ShouldKeepAlarmRinging_Derived() : FALSE;
}

// DISHONORED(port): 2012 rva 0x79cce0 / 2013 0x73b6e0 (272 / 266 B)
void UDisAIBrainProcessAttention::GetTargetsAtAttentionLevel( BYTE _AttentionLevel, DIS_MEMSTACK_ARRAY( IDisAttentionTargetInterface* )& _rOut_Targets,
	BYTE _LifeStateFilter, BYTE _RelationshipFilter, BYTE _MinStatus ) const
{
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		IDisAttentionTargetInterface* Target = DisAttnTargetOf( m_AttentionInfos(i).m_pAttentionTarget );
		const UDisAttentionInfo_Base* Info = m_AttentionInfos(i).m_pAttentionInfo;
		if( !Target || !Info || Info->GetAttnLevel_Derived() < _AttentionLevel )
		{
			continue;
		}
		if( _RelationshipFilter != Relationship_MAX && DisRelationshipOfOwnerTo( this, Target ) != _RelationshipFilter )
		{
			continue;
		}
		if( _LifeStateFilter == DLSF_Alive && Target->IsAttnTargetDead() )
		{
			continue;
		}
		if( _LifeStateFilter == DLSF_Dead && !Target->IsAttnTargetDead() )
		{
			continue;
		}
		if( Info->m_ProxyInfo.m_Status >= _MinStatus )
		{
			_rOut_Targets.AddUniqueItem( Target );
		}
	}
}

// DISHONORED(port): 2012 rva 0x793230 / 2013 0x730ce0 (101 / 96 B)
void UDisAIBrainProcessAttention::ClearAllMinAttention( BYTE _Type )
{
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		if( m_AttentionInfos(i).m_pAttentionInfo )
		{
			m_AttentionInfos(i).m_pAttentionInfo->SetMinAttentionLevel_Derived( _Type, DAL_Unaware );
		}
	}
}

/*-----------------------------------------------------------------------------
	Corpses. DISHONORED(bringup): IDisCorpseInterface has no methods in this tree (FDisNPCDeathInfo and its
	ShouldIgnoreDeath / IsCorpseHandled / CanDeathBeWitnessed are unported), so of retail's three tests only the tweak flag and
	the global manager's override list are asked; a death info that says "ignore" or "handled" is not seen here.
-----------------------------------------------------------------------------*/

static const IDisCorpseInterface* DisCorpseOf( const IDisAttentionTargetInterface* _pTarget )
{
	UObject* Object = _pTarget ? const_cast<IDisAttentionTargetInterface*>( _pTarget )->GetAttnTargetActor() : NULL;
	return Object ? (const IDisCorpseInterface*)Object->GetInterfaceAddress( UDisCorpseInterface::StaticClass() ) : NULL;
}

// DISHONORED(port): 2012 rva 0x7a65e0 / 2013 0x7303a0 (118 B, the corpse form) and 0x7a7810 / 0x736360 (108 / 106 B, the
// target form, which also answers whether the death can be witnessed - FDisNPCDeathInfo's answer, TRUE here).
UBOOL UDisAIBrainProcessAttention::ShouldIgnoreDeathOf( const IDisAttentionTargetInterface* const _pTarget, UBOOL* _pOut_bCanDeathBeWitnessed ) const
{
	const IDisCorpseInterface* Corpse = DisCorpseOf( _pTarget );
	if( !Corpse )
	{
		return TRUE;
	}
	if( _pOut_bCanDeathBeWitnessed )
	{
		*_pOut_bCanDeathBeWitnessed = TRUE;
	}
	if( GetAttentionProcessTweaks()->m_bIgnoreAllCorpses )
	{
		return TRUE;
	}
	UDishonoredGlobalAIManager* Manager = DisGetGlobalAIManagerUnchecked();
	return Manager && m_pOwningBrain && Manager->IsIgnoreDeathOverridden( *m_pOwningBrain, *Corpse );
}

// DISHONORED(port): 2012 rva 0x7a6660 / 2013 0x730420 (59 B, the corpse form) and 0x7a66a0 / 0x730460 (110 / 108 B)
UBOOL UDisAIBrainProcessAttention::ShouldSeeBodyAsHandled( const IDisAttentionTargetInterface* const _pTarget ) const
{
	if( !DisCorpseOf( _pTarget ) )
	{
		return TRUE;
	}
	return ShouldIgnoreDeathOf( _pTarget, NULL );
}

/*-----------------------------------------------------------------------------
	The process virtuals.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7a6500 / 2013 0x730270 (100 / 109 B): the four stop-threshold rows are rolled once per brain.
void UDisAIBrainProcessAttention::InitBrainProcess_Derived()
{
	FAttentionTweakCategoryInfo Infos[DATC_MAX + 1];
	appMemzero( Infos, sizeof(Infos) );
	BuildAttnCategoryArray( Infos );
	for( INT Category = 0; Category < DATC_MAX; Category++ )
	{
		const UDisTweaks_PawnAttention* Tweaks = Infos[Category].m_pComplexAttnTweaks ? Infos[Category].m_pComplexAttnTweaks : UDisTweaks_PawnAttention::StaticClass()->GetDefaultObject<UDisTweaks_PawnAttention>();
		for( INT Level = 0; Level < 4; Level++ )
		{
			Infos[Category].m_pComplexAttnStopThresholdArray[Level] = Tweaks->m_AttentionThresholds[Level].CalculateStoppingThreshold();
		}
	}
	m_LatestAttentionTagForPlayer = -1;
	m_LatestAttentionTagForAnyTarget = -1;
}

// DISHONORED(port): 2012 rva 0x7a1f80 / 2013 0x73f0f0 (33 B)
void UDisAIBrainProcessAttention::TermBrainProcess_Derived()
{
	while( m_AttentionInfos.Num() > 0 )
	{
		RemoveAttentionInfoIdx( 0 );
	}
}

// DISHONORED(port): 2012 rva 0x7aa770 / 2013 0x7464b0 (48 B)
void UDisAIBrainProcessAttention::TickBrainProcess_Derived( FLOAT _fDeltaSeconds )
{
	// DISHONORED(bringup): DisIsActorFrozenByBendTimePower (2013 0xbbf370) is the per-actor test; this tree has the global DisIsBendTimeOn() (agent GD).
	if( DisIsBendTimeOn() )
	{
		return;
	}
	if( DisGISightStandInEnabled() && m_pOwningBrain )
	{
		DisGISightStandInTick( m_pOwningBrain );
	}
	UpdateSoireeAttention();
	TickAttention( _fDeltaSeconds );
	if( DisGIDiagEnabled() )
	{
		DisGIDiagReport( this, _fDeltaSeconds );
	}
}

// DISHONORED(port): 2012 rva 0x792f70 / 2013 0x73f360 (105 / 135 B): after a load every slot's target is told it is attended again.
void UDisAIBrainProcessAttention::PostGameLoad_BrainProcess_Derived()
{
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		IDisAttentionTargetInterface* Target = DisAttnTargetOf( m_AttentionInfos(i).m_pAttentionTarget );
		if( Target )
		{
			Target->OnAttentionReceived( m_pOwningBrain );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7a1fb0 / 2013 0x73f2c0 (210 / 157 B): a proxy whose update reason named the gone actor loses it,
// every info is told, and the gone actor's own slot is removed.
void UDisAIBrainProcessAttention::OnOtherActorTerminated_AIBrainProcess_Derived( const AActor& _rActor )
{
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		UDisAttentionInfo_Base* Info = m_AttentionInfos(i).m_pAttentionInfo;
		if( !Info )
		{
			continue;
		}
		if( Info->m_ProxyInfo.m_ProxyUpdateReason.m_ReasonInfo.m_pActor == &_rActor )
		{
			Info->m_ProxyInfo.m_ProxyUpdateReason = FDisAttentionChangeReason::Drain();
		}
		Info->OnOtherActorTerminated_AttnInfo_Derived( _rActor );
	}
	const IDisAttentionTargetInterface* Target = (const IDisAttentionTargetInterface*)const_cast<AActor&>( _rActor ).GetInterfaceAddress( UDisAttentionTargetInterface::StaticClass() );
	INT Index = INDEX_NONE;
	if( Target && FindAttentionInfo( Target, &Index ) )
	{
		RemoveAttentionInfoIdx( Index );
	}
}

// DISHONORED(port): 2012 rva 0x792f10 / 2013 0x730a80 (92 / 87 B)
void UDisAIBrainProcessAttention::OnDifficultyChange_AIBrainProcess_Derived()
{
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		if( m_AttentionInfos(i).m_pAttentionInfo )
		{
			m_AttentionInfos(i).m_pAttentionInfo->OnDifficultyChange_AttnInfo_Derived();
		}
	}
}

// DISHONORED(port): 2012 rva 0x78d890 (276 B, 32 ids) / 2013 0x729000 (312 B, 38 ids): the 2013 mask, read off the byte offsets
// the 2013 body sets against its mask base (VA 0x1466618): 2 14 15 16 22 23 26 31 35 36 37 38 41 46 47 48 52 61 62 65 74 78 79
// 80 81 90 91 98 99 100 101 110 111 114 116 117 118 119 - this tree's EAIStimID. The six 2013-only ids (ReachabilityChange,
// WitnessShadowKill, the four DLC06/07 stims) have no 2012 body; their 2013 filters are inventoried in agentGI.md, not ported.
const BYTE* UDisAIBrainProcessAttention::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = {
		EAIStimID_Alarm, EAIStimID_ClearAllMinAttention, EAIStimID_ClearAttention, EAIStimID_ClearMinAttentionForTarget,
		EAIStimID_DialogAttentionChange, EAIStimID_DeathByWoL, EAIStimID_DiscoveredCorpse, EAIStimID_DoorUsedByPlayerWhileWary,
		EAIStimID_EscapedBeingChoked, EAIStimID_EscapedBeingPossessed, EAIStimID_EvadedMelee_Incoming, EAIStimID_EvadedMelee_Outgoing,
		EAIStimID_ForceRingAlarm, EAIStimID_HeardSomething, EAIStimID_HeardCorpseSplat, EAIStimID_Help, EAIStimID_IncomingDamage,
		EAIStimID_MaxOutAttention, EAIStimID_MeleeConnected_Outgoing, EAIStimID_NoticeRequest, EAIStimID_PlayerHideoutTransition,
		EAIStimID_PsychicAttentionEnabled, EAIStimID_PsychicAttentionDisabled, EAIStimID_PsychicAttentionDisabled_AllTargets,
		EAIStimID_ReachabilityChange, EAIStimID_SearchRequest, EAIStimID_SetMinAttentionForTarget, EAIStimID_SuspicionLevelChanged,
		EAIStimID_TargetSighted, EAIStimID_TargetTouched, EAIStimID_TargetUnsighted, EAIStimID_Traveled, EAIStimID_WitnessDeath,
		EAIStimID_WitnessShadowKill, EAIStimID_DLC06_SmokeBombed, EAIStimID_DLC06_WitnessSilentExplosion, EAIStimID_DLC07_Resurrected,
		EAIStimID_DLC07_Pulled };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x7aa490 (735 B) / 2013 0x747d40 (580 B): the 2013 switch. The six 2013-only cases (81, 114, 116..119)
// return the null delegate here (the mask admits them, the null delegate lets them through to the behaviours, which is what an
// unbound retail delegate does too). The match table labels the 2013 delegator at VA 0xb41690 "FilterTargetTouched"; the 2013
// switch binds it to id 91 (SetMinAttentionForTarget) and binds id 100 (TargetTouched) to VA 0xb41610 -> 0xb3f420 - the names
// below follow the 2013 switch, not the mislabel.
FDisStimPredicateDelegate UDisAIBrainProcessAttention::GetFilterStimDelegate_BrainProcess( BYTE _StimID )
{
	switch( _StimID )
	{
	case EAIStimID_Alarm: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_Alarm, FilterAlarm );
	case EAIStimID_ClearAllMinAttention: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_ClearAllMinAttention, FilterClearAllMinAttention );
	case EAIStimID_ClearAttention: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_ClearAttention, FilterClearAttention );
	case EAIStimID_ClearMinAttentionForTarget: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_ClearMinAttentionForTarget, FilterClearMinAttentionForTarget );
	case EAIStimID_DialogAttentionChange: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_DialogAttentionChange, FilterDialogAttentionChange );
	case EAIStimID_DeathByWoL: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_DeathByWoL, FilterDeathByWoL );
	case EAIStimID_DoorUsedByPlayerWhileWary: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_DoorUsedByPlayerWhileWary, FilterDoorUsedByPlayerWhileWary );
	case EAIStimID_EscapedBeingChoked:
	case EAIStimID_EscapedBeingPossessed: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_EscapedBeingChoked, FilterEscapedBeingChoked );
	case EAIStimID_EvadedMelee_Incoming: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_EvadedMelee_Incoming, FilterEvadedMelee_Incoming );
	case EAIStimID_EvadedMelee_Outgoing: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_EvadedMelee_Outgoing, FilterEvadedMelee_Outgoing );
	case EAIStimID_ForceRingAlarm: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_ForceRingAlarm, FilterForceRingAlarm );
	case EAIStimID_HeardSomething: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_HeardSomething, FilterHeardSomething );
	case EAIStimID_Help: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_Help, FilterHelp );
	case EAIStimID_IncomingDamage: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_IncomingDamage, FilterIncomingDamage );
	case EAIStimID_MaxOutAttention: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_MaxOutAttention, FilterMaxOutAttention );
	case EAIStimID_MeleeConnected_Outgoing: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_MeleeConnected_Outgoing, FilterMeleeConnected_Outgoing );
	case EAIStimID_NoticeRequest: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_NoticeRequest, FilterNoticeRequest );
	case EAIStimID_PsychicAttentionEnabled: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_PsychicAttentionEnabled, FilterPsychicAttentionEnabled );
	case EAIStimID_PsychicAttentionDisabled: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_PsychicAttentionDisabled, FilterPsychicAttentionDisabled );
	case EAIStimID_PsychicAttentionDisabled_AllTargets: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_PsychicAttentionDisabled_AllTargets, FilterPsychicAttentionDisabled_AllTargets );
	case EAIStimID_SearchRequest: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_SearchRequest, FilterSearchRequest );
	case EAIStimID_SetMinAttentionForTarget: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_SetMinAttentionForTarget, FilterSetMinAttentionForTarget );
	case EAIStimID_SuspicionLevelChanged: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_SuspicionLevelChanged, FilterSuspicionLevelChanged );
	case EAIStimID_TargetSighted: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_TargetSighted, FilterTargetSighted );
	case EAIStimID_TargetTouched: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_TargetTouched, FilterTargetTouched );
	case EAIStimID_TargetUnsighted: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_TargetUnsighted, FilterTargetUnsighted );
	case EAIStimID_Traveled: return DIS_BIND_STIM_PREDICATE( UDisAIBrainProcessAttention, FAIStimStruct_Traveled, FilterTraveled );
	case EAIStimID_DiscoveredCorpse:
	case EAIStimID_WitnessDeath:
	case EAIStimID_HeardCorpseSplat:
	case EAIStimID_PlayerHideoutTransition:
	case EAIStimID_ReachabilityChange:
	case EAIStimID_WitnessShadowKill:
	case EAIStimID_DLC06_SmokeBombed:
	case EAIStimID_DLC06_WitnessSilentExplosion:
	case EAIStimID_DLC07_Resurrected:
	case EAIStimID_DLC07_Pulled:
	default:
		return FDisStimPredicateDelegate();
	}
}

/*-----------------------------------------------------------------------------
	The tick.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7aa040 (1,089 B) / 2013 0x745910 (833 B). The 2013 shape: per slot the clear timer runs down,
// HandleStateChanges, the info's own tick (2013 folds the three calls into sub_B60C10, VA 0xb60c10), the corpse response,
// the messages to the brain, a psychic target clears the noted magic, and a slot that has nothing left (no witnessed-death
// state pending, no clear timer, no attention) is removed. Outside combat the reaction contexts are marked and the seen
// non-enemies collected for the empathy pass. Then the reactions, the top target, the empathy.
void UDisAIBrainProcessAttention::TickAttention( FLOAT _fDeltaSeconds )
{
	if( _fDeltaSeconds <= 0.f )
	{
		return;
	}
	m_Debug_DuringAttentionTick = 1;
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = GetAttentionProcessTweaks();
	FMemMark Mark( GMainThreadMemStack );
	DIS_MEMSTACK_ARRAY( IDisAttentionTargetInterface* ) FirstTargetsWithActiveContext;
	DIS_MEMSTACK_ARRAY( IDisAttentionTargetInterface* ) NonEnemiesThatAreSeen;
	DIS_MEMSTACK_ARRAY( FDisAppliedAttentionChangeInfo ) AttentionChangeInfos;
	DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage ) AttentionMessages;
	FirstTargetsWithActiveContext.AddZeroed( Tweaks->m_AttentionReactions.Num() );
	NonEnemiesThatAreSeen.Empty( m_AttentionInfos.Num() );
	AttentionChangeInfos.Empty( DACRT_MAX );
	AttentionMessages.Empty( DACRT_MAX + 1 );
	const UBOOL bInCombat = m_pOwningBrain->GetAwarenessLevel() == EAIAwareness_InCombat;
	for( INT AttentionIdx = 0; AttentionIdx < m_AttentionInfos.Num(); AttentionIdx++ )
	{
		FDisAttentionSlot& Slot = m_AttentionInfos(AttentionIdx);
		IDisAttentionTargetInterface* Target = DisAttnTargetOf( Slot.m_pAttentionTarget );
		UDisAttentionInfo_Base* Info = Slot.m_pAttentionInfo;
		if( !Target || !Info )
		{
			m_Debug_DuringAttentionTick = 0;
			RemoveAttentionInfoIdx( AttentionIdx-- );
			m_Debug_DuringAttentionTick = 1;
			continue;
		}
		const UBOOL bClearAttention = Info->m_fClearTimer > 0.f;
		Info->m_fClearTimer -= _fDeltaSeconds;
		AttentionChangeInfos.Empty( DACRT_MAX );
		Info->HandleStateChanges( Target, this, m_pOwningBrain, m_bAttentionInfosTweaksUpdated, AttentionMessages );
		Info->TickAttnInfo_Derived( _fDeltaSeconds, Target, this, m_pOwningBrain, bClearAttention, AttentionChangeInfos, AttentionMessages );
		const BYTE WitnessDeathRequest = Info->GetCurrentCorpseResponse( Target, this );
		TickAttention_Messages( Target, AttentionMessages );
		if( Info->HasPsychicAttention() )
		{
			ADishonoredPawn* TargetPawn = Cast<ADishonoredPawn>( Target->GetAttnTargetActor() );
			if( TargetPawn )
			{
				m_pOwningBrain->ClearNotedMagic( TargetPawn );
			}
		}
		const BYTE DeathState = Info->m_WitnessDeathState;
		const UBOOL bDeathPending = DeathState == (BYTE)DWDS_DiedUnseen || DeathState == (BYTE)DWDS_DiedWitnessed || DeathState == (BYTE)DWDS_CorpseToFind;
		if( bDeathPending || Info->m_fClearTimer > 0.f || Info->HasAnyAttention_Derived() )
		{
			if( !bInCombat )
			{
				MarkActiveReactionContexts( Slot, AttentionChangeInfos, FirstTargetsWithActiveContext );
				if( Info->GetAttnLevel_Derived() >= DAL_Busted && Info->m_PreviousRelationship != Relationship_Enemy )
				{
					NonEnemiesThatAreSeen.AddItem( Target );
				}
			}
			TickAttention_UpdateLatestAttentionTag( Slot );
			TickAttention_ReactToCorpses( Slot, WitnessDeathRequest );
		}
		else
		{
			m_Debug_DuringAttentionTick = 0;
			RemoveAttentionInfoIdx( AttentionIdx-- );
			m_Debug_DuringAttentionTick = 1;
		}
		AttentionMessages.Empty( DACRT_MAX + 1 );
	}
	CheckAttentionReactions( FirstTargetsWithActiveContext );
	HandleTopAttnTarget();
	m_Debug_DuringAttentionTick = 0;
	m_bAttentionInfosTweaksUpdated = 0;
	TickAttention_Empathy( NonEnemiesThatAreSeen );
	Mark.Pop();
}

// DISHONORED(port): 2012 rva 0x798860 / 2013 0x73b350 (264 / 263 B): every level change goes to the brain (the Busted / Unbusted
// handling and the behaviours' stims); a message that resets the attention tells the target it was lost and received again;
// the Kismet attention events see the first level and the last.
void UDisAIBrainProcessAttention::TickAttention_Messages( IDisAttentionTargetInterface* _pAttnTarget, const DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rMessages )
{
	if( _rMessages.Num() <= 0 )
	{
		return;
	}
	AActor* TargetActor = _pAttnTarget->GetAttnTargetActor();
	for( INT i = 0; i < _rMessages.Num(); i++ )
	{
		const DisAttention::FDisAttentionChangeMessage& Message = _rMessages(i);
		if( Message.m_bResetAttentionReceived )
		{
			_pAttnTarget->OnAttentionLost( m_pOwningBrain );
		}
		m_pOwningBrain->SendAttentionLevelChanged( TargetActor, Message.m_PreviousAttentionLevel, Message.m_NextAttentionLevel, Message.m_ChangeReason, Message.m_ProxyLoc );
		if( Message.m_bResetAttentionReceived )
		{
			_pAttnTarget->OnAttentionReceived( m_pOwningBrain );
		}
	}
	TickAttention_Kismet( TargetActor, _rMessages(0).m_PreviousAttentionLevel, _rMessages( _rMessages.Num() - 1 ).m_NextAttentionLevel );
}

// DISHONORED(port): 2012 rva 0x792e10 (250 B) / 2013 0x7363d0 (the callee of TickAttention_Messages, unmatched by the table):
// the pawn's enabled DisSeqEvent_AttentionIncreasedTo / DecreasedTo events fire with the new level.
void UDisAIBrainProcessAttention::TickAttention_Kismet( AActor* _pAttnTargetActor, BYTE _OldAttentionLevel, BYTE _NewAttentionLevel )
{
	if( _OldAttentionLevel == _NewAttentionLevel || !m_pOwningPawn )
	{
		return;
	}
	const INT NumEvents = m_pOwningPawn->GeneratedEvents.Num();
	for( INT i = 0; i < NumEvents; i++ )
	{
		USequenceEvent* Event = m_pOwningPawn->GeneratedEvents(i);
		if( !Event || !Event->bEnabled )
		{
			continue;
		}
		UDisSeqEvent_AttentionIncreasedTo* Increased = Cast<UDisSeqEvent_AttentionIncreasedTo>( Event );
		if( Increased && _NewAttentionLevel > _OldAttentionLevel )
		{
			Increased->m_eActivatedAttentionLevel = _NewAttentionLevel;
			Increased->CheckActivate( m_pOwningPawn, _pAttnTargetActor, FALSE, NULL, FALSE );
		}
		UDisSeqEvent_AttentionDecreasedTo* Decreased = Cast<UDisSeqEvent_AttentionDecreasedTo>( Event );
		if( Decreased && _NewAttentionLevel < _OldAttentionLevel )
		{
			Decreased->m_eActivatedAttentionLevel = _NewAttentionLevel;
			Decreased->CheckActivate( m_pOwningPawn, _pAttnTargetActor, FALSE, NULL, FALSE );
		}
	}
}

// DISHONORED(port): 2012 rva 0x798970 / 2013 0x72ca20 (88 / 85 B)
void UDisAIBrainProcessAttention::TickAttention_UpdateLatestAttentionTag( const FDisAttentionSlot& _rCurrentSlot )
{
	const INT Tag = _rCurrentSlot.m_pAttentionInfo->m_ProxyInfo.m_AttentionTag;
	m_LatestAttentionTagForAnyTarget = Max( m_LatestAttentionTagForAnyTarget, Tag );
	const IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rCurrentSlot.m_pAttentionTarget );
	if( Target && Target->IsAttnTargetPlayer() )
	{
		m_LatestAttentionTagForPlayer = Max( m_LatestAttentionTagForPlayer, Tag );
	}
}

// DISHONORED(port): 2012 rva 0x79caa0 (568 B) / 2013 0x7433d0 (514 B): a death witnessed raises WitnessDeath, a corpse found in
// sight DiscoveredCorpse, one found indirectly HeardCorpseSplat, and any of them plus an existing corpse escalates the
// suspicion level. DISHONORED(bringup): the corpse's FDisNPCDeathInfo (killer, freshness) is not reachable (IDisCorpseInterface
// has no methods here): the culprit is unknown, the freshness test is not made, and the stims carry the target's location.
void UDisAIBrainProcessAttention::TickAttention_ReactToCorpses( const FDisAttentionSlot& _rCurrentSlot, BYTE _WitnessDeathRequest )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rCurrentSlot.m_pAttentionTarget );
	AActor* TargetActor = Target ? Target->GetAttnTargetActor() : NULL;
	const IDisCorpseInterface* Corpse = TargetActor ? (const IDisCorpseInterface*)TargetActor->GetInterfaceAddress( UDisCorpseInterface::StaticClass() ) : NULL;
	switch( _WitnessDeathRequest )
	{
	case DWDR_WitnessedDeath:
		if( Corpse )
		{
			FAIStimStruct_WitnessDeath Stim( EC_EventParm );
			Stim.m_StimID = EAIStimID_WitnessDeath;
			Stim.m_pDeadBody.SetObject( TargetActor );
			Stim.m_pDeadBody.SetInterface( const_cast<IDisCorpseInterface*>( Corpse ) );
			Stim.m_pCulprit = NULL;
			Stim.m_KillerLocation = TargetActor->Location;
			DisHandleAIStim( m_pOwningBrain, Stim, this );
		}
		break;
	case DWDR_FoundNewCorpse_Direct:
		if( Corpse )
		{
			FAIStimStruct_DiscoveredCorpse Stim( EC_EventParm );
			Stim.m_StimID = EAIStimID_DiscoveredCorpse;
			Stim.m_pDeadBody.SetObject( TargetActor );
			Stim.m_pDeadBody.SetInterface( const_cast<IDisCorpseInterface*>( Corpse ) );
			Stim.m_pCulprit = ADishonoredPlayerPawn::s_pInstance;
			Stim.m_DiscoveredLoc = _rCurrentSlot.m_pAttentionInfo->m_ProxyInfo.m_Loc;
			DisHandleAIStim( m_pOwningBrain, Stim, this );
		}
		break;
	case DWDR_FoundNewCorpse_Indirect:
		if( Corpse )
		{
			FAIStimStruct_HeardCorpseSplat Stim( EC_EventParm );
			Stim.m_StimID = EAIStimID_HeardCorpseSplat;
			Stim.m_pCorpse.SetObject( TargetActor );
			Stim.m_pCorpse.SetInterface( const_cast<IDisCorpseInterface*>( Corpse ) );
			DisHandleAIStim( m_pOwningBrain, Stim, this );
		}
		break;
	case DWDR_ExistingCorpse:
		break;
	default:
		return;
	}
	m_pOwningBrain->EscalateSuspicionLevel();
}

// DISHONORED(port): 2012 rva 0x7a9f50 / 2013 0x744d80 (236 B)
void UDisAIBrainProcessAttention::TickAttention_Empathy( DIS_MEMSTACK_ARRAY( IDisAttentionTargetInterface* )& _rNonEnemyTargets )
{
	if( !m_pOwningBrain->CanEmpathize() )
	{
		return;
	}
	for( INT i = 0; i < _rNonEnemyTargets.Num(); i++ )
	{
		TickAttention_ReactToAlly( _rNonEnemyTargets(i) );
		TickAttention_CheckForMurder( _rNonEnemyTargets(i) );
		TickAttention_CheckForCarryingCorpse( _rNonEnemyTargets(i) );
	}
}

// DISHONORED(port): 2012 rva 0x7a9900 (337 B) / 2013 0x7435f0 (352 B): a seen ally with a reaction target of its own (its
// search, combat or panic target) that this brain should care about becomes this brain's enemy and gets the matching increase.
void UDisAIBrainProcessAttention::TickAttention_ReactToAlly( const IDisAttentionTargetInterface* _pNonEnemyTarget )
{
	const UDishonoredAIBrain* AllyBrain = _pNonEnemyTarget->GetAttnTargetBrain();
	const IDisRelationshipInterface* AllyRelator = _pNonEnemyTarget->ToRelationshipInterface();
	if( !AllyBrain || !AllyRelator )
	{
		return;
	}
	BYTE ReasonType = DACRT_MAX;
	IDisAttentionTargetInterface* ReactionTarget = AllyBrain->GetAllyReactionTarget( ReasonType );
	if( !ReactionTarget )
	{
		return;
	}
	const IDisRelationshipInterface* TargetRelator = ReactionTarget->ToRelationshipInterface();
	if( !TargetRelator || !m_pOwningBrain->ShouldCareAboutThisCrime( AllyRelator, TargetRelator ) )
	{
		return;
	}
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( ReactionTarget );
	if( !Tweaks )
	{
		return;
	}
	FDisAttentionIncrease Increase( 0.f, 0 );
	switch( ReasonType )
	{
	case DACRT_SeeAllySearch: Increase = Tweaks->m_IncreaseForAllySearchTarget; break;
	case DACRT_SeeAllyCombat: Increase = Tweaks->m_IncreaseForAllyCombatTarget; break;
	case DACRT_SeeAllyPanic: Increase = Tweaks->m_IncreaseForAllyPanicTarget; break;
	default: return;
	}
	m_pOwningPawn->OverrideRelationToIndividual( *TargetRelator, Relationship_Enemy, TRUE );
	FDisAttentionProxyInfo Info( EC_EventParm );
	AllyBrain->GetAttentionProxyInfo( ReactionTarget, Info );
	if( Info.m_AttentionTag )
	{
		BumpAttention( ReactionTarget, Increase, FDisAttentionChangeReason::Drain(), FDisAttentionIncreaseInfo( Info ) );
	}
}

// DISHONORED(port): 2012 rva 0x7a9a60 (368 B) / 2013 0x743750 (365 B): a seen non-enemy being murdered by someone this brain
// already attends at least m_MinEnemyAttnForMurderDetection makes the murderer an enemy, with the ally-combat increase.
void UDisAIBrainProcessAttention::TickAttention_CheckForMurder( IDisAttentionTargetInterface* _pNonEnemyTarget )
{
	IDisAttentionTargetInterface* Murderer = NULL;
	if( !_pNonEnemyTarget->IsAttnTargetBeingMurdered( &Murderer ) || !Murderer )
	{
		return;
	}
	const UDisAttentionInfo_Base* MurdererInfo = FindAttentionInfo( Murderer, NULL );
	const BYTE MurdererLevel = MurdererInfo ? MurdererInfo->GetAttnLevel_Derived() : (BYTE)DAL_Unaware;
	if( MurdererLevel < GetAttentionProcessTweaks()->m_MinEnemyAttnForMurderDetection )
	{
		return;
	}
	const IDisRelationshipInterface* VictimRelator = _pNonEnemyTarget->ToRelationshipInterface();
	const IDisRelationshipInterface* MurdererRelator = Murderer->ToRelationshipInterface();
	if( !VictimRelator || !MurdererRelator || !m_pOwningBrain->ShouldCareAboutThisCrime( VictimRelator, MurdererRelator ) )
	{
		return;
	}
	m_pOwningPawn->OverrideRelationToIndividual( *MurdererRelator, Relationship_Enemy, TRUE );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Murderer );
	FDisAttentionIncreaseInfo Info( Murderer->GetAttnTargetFocalPoint() );
	Info.m_fHeight = DisAttention::FindAttentionHeight( Murderer->GetAttnTargetLocation(), Murderer->GetAttnTargetExtent(), Murderer->GetAttnTargetFocalPoint() );
	BumpAttention( Murderer, Tweaks->m_IncreaseForAllyCombatTarget, DisAttention::MakeAttentionChangeReason( DACRT_SeeAllyCombat, NULL, 0, 0 ), Info );
}

// DISHONORED(port): 2012 rva 0x7a6710 (141 B) / 2013 0x7304d0 (197 B, the 2013 shape: the escalation is behind
// m_bEscalateSuspicionIfSeeAnyCorpse, the enmity behind m_bBecomeEnemyIfSeeCorpseCarry; 2012 always escalated).
void UDisAIBrainProcessAttention::TickAttention_CheckForCarryingCorpse( IDisAttentionTargetInterface* _pNonEnemyTarget )
{
	if( !_pNonEnemyTarget->IsAttnTargetCarryingACorpse() )
	{
		return;
	}
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = GetAttentionProcessTweaks();
	if( Tweaks->m_bEscalateSuspicionIfSeeAnyCorpse )
	{
		m_pOwningBrain->EscalateSuspicionLevel();
	}
	if( Tweaks->m_bBecomeEnemyIfSeeCorpseCarry )
	{
		const IDisRelationshipInterface* Relator = _pNonEnemyTarget->ToRelationshipInterface();
		if( Relator && m_pOwningPawn->DetermineRelationship( *Relator ) != Relationship_Ally )
		{
			m_pOwningPawn->OverrideRelationToIndividual( *Relator, Relationship_Enemy, TRUE );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7a7ce0 (334 B) / 2013 0x73f120 (329 B): every pawn acting in a matinee this pawn is in is held at
// Busted for as long as the matinee runs (the soiree minimum-attention type), re-stated every tick.
void UDisAIBrainProcessAttention::UpdateSoireeAttention()
{
	ClearAllMinAttention( DMALT_Soiree );
	if( !m_pOwningPawn )
	{
		return;
	}
	for( INT MatIdx = 0; MatIdx < m_pOwningPawn->m_MatineeControllersInfos.Num(); MatIdx++ )
	{
		USeqAct_Interp* Matinee = m_pOwningPawn->m_MatineeControllersInfos(MatIdx).m_pMatinee;
		if( !Matinee )
		{
			continue;
		}
		for( INT GroupIdx = 0; GroupIdx < Matinee->GroupInst.Num(); GroupIdx++ )
		{
			UInterpGroupInst* GroupInst = Matinee->GroupInst(GroupIdx);
			ADishonoredPawn* Pawn = GroupInst ? Cast<ADishonoredPawn>( GroupInst->GetGroupActor() ) : NULL;
			if( Pawn && Pawn != m_pOwningPawn )
			{
				GetAttentionInfo( DisAttnTargetOf( Pawn ) ).SetMinAttentionLevel_Derived( DMALT_Soiree, DAL_Busted );
			}
		}
	}
}

// DISHONORED(port): 2012 rva 0x7a67a0 (1,217 B) / 2013 0x7305a0 (1,233 B, unmatched by the table: the callee of TickAttention
// between CheckAttentionReactions and TickAttention_Empathy). The most obvious target is elected by priority type (a live
// enemy over a dead ally over a live neutral over a live ally over a dead neutral or enemy), then by attention level; two live
// enemies at the same level are ordered by whether they attack us and by distance (the current top enemy keeps its place
// within m_fDistanceDeltaForNewTopEnemy). A slot whose sense is blind to that kind of target (IsBlindTo: the sense mask or a
// soiree fake death, the 2013 test; 2012 read the mask bit alone) is skipped. The winner goes to the brain's two top
// targets, is told it is the top target, and the top enemy's slot is swapped to the front.
void UDisAIBrainProcessAttention::HandleTopAttnTarget()
{
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = GetAttentionProcessTweaks();
	UDishonoredAIBrain* Brain = m_pOwningBrain;
	INT MostObviousAttnIndex = INDEX_NONE;
	IDisAttentionTargetInterface* MostObviousTarget = NULL;
	BYTE MostObviousTargetAttentionLevel = DAL_Unaware;
	FDisAttentionChangeReason MostObviousTargetAttentionChangeReason = FDisAttentionChangeReason::Drain();
	BYTE MostObviousTargetRelationship = Relationship_Ally;
	UBOOL bMostObviousTargetDead = FALSE;
	INT MostObviousTargetType = DTATPT_DontCare;
	INT MostObviousEnemyIndex = INDEX_NONE;
	IDisAttentionTargetInterface* MostObviousEnemy = NULL;
	UBOOL bMostObviousEnemyIsCurrentTop = FALSE;
	BYTE MostObviousEnemyAttentionLevel = DAL_Unaware;
	UBOOL bMostObviousEnemyAttackingUs = FALSE;
	FLOAT fMostObviousEnemyDistanceFromSelf = 3.4e38f;
	const INT AttentionInfosCount = m_AttentionInfos.Num();
	for( INT ii = 0; ii < AttentionInfosCount; ii++ )
	{
		IDisAttentionTargetInterface* Target = DisAttnTargetOf( m_AttentionInfos(ii).m_pAttentionTarget );
		UDisAttentionInfo_Base* Info = m_AttentionInfos(ii).m_pAttentionInfo;
		if( !Target || !Info )
		{
			continue;
		}
		const BYTE AttentionLevel = Info->GetAttnLevel_Derived();
		if( AttentionLevel <= DAL_Unaware || Brain->IsBlindTo( Target->IsAttnTargetPlayer() ) )
		{
			continue;
		}
		const BYTE Relationship = DisRelationshipOfOwnerTo( this, Target );
		const UBOOL bIsDead = Info->m_WitnessDeathState != (BYTE)DWDS_Alive;
		UBOOL bHasHigherAttention = AttentionLevel > MostObviousTargetAttentionLevel;
		const UBOOL bIsAPotentialEnemy = !bIsDead && Relationship == Relationship_Enemy;
		const ADishonoredPawn* EnemyPawn = NULL;
		UBOOL bEnemyAttackingUs = FALSE;
		FLOAT fEnemyDistanceFromSelf = 0.f;
		if( bIsAPotentialEnemy )
		{
			EnemyPawn = Cast<ADishonoredPawn>( Target->GetAttnTargetActor() );
			// DISHONORED(bringup): ADishonoredPawn::IsAttacking( const ADishonoredPawn* ) (2013 vtable +1812) is the combat
			// manager's and is not ported; an enemy is never "attacking us" here, so two enemies at one level order by distance.
			bEnemyAttackingUs = FALSE;
			fEnemyDistanceFromSelf = ( Info->m_ProxyInfo.m_Loc - m_pOwningPawn->Location ).Size();
			if( AttentionLevel == MostObviousEnemyAttentionLevel )
			{
				const UBOOL bCloser = bMostObviousEnemyIsCurrentTop
					? ( fMostObviousEnemyDistanceFromSelf - fEnemyDistanceFromSelf ) > Tweaks->m_fDistanceDeltaForNewTopEnemy
					: fMostObviousEnemyDistanceFromSelf > fEnemyDistanceFromSelf;
				if( bMostObviousEnemyAttackingUs )
				{
					bHasHigherAttention = bEnemyAttackingUs && bCloser;
				}
				else
				{
					bHasHigherAttention = bEnemyAttackingUs || bCloser;
				}
			}
		}
		INT TargetType = DTATPT_DontCare;
		switch( Relationship )
		{
		case Relationship_Neutral: TargetType = bIsDead ? DTATPT_DeadNeutralOrEnemy : DTATPT_LiveNeutral; break;
		case Relationship_Ally: TargetType = bIsDead ? DTATPT_DeadAlly : DTATPT_LiveAlly; break;
		case Relationship_Enemy: TargetType = bIsDead ? DTATPT_DeadNeutralOrEnemy : DTATPT_LiveEnemy; break;
		default: TargetType = bIsDead ? DTATPT_DeadNeutralOrEnemy : DTATPT_DontCare; break;
		}
		if( TargetType == DTATPT_DontCare || !( TargetType > MostObviousTargetType || ( bHasHigherAttention && TargetType >= MostObviousTargetType ) ) )
		{
			continue;
		}
		MostObviousAttnIndex = ii;
		MostObviousTarget = Target;
		MostObviousTargetAttentionLevel = AttentionLevel;
		MostObviousTargetAttentionChangeReason = Info->GetLastChangeReason_Derived();
		MostObviousTargetRelationship = Relationship;
		bMostObviousTargetDead = bIsDead;
		MostObviousTargetType = TargetType;
		if( bIsAPotentialEnemy )
		{
			MostObviousEnemyIndex = ii;
			MostObviousEnemy = Target;
			MostObviousEnemyAttentionLevel = AttentionLevel;
			bMostObviousEnemyAttackingUs = bEnemyAttackingUs;
			fMostObviousEnemyDistanceFromSelf = fEnemyDistanceFromSelf;
			if( ii == 0 && EnemyPawn && Brain->GetTopEnemyProxy().IsEqualToActor( *EnemyPawn ) )
			{
				bMostObviousEnemyIsCurrentTop = TRUE;
			}
		}
	}
	FDisAttentionProxy TopProxy( EC_EventParm );
	if( MostObviousTarget )
	{
		TopProxy = FDisAttentionProxy( Brain, MostObviousTarget );
	}
	const UDisTweaks_PawnAttention* AttentionTweaks = NULL;
	if( TopProxy.IsValid() && TopProxy.GetProxyAttnTarget() )
	{
		AttentionTweaks = FindAttentionTweaks( TopProxy.GetProxyAttnTarget() );
	}
	Brain->SetTopAttnTarget( TopProxy, MostObviousTargetRelationship, bMostObviousTargetDead, MostObviousTargetAttentionLevel, MostObviousTargetAttentionChangeReason, AttentionTweaks );
	if( MostObviousTarget == MostObviousEnemy )
	{
		Brain->SetTopEnemyAttnTarget( TopProxy, MostObviousEnemyAttentionLevel );
	}
	else
	{
		FDisAttentionProxy EnemyProxy( EC_EventParm );
		if( MostObviousEnemy )
		{
			EnemyProxy = FDisAttentionProxy( Brain, MostObviousEnemy );
		}
		Brain->SetTopEnemyAttnTarget( EnemyProxy, MostObviousEnemyAttentionLevel );
	}
	if( MostObviousAttnIndex >= 0 && MostObviousAttnIndex < m_AttentionInfos.Num() )
	{
		m_AttentionInfos(MostObviousAttnIndex).m_pAttentionInfo->SetAsTopAttnTarget_Derived( Brain, TopProxy );
	}
	if( MostObviousEnemyIndex > 0 )
	{
		Exchange( m_AttentionInfos(0), m_AttentionInfos(MostObviousEnemyIndex) );
	}
}

// DISHONORED(port): 2012 rva 0x7a6c70 (880 B) / 2013 0x736680 (865 B): for every reaction of the tweaks not yet activated (or
// repeatable), the first target this tick whose level, our suspicion level, the relationship and the summed change rate of
// the allowed reasons all match is remembered as that reaction's context.
void UDisAIBrainProcessAttention::MarkActiveReactionContexts( FDisAttentionSlot& _rAttentionSlot, const DIS_MEMSTACK_ARRAY( FDisAppliedAttentionChangeInfo )& _rAttentionChangeInfos,
	DIS_MEMSTACK_ARRAY( IDisAttentionTargetInterface* )& _rOut_ActiveContexts )
{
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = GetAttentionProcessTweaks();
	const BYTE CurrentAttentionLevel = _rAttentionSlot.m_pAttentionInfo->GetAttnLevel_Derived();
	const BYTE OwnerSuspicionLevel = m_pOwningBrain->GetSuspicionLevel();
	const BYTE CurrentRelationship = _rAttentionSlot.m_pAttentionInfo->m_PreviousRelationship;
	const INT NumReactions = Tweaks->m_AttentionReactions.Num();
	if( m_ReactionAlreadyActivated.Num() < NumReactions )
	{
		m_ReactionAlreadyActivated.AddZeroed( NumReactions - m_ReactionAlreadyActivated.Num() );
	}
	for( INT ReactionIdx = 0; ReactionIdx < NumReactions && ReactionIdx < _rOut_ActiveContexts.Num(); ReactionIdx++ )
	{
		const FDisAttentionReaction& Reaction = Tweaks->m_AttentionReactions(ReactionIdx);
		if( !Reaction.m_bAllowRepeatReactions && m_ReactionAlreadyActivated(ReactionIdx) )
		{
			continue;
		}
		if( _rOut_ActiveContexts(ReactionIdx) )
		{
			continue;
		}
		const UBOOL bRightAttentionLevel = DisAttentionLevelEquals( CurrentAttentionLevel, Reaction.m_Context.m_AttentionLevel );
		const UBOOL bRightSuspicionLevel = Reaction.m_Context.m_SuspicionLevel == DAISL_MAX || Reaction.m_Context.m_SuspicionLevel == OwnerSuspicionLevel;
		const UBOOL bRightRelationship = Reaction.m_Context.m_Relationship == Relationship_MAX || Reaction.m_Context.m_Relationship == CurrentRelationship;
		UBOOL bAnyAllowedReason = FALSE;
		FLOAT fTotalChangeFromAllowedReasons = 0.f;
		for( INT ChangeIdx = 0; ChangeIdx < _rAttentionChangeInfos.Num(); ChangeIdx++ )
		{
			if( Reaction.m_Context.EvaluateAttnChangeReason( _rAttentionChangeInfos(ChangeIdx).m_ProxyInfoFromChange.m_ProxyUpdateReason ) )
			{
				fTotalChangeFromAllowedReasons += _rAttentionChangeInfos(ChangeIdx).m_fChangeAmount;
				bAnyAllowedReason = TRUE;
			}
		}
		const UBOOL bRightChangeRate = bAnyAllowedReason && fTotalChangeFromAllowedReasons >= Reaction.m_Context.m_fChangeRate;
		if( bRightAttentionLevel && bRightSuspicionLevel && bRightRelationship && bRightChangeRate )
		{
			m_ReactionAlreadyActivated(ReactionIdx) = 1;
			_rOut_ActiveContexts(ReactionIdx) = DisAttnTargetOf( _rAttentionSlot.m_pAttentionTarget );
		}
	}
}

// DISHONORED(port): 2012 rva 0x7a6fe0 (508 B) / 2013 0x7369f0 (521 B): a reaction whose context just became active fires once.
void UDisAIBrainProcessAttention::CheckAttentionReactions( const DIS_MEMSTACK_ARRAY( IDisAttentionTargetInterface* )& _rFirstTargetsWithActiveContext )
{
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = GetAttentionProcessTweaks();
	const INT NumReactions = Tweaks->m_AttentionReactions.Num();
	if( m_ActiveAttentionReactionContexts.Num() < NumReactions )
	{
		m_ActiveAttentionReactionContexts.AddZeroed( NumReactions - m_ActiveAttentionReactionContexts.Num() );
	}
	for( INT i = 0; i < NumReactions && i < _rFirstTargetsWithActiveContext.Num(); i++ )
	{
		const UBOOL bIsActive = _rFirstTargetsWithActiveContext(i) != NULL;
		if( !m_ActiveAttentionReactionContexts(i) && bIsActive )
		{
			DoAttentionReactions( _rFirstTargetsWithActiveContext(i), Tweaks->m_AttentionReactions(i) );
		}
		m_ActiveAttentionReactionContexts(i) = bIsActive ? 1 : 0;
	}
}

// DISHONORED(port): 2012 rva 0x7931c0 / 2013 0x730c70 (98 B)
void UDisAIBrainProcessAttention::DoAttentionReactions( IDisAttentionTargetInterface* _pTargetThatCausedReaction, const FDisAttentionReaction& _rReaction )
{
	for( INT i = 0; i < _rReaction.m_EnterAction.m_Reactions.Num(); i++ )
	{
		m_pOwningBrain->DoOneAttentionReaction( _pTargetThatCausedReaction, _rReaction.m_EnterAction.m_Reactions(i) );
	}
}

/*-----------------------------------------------------------------------------
	The increases and the filters.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7a7e30 (109 B) / 2013 0x73f270 (65 B, the shape here: the brain's IsBlindTo). 2012 additionally
// marked a witnessed death on the info (m_WitnessDeathState 2 unless handled) when the increase said so; 2013 dropped it.
void UDisAIBrainProcessAttention::BumpAttention( IDisAttentionTargetInterface* _pForTarget, const FDisAttentionIncrease& _rIncrease, const FDisAttentionChangeReason& _rReason, const FDisAttentionIncreaseInfo& _rIncreaseInfo )
{
	if( !_pForTarget || m_pOwningBrain->IsBlindTo( _pForTarget->IsAttnTargetPlayer() ) )
	{
		return;
	}
	UBOOL bWitnessedDeath = FALSE;
	GetAttentionInfo( _pForTarget ).IncreaseAttention_Derived( _rIncrease, _rReason, _rIncreaseInfo, bWitnessedDeath );
}

// DISHONORED(port): 2012 rva 0x7a7ea0 / 2013 0x73f3f0 (46 / 41 B)
UBOOL UDisAIBrainProcessAttention::FilterTargetSighted( const FAIStimStruct_TargetSighted& _rStim )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rStim.m_pTarget );
	if( Target )
	{
		GetAttentionInfo( Target ).GainLOS_Derived();
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7989d0 / 2013 0x7365b0 (52 / 47 B)
UBOOL UDisAIBrainProcessAttention::FilterTargetUnsighted( const FAIStimStruct_TargetUnsighted& _rStim )
{
	UDisAttentionInfo_Base* Info = FindAttentionInfo( DisAttnTargetOf( _rStim.m_pTarget ), NULL );
	if( Info )
	{
		Info->LoseLOS_Derived();
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a7ed0 (67 B) / 2013 0x73f420 (249 B, the shape here): a live touched target is marked touched;
// 2013 adds the unpossess touch, which maxes the attention at once with a Touched reason (unless blind to that target).
UBOOL UDisAIBrainProcessAttention::FilterTargetTouched( const FAIStimStruct_TargetTouched& _rStim )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rStim.m_pTarget );
	if( Target && !Target->IsAttnTargetDead() )
	{
		GetAttentionInfo( Target ).BeTouched_Derived();
		if( _rStim.m_bUnpossessTouch && !m_pOwningBrain->IsBlindTo( Target->IsAttnTargetPlayer() ) )
		{
			UBOOL bWitnessedDeath = FALSE;
			GetAttentionInfo( Target ).IncreaseAttention_Derived( FDisAttentionIncrease( 0.f, DALFI_MaxAttention ), FDisAttentionChangeReason( DACRT_Touched ), FDisAttentionIncreaseInfo( Target->GetAttnTargetFocalPoint() ), bWitnessedDeath );
		}
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a7f20 / 2013 0x73f7b0-neighbourhood (0x7a7f20 -> see the match table; 336 B): damage from an
// attention target that this brain is neither blind nor numb to raises AttackedBy (or AttackedBy_Indirect with the causer's
// location). Not consumed: the behaviours see the damage too.
UBOOL UDisAIBrainProcessAttention::FilterIncomingDamage( const FAIStimStruct_IncomingDamage& _rStim )
{
	if( !_rStim.m_pAttacker )
	{
		return FALSE;
	}
	IDisAttentionTargetInterface* Attacker = (IDisAttentionTargetInterface*)_rStim.m_pAttacker->GetInterfaceAddress( UDisAttentionTargetInterface::StaticClass() );
	if( !Attacker )
	{
		return FALSE;
	}
	const INT Mask = Attacker->IsAttnTargetPlayer() ? m_pOwningBrain->m_FinalSenseMask_Player : m_pOwningBrain->m_FinalSenseMask_NonPlayer;
	if( Mask & 9 )
	{
		return FALSE;
	}
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Attacker );
	if( Tweaks )
	{
		const UBOOL bIndirect = _rStim.m_bHasDamageCauserLoc;
		FDisAttentionIncreaseInfo Info( bIndirect ? _rStim.m_DamageCauserLoc : Attacker->GetAttnTargetFocalPoint() );
		BumpAttention( Attacker, Tweaks->m_IncreaseForAttackedBy, FDisAttentionChangeReason( bIndirect ? DACRT_AttackedBy_Indirect : DACRT_AttackedBy ), Info );
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7a8070 / 2013 0x741650-bound (192 B)
UBOOL UDisAIBrainProcessAttention::FilterMaxOutAttention( const FAIStimStruct_MaxOutAttention& _rStim )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rStim.m_pForTarget );
	if( Target )
	{
		BumpAttention( Target, FDisAttentionIncrease( 0.f, DALFI_MaxAttention ), FDisAttentionChangeReason( DACRT_Psychic ), FDisAttentionIncreaseInfo( Target->GetAttnTargetFocalPoint() ) );
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a8130 (96 B), 0x798a10 (96 B), 0x793090 (160 B): the psychic flags are one bit per
// EDisMinAttentionLevelType; the first set bit gains psychic attention, the last cleared bit loses it.
UBOOL UDisAIBrainProcessAttention::FilterPsychicAttentionEnabled( const FAIStimStruct_PsychicAttentionEnabled& _rStim )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rStim.m_pForTarget );
	if( Target )
	{
		UDisAttentionInfo_Base& Info = GetAttentionInfo( Target );
		const BYTE OldFlags = Info.m_bPsychicFlags;
		Info.m_bPsychicFlags = OldFlags | ( 1 << _rStim.m_Type );
		if( !OldFlags && Info.m_bPsychicFlags )
		{
			Info.GainPsychicAttention_Derived();
		}
	}
	return TRUE;
}

UBOOL UDisAIBrainProcessAttention::FilterPsychicAttentionDisabled( const FAIStimStruct_PsychicAttentionDisabled& _rStim )
{
	UDisAttentionInfo_Base* Info = FindAttentionInfo( DisAttnTargetOf( _rStim.m_pForTarget ), NULL );
	if( Info )
	{
		const BYTE OldFlags = Info->m_bPsychicFlags;
		Info->m_bPsychicFlags = OldFlags & ~( 1 << _rStim.m_Type );
		if( OldFlags && !Info->m_bPsychicFlags )
		{
			Info->LosePsychicAttention_Derived();
		}
	}
	return TRUE;
}

UBOOL UDisAIBrainProcessAttention::FilterPsychicAttentionDisabled_AllTargets( const FAIStimStruct_PsychicAttentionDisabled_AllTargets& _rStim )
{
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		UDisAttentionInfo_Base* Info = m_AttentionInfos(i).m_pAttentionInfo;
		if( !Info )
		{
			continue;
		}
		const BYTE OldFlags = Info->m_bPsychicFlags;
		Info->m_bPsychicFlags = OldFlags & ~( 1 << _rStim.m_Type );
		if( OldFlags && !Info->m_bPsychicFlags )
		{
			Info->LosePsychicAttention_Derived();
		}
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a8190 (64 B) / 2013 0x73f7b0 (the table's "FilterTargetTouched" row, 55 B), 0x798a70 (64 B),
// 0x798ab0 (26 B)
UBOOL UDisAIBrainProcessAttention::FilterSetMinAttentionForTarget( const FAIStimStruct_SetMinAttentionForTarget& _rStim )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rStim.m_pForTarget );
	if( Target )
	{
		GetAttentionInfo( Target ).SetMinAttentionLevel_Derived( _rStim.m_Type, _rStim.m_MinAttentionLevel );
	}
	return TRUE;
}

UBOOL UDisAIBrainProcessAttention::FilterClearMinAttentionForTarget( const FAIStimStruct_ClearMinAttentionForTarget& _rStim )
{
	UDisAttentionInfo_Base* Info = FindAttentionInfo( DisAttnTargetOf( _rStim.m_pForTarget ), NULL );
	if( Info )
	{
		Info->SetMinAttentionLevel_Derived( _rStim.m_Type, DAL_Unaware );
	}
	return TRUE;
}

UBOOL UDisAIBrainProcessAttention::FilterClearAllMinAttention( const FAIStimStruct_ClearAllMinAttention& _rStim )
{
	ClearAllMinAttention( _rStim.m_Type );
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x792fe0 (166 B): the clear timer of one slot, or of every slot when the stim names no target; a
// duration under a millisecond still clears for one tick.
UBOOL UDisAIBrainProcessAttention::FilterClearAttention( const FAIStimStruct_ClearAttention& _rStim )
{
	const IDisAttentionTargetInterface* ForTarget = DisAttnTargetOf( _rStim.m_pForTarget );
	for( INT i = 0; i < m_AttentionInfos.Num(); i++ )
	{
		if( !ForTarget || DisAttnTargetOf( m_AttentionInfos(i).m_pAttentionTarget ) == ForTarget )
		{
			if( m_AttentionInfos(i).m_pAttentionInfo )
			{
				m_AttentionInfos(i).m_pAttentionInfo->m_fClearTimer = Max( 0.001f, _rStim.m_fForHowLong );
			}
		}
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x78ff40 (76 B): an aural request is heard unless deaf to non-players, or deaf to the player when
// the request is about the player.
UBOOL UDisAIBrainProcessAttention::CanHearAuralRequest( const TScriptInterface<IDisAttentionTargetInterface>& _pRequestTarget ) const
{
	if( m_pOwningBrain->m_FinalSenseMask_NonPlayer & 3 )
	{
		return FALSE;
	}
	const IDisAttentionTargetInterface* Target = DisAttnTargetOf( _pRequestTarget );
	return !Target || !Target->IsAttnTargetPlayer() || !( m_pOwningBrain->m_FinalSenseMask_Player & 3 );
}

// DISHONORED(port): 2012 rva 0x7a81d0 (208 B): an ally's notice request about a target this brain can hear.
UBOOL UDisAIBrainProcessAttention::FilterNoticeRequest( const FAIStimStruct_NoticeRequest& _rStim )
{
	if( !_rStim.m_pNoticer || !CanHearAuralRequest( _rStim.m_pNoticedTarget ) )
	{
		return TRUE;
	}
	if( m_pOwningPawn->DetermineRelationship( *static_cast<const IDisRelationshipInterface*>( _rStim.m_pNoticer ) ) != Relationship_Ally )
	{
		return TRUE;
	}
	IDisAttentionTargetInterface* Noticed = DisAttnTargetOf( _rStim.m_pNoticedTarget );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Noticed );
	if( Noticed && Tweaks )
	{
		FDisAttentionChangeReason Reason( DACRT_HeardNoticeRequest );
		Reason.m_ReasonInfo.m_pActor = Noticed->GetAttnTargetActor();
		BumpAttention( Noticed, Tweaks->m_IncreaseForNoticeRequest, Reason, _rStim.m_IncreaseInfo );
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a9bd0 / 2013 0x7438c0 (119 / 122 B) and HandleSearchRequest 0x7a8f70 (128 B). The 2012 test
// `m_ActorTypeFlags != 36` is "the trouble maker is not the player"; it is read as IsAttnTargetPlayer here.
UBOOL UDisAIBrainProcessAttention::FilterSearchRequest( const FAIStimStruct_SearchRequest& _rStim )
{
	ADishonoredNPCPawn* SearchingAlly = _rStim.m_pSearchingAlly;
	ADishonoredPawn* TroubleMaker = _rStim.m_pTroubleMaker;
	if( !SearchingAlly )
	{
		return TRUE;
	}
	if( m_pOwningBrain->m_FinalSenseMask_NonPlayer & 3 )
	{
		return TRUE;
	}
	if( TroubleMaker && TroubleMaker->IsAttnTargetPlayer() && ( m_pOwningBrain->m_FinalSenseMask_Player & 3 ) )
	{
		return TRUE;
	}
	if( m_pOwningPawn->DetermineRelationship( *static_cast<const IDisRelationshipInterface*>( SearchingAlly ) ) == Relationship_Ally )
	{
		HandleSearchRequest( TroubleMaker, _rStim.m_SearchRequestCause, _rStim.m_SearchRequesterTransitSpeed, _rStim.m_ProxyInfo );
	}
	return TRUE;
}

void UDisAIBrainProcessAttention::HandleSearchRequest( ADishonoredPawn* _pSearchTarget, BYTE _SearchRequestCause, BYTE _SearchRequesterTransitSpeed, const FDisAttentionIncreaseInfo& _rIncreaseInfo )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _pSearchTarget );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Target );
	if( !Target || !Tweaks )
	{
		return;
	}
	FDisAttentionChangeReason Reason( DACRT_HeardSearchRequest );
	Reason.m_ReasonInfo.m_iParam1 = _SearchRequestCause;
	Reason.m_ReasonInfo.m_iParam2 = _SearchRequesterTransitSpeed;
	Reason.m_ReasonInfo.m_pActor = _pSearchTarget;
	BumpAttention( Target, Tweaks->m_IncreaseForSearchRequest, Reason, _rIncreaseInfo );
}

// DISHONORED(port): 2012 rva 0x7a9c50 (83 B) and FilterHelpRequest 0x7a8bb0 / 2013 0x740640 (194 / 245 B): a help call about
// someone else, heard, from a requester whose trouble this brain cares about, makes a neutral trouble maker an enemy and
// raises the help-sound increase. Consumed only when the request was acted on... retail returns FALSE when it bumped and
// TRUE when ShouldCareAboutThisCrime said no.
UBOOL UDisAIBrainProcessAttention::FilterHelp( const FAIStimStruct_Help& _rStim )
{
	if( _rStim.m_pTroubleMaker == m_pOwningPawn || _rStim.m_pNPCInTrouble == m_pOwningPawn )
	{
		return FALSE;
	}
	if( m_pOwningBrain->m_FinalSenseMask_NonPlayer & 3 )
	{
		return FALSE;
	}
	if( _rStim.m_pTroubleMaker && _rStim.m_pTroubleMaker->IsAttnTargetPlayer() && ( m_pOwningBrain->m_FinalSenseMask_Player & 3 ) )
	{
		return FALSE;
	}
	return FilterHelpRequest( _rStim.m_pNPCInTrouble, _rStim.m_pTroubleMaker, _rStim.m_ProxyInfo );
}

UBOOL UDisAIBrainProcessAttention::FilterHelpRequest( ADishonoredNPCPawn* _pRequester, ADishonoredPawn* _pTroublemaker, const FDisAttentionIncreaseInfo& _rIncreaseInfo )
{
	if( !_pTroublemaker )
	{
		return FALSE;
	}
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _pTroublemaker );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Target );
	if( !Tweaks )
	{
		return FALSE;
	}
	const IDisRelationshipInterface* TroubleRelator = static_cast<const IDisRelationshipInterface*>( _pTroublemaker );
	const IDisRelationshipInterface* RequesterRelator = _pRequester ? static_cast<const IDisRelationshipInterface*>( _pRequester ) : NULL;
	if( !m_pOwningBrain->ShouldCareAboutThisCrime( RequesterRelator, TroubleRelator ) )
	{
		return TRUE;
	}
	if( m_pOwningPawn->DetermineRelationship( *TroubleRelator ) == Relationship_Neutral )
	{
		m_pOwningPawn->OverrideRelationToIndividual( *TroubleRelator, Relationship_Enemy, TRUE );
	}
	FDisAttentionChangeReason Reason( DACRT_HeardHelpRequest );
	Reason.m_ReasonInfo.m_pActor = _pRequester;
	BumpAttention( Target, Tweaks->m_IncreaseForHelpSound, Reason, _rIncreaseInfo );
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x78d9b0 (16 B): the tweaks of every info are re-read next tick.
UBOOL UDisAIBrainProcessAttention::FilterSuspicionLevelChanged( const FAIStimStruct_SuspicionLevelChanged& _rStim )
{
	m_bAttentionInfosTweaksUpdated = 1;
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7a8820 (224 B): a conversation raising attention to a threshold, TalkedTo.
UBOOL UDisAIBrainProcessAttention::FilterDialogAttentionChange( const FAIStimStruct_DialogAttentionChange& _rStim )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _rStim.m_pTarget );
	if( Target )
	{
		BumpAttention( Target, FDisAttentionIncrease( 0.f, _rStim.m_ToThreshold ), FDisAttentionChangeReason( DACRT_TalkedTo ), FDisAttentionIncreaseInfo( Target->GetAttnTargetFocalPoint() ) );
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a8af0 (192 B): the player travelling (a Blink) maxes the attention at where he was, Psychic.
UBOOL UDisAIBrainProcessAttention::FilterTraveled( const FAIStimStruct_Traveled& _rStim )
{
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	if( Player )
	{
		BumpAttention( DisAttnTargetOf( Player ), FDisAttentionIncrease( 0.f, DALFI_MaxAttention ), FDisAttentionChangeReason( DACRT_Psychic ), FDisAttentionIncreaseInfo( Player->m_PreviousTravelLocation ) );
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a9cb0 / 2013 0x7439a0 (26 B)
UBOOL UDisAIBrainProcessAttention::FilterHeardSomething( const FAIStimStruct_HeardSomething& _rStim )
{
	if( _rStim.m_NoiseParams.m_pNoiseMakerPawn )
	{
		HandleHearSomething( _rStim.m_NoiseParams );
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7a8ff0 (942 B, 2013 unmatched by the table: the callee of FilterHeardSomething 0x7439a0): the
// noise context picks the increase (anomaly, threatening, danger, death rattle), a footstep (context 2) is heard at the
// maker's look-at height with his velocity, a Busted maker is heard at his look-at point, and a death rattle whose noted actor
// is a pawn also bumps that pawn (the killer) when the tweaks say so. A player noise is noted for the PlayerHeard Kismet event.
// The noise contexts are this tree's EDisNoiseContext ordinals as retail switches on them (1, 2, 3, 9, 4, 7, 14, 11).
void UDisAIBrainProcessAttention::HandleHearSomething( const FDisAINoiseParameters& _rNoiseParams )
{
	ADishonoredPawn* NoiseMaker = _rNoiseParams.m_pNoiseMakerPawn;
	if( !NoiseMaker )
	{
		return;
	}
	IDisAttentionTargetInterface* MakerTarget = DisAttnTargetOf( NoiseMaker );
	const INT Mask = MakerTarget->IsAttnTargetPlayer() ? m_pOwningBrain->m_FinalSenseMask_Player : m_pOwningBrain->m_FinalSenseMask_NonPlayer;
	if( Mask & 3 )
	{
		return;
	}
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( MakerTarget );
	if( !Tweaks )
	{
		return;
	}
	FDisAttentionIncrease Increase( 0.f, 0 );
	FVector AttentionLoc = _rNoiseParams.m_NotedPosition;
	FVector AttentionVel( 0.f, 0.f, 0.f );
	FLOAT fAttentionHeight = 3.4028235e38f;
	BYTE ReasonType = DACRT_Heard;
	switch( _rNoiseParams.m_NoiseContext )
	{
	case 1:
		Increase = Tweaks->m_IncreaseForAnomalySound;
		break;
	case 2:
		{
			Increase = Tweaks->m_IncreaseForAnomalySound;
			const FLOAT fLookAtZ = NoiseMaker->GetLookAtPoint().Z;
			fAttentionHeight = fLookAtZ - DisGetPawnFeet( NoiseMaker ).Z;
			AttentionLoc.Z += fAttentionHeight;
			AttentionVel = NoiseMaker->Velocity;
		}
		break;
	case 3:
	case 9:
		Increase = Tweaks->m_IncreaseForThreateningSound;
		break;
	case 4:
	case 7:
	case 14:
		Increase = Tweaks->m_IncreaseForDangerSound;
		break;
	case 11:
		Increase = Tweaks->m_IncreaseForDeathRattleSound;
		ReasonType = DACRT_HeardDeath;
		if( Tweaks->m_bDeathRattleGivesAttentionForKiller && !ShouldIgnoreDeathOf( MakerTarget, NULL ) )
		{
			ADishonoredPawn* Killer = Cast<ADishonoredPawn>( _rNoiseParams.m_pNotedActor );
			if( Killer )
			{
				BumpAttention( DisAttnTargetOf( Killer ), Increase, DisAttention::MakeAttentionChangeReason( DACRT_HeardDeath, NoiseMaker, _rNoiseParams.m_NoiseContext, 0 ), FDisAttentionIncreaseInfo( AttentionLoc ) );
			}
		}
		break;
	default:
		break;
	}
	if( GetAttentionLevel( MakerTarget ) >= DAL_Busted )
	{
		AttentionLoc = NoiseMaker->GetLookAtPoint();
		AttentionVel = NoiseMaker->Velocity;
		fAttentionHeight = Max( 0.f, AttentionLoc.Z - DisGetPawnFeet( NoiseMaker ).Z );
	}
	FDisAttentionIncreaseInfo Info( AttentionLoc );
	Info.m_Vel = AttentionVel;
	Info.m_fHeight = fAttentionHeight;
	FDisAttentionChangeReason Reason( (EDisAttentionChangeReasonType)ReasonType );
	Reason.m_ReasonInfo.m_iParam1 = _rNoiseParams.m_NoiseContext;
	BumpAttention( MakerTarget, Increase, Reason, Info );
	if( NoiseMaker->IsA( ADishonoredPlayerPawn::StaticClass() ) )
	{
		m_pOwningBrain->HandleHearSomething( _rNoiseParams.m_NoiseCauserName );
	}
}

// DISHONORED(port): 2012 rva 0x7a9db0 (26 B), 0x7a9dd0 (26 B), HandleAttackedEnemy 0x7a8e90 (218 B): hitting an enemy, or an
// enemy blocking our swing, raises AttackedEnemy on him.
UBOOL UDisAIBrainProcessAttention::FilterMeleeConnected_Outgoing( const FAIStimStruct_MeleeConnected_Outgoing& _rStim )
{
	HandleAttackedEnemy( _rStim.m_pEnemy );
	return FALSE;
}

// DISHONORED(bringup): UDisItemContext::GetOwningPawn (the item context layer) is not ported; the blocker's pawn is read
// off the block context's Outer chain, which is where retail's accessor finds it.
static ADishonoredPawn* DisItemContextOwningPawn( UObject* _pContext )
{
	for( UObject* Outer = _pContext ? _pContext->GetOuter() : NULL; Outer; Outer = Outer->GetOuter() )
	{
		if( ADishonoredPawn* Pawn = Cast<ADishonoredPawn>( Outer ) )
		{
			return Pawn;
		}
	}
	return NULL;
}

UBOOL UDisAIBrainProcessAttention::FilterEvadedMelee_Outgoing( const FAIStimStruct_EvadedMelee_Outgoing& _rStim )
{
	HandleAttackedEnemy( DisItemContextOwningPawn( _rStim.m_pEnemyBlockContext ) );
	return FALSE;
}

void UDisAIBrainProcessAttention::HandleAttackedEnemy( ADishonoredPawn* _pEnemy )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _pEnemy );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Target );
	if( !Target || !Tweaks )
	{
		return;
	}
	FDisAttentionChangeReason Reason( DACRT_AttackedEnemy );
	Reason.m_ReasonInfo.m_pActor = _pEnemy;
	BumpAttention( Target, Tweaks->m_IncreaseForAttackedEnemy, Reason, FDisAttentionIncreaseInfo( Target->GetAttnTargetFocalPoint() ) );
}

// DISHONORED(port): 2012 rva 0x7a82a0 (222 B): dodging an attack raises AttackedBy on the attacker.
UBOOL UDisAIBrainProcessAttention::FilterEvadedMelee_Incoming( const FAIStimStruct_EvadedMelee_Incoming& _rStim )
{
	IDisAttentionTargetInterface* Attacker = DisAttnTargetOf( DisItemContextOwningPawn( _rStim.m_pMeleeAttackContext ) );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Attacker );
	if( Attacker && Tweaks )
	{
		BumpAttention( Attacker, Tweaks->m_IncreaseForAttackedBy, FDisAttentionChangeReason( DACRT_AttackedBy ), FDisAttentionIncreaseInfo( Attacker->GetAttnTargetFocalPoint() ) );
	}
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7a8380 (272 B): a forced alarm escalates the suspicion and investigates the enemy (or the
// override location), Magic.
UBOOL UDisAIBrainProcessAttention::FilterForceRingAlarm( const FAIStimStruct_ForceRingAlarm& _rStim )
{
	m_pOwningBrain->EscalateSuspicionLevel();
	IDisAttentionTargetInterface* Enemy = DisAttnTargetOf( _rStim.m_pEnemy );
	if( !Enemy )
	{
		return FALSE;
	}
	const FVector AttentionLocation = _rStim.m_bUseEnemyLocOverride ? _rStim.m_EnemyLocOverride : Enemy->GetAttnTargetFocalPoint();
	FDisAttentionChangeReason Reason( DACRT_Magic );
	Reason.m_ReasonInfo.m_pActor = _rStim.m_pEnemy;
	BumpAttention( Enemy, FDisAttentionIncrease( 0.f, DALFI_Investigate ), Reason, FDisAttentionIncreaseInfo( AttentionLocation ) );
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x7a8490 (222 B): the choker (or the possessor, the same body) gets Busted-level attention, AttackedBy.
UBOOL UDisAIBrainProcessAttention::FilterEscapedBeingChoked( const FAIStimStruct_EscapedBeingChoked& _rStim )
{
	IDisAttentionTargetInterface* Choker = DisAttnTargetOf( _rStim.m_pChoker );
	if( Choker )
	{
		BumpAttention( Choker, FDisAttentionIncrease( 0.f, DALFI_Busted ), FDisAttentionChangeReason( DACRT_AttackedBy ), FDisAttentionIncreaseInfo( Choker->GetAttnTargetFocalPoint() ) );
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a8900 (208 B): a wary NPC seeing the player use a door investigates the player, Seen.
UBOOL UDisAIBrainProcessAttention::FilterDoorUsedByPlayerWhileWary( const FAIStimStruct_DoorUsedByPlayerWhileWary& _rStim )
{
	ADishonoredPlayerPawn* Player = ADishonoredPlayerPawn::s_pInstance;
	if( Player )
	{
		BumpAttention( DisAttnTargetOf( Player ), FDisAttentionIncrease( 0.f, DALFI_Investigate ), FDisAttentionChangeReason( DACRT_Seen ), FDisAttentionIncreaseInfo( Player->GetAttnTargetFocalPoint() ) );
	}
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a9df0 (64 B) and HandleDeathByWoL 0x7a9490 (192 B): a pawn seen dying in a wall of light is
// noticed at the wall's location, Seen.
UBOOL UDisAIBrainProcessAttention::FilterDeathByWoL( const FAIStimStruct_DeathByWoL& _rStim )
{
	if( _rStim.m_pWoL )
	{
		HandleDeathByWoL( _rStim.m_pWoL->Location, _rStim.m_pDeadPawn );
	}
	return FALSE;
}

void UDisAIBrainProcessAttention::HandleDeathByWoL( FVector _Location, ADishonoredPawn* _pSeenPawn )
{
	IDisAttentionTargetInterface* Target = DisAttnTargetOf( _pSeenPawn );
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( Target );
	if( Target && Tweaks )
	{
		BumpAttention( Target, Tweaks->m_IncreaseForDeathByWoL, FDisAttentionChangeReason( DACRT_Seen ), FDisAttentionIncreaseInfo( _Location ) );
	}
}

// DISHONORED(port): 2012 rva 0x7a9cd0 / 2013 0x7439c0 (41 B) and HandleHearAlarm 0x7a93a0 / 0x740f60 (234 / 302 B).
// DISHONORED(bringup): ADisAlarmBell::GetPreviousValidNPCPosition (the bell's last known position of the trouble maker) is
// not ported; the bell's own location stands in for it, named here.
UBOOL UDisAIBrainProcessAttention::FilterAlarm( const FAIStimStruct_Alarm& _rStim )
{
	if( _rStim.m_pAlarm )
	{
		HandleHearAlarm( DisAttnTargetOf( _rStim.m_pTroubleMaker ), *_rStim.m_pAlarm, _rStim.m_AttentionTag );
	}
	return FALSE;
}

void UDisAIBrainProcessAttention::HandleHearAlarm( IDisAttentionTargetInterface* _pTroubleMaker, ADisAlarmBell& _rAlarm, INT _AttentionTag )
{
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( _pTroubleMaker );
	if( !_pTroubleMaker || !Tweaks )
	{
		return;
	}
	const INT Mask = _pTroubleMaker->IsAttnTargetPlayer() ? m_pOwningBrain->m_FinalSenseMask_Player : m_pOwningBrain->m_FinalSenseMask_NonPlayer;
	if( Mask & 3 )
	{
		return;
	}
	FDisAttentionIncreaseInfo Info( _rAlarm.Location );
	Info.m_AttentionTag = _AttentionTag;
	FDisAttentionChangeReason Reason( DACRT_Alarm );
	Reason.m_ReasonInfo.m_pActor = &_rAlarm;
	Reason.m_ReasonInfo.m_iParam1 = 6;
	BumpAttention( _pTroubleMaker, Tweaks->m_IncreaseForAlarmSound, Reason, Info );
}

// DISHONORED(port): 2012 rva 0x7a9550 (200 B) / 2013 0x741190 (268 B)
UBOOL UDisAIBrainProcessAttention::HandleWitnessedMagic( IDisAttentionTargetInterface* _pMagicUserTarget )
{
	const UDisTweaks_PawnAttention* Tweaks = FindAttentionTweaks( _pMagicUserTarget );
	if( !_pMagicUserTarget || !Tweaks )
	{
		return FALSE;
	}
	BumpAttention( _pMagicUserTarget, Tweaks->m_IncreaseForMagic, FDisAttentionChangeReason( DACRT_Magic ), FDisAttentionIncreaseInfo( _pMagicUserTarget->GetAttnTargetFocalPoint() ) );
	return TRUE;
}

// DISHONORED(port): 2012 rva 0x7a9620 (252 B) / 2013 0x7412a0 (260 B): another brain's attention on a target becomes ours at the
// matching threshold, as a heard (noise context 1; a Busted source says 4) increase.
void UDisAIBrainProcessAttention::TransferAttentionForTarget( UDishonoredAIBrain* _pSourceBrain, IDisAttentionTargetInterface* const _pTarget )
{
	if( !_pSourceBrain || !_pTarget )
	{
		return;
	}
	FDisAttentionIncrease Increase( 0.f, DALFI_None );
	FDisAttentionChangeReason Reason( DACRT_Heard );
	Reason.m_ReasonInfo.m_iParam1 = 1;
	switch( _pSourceBrain->GetAttentionLevel( _pTarget ) )
	{
	case DAL_HeadTrack: Increase.m_ToThreshold = DALFI_HeadTrack; break;
	case DAL_TurnToFace: Increase.m_ToThreshold = DALFI_TurnToFace; break;
	case DAL_Investigate: Increase.m_ToThreshold = DALFI_Investigate; break;
	case DAL_Busted: Increase.m_ToThreshold = DALFI_MaxAttention; Reason.m_ReasonInfo.m_iParam1 = 4; break;
	default: break;
	}
	FDisAttentionProxyInfo Info( EC_EventParm );
	_pSourceBrain->GetAttentionProxyInfo( _pTarget, Info );
	BumpAttention( _pTarget, Increase, Reason, FDisAttentionIncreaseInfo( Info ) );
}

void UDisAIBrainProcessAttention::FinishDestroy()
{
	if( DisGIDiagEnabled() )
	{
		debugf( TEXT("gidiag destroy %s at 0x%08x outer %s"), *GetFullName(), (DWORD)(PTRINT)this, GetOuter() ? *GetOuter()->GetName() : TEXT("none") );
	}
	Super::FinishDestroy();
}
