// DishonoredGame/src/aistimstruct.cpp
// DISHONORED(port): agent CG. FAIStimStruct's own five virtual bodies, the per-EAIStimID type registry, the explicit
// dispatch table described in Inc/aistimstruct.h, FDisStimRef's reference count, and the file-local archive
// FAIStimStruct::IsSaveable runs a stim through.
//
// The 120 stim types themselves are generated (reflected script structs in dishonoredgameclasses.h); this unit gives
// each one its table row, which is retail's FAIStimStruct::FAIStimTypeInfoRegister. Read the header first: the reason
// the dispatch is a table of function pointers rather than a C++ vtable is written up there, together with the
// one-line generator change that turns it into a real vtable.
//
// PDB functions attributed to this file (6, 2012 rvas):
//   0x764de0  FAIStimStruct::GetPendingStimFilterDelegate( EAIStimID ) const
//   0x7674b0  FAIStimStruct::Serialize( FArchive& )
//   0x76f1c0  FDisArchiveCheckForItemContexts::operator<<( UObject*& )
//   0x76f220  FAIStimStruct::IsSaveable() const
//   0x772f70  FAIStimStruct::GetStimDebugString() const
//   0x777030  FAIStimStruct::FAIStimStruct( const FAIStimStruct& )

#include "DishonoredGame.h"
#include "aistimstruct.h"

/*-----------------------------------------------------------------------------
	The type registry.
-----------------------------------------------------------------------------*/

/** DISHONORED(layout): retail's FAIStimStruct::g_StimTypeInfos, one row per EAIStimID, filled during static
    initialisation by the FAIStimTypeInfoRegister objects at the bottom of this file. A plain array rather than a
    TArray because the registrars run before the allocator is guaranteed to be up. */
static FAIStimTypeInfo GStimTypeInfos[ EAIStimID_MAX ];

FAIStimTypeInfoRegister::FAIStimTypeInfoRegister( BYTE _StimID, const FAIStimTypeInfo& _rInfo )
{
	if( _StimID < EAIStimID_MAX )
	{
		GStimTypeInfos[ _StimID ] = _rInfo;
	}
}

const FAIStimTypeInfo* DisGetStimTypeInfo( BYTE _StimID )
{
	if( _StimID >= EAIStimID_MAX || GStimTypeInfos[ _StimID ].m_pVTable == NULL )
	{
		return NULL;
	}
	return &GStimTypeInfos[ _StimID ];
}

/*-----------------------------------------------------------------------------
	Dispatch.

	DISHONORED(written): the table pointer lives in the 4 bytes at offset 0 that retail's UnrealScript reserves as
	m_VTable_Pointer_Dummy and that retail's C++ uses for the compiler's own vtable pointer. Reading and writing it
	goes through these two, so there is exactly one place to change when the generator makes the structs polymorphic.
-----------------------------------------------------------------------------*/

const FAIStimStructVTable* DisStimVTable( const FAIStimStruct* _pStim )
{
	if( !_pStim )
	{
		return NULL;
	}
	const FAIStimStructVTable* pVTable = (const FAIStimStructVTable*)_pStim->m_VTable_Pointer_Dummy;
	if( pVTable )
	{
		return pVTable;
	}
	// A stim built through the generated FAIStimStruct(EEventParm) constructor has a zeroed slot: fall back on its id.
	const FAIStimTypeInfo* pTypeInfo = DisGetStimTypeInfo( _pStim->m_StimID );
	return pTypeInfo ? pTypeInfo->m_pVTable : NULL;
}

void DisStimSetVTable( FAIStimStruct* _pStim, const FAIStimStructVTable* _pVTable )
{
	if( _pStim )
	{
		_pStim->m_VTable_Pointer_Dummy = (FPointer)_pVTable;
	}
}

void DisStimDestruct( FAIStimStruct* _pStim )
{
	const FAIStimStructVTable* pVTable = DisStimVTable( _pStim );
	if( pVTable && pVTable->Destruct )
	{
		pVTable->Destruct( _pStim );
	}
}

void DisStimSerialize( FAIStimStruct* _pStim, FArchive& _rAr )
{
	const FAIStimStructVTable* pVTable = DisStimVTable( _pStim );
	if( pVTable && pVTable->Serialize )
	{
		pVTable->Serialize( _pStim, _rAr );
		return;
	}
	DisStimSerialize_Base( _pStim, _rAr );
}

FLOAT DisStimGetDelay( const FAIStimStruct* _pStim, UDishonoredAIBrain* _pBrain )
{
	const FAIStimStructVTable* pVTable = DisStimVTable( _pStim );
	if( pVTable && pVTable->GetDelay )
	{
		return pVTable->GetDelay( _pStim, _pBrain );
	}
	return DisStimGetDelay_Base( _pStim, _pBrain );
}

FDisStimPendingFilterDelegate DisStimGetPendingStimFilterDelegate( const FAIStimStruct* _pStim, BYTE _StimID )
{
	const FAIStimStructVTable* pVTable = DisStimVTable( _pStim );
	if( pVTable && pVTable->GetPendingStimFilterDelegate )
	{
		return pVTable->GetPendingStimFilterDelegate( _pStim, _StimID );
	}
	return DisStimGetPendingStimFilterDelegate_Base( _pStim, _StimID );
}

FString DisStimGetStimDebugString( const FAIStimStruct* _pStim )
{
	const FAIStimStructVTable* pVTable = DisStimVTable( _pStim );
	if( pVTable && pVTable->GetStimDebugString )
	{
		return pVTable->GetStimDebugString( _pStim );
	}
	return DisStimGetStimDebugString_Base( _pStim );
}

const UScriptStruct* DisStimGetScriptStruct( const FAIStimStruct* _pStim )
{
	const FAIStimStructVTable* pVTable = DisStimVTable( _pStim );
	if( pVTable && pVTable->GetScriptStruct )
	{
		return pVTable->GetScriptStruct( _pStim );
	}
	return NULL;
}

/*-----------------------------------------------------------------------------
	FAIStimStruct's own virtual bodies.
-----------------------------------------------------------------------------*/

/** DISHONORED(written): slot 0. Retail's is the compiler-generated destructor of a struct with no destructible
    member, so it only resets the table pointer; the pool block itself is handed back by FDisStimRef. */
void DisStimDestruct_Base( FAIStimStruct* _pStim )
{
	DisStimSetVTable( _pStim, NULL );
}

// DISHONORED(port): 2013 rva 0x704980 (2012 0x7674b0): only the two object references, so an object-reference
// collector archive sees the manager and the source. A subclass chains to this and adds its own.
void DisStimSerialize_Base( FAIStimStruct* _pStim, FArchive& _rAr )
{
	_rAr << _pStim->m_pStimManager;
	_rAr << _pStim->m_pSource;
}

/** DISHONORED(written): slot 8. The base delay is zero, i.e. a stim with no override is processed on the tick it was
    enqueued. FAIStimStruct_Stolen is the retail stim that overrides it (it carries m_fDelay). */
FLOAT DisStimGetDelay_Base( const FAIStimStruct* /*_pStim*/, UDishonoredAIBrain* /*_pBrain*/ )
{
	return 0.f;
}

// DISHONORED(port): 2013 rva 0x7015d0 (2012 0x764de0): the base answers the shared empty delegate, i.e. "no
// opinion", which UDishonoredAIBrain::EnqueueStim reads as DPSFR_KeepBothStims.
FDisStimPendingFilterDelegate DisStimGetPendingStimFilterDelegate_Base( const FAIStimStruct* /*_pStim*/, BYTE /*_StimID*/ )
{
	return FDisStimPendingFilterDelegate::s_NullDelegate;
}

// DISHONORED(port): 2013 rva 0x70c550 (2012 0x772f70): the base string is empty and every stim with something to say
// overrides it. Retail builds it from a static empty string constant.
FString DisStimGetStimDebugString_Base( const FAIStimStruct* /*_pStim*/ )
{
	return FString();
}

/*-----------------------------------------------------------------------------
	IsSaveable.
-----------------------------------------------------------------------------*/

/** DISHONORED(port): 2013 rva 0x70ae10 (2012 0x76f1c0): an object-reference-collector archive that trips on the first
    UDisItemContext it is handed. Retail declares it file-local in this unit. */
class FDisArchiveCheckForItemContexts : public FArchive
{
public:
	UBOOL m_bFoundItemContextReference;

	FDisArchiveCheckForItemContexts() : m_bFoundItemContextReference( FALSE )
	{
		ArIsObjectReferenceCollector = TRUE;
	}

	virtual FArchive& operator<<( UObject*& _pObj )
	{
		if( !m_bFoundItemContextReference )
		{
			m_bFoundItemContextReference = ( _pObj != NULL && _pObj->IsA( UDisItemContext::StaticClass() ) );
		}
		return *this;
	}
};

// DISHONORED(port): 2013 rva 0x7048d0 (2012 0x76f220): a stim is saveable unless serializing it turns up an item
// context, because item contexts are rebuilt on load rather than written out.
UBOOL DisStimIsSaveable( const FAIStimStruct* _pStim )
{
	FDisArchiveCheckForItemContexts Ar;
	DisStimSerialize( const_cast< FAIStimStruct* >( _pStim ), Ar );
	return !Ar.m_bFoundItemContextReference;
}

/*-----------------------------------------------------------------------------
	FDisStimRef's reference count (retail's disstimref.cpp; the header says why it is here).
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x701800 (2012 0x764ee0)
void DisStimRefInit( FDisStimRef& _rRef, const FAIStimStruct* _pStim, FLOAT _fDelayTime )
{
	_rRef.m_pStim = (FPointer)_pStim;
	_rRef.m_fDelayTimer = _fDelayTime;
	if( _pStim )
	{
		++const_cast< FAIStimStruct* >( _pStim )->m_RefCount;
	}
}

// DISHONORED(port): 2013 rva 0x70cf70 (2012 0x76fca0): the count is post-decremented and tested against 1, so the
// holder that took it to zero destructs the stim and hands its block back. A stim with no manager was never pooled.
void DisStimRefRelease( FDisStimRef& _rRef )
{
	FAIStimStruct* pStim = (FAIStimStruct*)_rRef.m_pStim;
	if( !pStim )
	{
		return;
	}
	if( pStim->m_RefCount-- == 1 )
	{
		UDisStimManager* pStimManager = pStim->m_pStimManager;
		if( pStimManager )
		{
			DisStimDestruct( pStim );
			pStimManager->ReleaseBlock( pStim );
		}
	}
	_rRef.m_pStim = NULL;
}

/*-----------------------------------------------------------------------------
	The type table: one row per EAIStimID, which is retail's FAIStimTypeInfoRegister set.

	Generated from the 120 reflected FAIStimStruct_* structs of dishonoredgameclasses.h paired with EAIStimID by name
	(build/agentCG_work/stim_pairs.txt). All 120 matched; EAIStimID_Terrorized is the only id with no struct of its
	own and is raised with the base FAIStimStruct.
	A stim that needs one of the five virtuals overridden replaces its DIS_IMPLEMENT_STIM line with
	DIS_IMPLEMENT_STIM_VTABLE and its own table. None of those overrides exists yet: they live in the behaviour and
	attention units, which are still skeletons.
-----------------------------------------------------------------------------*/

DIS_IMPLEMENT_STIM( FAIStimStruct_ActivateAlarm, EAIStimID_ActivateAlarm )
DIS_IMPLEMENT_STIM( FAIStimStruct_ActorTamperedWith, EAIStimID_ActorTamperedWith )
DIS_IMPLEMENT_STIM( FAIStimStruct_Alarm, EAIStimID_Alarm )
DIS_IMPLEMENT_STIM( FAIStimStruct_AllyBusted, EAIStimID_AllyBusted )
DIS_IMPLEMENT_STIM( FAIStimStruct_AmbushAbortRequest, EAIStimID_AmbushAbortRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_AmbushRequest, EAIStimID_AmbushRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_AttackedByEnemy, EAIStimID_AttackedByEnemy )
DIS_IMPLEMENT_STIM( FAIStimStruct_AttentionBehaviorBegin, EAIStimID_AttentionBehaviorBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_BattleVictory, EAIStimID_BattleVictory )
DIS_IMPLEMENT_STIM( FAIStimStruct_BehaviorAbort, EAIStimID_BehaviorAbort )
DIS_IMPLEMENT_STIM( FAIStimStruct_BlockingDialogEnd, EAIStimID_BlockingDialogEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_BlockingDialogRequest, EAIStimID_BlockingDialogRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_BrainInit, EAIStimID_BrainInit )
DIS_IMPLEMENT_STIM( FAIStimStruct_CarryCorpseOrBodyPart, EAIStimID_CarryingCorpseOrBodyPart )
DIS_IMPLEMENT_STIM( FAIStimStruct_ClearAllMinAttention, EAIStimID_ClearAllMinAttention )
DIS_IMPLEMENT_STIM( FAIStimStruct_ClearAttention, EAIStimID_ClearAttention )
DIS_IMPLEMENT_STIM( FAIStimStruct_ClearMinAttentionForTarget, EAIStimID_ClearMinAttentionForTarget )
DIS_IMPLEMENT_STIM( FAIStimStruct_CombatBegin, EAIStimID_CombatBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_CombatEnd, EAIStimID_CombatEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_CombatEngageRejected, EAIStimID_CombatEngageRejected )
DIS_IMPLEMENT_STIM( FAIStimStruct_CombatToSearch, EAIStimID_CombatToSearch )
DIS_IMPLEMENT_STIM( FAIStimStruct_CoordinatedAttackRequest, EAIStimID_CoordinatedAttackRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_DLC06_OnAssassummon, EAIStimID_DLC06_OnAssassummon )
DIS_IMPLEMENT_STIM( FAIStimStruct_DLC06_SmokeBombed, EAIStimID_DLC06_SmokeBombed )
DIS_IMPLEMENT_STIM( FAIStimStruct_DLC06_WitnessSilentExplosion, EAIStimID_DLC06_WitnessSilentExplosion )
DIS_IMPLEMENT_STIM( FAIStimStruct_DLC07_Pulled, EAIStimID_DLC07_Pulled )
DIS_IMPLEMENT_STIM( FAIStimStruct_DLC07_Resurrected, EAIStimID_DLC07_Resurrected )
DIS_IMPLEMENT_STIM( FAIStimStruct_DLC07_SightedRiverKrust, EAIStimID_DLC07_SightedRiverKrust )
DIS_IMPLEMENT_STIM( FAIStimStruct_DeathByWoL, EAIStimID_DeathByWoL )
DIS_IMPLEMENT_STIM( FAIStimStruct_DestinationReached, EAIStimID_DestinationReached )
DIS_IMPLEMENT_STIM( FAIStimStruct_DialogAttentionChange, EAIStimID_DialogAttentionChange )
DIS_IMPLEMENT_STIM( FAIStimStruct_DifficultyChanged, EAIStimID_DifficultyChanged )
DIS_IMPLEMENT_STIM( FAIStimStruct_DiscoveredCorpse, EAIStimID_DiscoveredCorpse )
DIS_IMPLEMENT_STIM( FAIStimStruct_Distracted_Anim, EAIStimID_Distracted_Anim )
DIS_IMPLEMENT_STIM( FAIStimStruct_Distracted_HeadLook, EAIStimID_Distracted_HeadLook )
DIS_IMPLEMENT_STIM( FAIStimStruct_DocileRatIsNear, EAIStimID_DocileRatIsNear )
DIS_IMPLEMENT_STIM( FAIStimStruct_DoorUsedByPlayer, EAIStimID_DoorUsedByPlayer )
DIS_IMPLEMENT_STIM( FAIStimStruct_DoorUsedByPlayerWhileWary, EAIStimID_DoorUsedByPlayerWhileWary )
DIS_IMPLEMENT_STIM( FAIStimStruct_EndDistracted, EAIStimID_EndDistracted )
DIS_IMPLEMENT_STIM( FAIStimStruct_EndPossession, EAIStimID_EndPossession )
DIS_IMPLEMENT_STIM( FAIStimStruct_EnemyBusted, EAIStimID_EnemyBusted )
DIS_IMPLEMENT_STIM( FAIStimStruct_EscapedBeingChoked, EAIStimID_EscapedBeingChoked )
DIS_IMPLEMENT_STIM( FAIStimStruct_EscapedBeingPossessed, EAIStimID_EscapedBeingPossessed )
DIS_IMPLEMENT_STIM( FAIStimStruct_EvadedMelee_Incoming, EAIStimID_EvadedMelee_Incoming )
DIS_IMPLEMENT_STIM( FAIStimStruct_EvadedMelee_Outgoing, EAIStimID_EvadedMelee_Outgoing )
DIS_IMPLEMENT_STIM( FAIStimStruct_Explosion, EAIStimID_Explosion )
DIS_IMPLEMENT_STIM( FAIStimStruct_FollowRequest, EAIStimID_FollowRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_ForceRingAlarm, EAIStimID_ForceRingAlarm )
DIS_IMPLEMENT_STIM( FAIStimStruct_GoToRequest, EAIStimID_GoToRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_GuardRequest, EAIStimID_GuardRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_HeadTrackBegin, EAIStimID_HeadTrackBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_HeadTrackEnd, EAIStimID_HeadTrackEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_HeardCorpseSplat, EAIStimID_HeardCorpseSplat )
DIS_IMPLEMENT_STIM( FAIStimStruct_HeardSomething, EAIStimID_HeardSomething )
DIS_IMPLEMENT_STIM( FAIStimStruct_Help, EAIStimID_Help )
DIS_IMPLEMENT_STIM( FAIStimStruct_HelpFromRats, EAIStimID_HelpFromRats )
DIS_IMPLEMENT_STIM( FAIStimStruct_IdleRequest, EAIStimID_IdleRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_ImpendingExplosion, EAIStimID_ImpendingExplosion )
DIS_IMPLEMENT_STIM( FAIStimStruct_IncomingDamage, EAIStimID_IncomingDamage )
DIS_IMPLEMENT_STIM( FAIStimStruct_InhibitBegin, EAIStimID_InhibitBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_InhibitEnd, EAIStimID_InhibitEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_InteractBegin, EAIStimID_InteractBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_InteractEnd, EAIStimID_InteractEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_Intimidated, EAIStimID_Intimidated )
DIS_IMPLEMENT_STIM( FAIStimStruct_ItemContext_End, EAIStimID_ItemContext_End )
DIS_IMPLEMENT_STIM( FAIStimStruct_ItemContext_Start, EAIStimID_ItemContext_Start )
DIS_IMPLEMENT_STIM( FAIStimStruct_LocoPushedByPlayer, EAIStimID_LocoPushedByPlayer )
DIS_IMPLEMENT_STIM( FAIStimStruct_MaxOutAttention, EAIStimID_MaxOutAttention )
DIS_IMPLEMENT_STIM( FAIStimStruct_MeleeConnected_Outgoing, EAIStimID_MeleeConnected_Outgoing )
DIS_IMPLEMENT_STIM( FAIStimStruct_NPCReachAttentionLevel, EAIStimID_NPCReachAttentionLevel )
DIS_IMPLEMENT_STIM( FAIStimStruct_NoMoreWolfHoundInFight, EAIStimID_NoMoreWolfHoundInFight )
DIS_IMPLEMENT_STIM( FAIStimStruct_NoticeBegin, EAIStimID_NoticeBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_NoticeEnd, EAIStimID_NoticeEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_NoticeRequest, EAIStimID_NoticeRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_Panicked, EAIStimID_Panicked )
DIS_IMPLEMENT_STIM( FAIStimStruct_PathingFail, EAIStimID_PathingFail )
DIS_IMPLEMENT_STIM( FAIStimStruct_PathingSuccess, EAIStimID_PathingSuccess )
DIS_IMPLEMENT_STIM( FAIStimStruct_PatrolRequest, EAIStimID_PatrolRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_PatrolSearchRequest, EAIStimID_PatrolSearchRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_PlagueZone, EAIStimID_PlagueZone )
DIS_IMPLEMENT_STIM( FAIStimStruct_PlayerHideoutTransition, EAIStimID_PlayerHideoutTransition )
DIS_IMPLEMENT_STIM( FAIStimStruct_PlayerMessingWithActor, EAIStimID_PlayerMessingWithActor )
DIS_IMPLEMENT_STIM( FAIStimStruct_PlayerUsed, EAIStimID_PlayerUsed )
DIS_IMPLEMENT_STIM( FAIStimStruct_ProjectileLaunched, EAIStimID_ProjectileLaunched )
DIS_IMPLEMENT_STIM( FAIStimStruct_PsychicAttentionDisabled, EAIStimID_PsychicAttentionDisabled )
DIS_IMPLEMENT_STIM( FAIStimStruct_PsychicAttentionDisabled_AllTargets, EAIStimID_PsychicAttentionDisabled_AllTargets )
DIS_IMPLEMENT_STIM( FAIStimStruct_PsychicAttentionEnabled, EAIStimID_PsychicAttentionEnabled )
DIS_IMPLEMENT_STIM( FAIStimStruct_ReachabilityChange, EAIStimID_ReachabilityChange )
DIS_IMPLEMENT_STIM( FAIStimStruct_ReactionRequest, EAIStimID_ReactionRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_RelationshipChanged, EAIStimID_RelationshipChanged )
DIS_IMPLEMENT_STIM( FAIStimStruct_ReturnTravelRequest, EAIStimID_ReturnTravelRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_RotationReached, EAIStimID_RotationReached )
DIS_IMPLEMENT_STIM( FAIStimStruct_ScrambleRequest, EAIStimID_ScrambleRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_SearchBegin, EAIStimID_SearchBegin )
DIS_IMPLEMENT_STIM( FAIStimStruct_SearchEnd, EAIStimID_SearchEnd )
DIS_IMPLEMENT_STIM( FAIStimStruct_SearchReachedProxy, EAIStimID_SearchReachedProxy )
DIS_IMPLEMENT_STIM( FAIStimStruct_SearchRequest, EAIStimID_SearchRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_SetMinAttentionForTarget, EAIStimID_SetMinAttentionForTarget )
DIS_IMPLEMENT_STIM( FAIStimStruct_ShootRequest, EAIStimID_ShootRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_ShownRangedThreat, EAIStimID_ShownRangedThreat )
DIS_IMPLEMENT_STIM( FAIStimStruct_SightedRatSwarm, EAIStimID_SightedRatSwarm )
DIS_IMPLEMENT_STIM( FAIStimStruct_Soiree, EAIStimID_Soiree )
DIS_IMPLEMENT_STIM( FAIStimStruct_SoireeRejected, EAIStimID_SoireeRejected )
DIS_IMPLEMENT_STIM( FAIStimStruct_Stolen, EAIStimID_Stolen )
DIS_IMPLEMENT_STIM( FAIStimStruct_SuspicionLevelChanged, EAIStimID_SuspicionLevelChanged )
DIS_IMPLEMENT_STIM( FAIStimStruct_TargetSighted, EAIStimID_TargetSighted )
DIS_IMPLEMENT_STIM( FAIStimStruct_TargetTouched, EAIStimID_TargetTouched )
DIS_IMPLEMENT_STIM( FAIStimStruct_TargetUnsighted, EAIStimID_TargetUnsighted )
DIS_IMPLEMENT_STIM( FAIStimStruct_Teleported, EAIStimID_Teleported )
DIS_IMPLEMENT_STIM( FAIStimStruct_TetherRequest, EAIStimID_TetherRequest )
DIS_IMPLEMENT_STIM( FAIStimStruct_TopAttnProxyReplaced, EAIStimID_TopAttnProxyReplaced )
DIS_IMPLEMENT_STIM( FAIStimStruct_TopAttnProxyUpdated, EAIStimID_TopAttnProxyUpdated )
DIS_IMPLEMENT_STIM( FAIStimStruct_TouchedAlly, EAIStimID_TouchedAlly )
DIS_IMPLEMENT_STIM( FAIStimStruct_TouchedEnemy, EAIStimID_TouchedEnemy )
DIS_IMPLEMENT_STIM( FAIStimStruct_TouchedNeutral, EAIStimID_TouchedNeutral )
DIS_IMPLEMENT_STIM( FAIStimStruct_Traveled, EAIStimID_Traveled )
DIS_IMPLEMENT_STIM( FAIStimStruct_WitnessDeath, EAIStimID_WitnessDeath )
DIS_IMPLEMENT_STIM( FAIStimStruct_WitnessMagic, EAIStimID_WitnessMagic )
DIS_IMPLEMENT_STIM( FAIStimStruct_WitnessPickpocket, EAIStimID_WitnessPickpocket )
DIS_IMPLEMENT_STIM( FAIStimStruct_WitnessShadowKill, EAIStimID_WitnessShadowKill )
