// DishonoredGame/src/disattentioninfo_complex.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (45): the forty below (GameSave is agent ED's, in dissavegame.cpp, GameLoad agent EJ's)
// plus the three class-registration helpers and the constructor.
//
// ---- agent GI ports (PHASE20 GI): UDisAttentionInfo_Complex, the time-based attention meter the player gets ----
// Resolved by hand in both databases (build/agentGI/idb; decompiles in build/agentGI/decomp2012, the 2013 bodies through the
// idalib session). The flags dword at +1296 (2012 and 2013) is this tree's bitfield: m_bHasLOS 1, m_bIsSighted 2, m_bIsTouched 4,
// m_bIsPsychic 8, m_bIsWithinHideout 16, m_bProxyUpdated 32, m_bDebug_FinalVisLevelIsClamped 64.
//
// The meter: every tick SetupTickParamsFromTarget gathers the target (location, focal point, velocity, the observable's
// visibility and spotted distance, whether it allows gain) and the two tweak snapshots (refreshed when dirty); the sight,
// touch and psychic branches write pending changes into m_PendingAttentionChangeAmount[reason] alongside the instantaneous
// increases BumpAttention wrote; ApplyAttentionIncreases pours them into the leaky accumulator (raising it to each
// increase's threshold), ApplyAttentionDrain lets it leak, UpdateAttentionInfo turns the value into a level against the start
// thresholds going up and this brain's rolled stop thresholds going down (clamped to Investigate when a Busted cannot be
// reached without sight), and UpdateAttentionProxy moves the proxy - where this brain believes the target is - to the
// location of the strongest change, or on to the target itself while attached.

#include "DishonoredGame.h"
#include "aistimstruct.h"
#include "disattentioninfo.h"
#include "dishonoredutilities.h"

void DishonoredLinkUDisAttentionInfo_Complex()
{
}

static const UDisTweaks_PawnAttention* DisPawnAttentionTweaksOf( const UDisAttentionInfo_Complex* _pInfo )
{
	const UDisTweaks_PawnAttention* Tweaks = Cast<UDisTweaks_PawnAttention>( _pInfo->GetTweaks() );
	return Tweaks ? Tweaks : UDisTweaks_PawnAttention::StaticClass()->GetDefaultObject<UDisTweaks_PawnAttention>();
}

static void DisResetPendingChange( FDisAttentionPendingChange& _rChange )
{
	_rChange.m_PendingChangeInfo.m_Loc = FVector( 0.f, 0.f, 0.f );
	_rChange.m_PendingChangeInfo.m_Vel = FVector( 0.f, 0.f, 0.f );
	_rChange.m_PendingChangeInfo.m_fHeight = 3.4028235e38f;
	_rChange.m_PendingChangeInfo.m_AttentionTag = DisAttention::NewInformationAttentionTag;
	appMemzero( &_rChange.m_PendingReasonInfo, sizeof(_rChange.m_PendingReasonInfo) );
}

// DISHONORED(port): 2012 rva 0x7a2120 (797 B) / 2013 0x768e10 (821 B): one tick's view of the target. The attention-target tweak
// snapshot is refreshed when dirty from the target's UDisTweaks_AttentionTarget (the class default when it has none); the
// pawn-attention snapshot when dirty from the complex tweaks, with the vision increase scalar picked for the player (its
// FDisAttribute at the current difficulty), a dead NPC or a live NPC. The observable component supplies the visibility level
// and the spotted distance from this brain's vision component; the sense flags fold the brain's final sense mask.
// DISHONORED(bringup): FDisComponentObservable / FDisComponentVisionNPC are not ported, so m_bAttnTargetHasObservableComponent
// is FALSE for every target (the sight branch stays dark) - unless the -gisight STAND-IN is on, which then supplies a lit
// visibility of 1 and the normalised distance for the player alone, and says so in the log.
namespace DisAttentionInfo_Complex
{
	void SetupTickParamsFromTarget( IDisAttentionTargetInterface* const _pAttentionTarget, const UDisTweaks_PawnAttention* const _pAttnTweaks, const UDisAIBrainProcessAttention* const _pAttnProcess,
		const UDishonoredAIBrain* const _pAttentiveBrain, FDisTickAttentionParams& _rOut_TickParams, FDisTickAttentionParams_PawnAttentionTweaks& _rOut_TickParams_PawnAttention,
		FDisTickAttentionParams_AttentionTargetTweaks& _rOut_TickParams_AttentionTarget )
	{
		const ADishonoredNPCPawn* OwningPawn = _pAttentiveBrain->GetOwningPawn();
		_rOut_TickParams.m_OwnerLocation = OwningPawn ? OwningPawn->Location : FVector( 0.f, 0.f, 0.f );
		_rOut_TickParams.m_LatestAttentionTagForPlayer = _pAttnProcess->m_LatestAttentionTagForPlayer;
		FDisAttnTargetParams ParamsFromTarget( EC_EventParm );
		_pAttentionTarget->CollectAttnTargetParams( ParamsFromTarget );
		if( _rOut_TickParams_AttentionTarget.m_bDirty )
		{
			const UDisTweaks_AttentionTarget* TargetTweaks = (const UDisTweaks_AttentionTarget*)ParamsFromTarget.m_pTargetTweaks;
			if( !TargetTweaks )
			{
				TargetTweaks = UDisTweaks_AttentionTarget::StaticClass()->GetDefaultObject<UDisTweaks_AttentionTarget>();
			}
			appMemcpy( _rOut_TickParams_AttentionTarget.m_fAttentionProxyMinIncreaseRateForUpdate, TargetTweaks->m_fAttentionProxyMinIncreaseRateForUpdate, sizeof(_rOut_TickParams_AttentionTarget.m_fAttentionProxyMinIncreaseRateForUpdate) );
			_rOut_TickParams_AttentionTarget.m_AttentionProxyMovementMode = TargetTweaks->m_AttentionProxyMovementMode;
			_rOut_TickParams_AttentionTarget.m_fAttnProxyDetachmentDelay = TargetTweaks->m_fAttnProxyDetachmentDelay;
			_rOut_TickParams_AttentionTarget.m_bDirty = 0;
		}
		_rOut_TickParams.m_pAttnTargetActor = ParamsFromTarget.m_pActor;
		_rOut_TickParams.m_AttnTargetLocation = ParamsFromTarget.m_Location;
		_rOut_TickParams.m_AttnTargetVelocity = ParamsFromTarget.m_Velocity;
		_rOut_TickParams.m_AttnTargetFocalPoint = ParamsFromTarget.m_FocalPoint;
		_rOut_TickParams.m_pAttentionTargetTweaks = &_rOut_TickParams_AttentionTarget;
		_rOut_TickParams.m_bAttnTargetAllowsAttentionGain = _pAttentionTarget->AllowAttentionGain();
		_rOut_TickParams.m_bAttnTargetHasObservableComponent = FALSE;
		_rOut_TickParams.m_fAttnTargetVisibilityLevel = 0.f;
		_rOut_TickParams.m_fAttnTargetSpottedDistance = 1.f;
		_rOut_TickParams.m_bShouldMagicDrainAttn = FALSE;
		_rOut_TickParams.m_bBlinkInProgress = FALSE;
		_rOut_TickParams.m_bCanLoseBustedTarget = FALSE;
		if( ParamsFromTarget.m_pObservableComponent )
		{
			// DISHONORED(bringup): IArkComponentObservable::GetVisibilityLevel / GetSpottedDistance( this brain's vision ) belong to the
			// unported FDisComponentObservable; a pawn never carries one in this tree (m_pCpntObservable stays NULL), so this branch is
			// retail's shape with nothing to read.
			_rOut_TickParams.m_bAttnTargetHasObservableComponent = TRUE;
			_rOut_TickParams.m_fAttnTargetVisibilityLevel = 1.f;
			_rOut_TickParams.m_fAttnTargetSpottedDistance = 1.f;
			_rOut_TickParams.m_bShouldMagicDrainAttn = _pAttentiveBrain->ShouldMagicDrainAttn( ParamsFromTarget.m_pActor, _rOut_TickParams.m_bBlinkInProgress );
			_rOut_TickParams.m_bCanLoseBustedTarget = _pAttentiveBrain->CanLoseBustedTarget();
		}
		else if( ParamsFromTarget.m_bIsPlayer && DisGISightStandInEnabled() )
		{
			FLOAT fNormalisedDistance = 1.f;
			DisGISightStandInSeesPlayer( _pAttentiveBrain, fNormalisedDistance );
			_rOut_TickParams.m_bAttnTargetHasObservableComponent = TRUE;
			_rOut_TickParams.m_fAttnTargetVisibilityLevel = 1.f;
			_rOut_TickParams.m_fAttnTargetSpottedDistance = fNormalisedDistance;
			_rOut_TickParams.m_bShouldMagicDrainAttn = _pAttentiveBrain->ShouldMagicDrainAttn( ParamsFromTarget.m_pActor, _rOut_TickParams.m_bBlinkInProgress );
			_rOut_TickParams.m_bCanLoseBustedTarget = _pAttentiveBrain->CanLoseBustedTarget();
		}
		if( _rOut_TickParams_PawnAttention.m_bDirty )
		{
			_rOut_TickParams_PawnAttention.m_fDistanceToLoseSight = _pAttnTweaks->m_fDistanceToLoseSight;
			_rOut_TickParams_PawnAttention.m_DistanceValueTuning = _pAttnTweaks->m_DistanceValueTuning;
			const ADishonoredGameInfo* GameInfo = DisGetGameInfo();
			const BYTE Difficulty = GameInfo ? GameInfo->m_Difficulty : (BYTE)EDifficulty_Normal;
			if( ParamsFromTarget.m_bIsPlayer )
			{
				_rOut_TickParams_PawnAttention.m_fVisionIncreaseScalar = _pAttnTweaks->m_SeeingPawnAttentionIncreaseScalar_ToPlayer.GetBaseValue( Difficulty );
			}
			else if( ParamsFromTarget.m_bIsDead )
			{
				_rOut_TickParams_PawnAttention.m_fVisionIncreaseScalar = _pAttnTweaks->m_fSeeingPawnAttentionIncreaseScalar_ToDeadNPC;
			}
			else
			{
				_rOut_TickParams_PawnAttention.m_fVisionIncreaseScalar = _pAttnTweaks->m_fSeeingPawnAttentionIncreaseScalar_ToLiveNPC;
			}
			_rOut_TickParams_PawnAttention.m_fTouchingPawnAttentionIncreaseRate = _pAttnTweaks->m_fTouchingPawnAttentionIncreaseRate;
			_rOut_TickParams_PawnAttention.m_fAttentionLeakRate = _pAttnTweaks->m_fAttentionLeakRate;
			_rOut_TickParams_PawnAttention.m_fAttentionLeakRate_InConvo = _pAttnTweaks->m_fAttentionLeakRate_InConvo;
			_rOut_TickParams_PawnAttention.m_fMaxAttention = _pAttnTweaks->m_fMaxAttention;
			appMemcpy( _rOut_TickParams_PawnAttention.m_AttentionThresholds, _pAttnTweaks->m_AttentionThresholds, sizeof(_rOut_TickParams_PawnAttention.m_AttentionThresholds) );
			_rOut_TickParams_PawnAttention.m_bAlarmsKeepRingingUntilNPCsLoseAttention = _pAttnTweaks->m_bAlarmsKeepRingingUntilNPCsLoseAttention;
			_rOut_TickParams_PawnAttention.m_AlarmsKeepRingingAttentionLevel = _pAttnTweaks->m_AlarmsKeepRingingAttentionLevel;
			appMemcpy( _rOut_TickParams_PawnAttention.m_ReasonSettings, _pAttnTweaks->m_ReasonSettings, sizeof(_rOut_TickParams_PawnAttention.m_ReasonSettings) );
			_rOut_TickParams_PawnAttention.m_fMinRangeToUnsightedBustedProxy = _pAttnTweaks->m_fMinRangeToUnsightedBustedProxy;
			_rOut_TickParams_PawnAttention.m_bClampTargetVisibility = _pAttnTweaks->m_bClampTargetVisibility;
			_rOut_TickParams_PawnAttention.m_bDirty = 0;
		}
		_rOut_TickParams.m_pPawnAttentionTweaks = &_rOut_TickParams_PawnAttention;
		_rOut_TickParams.m_fAttnTargetHeight = DisAttention::FindAttentionHeight( ParamsFromTarget.m_Location, ParamsFromTarget.m_Extent, ParamsFromTarget.m_FocalPoint );
		const INT Mask = ParamsFromTarget.m_bIsPlayer ? _pAttentiveBrain->m_FinalSenseMask_Player : _pAttentiveBrain->m_FinalSenseMask_NonPlayer;
		_rOut_TickParams.m_SenseFlags = ( ( Mask & 1 ) ? DTASF_Blind : 0 ) | ( ( Mask & 3 ) ? DTASF_Deaf : 0 ) | ( ( Mask & 5 ) ? DTASF_BlindOrNumb : 0 ) | ( ( Mask & 9 ) ? DTASF_BlindOrNumbTouch : 0 );
	}
}

// DISHONORED(port): 2012 rva 0x7a7430 (158 B) / 2013 0x74c1d0 (128 B): the complex tweaks of the target's category, this brain's
// stop thresholds for it, both snapshots marked dirty.
void UDisAttentionInfo_Complex::InitAttnInfo_Common( const UDisAIBrainProcessAttention* _pAttnProcess, const IDisAttentionTargetInterface* _pTarget )
{
	UDisTweaks_PawnAttention* Tweaks = _pAttnProcess->FindAttentionTweaks( _pTarget );
	if( GetTweaks() != Tweaks )
	{
		SetTweaks( Tweaks );
		ApplyTweakChanges();
	}
	const INT Category = _pAttnProcess->FindAttentionTweakCategory( m_PreviousRelationship );
	FAttentionTweakCategoryInfo Infos[DATC_MAX + 1];
	appMemzero( Infos, sizeof(Infos) );
	_pAttnProcess->BuildAttnCategoryArray( Infos );
	const FLOAT* StopThresholds = Infos[Category].m_pComplexAttnStopThresholdArray;
	for( INT i = 0; i < 4; i++ )
	{
		m_StopThresholds[i] = StopThresholds ? StopThresholds[i] : 0.f;
	}
	m_PawnAttentionTweaks.m_bDirty = 1;
	m_AttentionTargetTweaks.m_bDirty = 1;
}

// DISHONORED(port): 2012 rva 0x7a7b50 / 2013 0x74f920 (68 B)
void UDisAttentionInfo_Complex::InitAttnInfo_Derived( const UDisAIBrainProcessAttention* _pAttnProcess, const IDisAttentionTargetInterface* _pTarget )
{
	m_AttentionAccumulator.Init();
	InitAttnInfo_Common( _pAttnProcess, _pTarget );
	m_ProxyInfo.m_Loc = _pTarget->GetAttnTargetFocalPoint();
	m_ProxyAttachmentState = 0;
}

// DISHONORED(port): 2012 rva 0x7a7770 / 2013 0x74c250 (44 B)
void UDisAttentionInfo_Complex::OnAttnTargetStateChange( const UDisAIBrainProcessAttention* _pAttnProcess, const IDisAttentionTargetInterface* _pTarget )
{
	InitAttnInfo_Common( _pAttnProcess, _pTarget );
	m_CurrentAttentionLevel = DAL_Unaware;
	m_fPreviousAttentionValue = 0.f;
}

// DISHONORED(port): 2012 rva 0x7a74d0 / 2013 0x769150 (666 B): the tick described in the file banner.
void UDisAttentionInfo_Complex::TickAttnInfo_Derived( FLOAT _fDeltaSeconds, IDisAttentionTargetInterface* _pTarget, const UDisAIBrainProcessAttention* _pAttnProcess, const UDishonoredAIBrain* _pAttentiveBrain,
	UBOOL _bClearAttention, DIS_MEMSTACK_ARRAY( FDisAppliedAttentionChangeInfo )& _rOut_Changes, DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rOut_Messages )
{
	const UDisTweaks_AIBrainProcess_Attention* ProcessTweaks = _pAttnProcess->GetAttentionProcessTweaks();
	FDisTickAttentionParams TickParams;
	appMemzero( &TickParams, sizeof(TickParams) );
	DisAttentionInfo_Complex::SetupTickParamsFromTarget( _pTarget, DisPawnAttentionTweaksOf( this ), _pAttnProcess, _pAttentiveBrain, TickParams, m_PawnAttentionTweaks, m_AttentionTargetTweaks );
	const FLOAT fOldAttentionValue = m_AttentionAccumulator.GetCurrentValue();
	SetAccumulatorParams( TickParams, _pAttentiveBrain );
	FDisAttentionChangeReason NonZeroChangeReason = FDisAttentionChangeReason::Drain();
	m_fDebug_RawDistance = 0.f;
	m_fDebug_AttentionAccumulationRate = 0.f;
	m_fDebug_AttentionDrainRate = 0.f;
	UpdateTimeBasedAttention_Sight_UpdateSightedFlag( TickParams );
	UpdateTimeBasedAttention_Sight_MakePendingChange( TickParams, _fDeltaSeconds );
	UpdateTimeBasedAttention_Touch( TickParams, _fDeltaSeconds );
	UpdateTimeBasedAttention_Psychic( TickParams );
	// DISHONORED(bringup): ADisAlarmBell::IsRinging is not ported; a heard alarm is kept until the slot is cleared.
	if( m_ProxyInfo.m_Status > DAPS_PreviouslyVerified )
	{
		m_ProxyInfo.m_Status = DAPS_PreviouslyVerified;
	}
	const UBOOL bIncreased = !_bClearAttention && ApplyAttentionIncreases( TickParams, _fDeltaSeconds, _rOut_Changes, NonZeroChangeReason, _rOut_Messages );
	if( ProcessTweaks->m_bAllowDrainDuringIncrease || !bIncreased )
	{
		FDisAppliedAttentionChangeInfo& DrainChange = _rOut_Changes( _rOut_Changes.AddZeroed() );
		DrainChange.m_ProxyInfoFromChange.m_ProxyUpdateReason.m_ReasonType = DACRT_Drain;
		ApplyAttentionDrain( TickParams, _fDeltaSeconds, _bClearAttention, DrainChange, _rOut_Messages );
	}
	const FLOAT fCurrentAttentionValue = m_AttentionAccumulator.GetCurrentValue();
	if( fCurrentAttentionValue > fOldAttentionValue )
	{
		m_MostRecentChangeReason = NonZeroChangeReason;
	}
	else if( fOldAttentionValue > fCurrentAttentionValue )
	{
		m_MostRecentChangeReason = FDisAttentionChangeReason::Drain();
	}
	const BYTE DeathState = m_WitnessDeathState;
	const UBOOL bDeathPending = DeathState == (BYTE)DWDS_DiedUnseen || DeathState == (BYTE)DWDS_DiedWitnessed || DeathState == (BYTE)DWDS_CorpseToFind;
	if( bDeathPending || m_fClearTimer > 0.f || HasAnyAttention_Derived() )
	{
		UpdateAttentionProxy( TickParams, _rOut_Changes, _fDeltaSeconds );
	}
	else
	{
		m_ProxyAttachmentState = 0;
	}
}

// DISHONORED(port): 2012 rva 0x7934f0 / 2013 0x74fc40 (156 B): the leak rate (the in-conversation one when the brain lowers its
// alertness), the floor the minimum attention level implies (the start value of that level), the ceiling.
void UDisAttentionInfo_Complex::SetAccumulatorParams( const FDisTickAttentionParams& _rTickParams, const UDishonoredAIBrain* const _pOwningBrain )
{
	const FDisTickAttentionParams_PawnAttentionTweaks& Tweaks = *_rTickParams.m_pPawnAttentionTweaks;
	const BYTE MinAttentionLevel = GetMinAttentionLevel( Tweaks.m_bAlarmsKeepRingingUntilNPCsLoseAttention, Tweaks.m_AlarmsKeepRingingAttentionLevel );
	FLOAT fMinAttentionValue = 0.f;
	if( MinAttentionLevel > DAL_Unaware && MinAttentionLevel <= DAL_Busted )
	{
		fMinAttentionValue = Tweaks.m_AttentionThresholds[MinAttentionLevel - 1].m_fAttentionLevelToStart;
	}
	m_AttentionAccumulator.SetLeakRate( _pOwningBrain->ShouldLowerAlertness() ? Tweaks.m_fAttentionLeakRate_InConvo : Tweaks.m_fAttentionLeakRate );
	m_AttentionAccumulator.SetMinValue( fMinAttentionValue );
	m_AttentionAccumulator.SetMaxValue( Tweaks.m_fMaxAttention );
}

// DISHONORED(port): 2012 rva 0x793410 (211 B) / 2013 0x74f970 (220 B): with an observable target, a line of sight makes it sighted
// and remembers the spotted distance; without one it stays sighted until the target disallows gain or moves beyond
// m_fDistanceToLoseSight (2D, or vertically); a target with no observable component is never sighted.
void UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Sight_UpdateSightedFlag( const FDisTickAttentionParams& _rTickParams )
{
	if( !_rTickParams.m_bAttnTargetHasObservableComponent )
	{
		m_bIsSighted = 0;
		return;
	}
	if( m_bHasLOS )
	{
		m_bIsSighted = 1;
		m_fLastLOSDistance = _rTickParams.m_fAttnTargetSpottedDistance;
	}
	else if( m_bIsSighted )
	{
		const FLOAT fLose = _rTickParams.m_pPawnAttentionTweaks->m_fDistanceToLoseSight;
		const FLOAT dX = _rTickParams.m_AttnTargetLocation.X - _rTickParams.m_OwnerLocation.X;
		const FLOAT dY = _rTickParams.m_AttnTargetLocation.Y - _rTickParams.m_OwnerLocation.Y;
		const FLOAT dZ = _rTickParams.m_AttnTargetLocation.Z - _rTickParams.m_OwnerLocation.Z;
		const UBOOL bWithinZ = dZ >= -fLose && dZ <= fLose;
		if( !_rTickParams.m_bAttnTargetAllowsAttentionGain || ( dX * dX + dY * dY ) >= fLose * fLose || !bWithinZ )
		{
			m_bIsSighted = 0;
		}
	}
}

// DISHONORED(port): 2012 rva 0x7900c0 (357 B) / 2013 0x74fa50 (the callee of TickAttnInfo_Derived, unmatched by the table): a sighted
// target this brain is not blind to accrues the vision scalar times the tuned visibility (clamped to [0,1] when the tweaks say so)
// per second, as a Seen change at the focal point with the target's planar velocity.
void UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Sight_MakePendingChange( const FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds )
{
	if( !m_bIsSighted || ( _rTickParams.m_SenseFlags & DTASF_BlindOrNumb ) )
	{
		return;
	}
	const FDisTickAttentionParams_PawnAttentionTweaks& Tweaks = *_rTickParams.m_pPawnAttentionTweaks;
	const FLOAT fDistanceParam = 1.f - m_fLastLOSDistance;
	const FLOAT fFinalVisLevel_Unclamped = Tweaks.m_DistanceValueTuning.Apply( fDistanceParam, _rTickParams.m_fAttnTargetVisibilityLevel );
	const FLOAT fFinalVisLevel_Clamped = Clamp( fFinalVisLevel_Unclamped, 0.f, 1.f );
	const FLOAT fRate = Tweaks.m_fVisionIncreaseScalar * ( Tweaks.m_bClampTargetVisibility ? fFinalVisLevel_Clamped : fFinalVisLevel_Unclamped );
	m_PendingAttentionChangeAmount[DACRT_Seen].m_fByAmount = fRate * _fDeltaSeconds;
	m_bDebug_FinalVisLevelIsClamped = Tweaks.m_bClampTargetVisibility;
	m_fDebug_FinalVisLevel_Clamped = fFinalVisLevel_Clamped;
	m_fDebug_AttentionAccumulationRate = fRate;
	m_fDebug_FinalVisLevel_Unclamped = fFinalVisLevel_Unclamped;
	m_fDebug_RawDistance = fDistanceParam;
	FDisAttentionPendingChange& Pending = m_PendingAttentionChange[DACRT_Seen];
	Pending.m_PendingChangeInfo.m_Loc = _rTickParams.m_AttnTargetFocalPoint;
	Pending.m_PendingChangeInfo.m_Vel = FVector( _rTickParams.m_AttnTargetVelocity.X, _rTickParams.m_AttnTargetVelocity.Y, 0.f );
	Pending.m_PendingChangeInfo.m_fHeight = _rTickParams.m_fAttnTargetHeight;
}

// DISHONORED(port): 2012 rva 0x78dc80 / 2013 0x7486d0 (101 B): a touch this tick (not numb, gain allowed) accrues the touch rate.
void UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Touch( const FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds )
{
	if( m_bIsTouched && !( _rTickParams.m_SenseFlags & DTASF_BlindOrNumbTouch ) && _rTickParams.m_bAttnTargetAllowsAttentionGain )
	{
		m_PendingAttentionChangeAmount[DACRT_Touched].m_fByAmount = _rTickParams.m_pPawnAttentionTweaks->m_fTouchingPawnAttentionIncreaseRate * _fDeltaSeconds;
		m_PendingAttentionChange[DACRT_Touched].m_PendingChangeInfo.m_Loc = _rTickParams.m_AttnTargetFocalPoint;
		m_PendingAttentionChange[DACRT_Touched].m_PendingChangeInfo.m_fHeight = _rTickParams.m_fAttnTargetHeight;
	}
	m_bIsTouched = 0;
}

// DISHONORED(port): 2012 rva 0x790230 / 2013 0x74c400 (137 B): psychic attention is the maximum, every tick.
void UDisAttentionInfo_Complex::UpdateTimeBasedAttention_Psychic( const FDisTickAttentionParams& _rTickParams )
{
	if( !m_bIsPsychic )
	{
		return;
	}
	m_PendingAttentionChangeAmount[DACRT_Psychic].m_fByAmount = _rTickParams.m_pPawnAttentionTweaks->m_fMaxAttention;
	FDisAttentionPendingChange& Pending = m_PendingAttentionChange[DACRT_Psychic];
	Pending.m_PendingChangeInfo.m_Loc = _rTickParams.m_AttnTargetFocalPoint;
	Pending.m_PendingChangeInfo.m_Vel = FVector( _rTickParams.m_AttnTargetVelocity.X, _rTickParams.m_AttnTargetVelocity.Y, 0.f );
	Pending.m_PendingChangeInfo.m_fHeight = _rTickParams.m_fAttnTargetHeight;
}

// DISHONORED(port): 2012 rva 0x79ce40 (979 B) / 2013 0x760ca0 (919 B): every pending reason with an amount or a threshold is poured
// into the accumulator - if its tag is new information (-1) or newer than the proxy's (or, without a proxy tag, the process's
// latest player tag) and the target allows gain - raised at least to the threshold value, and becomes an applied change
// (the rate, the location, a status: a verified reason (16+, or the hideout transition 14) is Verified, any other at least
// Unverified). An Alarm reason remembers the bell when alarms keep ringing. Every pending entry is then reset.
UBOOL UDisAttentionInfo_Complex::ApplyAttentionIncreases( const FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds, DIS_MEMSTACK_ARRAY( FDisAppliedAttentionChangeInfo )& _rOut_AttentionChangeInfos,
	FDisAttentionChangeReason& _rOut_NonZeroChangeReason, DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rOut_AttentionChangeMessages )
{
	UBOOL bAttentionIncreased = FALSE;
	const FDisTickAttentionParams_PawnAttentionTweaks& Tweaks = *_rTickParams.m_pPawnAttentionTweaks;
	for( INT ReasonType = 0; ReasonType < DACRT_MAX; ReasonType++ )
	{
		FDisAttentionPendingChangeAmount& Amount = m_PendingAttentionChangeAmount[ReasonType];
		FDisAttentionPendingChange& Pending = m_PendingAttentionChange[ReasonType];
		if( Amount.m_fByAmount == 0.f && Amount.m_ToThreshold == 0 )
		{
			continue;
		}
		const INT PendingTag = Pending.m_PendingChangeInfo.m_AttentionTag;
		INT LatestTag = m_ProxyInfo.m_AttentionTag;
		if( !LatestTag )
		{
			LatestTag = _rTickParams.m_LatestAttentionTagForPlayer;
		}
		if( ( PendingTag == DisAttention::NewInformationAttentionTag || PendingTag > LatestTag ) && _rTickParams.m_bAttnTargetAllowsAttentionGain )
		{
			const FDisAttentionChangeReason Reason( (EDisAttentionChangeReasonType)ReasonType, Pending.m_PendingReasonInfo );
			_rOut_NonZeroChangeReason = Reason;
			bAttentionIncreased = TRUE;
			const FLOAT fAttentionBefore = m_AttentionAccumulator.GetCurrentValue();
			m_AttentionAccumulator.Accumulate( Amount.m_fByAmount );
			FLOAT fThresholdValue = 0.f;
			if( Amount.m_ToThreshold == DALFI_MaxAttention )
			{
				fThresholdValue = Tweaks.m_fMaxAttention;
			}
			else if( Amount.m_ToThreshold != DALFI_None && Amount.m_ToThreshold <= DALFI_Busted )
			{
				fThresholdValue = Tweaks.m_AttentionThresholds[Amount.m_ToThreshold - 1].m_fAttentionLevelToStart;
			}
			if( fThresholdValue > m_AttentionAccumulator.GetCurrentValue() )
			{
				m_AttentionAccumulator.SetCurrentValue( fThresholdValue );
			}
			const FLOAT fAttentionAfter = m_AttentionAccumulator.GetCurrentValue();
			INT AttentionTag = PendingTag;
			if( AttentionTag == DisAttention::NewInformationAttentionTag )
			{
				UDishonoredGlobalAIManager* Manager = DisGetGlobalAIManagerUnchecked();
				AttentionTag = Manager ? Manager->MakeNewAttentionTag() : 1;
			}
			FDisAppliedAttentionChangeInfo& Change = _rOut_AttentionChangeInfos( _rOut_AttentionChangeInfos.AddZeroed() );
			Change.m_fChangeAmount = ( fAttentionAfter - fAttentionBefore ) / _fDeltaSeconds;
			Change.m_fChangeAmount_Unclamped = Max( Change.m_fChangeAmount, Amount.m_fByAmount );
			Change.m_ProxyInfoFromChange.m_Loc = Pending.m_PendingChangeInfo.m_Loc;
			Change.m_ProxyInfoFromChange.m_Vel = Pending.m_PendingChangeInfo.m_Vel;
			Change.m_ProxyInfoFromChange.m_MostRecentVerifiedLoc = Pending.m_PendingChangeInfo.m_Loc;
			Change.m_ProxyInfoFromChange.m_fHeight = Pending.m_PendingChangeInfo.m_fHeight;
			Change.m_ProxyInfoFromChange.m_ProxyUpdateReason = Reason;
			Change.m_ProxyInfoFromChange.m_AttentionTag = AttentionTag;
			if( ReasonType >= DACRT_AttackedBy || ReasonType == DACRT_HideOutTransition )
			{
				Change.m_ProxyInfoFromChange.m_Status = DAPS_Verified;
			}
			else if( ReasonType != DACRT_Drain )
			{
				Change.m_ProxyInfoFromChange.m_Status = Max<BYTE>( DAPS_Unverified, m_ProxyInfo.m_Status );
			}
			else
			{
				Change.m_ProxyInfoFromChange.m_Status = m_ProxyInfo.m_Status;
			}
			UpdateAttentionInfo( _rTickParams, Change, _rOut_AttentionChangeMessages );
		}
		if( Tweaks.m_bAlarmsKeepRingingUntilNPCsLoseAttention && ReasonType == DACRT_Alarm )
		{
			ADisAlarmBell* Bell = Cast<ADisAlarmBell>( Pending.m_PendingReasonInfo.m_pActor );
			if( Bell && !m_pHeardAlarm )
			{
				m_pHeardAlarm = Bell;
			}
		}
		Amount.m_fByAmount = 0.f;
		Amount.m_ToThreshold = 0;
		DisResetPendingChange( Pending );
	}
	return bAttentionIncreased;
}

// DISHONORED(port): 2012 rva 0x79b180 / 2013 0x761040 (172 B): a cleared info drops to zero, otherwise the accumulator leaks; the
// drain is an applied change of its own (a negative rate) and moves the level when the value moved.
void UDisAttentionInfo_Complex::ApplyAttentionDrain( const FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds, UBOOL _bClearAttention, FDisAppliedAttentionChangeInfo& _rOut_DrainChangeInfo,
	DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rOut_AttentionChangeMessages )
{
	const FLOAT fAttentionBeforeTick = m_AttentionAccumulator.GetCurrentValue();
	if( _bClearAttention )
	{
		m_AttentionAccumulator.SetMinValue( 0.f );
		m_AttentionAccumulator.Clear();
	}
	else
	{
		m_AttentionAccumulator.Tick( _fDeltaSeconds );
	}
	const FLOAT fAttentionAfterTick = m_AttentionAccumulator.GetCurrentValue();
	const FLOAT fRate = ( fAttentionAfterTick - fAttentionBeforeTick ) / _fDeltaSeconds;
	_rOut_DrainChangeInfo.m_fChangeAmount = fRate;
	_rOut_DrainChangeInfo.m_fChangeAmount_Unclamped = fRate;
	_rOut_DrainChangeInfo.m_ProxyInfoFromChange.m_Status = m_ProxyInfo.m_Status;
	m_fDebug_AttentionDrainRate = -fRate;
	if( fAttentionAfterTick != fAttentionBeforeTick )
	{
		UpdateAttentionInfo( _rTickParams, _rOut_DrainChangeInfo, _rOut_AttentionChangeMessages );
	}
}

// DISHONORED(port): 2012 rva 0x798d20 (377 B) / 2013 0x75b680 (323 B): the value becomes a level (up against the start thresholds, down
// against this brain's stop thresholds); a Busted that must be clamped to Investigate caps the value at the Busted start value
// and, when the target was lost, drops the change's proxy status to Indeterminate; a level change is one message.
void UDisAttentionInfo_Complex::UpdateAttentionInfo( const FDisTickAttentionParams& _rTickParams, FDisAppliedAttentionChangeInfo& _rChangeInfo,
	DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rOut_AttentionChangeMessages )
{
	FLOAT fCurrentAttentionValue = m_AttentionAccumulator.GetCurrentValue();
	const BYTE OldAttentionLevel = m_CurrentAttentionLevel;
	BYTE NewAttentionLevel = GetAttentionLevelFromAttentionValue( _rTickParams, fCurrentAttentionValue >= m_fPreviousAttentionValue );
	UBOOL bLostTarget = FALSE;
	if( ShouldClampAttentionToInvestigate( _rTickParams, _rChangeInfo.m_ProxyInfoFromChange.m_ProxyUpdateReason, OldAttentionLevel, NewAttentionLevel, bLostTarget ) )
	{
		NewAttentionLevel = DAL_Investigate;
		const FLOAT fBustedStart = _rTickParams.m_pPawnAttentionTweaks->m_AttentionThresholds[3].m_fAttentionLevelToStart;
		m_AttentionAccumulator.SetCurrentValue( Min( fBustedStart, fCurrentAttentionValue ) );
		fCurrentAttentionValue = m_AttentionAccumulator.GetCurrentValue();
		if( bLostTarget )
		{
			_rChangeInfo.m_ProxyInfoFromChange.m_Status = DAPS_Indeterminate;
		}
	}
	if( m_CurrentAttentionLevel != NewAttentionLevel )
	{
		m_CurrentAttentionLevel = NewAttentionLevel;
		DisAttention::FDisAttentionChangeMessage Message;
		Message.m_bActorDead = m_WitnessDeathState != (BYTE)DWDS_Alive;
		Message.m_PreviousAttentionLevel = OldAttentionLevel;
		Message.m_NextAttentionLevel = NewAttentionLevel;
		Message.m_ChangeReason = _rChangeInfo.m_ProxyInfoFromChange.m_ProxyUpdateReason;
		Message.m_ProxyLoc = m_ProxyInfo.m_Loc;
		Message.m_bResetAttentionReceived = FALSE;
		_rOut_AttentionChangeMessages.AddItem( Message );
	}
	m_fPreviousAttentionValue = fCurrentAttentionValue;
}

// DISHONORED(port): 2012 rva 0x798b30 / 2013 0x755390 (496 B): the proxy takes the location of the strongest applied change whose
// rate clears the target tweaks' per-reason minimum (a Verified change attaches the proxy to the target for the detachment
// delay); a magic drain or a blink in progress detaches a player proxy; with no change the proxy sticks to the owner or
// follows the player's own rule.
void UDisAttentionInfo_Complex::UpdateAttentionProxy( const FDisTickAttentionParams& _rTickParams, const DIS_MEMSTACK_ARRAY( FDisAppliedAttentionChangeInfo )& _rAttentionChangeInfos, FLOAT _fDeltaSeconds )
{
	FDisAttentionProxyInfo FinalProxyInfo( EC_EventParm );
	m_bProxyUpdated = 0;
	const FDisTickAttentionParams_AttentionTargetTweaks& TargetTweaks = *_rTickParams.m_pAttentionTargetTweaks;
	const BYTE MovementMode = TargetTweaks.m_AttentionProxyMovementMode;
	if( ( _rTickParams.m_bShouldMagicDrainAttn || _rTickParams.m_bBlinkInProgress ) && MovementMode == DAPMM_Player )
	{
		m_ProxyAttachmentState = 2;
	}
	for( INT ChangeIdx = 0; ChangeIdx < _rAttentionChangeInfos.Num(); ChangeIdx++ )
	{
		const FDisAppliedAttentionChangeInfo& Change = _rAttentionChangeInfos(ChangeIdx);
		const BYTE ReasonType = Change.m_ProxyInfoFromChange.m_ProxyUpdateReason.m_ReasonType;
		if( ReasonType == DACRT_Drain )
		{
			if( Change.m_ProxyInfoFromChange.m_Status == DAPS_Indeterminate )
			{
				m_ProxyInfo.m_Status = DAPS_Indeterminate;
			}
			continue;
		}
		if( m_ProxyAttachmentState == 2 || Change.m_fChangeAmount_Unclamped < TargetTweaks.m_fAttentionProxyMinIncreaseRateForUpdate[ReasonType] * _fDeltaSeconds )
		{
			continue;
		}
		FinalProxyInfo = Change.m_ProxyInfoFromChange;
		if( Change.m_ProxyInfoFromChange.m_Status == DAPS_Verified )
		{
			FinalProxyInfo.m_MostRecentVerifiedLoc = FinalProxyInfo.m_Loc;
			FinalProxyInfo.m_Status = DAPS_Verified;
			m_ProxyAttachmentState = 1;
			m_fProxyDetachTimer = TargetTweaks.m_fAttnProxyDetachmentDelay;
		}
		m_bProxyUpdated = 1;
	}
	if( m_bProxyUpdated )
	{
		m_ProxyInfo = FinalProxyInfo;
	}
	else if( MovementMode == DAPMM_Player )
	{
		UpdateAttentionProxy_Player( _rTickParams, _fDeltaSeconds );
	}
	else if( MovementMode == DAPMM_StickToOwner )
	{
		m_ProxyInfo.m_Loc = _rTickParams.m_AttnTargetFocalPoint;
		m_ProxyInfo.m_Vel = FVector( 0.f, 0.f, 0.f );
	}
}

// DISHONORED(port): 2012 rva 0x7902c0 / 2013 0x74c490 (367 B): an attached proxy follows the player's focal point while he falls or
// until the detachment delay runs out (retail counts the player's last frame delta; the tick's delta here); a detaching proxy
// drops to the floor under its last location through a world line check and detaches.
void UDisAttentionInfo_Complex::UpdateAttentionProxy_Player( const FDisTickAttentionParams& _rTickParams, FLOAT _fDeltaSeconds )
{
	if( m_ProxyAttachmentState == 1 )
	{
		const UBOOL bFalling = _rTickParams.m_pAttnTargetActor && _rTickParams.m_pAttnTargetActor->Physics == PHYS_Falling;
		m_fProxyDetachTimer -= _fDeltaSeconds;
		if( bFalling || m_fProxyDetachTimer > 0.f )
		{
			m_ProxyInfo.m_Loc = _rTickParams.m_AttnTargetFocalPoint;
		}
		else
		{
			m_ProxyAttachmentState = 0;
		}
	}
	else if( m_ProxyAttachmentState == 2 )
	{
		if( GWorld && _rTickParams.m_pAttnTargetActor )
		{
			const FVector Start = m_ProxyInfo.m_Loc;
			const FVector End = Start - FVector( 0.f, 0.f, 100000.f );
			FCheckResult Hit( 1.f );
			if( !GWorld->SingleLineCheck( Hit, _rTickParams.m_pAttnTargetActor, End, Start, TRACE_World, _rTickParams.m_pAttnTargetActor->GetCylinderExtent() ) )
			{
				m_ProxyInfo.m_Loc = Hit.Location;
			}
		}
		m_ProxyAttachmentState = 0;
		m_fProxyDetachTimer = 0.f;
	}
}

// DISHONORED(port): 2012 rva 0x790430 / 2013 0x74c600 (224 B)
BYTE UDisAttentionInfo_Complex::GetAttentionLevelFromAttentionValue( const FDisTickAttentionParams& _rTickParams, UBOOL _bAttentionIncreasing )
{
	INT Level = m_CurrentAttentionLevel;
	const FLOAT fValue = m_AttentionAccumulator.GetCurrentValue();
	if( _bAttentionIncreasing )
	{
		const FDisAttentionLevelThreshold* Thresholds = _rTickParams.m_pPawnAttentionTweaks->m_AttentionThresholds;
		for( INT i = 0; i < 4; i++ )
		{
			if( fValue >= Thresholds[i].m_fAttentionLevelToStart && Level < i + 1 )
			{
				Level = i + 1;
			}
		}
	}
	else
	{
		for( INT i = 3; i >= 0; i-- )
		{
			if( m_StopThresholds[i] >= fValue && Level > i )
			{
				Level = i;
			}
		}
	}
	return (BYTE)Level;
}

// DISHONORED(port): 2012 rva 0x790510 / 2013 0x74c6e0 (93 / 105 B): the highest of the five minimum types (the soiree one, type 5,
// is not in retail's loop), raised to the alarm level while a heard alarm keeps ringing.
BYTE UDisAttentionInfo_Complex::GetMinAttentionLevel( UBOOL _bAlarmsKeepRingingUntilNPCsLoseAttention, BYTE _AlarmsKeepRingingAttentionLevel ) const
{
	BYTE Result = DAL_Unaware;
	for( INT i = 0; i < 5; i++ )
	{
		if( m_MinAttentionLevel[i] > Result )
		{
			Result = m_MinAttentionLevel[i];
		}
	}
	if( _bAlarmsKeepRingingUntilNPCsLoseAttention && m_pHeardAlarm && Result < _AlarmsKeepRingingAttentionLevel )
	{
		Result = _AlarmsKeepRingingAttentionLevel;
	}
	return Result;
}

// DISHONORED(port): 2012 rva 0x793590 / 2013 0x74fce0 (234 B): a rise to Busted is held at Investigate when its reason may not bust
// (the tweaks' per-reason setting) and the target was not already busted, or when nothing direct (sight, touch, psychic,
// hideout) backs it and the proxy is close to this pawn but far from the target, or magic drains it, or the behaviour may lose
// a busted target, or the target disallows gain - the last four also mean the target was lost.
UBOOL UDisAttentionInfo_Complex::ShouldClampAttentionToInvestigate( const FDisTickAttentionParams& _rTickParams, const FDisAttentionChangeReason& _rAttentionChangeReason, BYTE _OldAttentionLevel, BYTE _NewAttentionLevel, UBOOL& _rbLostTarget )
{
	_rbLostTarget = FALSE;
	if( _NewAttentionLevel != DAL_Busted )
	{
		return FALSE;
	}
	const FDisTickAttentionParams_PawnAttentionTweaks& Tweaks = *_rTickParams.m_pPawnAttentionTweaks;
	if( GetMinAttentionLevel( Tweaks.m_bAlarmsKeepRingingUntilNPCsLoseAttention, Tweaks.m_AlarmsKeepRingingAttentionLevel ) >= DAL_Busted )
	{
		return FALSE;
	}
	if( _OldAttentionLevel < DAL_Busted && !Tweaks.m_ReasonSettings[ _rAttentionChangeReason.m_ReasonType ].m_bCanIncreaseToBusted )
	{
		return TRUE;
	}
	if( m_bIsSighted || m_bIsTouched || m_bIsPsychic || m_bIsWithinHideout )
	{
		return FALSE;
	}
	const FLOAT fTargetRangeSq = ( m_ProxyInfo.m_Loc - _rTickParams.m_AttnTargetLocation ).SizeSquared();
	const FLOAT fMinRangeSq = Tweaks.m_fMinRangeToUnsightedBustedProxy * Tweaks.m_fMinRangeToUnsightedBustedProxy;
	const FLOAT fOwnerRangeSq = ( _rTickParams.m_OwnerLocation - m_ProxyInfo.m_Loc ).SizeSquared();
	UBOOL bClamp = FALSE;
	if( ( fMinRangeSq >= fOwnerRangeSq && fMinRangeSq < fTargetRangeSq ) || _rTickParams.m_bShouldMagicDrainAttn || _rTickParams.m_bCanLoseBustedTarget || !_rTickParams.m_bAttnTargetAllowsAttentionGain )
	{
		bClamp = TRUE;
	}
	_rbLostTarget = bClamp;
	return bClamp;
}

// DISHONORED(port): 2012 rva 0x790010 / 2013 0x74c280 (164 B): an instantaneous increase for a reason adds to that reason's pending
// amount, raises its pending threshold, and takes the newest info - unless its tag is older than the one already pending.
void UDisAttentionInfo_Complex::IncreaseAttention_Derived( const FDisAttentionIncrease& _rIncrease, const FDisAttentionChangeReason& _rReason, const FDisAttentionIncreaseInfo& _rIncreaseInfo, UBOOL& _rbOut_WitnessedDeath )
{
	const INT ReasonType = _rReason.m_ReasonType;
	if( ReasonType < 0 || ReasonType >= DACRT_MAX )
	{
		return;
	}
	FDisAttentionPendingChange& Pending = m_PendingAttentionChange[ReasonType];
	if( _rIncreaseInfo.m_AttentionTag != DisAttention::NewInformationAttentionTag && _rIncreaseInfo.m_AttentionTag <= Pending.m_PendingChangeInfo.m_AttentionTag )
	{
		return;
	}
	FDisAttentionPendingChangeAmount& Amount = m_PendingAttentionChangeAmount[ReasonType];
	Amount.m_fByAmount += _rIncrease.m_fByAmount;
	Amount.m_ToThreshold = Max( Amount.m_ToThreshold, _rIncrease.m_ToThreshold );
	Pending.m_PendingReasonInfo = _rReason.m_ReasonInfo;
	Pending.m_PendingChangeInfo = _rIncreaseInfo;
}

// DISHONORED(port): 2012 rva 0x7a0860 / 2013 0x767cf0 (147 B): a proxy that moved this tick tells the brain's behaviours.
void UDisAttentionInfo_Complex::SetAsTopAttnTarget_Derived( UDishonoredAIBrain* _pBrain, const FDisAttentionProxy& _rTopAttnTargetProxy )
{
	if( !m_bProxyUpdated )
	{
		return;
	}
	FAIStimStruct_TopAttnProxyUpdated Stim( EC_EventParm );
	Stim.m_StimID = EAIStimID_TopAttnProxyUpdated;
	Stim.m_RelationshipToTopAttnProxy = m_PreviousRelationship;
	Stim.m_TopAttnProxy = _rTopAttnTargetProxy;
	DisHandleAIStim( _pBrain, Stim, this );
}

// DISHONORED(port): 2012 rva 0x793300 (263 B) / 2013 0x74c330 (202 B)
void UDisAttentionInfo_Complex::OnOtherActorTerminated_AttnInfo_Derived( const AActor& _rActor )
{
	for( INT i = 0; i < DACRT_MAX; i++ )
	{
		if( m_PendingAttentionChange[i].m_PendingReasonInfo.m_pActor == &_rActor )
		{
			DisResetPendingChange( m_PendingAttentionChange[i] );
		}
	}
	if( m_ProxyInfo.m_ProxyUpdateReason.m_ReasonInfo.m_pActor == &_rActor )
	{
		m_ProxyInfo.m_ProxyUpdateReason = FDisAttentionChangeReason::Drain();
	}
	if( m_pHeardAlarm == &_rActor )
	{
		m_pHeardAlarm = NULL;
	}
	if( m_MostRecentChangeReason.m_ReasonInfo.m_pActor == &_rActor )
	{
		m_MostRecentChangeReason = FDisAttentionChangeReason::Drain();
	}
}

// DISHONORED(port): the one-line virtuals, 2012 rvas 0x78dab0 / 2013 0x748500 (IsAttentionDecreasing), 0x78dae0 / 0x748530,
// 0x78daf0 / 0x748540, 0x78db00 / 0x748550, 0x78db10, 0x78db20, 0x78db30 / 0x748580, 0x78db40 / 0x748590, 0x78db50,
// 0x78db70 / 0x7485c0, 0x78db90, 0x78dbb0 / 0x748600, 0x78dbe0 / 0x748630, 0x78dc40 / 0x748690, 0x78dc50 / 0x7486a0,
// 0x78dcf0 / 0x748740, 0x78dd10 / 0x748760
UBOOL UDisAttentionInfo_Complex::IsAttentionDecreasing() const
{
	return m_fPreviousAttentionValue > m_AttentionAccumulator.GetCurrentValue();
}

void UDisAttentionInfo_Complex::GainLOS_Derived() { m_bHasLOS = 1; }
void UDisAttentionInfo_Complex::LoseLOS_Derived() { m_bHasLOS = 0; }
void UDisAttentionInfo_Complex::BeTouched_Derived() { m_bIsTouched = 1; }
void UDisAttentionInfo_Complex::EnterHideout_Derived() { m_bIsWithinHideout = 1; }
void UDisAttentionInfo_Complex::ExitHideout_Derived() { m_bIsWithinHideout = 0; }
void UDisAttentionInfo_Complex::GainPsychicAttention_Derived() { m_bIsPsychic = 1; }
void UDisAttentionInfo_Complex::LosePsychicAttention_Derived() { m_bIsPsychic = 0; }

void UDisAttentionInfo_Complex::SetMinAttentionLevel_Derived( BYTE _Type, BYTE _MinLevel )
{
	if( _Type < ARRAY_COUNT( m_MinAttentionLevel ) )
	{
		m_MinAttentionLevel[_Type] = _MinLevel;
	}
}

BYTE UDisAttentionInfo_Complex::GetMinAttentionLevel_Derived( BYTE _Type ) const
{
	return _Type < ARRAY_COUNT( m_MinAttentionLevel ) ? m_MinAttentionLevel[_Type] : (BYTE)DAL_Unaware;
}

void UDisAttentionInfo_Complex::OnDifficultyChange_AttnInfo_Derived()
{
	m_PawnAttentionTweaks.m_bDirty = 1;
	m_AttentionTargetTweaks.m_bDirty = 1;
}

UBOOL UDisAttentionInfo_Complex::HasAnyAttention_Derived() const
{
	return m_bIsSighted || m_bIsTouched || m_bIsPsychic || m_AttentionAccumulator.GetCurrentValue() > 0.f;
}

FLOAT UDisAttentionInfo_Complex::GetNormalizedAttnValue_Derived() const
{
	const FLOAT fMax = m_AttentionAccumulator.GetMaxValue();
	return fMax > 0.f ? m_AttentionAccumulator.GetCurrentValue() / fMax : 0.f;
}

BYTE UDisAttentionInfo_Complex::GetAttnLevel_Derived() const
{
	return m_CurrentAttentionLevel;
}

FDisAttentionChangeReason UDisAttentionInfo_Complex::GetLastChangeReason_Derived() const
{
	return m_MostRecentChangeReason;
}

UBOOL UDisAttentionInfo_Complex::HasDirectAttention() const
{
	return m_bIsSighted || m_bIsTouched || m_bIsPsychic;
}

UBOOL UDisAttentionInfo_Complex::HasPsychicAttention() const
{
	return m_bIsPsychic;
}

// DISHONORED(port): 2012 rva 0x793680 / 2013 0x74fdd0 (134 B): with alarms kept ringing until the attention drops, the alarm rings
// while the level is at least the tweaks' alarm level and the value is still above that level's start.
UBOOL UDisAttentionInfo_Complex::ShouldKeepAlarmRinging_Derived() const
{
	const UDisTweaks_PawnAttention* Tweaks = DisPawnAttentionTweaksOf( this );
	if( !Tweaks->m_bAlarmsKeepRingingUntilNPCsLoseAttention )
	{
		return FALSE;
	}
	const BYTE Level = Tweaks->m_AlarmsKeepRingingAttentionLevel;
	if( m_CurrentAttentionLevel < Level || Level < DAL_HeadTrack || Level > DAL_Busted )
	{
		return FALSE;
	}
	return m_AttentionAccumulator.GetCurrentValue() > Tweaks->m_AttentionThresholds[Level - 1].m_fAttentionLevelToStart + 0.0001f;
}

// DISHONORED(port): 2012 rva 0x793710 / 2013 0x74fe60 (153 B): how long the leak takes to fall below the Investigate stop threshold.
FLOAT UDisAttentionInfo_Complex::GetTimeLeftInInvestigate_Derived() const
{
	const UDisTweaks_PawnAttention* Tweaks = DisPawnAttentionTweaksOf( this );
	const FLOAT fAboveStop = m_AttentionAccumulator.GetCurrentValue() - m_StopThresholds[2];
	if( m_CurrentAttentionLevel >= DAL_Investigate && fAboveStop > 0.f && Tweaks->m_fAttentionLeakRate > 0.0001f )
	{
		return fAboveStop / Tweaks->m_fAttentionLeakRate;
	}
	return 0.f;
}

void UDisAttentionInfo_Complex::FinishDestroy()
{
	if( DisGIDiagEnabled() )
	{
		debugf( TEXT("gidiag destroy %s at 0x%08x outer %s"), *GetFullName(), (DWORD)(PTRINT)this, GetOuter() ? *GetOuter()->GetName() : TEXT("none") );
	}
	Super::FinishDestroy();
}
