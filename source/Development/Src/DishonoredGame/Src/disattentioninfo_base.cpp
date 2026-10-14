// DishonoredGame/src/disattentioninfo_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7): the four below plus the three class-registration helpers of the macro.
//
// ---- agent GI ports (PHASE20 GI): UDisAttentionInfo_Base, what every attention slot shares ----
// Resolved by hand in both databases (build/agentGI/idb; decompiles in build/agentGI/decomp2012). GameSave / GameLoad are
// agent ED's, in dissavegame.cpp.

#include "DishonoredGame.h"
#include "disattentioninfo.h"
#include "dishonoredutilities.h"

void DishonoredLinkUDisAttentionInfo_Base()
{
}

// DISHONORED(port): 2012 rva 0x78da90 / 2013 0x7484e0 (21 B)
FDisAttentionChangeReason UDisAttentionInfo_Base::GetLastChangeReason_Derived() const
{
	return FDisAttentionChangeReason::Drain();
}

// DISHONORED(port): 2012 rva 0x7a71e0 / 2013 0x7552d0 (183 B): the relationship the slot was made under, the corpse flags off the
// process tweaks (2013 bits 2 and 3 of the flags at +172: m_bReactToHandledCorpses, m_bIgnoreAllCorpses), and a target that is
// already dead starts as a corpse to find - or as handled when every corpse is ignored or this one counts as handled.
void UDisAttentionInfo_Base::InitAttnInfo( const UDisAIBrainProcessAttention* _pAttnProcess, const IDisAttentionTargetInterface* _pTarget, BYTE _OwnerRelationshipToTarget )
{
	const UDisTweaks_AIBrainProcess_Attention* Tweaks = _pAttnProcess->GetAttentionProcessTweaks();
	m_PreviousRelationship = _OwnerRelationshipToTarget;
	m_WitnessDeathState = DWDS_Alive;
	m_bReactToHandledCorpses = Tweaks->m_bReactToHandledCorpses;
	m_bIgnoreAllCorpses = Tweaks->m_bIgnoreAllCorpses;
	if( _pTarget->IsAttnTargetDead() )
	{
		if( Tweaks->m_bIgnoreAllCorpses )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
		}
		else
		{
			m_WitnessDeathState = DWDS_CorpseToFind;
			if( !Tweaks->m_bReactToHandledCorpses && _pAttnProcess->ShouldSeeBodyAsHandled( _pTarget ) )
			{
				m_WitnessDeathState = DWDS_CorpseHandled;
			}
		}
	}
	InitAttnInfo_Derived( _pAttnProcess, _pTarget );
}

// DISHONORED(port): 2012 rva 0x7a72a0 (400 B) / 2013 0x75b520 (340 B): when the tweaks changed, the relationship changed or the
// target's life state changed (deadness is re-read only while the attention holds at Busted and is not decreasing), the info
// drops to Unaware with a reset message and lets the subclass re-read its tweaks. A target newly dead becomes DiedUnseen, or
// DiedWitnessed when the brain always witnesses deaths, or handled when this info ignores corpses.
void UDisAttentionInfo_Base::HandleStateChanges( IDisAttentionTargetInterface* _pTarget, const UDisAIBrainProcessAttention* _pAttnProcess, const UDishonoredAIBrain* _pAttentiveBrain,
	UBOOL _bTweaksChanged, DIS_MEMSTACK_ARRAY( DisAttention::FDisAttentionChangeMessage )& _rOut_Messages )
{
	const UBOOL bDecreasing = IsAttentionDecreasing();
	const BYTE CurrentAttentionLevel = GetAttnLevel_Derived();
	const IDisRelationshipInterface* Relator = _pAttentiveBrain->GetOwningPawn() ? static_cast<const IDisRelationshipInterface*>( _pAttentiveBrain->GetOwningPawn() ) : NULL;
	const BYTE CurrentRelationship = _pTarget->GetAttnTargetIncomingRelationship( Relator );
	const UBOOL bOldBelievedToBeDead = m_WitnessDeathState != (BYTE)DWDS_Alive;
	const UBOOL bNowDead = ( !bDecreasing && CurrentAttentionLevel >= DAL_Busted ) ? _pTarget->IsAttnTargetDead() : bOldBelievedToBeDead;
	if( !_bTweaksChanged && m_PreviousRelationship == CurrentRelationship && bOldBelievedToBeDead == bNowDead )
	{
		return;
	}
	if( bOldBelievedToBeDead )
	{
		if( !bNowDead )
		{
			m_WitnessDeathState = DWDS_Alive;
		}
	}
	else if( bNowDead )
	{
		if( m_bIgnoreAllCorpses )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
		}
		else
		{
			m_WitnessDeathState = _pAttentiveBrain->ShouldAlwaysWitnessDeath() ? DWDS_DiedWitnessed : DWDS_DiedUnseen;
		}
	}
	DisAttention::FDisAttentionChangeMessage Message;
	Message.m_bActorDead = bOldBelievedToBeDead;
	Message.m_PreviousAttentionLevel = CurrentAttentionLevel;
	Message.m_NextAttentionLevel = DAL_Unaware;
	Message.m_ChangeReason = FDisAttentionChangeReason::Drain();
	Message.m_ProxyLoc = m_ProxyInfo.m_Loc;
	Message.m_bResetAttentionReceived = TRUE;
	_rOut_Messages.AddItem( Message );
	m_PreviousRelationship = CurrentRelationship;
	OnAttnTargetStateChange( _pAttnProcess, _pTarget );
}

// DISHONORED(port): 2012 rva 0x7a79e0 (368 B) / 2013 0x74bfc0 (344 B): the witnessed-death state machine, one step per tick.
BYTE UDisAttentionInfo_Base::GetCurrentCorpseResponse( const IDisAttentionTargetInterface* _pTarget, const UDisAIBrainProcessAttention* _pAttnProcess )
{
	UBOOL bCanDeathBeWitnessed = FALSE;
	switch( m_WitnessDeathState )
	{
	case DWDS_Alive:
		return DWDR_None;
	case DWDS_DiedUnseen:
	case DWDS_DiedWitnessed:
		if( _pAttnProcess->ShouldIgnoreDeathOf( _pTarget, &bCanDeathBeWitnessed ) )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
			return DWDR_None;
		}
		if( m_WitnessDeathState == (BYTE)DWDS_DiedWitnessed )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
			return DWDR_WitnessedDeath;
		}
		if( !bCanDeathBeWitnessed )
		{
			return DWDR_None;
		}
		if( GetAttnLevel_Derived() == DAL_Busted )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
			return DWDR_WitnessedDeath;
		}
		m_WitnessDeathState = DWDS_CorpseSeenNotBusted;
		return DWDR_None;
	case DWDS_CorpseToFind:
		if( ( !m_bReactToHandledCorpses && _pAttnProcess->ShouldSeeBodyAsHandled( _pTarget ) ) || _pAttnProcess->ShouldIgnoreDeathOf( _pTarget, &bCanDeathBeWitnessed ) )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
			return DWDR_None;
		}
		if( !bCanDeathBeWitnessed )
		{
			return DWDR_None;
		}
		if( GetAttnLevel_Derived() != DAL_Busted )
		{
			m_WitnessDeathState = DWDS_CorpseSeenNotBusted;
			return DWDR_None;
		}
		if( HasDirectAttention() )
		{
			m_WitnessDeathState = DWDS_CorpseHandled;
			return DWDR_FoundNewCorpse_Direct;
		}
		m_WitnessDeathState = DWDS_CorpseFoundIndirect;
		return DWDR_FoundNewCorpse_Indirect;
	case DWDS_CorpseSeenNotBusted:
		if( GetAttnLevel_Derived() != DAL_Busted )
		{
			return DWDR_None;
		}
		m_WitnessDeathState = DWDS_CorpseToFind;
		return DWDR_None;
	case DWDS_CorpseFoundIndirect:
		if( !HasDirectAttention() )
		{
			return DWDR_None;
		}
		m_WitnessDeathState = DWDS_CorpseHandled;
		return DWDR_FoundNewCorpse_Direct;
	case DWDS_CorpseHandled:
		return _pAttnProcess->ShouldIgnoreDeathOf( _pTarget, NULL ) ? DWDR_ExistingIgnoredCorpse : DWDR_ExistingCorpse;
	default:
		return DWDR_None;
	}
}

UDisTweaksBase* UDisAttentionInfo_Base::GetTweaks_Derived()
{
	return m_pAttentionTweaks;
}

void UDisAttentionInfo_Base::SetTweaks_Derived( UDisTweaksBase* _pTweaks )
{
	m_pAttentionTweaks = _pTweaks;
}

void UDisAttentionInfo_Base::ResetFromTemplate( UObject* _pTemplate )
{
	if( !_pTemplate )
	{
		return;
	}
	ExitProperties( (BYTE*)this, GetClass() );
	InitProperties( (BYTE*)this, GetClass()->GetPropertiesSize(), _pTemplate->GetClass(), (BYTE*)_pTemplate, _pTemplate->GetClass()->GetPropertiesSize(), this, this, NULL );
}
