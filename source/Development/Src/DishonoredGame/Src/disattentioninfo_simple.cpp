// DishonoredGame/src/disattentioninfo_simple.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (20): the seventeen below plus the three class-registration helpers of the macro.
//
// ---- agent GI ports (PHASE20 GI): UDisAttentionInfo_Simple, the pooled Unaware-or-Busted info of a non-player target ----
// Resolved by hand in both databases (build/agentGI/idb; decompiles in build/agentGI/decomp2012). The flags dword at +140
// (2012 and 2013) is this tree's bitfield: m_bHasLOS 1, m_bIsSighted 2, m_bIsPsychic 4, m_bGotInstantaneousAttention 8,
// m_bIsDumb 16, m_bIsBlind 32, m_bIsDeaf 64, m_bIsNumb 128, m_bAttentionActive 256.

#include "DishonoredGame.h"
#include "disattentioninfo.h"
#include "dishonoredutilities.h"

void DishonoredLinkUDisAttentionInfo_Simple()
{
}

static const UDisTweaks_SimpleAttention* DisSimpleTweaksOf( const UDisAttentionInfo_Simple* _pInfo )
{
	const UDisTweaks_SimpleAttention* Tweaks = Cast<UDisTweaks_SimpleAttention>( _pInfo->GetTweaks() );
	return Tweaks ? Tweaks : UDisTweaks_SimpleAttention::StaticClass()->GetDefaultObject<UDisTweaks_SimpleAttention>();
}

// DISHONORED(port): 2012 rva 0x7a7ba0 (231 B) / 2013 0x74c7a0 (219 B): the simple tweaks of the target's category, the proxy on
// the target's focal point with its velocity, the height from its feet, the new-information tag and the Verified status.
void UDisAttentionInfo_Simple::InitAttnInfo_Derived( const UDisAIBrainProcessAttention* _pAttnProcess, const IDisAttentionTargetInterface* _pTarget )
{
	UDisTweaks_SimpleAttention* Tweaks = _pAttnProcess->FindSimpleAttentionTweaks( _pTarget );
	if( GetTweaks() != Tweaks )
	{
		SetTweaks( Tweaks );
		ApplyTweakChanges();
	}
	const FVector TargetLocation = _pTarget->GetAttnTargetLocation();
	const FVector TargetFocalPoint = _pTarget->GetAttnTargetFocalPoint();
	const FVector TargetExtent = _pTarget->GetAttnTargetExtent();
	m_ProxyInfo.m_Loc = TargetFocalPoint;
	m_ProxyInfo.m_Vel = _pTarget->GetAttnTargetVelocity();
	m_ProxyInfo.m_fHeight = Max( 0.f, TargetFocalPoint.Z - ( TargetLocation.Z - TargetExtent.Z ) );
	m_ProxyInfo.m_MostRecentVerifiedLoc = TargetFocalPoint;
	m_ProxyInfo.m_AttentionTag = DisAttention::NewInformationAttentionTag;
	m_ProxyInfo.m_Status = DAPS_Verified;
}

// DISHONORED(port): 2012 rva 0x7a7c90 / 2013 0x74c880 (73 B): new tweaks, attention off.
void UDisAttentionInfo_Simple::OnAttnTargetStateChange( const UDisAIBrainProcessAttention* _pAttnProcess, const IDisAttentionTargetInterface* _pTarget )
{
	UDisTweaks_SimpleAttention* Tweaks = _pAttnProcess->FindSimpleAttentionTweaks( _pTarget );
	if( GetTweaks() != Tweaks )
	{
		SetTweaks( Tweaks );
		ApplyTweakChanges();
	}
	m_bAttentionActive = 0;
}

// DISHONORED(port): 2012 rva 0x7938b0 (343 B) / 2013 0x750030 (361 B): with a line of sight the target is sighted; a corpse (or a
// target being murdered, unless the brain always witnesses deaths) is sighted only when the corpse-visibility curve of the simple
// tweaks, fed the spotted distance and the observable's visibility, reaches the see-corpse (or see-death) level.
// DISHONORED(bringup): FDisComponentObservable and FDisComponentVisionNPC are not ported (see disaibrainprocessattention.cpp),
// so a corpse's visibility cannot be read and a dead target is never sighted here; a live one follows the line of sight.
void UDisAttentionInfo_Simple::UpdateSightedFlag( IDisAttentionTargetInterface* _pAttnTarget, const UDishonoredAIBrain* _pAttentiveBrain )
{
	m_bIsSighted = m_bHasLOS;
	if( !m_bHasLOS )
	{
		return;
	}
	const BYTE DeathState = m_WitnessDeathState;
	const UBOOL bCorpseOrMurder = DeathState != (BYTE)DWDS_Alive || ( _pAttnTarget->IsAttnTargetBeingMurdered( NULL ) && !_pAttentiveBrain->ShouldAlwaysWitnessDeath() );
	if( bCorpseOrMurder )
	{
		m_bIsSighted = 0;
		m_fDebug_FinalCorpseVisibility = 0.f;
	}
}

// DISHONORED(port): 2012 rva 0x798ea0 (738 B) / 2013 0x75b7d0 (740 B): the proxy follows the target (focal point, velocity, verified
// location); the four sense flags come off the brain's final sense mask for the player / non-player side (dumb 1, blind
// (mask & 5), deaf (mask & 3), numb (mask & 9)); the sighted flag is refreshed; the attention is active when sighted and not
// dumb or blind, psychic and not dumb, got an instantaneous increase, or has a minimum level - then the remaining time is the
// lost-contact timeout (the dead one for a corpse; negative means forever), otherwise it runs down and the attention goes.
// A cleared info or a target that disallows gain is off at once. A level change is one Seen message.
void UDisAttentionInfo_Simple::TickAttnInfo_Derived( FLOAT _fDeltaSeconds, IDisAttentionTargetInterface* _pTarget, const UDisAIBrainProcessAttention* _pAttnProcess, const UDishonoredAIBrain* _pAttentiveBrain,
	UBOOL _bClearAttention, DIS_MEMSTACK_ARRAY( FDisAppliedAttentionChangeInfo )& _rOut_Changes, DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rOut_Messages )
{
	const UDisTweaks_SimpleAttention* Tweaks = DisSimpleTweaksOf( this );
	const FVector FocalPoint = _pTarget->GetAttnTargetFocalPoint();
	m_ProxyInfo.m_Loc = FocalPoint;
	m_ProxyInfo.m_MostRecentVerifiedLoc = FocalPoint;
	m_ProxyInfo.m_Vel = _pTarget->GetAttnTargetVelocity();
	const INT Mask = _pTarget->IsAttnTargetPlayer() ? _pAttentiveBrain->m_FinalSenseMask_Player : _pAttentiveBrain->m_FinalSenseMask_NonPlayer;
	m_bIsDumb = ( Mask & 1 ) ? 1 : 0;
	m_bIsBlind = ( Mask & 5 ) ? 1 : 0;
	m_bIsDeaf = ( Mask & 3 ) ? 1 : 0;
	m_bIsNumb = ( Mask & 9 ) ? 1 : 0;
	const BYTE OldAttentionLevel = GetAttnLevel_Derived();
	UpdateSightedFlag( _pTarget, _pAttentiveBrain );
	UBOOL bActive = FALSE;
	if( m_bIsSighted && !m_bIsDumb && !m_bIsBlind )
	{
		bActive = TRUE;
	}
	if( m_bGotInstantaneousAttention && !m_bIsDumb )
	{
		bActive = TRUE;
	}
	if( m_bIsPsychic )
	{
		bActive = TRUE;
	}
	if( m_MinAttentionLevelFlags )
	{
		bActive = TRUE;
	}
	m_bGotInstantaneousAttention = 0;
	if( _bClearAttention )
	{
		m_fRemainingAttentionTime = 0.f;
		m_bAttentionActive = 0;
	}
	else if( !_pTarget->AllowAttentionGain() )
	{
		m_bAttentionActive = 0;
	}
	else if( bActive )
	{
		FLOAT fTimeOut = m_WitnessDeathState != (BYTE)DWDS_Alive ? Tweaks->m_fLostContactTimeOut_Dead : Tweaks->m_fLostContactTimeOut;
		if( fTimeOut < 0.f )
		{
			fTimeOut = 3.4028235e38f;
		}
		m_bAttentionActive = 1;
		m_fRemainingAttentionTime = fTimeOut;
	}
	else
	{
		m_fRemainingAttentionTime -= _fDeltaSeconds;
		if( m_fRemainingAttentionTime < 0.f )
		{
			m_bAttentionActive = 0;
		}
	}
	const BYTE NewAttentionLevel = GetAttnLevel_Derived();
	if( OldAttentionLevel != NewAttentionLevel )
	{
		DisAttention::FDisAttentionChangeMessage Message;
		Message.m_bActorDead = m_WitnessDeathState != (BYTE)DWDS_Alive;
		Message.m_PreviousAttentionLevel = OldAttentionLevel;
		Message.m_NextAttentionLevel = NewAttentionLevel;
		Message.m_ChangeReason = FDisAttentionChangeReason( DACRT_Seen );
		Message.m_ProxyLoc = m_ProxyInfo.m_Loc;
		Message.m_bResetAttentionReceived = FALSE;
		_rOut_Messages.AddItem( Message );
	}
}

// DISHONORED(port): 2012 rva 0x793a10 (364 B) / 2013 0x7501a0 (364 B): which instantaneous increases a simple info takes, per reason,
// off the simple tweaks' flags; a heard death or noise consults the per-context noise table and may count as a witnessed death.
void UDisAttentionInfo_Simple::IncreaseAttention_Derived( const FDisAttentionIncrease& _rIncrease, const FDisAttentionChangeReason& _rReason, const FDisAttentionIncreaseInfo& _rIncreaseInfo, UBOOL& _rbOut_WitnessedDeath )
{
	const UDisTweaks_SimpleAttention* Tweaks = DisSimpleTweaksOf( this );
	const UBOOL bDumbOrBlind = m_bIsDumb || m_bIsBlind;
	switch( _rReason.m_ReasonType )
	{
	case DACRT_HeardDeath:
	case DACRT_Heard:
		if( bDumbOrBlind )
		{
			if( !m_bIsDumb )
			{
				m_bGotInstantaneousAttention = Tweaks->m_bGainAttentionOnBeAttacked;
			}
		}
		else
		{
			const INT Context = _rReason.m_ReasonInfo.m_iParam1;
			if( Context >= 0 && Context < ARRAY_COUNT( Tweaks->m_bHearNoiseSettings ) )
			{
				const FDisSimpleAttnNoiseContextInfo& NoiseInfo = Tweaks->m_bHearNoiseSettings[Context];
				m_bGotInstantaneousAttention = NoiseInfo.m_bGainAttentionOnNoiseContext;
				_rbOut_WitnessedDeath = NoiseInfo.m_bGainAttentionOnNoiseContext && NoiseInfo.m_bCountsAsWitnessDeath;
			}
		}
		break;
	case DACRT_Alarm:
		if( !bDumbOrBlind )
		{
			m_bGotInstantaneousAttention = Tweaks->m_bGainAttentionOnHearAlarm;
			break;
		}
		// a dumb or blind listener treats an alarm as a help request (retail falls through)
	case DACRT_HeardHelpRequest:
		if( bDumbOrBlind )
		{
			if( !m_bIsDumb )
			{
				m_bGotInstantaneousAttention = Tweaks->m_bGainAttentionOnBeAttacked;
			}
		}
		else
		{
			m_bGotInstantaneousAttention = Tweaks->m_bGainAttentionOnHelpRequest;
		}
		break;
	case DACRT_AttackedBy:
		if( !m_bIsDumb )
		{
			m_bGotInstantaneousAttention = Tweaks->m_bGainAttentionOnBeAttacked;
		}
		break;
	case DACRT_AttackedEnemy:
		if( !m_bIsDumb )
		{
			m_bGotInstantaneousAttention = Tweaks->m_bGainAttentionOnAttackEnemy;
		}
		break;
	default:
		break;
	}
}

// DISHONORED(port): 2012 rva 0x793b80 / 2013 0x750340 (65 B), 0x78dd20 / 0x748770, 0x793bd0 / 0x750390 (62 B), 0x78dd30 / 0x748780,
// 0x78dd40 / 0x748790
void UDisAttentionInfo_Simple::GainLOS_Derived()
{
	if( DisSimpleTweaksOf( this )->m_bGainAttentionOnSight )
	{
		m_bHasLOS = 1;
	}
}

void UDisAttentionInfo_Simple::LoseLOS_Derived()
{
	m_bHasLOS = 0;
}

void UDisAttentionInfo_Simple::BeTouched_Derived()
{
	if( DisSimpleTweaksOf( this )->m_bGainAttentionOnTouch )
	{
		m_bGotInstantaneousAttention = 1;
	}
}

void UDisAttentionInfo_Simple::GainPsychicAttention_Derived()
{
	m_bIsPsychic = 1;
}

void UDisAttentionInfo_Simple::LosePsychicAttention_Derived()
{
	m_bIsPsychic = 0;
}

// DISHONORED(port): 2012 rva 0x78dd50 / 2013 0x7487a0 (40 B) and 0x78dd80 / 0x7487d0 (35 B): one bit per minimum type; any set
// bit reads as Busted.
void UDisAttentionInfo_Simple::SetMinAttentionLevel_Derived( BYTE _Type, BYTE _MinLevel )
{
	const BYTE Bit = 1 << _Type;
	if( _MinLevel <= DAL_Unaware )
	{
		m_MinAttentionLevelFlags &= ~Bit;
	}
	else
	{
		m_MinAttentionLevelFlags |= Bit;
	}
}

BYTE UDisAttentionInfo_Simple::GetMinAttentionLevel_Derived( BYTE _Type ) const
{
	return ( m_MinAttentionLevelFlags & ( 1 << _Type ) ) ? DAL_Busted : DAL_Unaware;
}

// DISHONORED(port): 2012 rva 0x793c10 / 2013 0x7503d0 (93 B)
UBOOL UDisAttentionInfo_Simple::HasAnyAttention_Derived() const
{
	return m_bIsSighted || m_bGotInstantaneousAttention || m_bIsPsychic || m_MinAttentionLevelFlags || m_fRemainingAttentionTime > 0.f;
}

// DISHONORED(port): 2012 rva 0x78ddb0 / 2013 0x748800 (51 B), 0x78ddf0 / 0x748840 (17 B), 0x78de10 / 0x748860 (23 B),
// 0x790570 / 0x74c8d0 (27 B), HasPsychicAttention 0x22ac50 (an ICF fold, 2013 0x748760-neighbourhood)
FLOAT UDisAttentionInfo_Simple::GetNormalizedAttnValue_Derived() const
{
	return m_bAttentionActive ? 1.f : 0.f;
}

BYTE UDisAttentionInfo_Simple::GetAttnLevel_Derived() const
{
	return m_bAttentionActive ? DAL_Busted : DAL_Unaware;
}

FDisAttentionChangeReason UDisAttentionInfo_Simple::GetLastChangeReason_Derived() const
{
	return FDisAttentionChangeReason( DACRT_Seen );
}

UBOOL UDisAttentionInfo_Simple::HasDirectAttention() const
{
	return m_bIsPsychic || ( m_bIsSighted && !m_bIsDumb && !m_bIsBlind );
}

UBOOL UDisAttentionInfo_Simple::HasPsychicAttention() const
{
	return m_bIsPsychic;
}

void UDisAttentionInfo_Simple::FinishDestroy()
{
	if( DisGIDiagEnabled() )
	{
		debugf( TEXT("gidiag destroy %s at 0x%08x outer %s"), *GetFullName(), (DWORD)(PTRINT)this, GetOuter() ? *GetOuter()->GetName() : TEXT("none") );
	}
	Super::FinishDestroy();
}
