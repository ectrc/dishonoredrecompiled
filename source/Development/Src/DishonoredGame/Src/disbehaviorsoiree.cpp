// DishonoredGame/src/disbehaviorsoiree.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (30):
//   0x724aa0  public: static void __cdecl UDisTweaks_AIBehavior_Soiree::InitializePrivateStaticClassUDisTweaks_AIBehavior_Soiree(void)
//   0x724ac0  public: virtual void __thiscall UDisBehaviorSoiree::OnBehaviorStart(void)
//   0x724ad0  private: virtual unsigned char const * __thiscall UDisBehaviorSoiree::BuildFilterStimMask(void)const
//   0x724ba0  private: virtual unsigned char const * __thiscall UDisBehaviorSoiree::BuildEvaluateStimMask(void)const
//   0x724be0  private: unsigned int __thiscall UDisBehaviorSoiree::FilterBlockingDialogRequest(struct FAIStimStruct_BlockingDialogRequest const &)
//   0x724c10  private: void __thiscall UDisBehaviorSoiree::SetupFromSoiree(struct FAIStimStruct_Soiree const &)
//   0x728f40  public: virtual void __thiscall UDisBehaviorSoiree::OnBehaviorStop(unsigned int)
//   0x728fd0  private: virtual unsigned char const * __thiscall UDisBehaviorSoiree::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x72c340  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorSoiree::GetSetupFromStimDelegate(enum EAIStimID)
//   0x736cb0  public: static class UClass * __cdecl UDisBehaviorSoiree::GetPrivateStaticClassUDisBehaviorSoiree(wchar_t const *)
//   0x738c80  public: static void __cdecl UDisBehaviorSoiree::InitializePrivateStaticClassUDisBehaviorSoiree(void)
//   0x73a8a0  public: static class UClass * __cdecl UDisBehaviorSoiree::StaticClassNoInline(void)
//   0x73a8d0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Soiree::GetPrivateStaticClassUDisTweaks_AIBehavior_Soiree(wchar_t const *)
//   0x73b060  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Soiree::StaticClassNoInline(void)
//   0x73fb00  public: enum ESoireeAIPriority __thiscall UDisBehaviorSoiree::GetSoireePriority(void)const
//   0x741ea0  public: virtual void __thiscall UDisBehaviorSoiree::OnBehaviorResume(void)
//   0x741f20  private: unsigned int __thiscall UDisBehaviorSoiree::FilterSoiree(struct FAIStimStruct_Soiree const &)
//   0x741fa0  private: unsigned int __thiscall UDisBehaviorSoiree::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x741ff0  private: unsigned int __thiscall UDisBehaviorSoiree::FilterBlockAtHighPriority(struct FAIStimStruct const &)
//   0x742050  private: unsigned int __thiscall UDisBehaviorSoiree::FilterFinishAtLowPriority(struct FAIStimStruct const &)
//   0x7420b0  private: unsigned int __thiscall UDisBehaviorSoiree::EvaluateSoiree(struct FAIStimStruct_Soiree const &)const
//   0x742110  private: unsigned int __thiscall UDisBehaviorSoiree::BehaviorShouldFinish(void)const
//   0x742170  private: virtual enum eDisDialogAvailability __thiscall UDisBehaviorSoiree::GetDialogAvailability(enum eDisConvPriority, int)const
//   0x7421c0  private: virtual unsigned int __thiscall UDisBehaviorSoiree::CanBlockSoiree(class FGuid, enum ESoireeAIPriority)const
//   0x742210  private: virtual unsigned int __thiscall UDisBehaviorSoiree::CanBeSurprised(void)const
//   0x743090  private: unsigned int __thiscall UDisBehaviorSoiree::FilterReactionRequest(struct FAIStimStruct_ReactionRequest const &)
//   0x7430e0  private: unsigned int __thiscall UDisBehaviorSoiree::FilterWitnessMagic(struct FAIStimStruct_WitnessMagic const &)
//   0x743190  private: virtual unsigned int __thiscall UDisBehaviorSoiree::IsBehaviorFinished(void)const
//   0x743b50  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorSoiree::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x7448e0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorSoiree::GetFilterStimDelegate(enum EAIStimID)

// ---- agent IC ports (PHASE23 IC): the soiree behaviour, from the 2013 bodies ----
//
// 2013 VAs (2012 rvas above). Named in 2013: OnBehaviorStop 0xae78f0, OnBehaviorResume 0xafafd0, FilterSoiree 0xafb050,
// FilterGoToRequest 0xafb0d0, FilterBlockAtHighPriority 0xafb120, FilterFinishAtLowPriority 0xafb180, EvaluateSoiree 0xafb200,
// BehaviorShouldFinish 0xafb260, GetDialogAvailability 0xafb2c0, CanBlockSoiree 0xafb310, CanBeSurprised 0xafb360,
// FilterWitnessMagic 0xafd880, IsBehaviorFinished 0xafd930, GetFilterStimDelegate 0xaff360, GetEvaluateStimDelegate 0xafee20,
// GetSetupFromStimDelegate 0xae9f90, BuildBehaviorFilterStimMasks 0xaef6d0, SetupFromSoiree 0xae45b0,
// FilterBlockingDialogRequest 0xae45a0, GetSoireePriority 0xaf4d90. Unnamed, found through ??_7UDisBehaviorSoiree@@6BUObject@@@
// (0x11270a0) against UDisAIBehaviorWithDesires' table: OnBehaviorStart 0xae4590 (slot 88), BuildEvaluateStimMask 0xae7a50
// (slot 80), BuildFilterStimMask 0xae7980 (slot 82), TickBehavior 0xaf4d20 (slot 96 - new in 2013, the base's TickBehavior slot
// as UDisBehaviorWeep's 0xae8150 shows); IDisDesiresInterface table 0x112704c: slot 6 = the folded `return &m_BodyIntentionRequest`
// (0xae4060), slot 13 = UDisBehaviorBlockingDialog::GetResumingBodyIntentionDesire 0xac2700 (folded, the same body).
//
// 2013 against 2012: the reaction filters no longer bark at once - ActorTamperedWith / DeathByWoL / ReactionRequest store the
// hook in m_PendingBark (DDH_INVALID = none) and TickBehavior fires it once the pawn is back in UStateNPCMasterWalk;
// NoticeBegin / SearchBegin left the filter list and TopAttnProxyUpdated (relationship Enemy) finishes a low-priority soiree.
//
// DISHONORED(bringup): named, not ported here -
//   GetDialogAvailability (0xafb2c0: DDA_REJECTED_BY_AI_BEHAVIOR when the dialog asks for flags 1|2 at priority 0) - this tree's
//     UDishonoredAIBehavior declares no GetDialogAvailability slot (the base's CallGetDialogAvailability is unported, as GoTo's);
//   the reaction filters bind retail's UDisBehaviorBlockingDialog::FilterReactions (0xac25e0) on the soiree object - an
//     identical-code fold; the same body is UDisBehaviorSoiree::FilterReactions here.

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "dishonoredutilities_ai.h"
#include "icdiag.h"

// The game event retail raises on a soiree start (OnBehaviorResume) and stop (OnBehaviorStop): FArkGameEvent type 23 with the
// pawn as instigator and FDisGameEventNPCSoiree_Params (one dword, the ESoireeEvent) as its parameters.
enum { DisGameEventType_NPCSoiree = 23 };

struct FDisGameEventNPCSoiree_Params
{
	INT m_SoireeStatus;
};

static void DisRaiseNPCSoireeEvent( ADishonoredNPCPawn* _pPawn, INT _SoireeStatus )
{
	FArkGameEventDispatcher* Dispatcher = FArkGameEventDispatcher::GetInstance();
	if( !Dispatcher )
	{
		return;
	}
	FDisGameEventNPCSoiree_Params Params;
	Params.m_SoireeStatus = _SoireeStatus;
	Dispatcher->ProcessEvent( FArkGameEvent( DisGameEventType_NPCSoiree, &Params, _pPawn ) );
}

static UBOOL DisSoireeStimAlwaysTrue( void* /*_pObject*/, const FAIStimStruct& /*_rStim*/ )
{
	return TRUE;
}

const UDisTweaks_AIBehavior_Soiree* UDisBehaviorSoiree::GetSoireeTweaks() const
{
	const UDisTweaks_AIBehavior_Soiree* Tweaks = Cast<UDisTweaks_AIBehavior_Soiree>( const_cast<UDisBehaviorSoiree*>( this )->GetTweaks_Derived() );
	return Tweaks ? Tweaks : (const UDisTweaks_AIBehavior_Soiree*)UDisTweaks_AIBehavior_Soiree::StaticClass()->GetDefaultObject();
}

// 0xaf4d90
BYTE UDisBehaviorSoiree::GetSoireePriority() const
{
	return GetSoireeTweaks()->m_SoireePriority;
}

// 0xae4590
void UDisBehaviorSoiree::OnBehaviorStart()
{
	m_bShouldForceFinish = FALSE;
	m_PendingBark = DDH_INVALID;
}

// 0xae78f0: the stop event, foot placement back on, and every matinee of this soiree is left
void UDisBehaviorSoiree::OnBehaviorStop( UBOOL _bIsBeingTerminated )
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
	DisRaiseNPCSoireeEvent( Pawn, ESE_Stop );
	Pawn->m_bAllowFootPlacement = TRUE;
	Pawn->ExitFromMatinees( m_SoireeGUID, NULL );
	if( DisICDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): icdiag soiree %s stop prio %d terminated %d forced %d controllers %d world %.2f"),
			*Pawn->GetName(), GetSoireePriority(), _bIsBeingTerminated ? 1 : 0, m_bShouldForceFinish ? 1 : 0,
			Pawn->m_MatineeControllersInfos.Num(), GWorld->GetTimeSeconds() );
	}
}

// 0xafafd0: the start event; retail's trailing GetMatineeControllerCount result is unused
void UDisBehaviorSoiree::OnBehaviorResume()
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
	DisRaiseNPCSoireeEvent( Pawn, ESE_Start );
	if( DisICDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): icdiag soiree %s resume prio %d controllers %d world %.2f"), *Pawn->GetName(),
			GetSoireePriority(), Pawn->GetMatineeControllerCount( GetSoireePriority(), FALSE ), GWorld->GetTimeSeconds() );
	}
}

// 0xaf4d20: a bark held back by a reaction filter is played once the pawn walks again
void UDisBehaviorSoiree::TickBehavior( FLOAT _fDeltaSeconds )
{
	if( m_PendingBark == DDH_INVALID )
	{
		return;
	}
	ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
	if( Pawn->m_pNPCMasterFSM->IsCurState( UStateNPCMasterWalk::StaticClass(), TRUE ) )
	{
		FireDialogHook( (eDisDialogHook)m_PendingBark, Pawn );
		m_PendingBark = DDH_INVALID;
	}
}

// 0xafb260
UBOOL UDisBehaviorSoiree::BehaviorShouldFinish() const
{
	const INT ControllerCount = m_pOwningBrain->m_pOwningPawn->GetMatineeControllerCount( GetSoireePriority(), FALSE );
	return m_bShouldForceFinish || ControllerCount == 0;
}

// 0xafd930
UBOOL UDisBehaviorSoiree::IsBehaviorFinished() const
{
	return BehaviorShouldFinish();
}

// 0xafb310
UBOOL UDisBehaviorSoiree::CanBlockSoiree( FGuid _SoireeGuid, BYTE _Priority ) const
{
	return GetSoireePriority() == ESAP_HighPriority && _Priority != ESAP_HighPriority;
}

// 0xafb360
UBOOL UDisBehaviorSoiree::CanBeSurprised() const
{
	return GetSoireePriority() != ESAP_HighPriority;
}

// 0xae7a50
const BYTE* UDisBehaviorSoiree::BuildEvaluateStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_Soiree };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// 0xae7980: the nineteen stims GetFilterStimDelegate binds
const BYTE* UDisBehaviorSoiree::BuildFilterStimMask()
{
	static const BYTE StimIDs[] =
	{
		EAIStimID_Soiree, EAIStimID_GoToRequest, EAIStimID_Stolen, EAIStimID_ReactionRequest, EAIStimID_WitnessMagic,
		EAIStimID_TopAttnProxyUpdated, EAIStimID_SightedRatSwarm, EAIStimID_Panicked, EAIStimID_ImpendingExplosion,
		EAIStimID_PlagueZone, EAIStimID_InteractBegin, EAIStimID_IncomingDamage, EAIStimID_EvadedMelee_Incoming,
		EAIStimID_BlockingDialogRequest, EAIStimID_Alarm, EAIStimID_DeathByWoL, EAIStimID_ActorTamperedWith,
		EAIStimID_DocileRatIsNear, EAIStimID_Distracted_Anim
	};
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// 0xaef6d0
const BYTE* UDisBehaviorSoiree::BuildBehaviorFilterStimMasks( const BYTE*& _rOutSubProcessesMask, const BYTE*& _rOutCompleteMask )
{
	static BYTE s_SubProcessesMask[EAIStimID_MAX];
	static BYTE s_CompleteMask[EAIStimID_MAX];
	static UBOOL s_bInitialized = FALSE;
	const BYTE* Mask = BuildFilterStimMask();
	if( !s_bInitialized )
	{
		if( Mask )
		{
			appMemcpy( s_CompleteMask, Mask, sizeof(s_CompleteMask) );
		}
		BuildInternalFilterStimMasks( s_SubProcessesMask, s_CompleteMask );
		s_bInitialized = TRUE;
	}
	_rOutSubProcessesMask = s_SubProcessesMask;
	_rOutCompleteMask = s_CompleteMask;
	return Mask;
}

// 0xafee20
FDisStimPredicateDelegate UDisBehaviorSoiree::GetEvaluateStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_Soiree )
	{
		return DIS_BIND_STIM_PREDICATE_CONST( UDisBehaviorSoiree, FAIStimStruct_Soiree, EvaluateSoiree );
	}
	return FDisStimPredicateDelegate();
}

// 0xae9f90
FDisStimSetupDelegate UDisBehaviorSoiree::GetSetupFromStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_Soiree )
	{
		return DIS_BIND_STIM_SETUP( UDisBehaviorSoiree, FAIStimStruct_Soiree, SetupFromSoiree );
	}
	return FDisStimSetupDelegate();
}

// 0xaff360
FDisStimPredicateDelegate UDisBehaviorSoiree::GetFilterStimDelegate( BYTE _StimID )
{
	switch( _StimID )
	{
	case EAIStimID_ActorTamperedWith:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct, FilterActorTamperedWith );
	case EAIStimID_Alarm:
	case EAIStimID_EvadedMelee_Incoming:
	case EAIStimID_ImpendingExplosion:
	case EAIStimID_IncomingDamage:
	case EAIStimID_InteractBegin:
	case EAIStimID_Panicked:
	case EAIStimID_PlagueZone:
	case EAIStimID_SightedRatSwarm:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct, FilterReactions );
	case EAIStimID_BlockingDialogRequest:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct_BlockingDialogRequest, FilterBlockingDialogRequest );
	case EAIStimID_DeathByWoL:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct, FilterDeathByWoL );
	case EAIStimID_Distracted_Anim:
	case EAIStimID_DocileRatIsNear:
		return FDisStimPredicateDelegate( NULL, &DisSoireeStimAlwaysTrue );
	case EAIStimID_GoToRequest:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct_GoToRequest, FilterGoToRequest );
	case EAIStimID_ReactionRequest:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct_ReactionRequest, FilterReactionRequest );
	case EAIStimID_Soiree:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct_Soiree, FilterSoiree );
	case EAIStimID_Stolen:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct, FilterBlockAtHighPriority );
	case EAIStimID_TopAttnProxyUpdated:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct_TopAttnProxyUpdated, FilterTopAttnProxyUpdated );
	case EAIStimID_WitnessMagic:
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorSoiree, FAIStimStruct_WitnessMagic, FilterWitnessMagic );
	default:
		return FDisStimPredicateDelegate();
	}
}

// 0xafb200: a non-blocking start at this behaviour's priority activates it
UBOOL UDisBehaviorSoiree::EvaluateSoiree( const FAIStimStruct_Soiree& _rSoireeStim ) const
{
	if( _rSoireeStim.m_SoireeEvent != ESE_Start || _rSoireeStim.m_bIsBlocking )
	{
		return FALSE;
	}
	return GetSoireePriority() == _rSoireeStim.m_SoireePriority;
}

// 0xae45b0
void UDisBehaviorSoiree::SetupFromSoiree( const FAIStimStruct_Soiree& _rSoireeStim )
{
	m_SoireeGUID = _rSoireeStim.m_SoireeGUID;
}

// 0xafb050: another start at the same priority is taken over (its matinee becomes this one), a start at another priority
// finishes this soiree, a stop is swallowed - the controller list, not the stim, ends the behaviour
UBOOL UDisBehaviorSoiree::FilterSoiree( const FAIStimStruct_Soiree& _rSoireeStim )
{
	if( _rSoireeStim.m_bIsBlocking )
	{
		return FALSE;
	}
	if( _rSoireeStim.m_SoireeEvent != ESE_Start )
	{
		return _rSoireeStim.m_SoireeEvent == ESE_Stop;
	}
	const UBOOL bSamePriority = _rSoireeStim.m_SoireePriority == GetSoireePriority();
	if( bSamePriority )
	{
		m_SoireeGUID = _rSoireeStim.m_SoireeGUID;
	}
	else
	{
		m_bShouldForceFinish = TRUE;
	}
	return bSamePriority;
}

// 0xafb0d0
UBOOL UDisBehaviorSoiree::FilterGoToRequest( const FAIStimStruct_GoToRequest& _rStim )
{
	if( GetSoireePriority() != ESAP_HighPriority )
	{
		m_bShouldForceFinish = TRUE;
	}
	return FALSE;
}

// 0xae45a0
UBOOL UDisBehaviorSoiree::FilterBlockingDialogRequest( const FAIStimStruct_BlockingDialogRequest& _rStim )
{
	m_bShouldForceFinish = TRUE;
	return FALSE;
}

// 0xafb120: a high-priority soiree swallows the stim; a lower one only while the matinee disables reactions
UBOOL UDisBehaviorSoiree::FilterBlockAtHighPriority( const FAIStimStruct& _rStim )
{
	if( GetSoireePriority() == ESAP_HighPriority )
	{
		return TRUE;
	}
	return m_pOwningBrain->m_pOwningPawn->AreReactionsDisabled();
}

// 0xafb180
UBOOL UDisBehaviorSoiree::FilterFinishAtLowPriority( const FAIStimStruct& _rStim )
{
	if( GetSoireePriority() != ESAP_HighPriority && !m_pOwningBrain->m_pOwningPawn->AreReactionsDisabled() )
	{
		m_bShouldForceFinish = TRUE;
	}
	return FALSE;
}

// 0xac25e0 (UDisBehaviorBlockingDialog::FilterReactions, bound here by the fold)
UBOOL UDisBehaviorSoiree::FilterReactions( const FAIStimStruct& _rStim )
{
	return m_pOwningBrain->m_pOwningPawn->AreReactionsDisabled();
}

// 0xafd840
UBOOL UDisBehaviorSoiree::FilterActorTamperedWith( const FAIStimStruct& _rStim )
{
	const UBOOL bBlocked = FilterBlockAtHighPriority( _rStim );
	if( bBlocked )
	{
		m_PendingBark = DDH_NOTICE_BROKEN_STUFF;
	}
	return bBlocked;
}

// 0xafd860
UBOOL UDisBehaviorSoiree::FilterDeathByWoL( const FAIStimStruct& _rStim )
{
	const UBOOL bBlocked = FilterBlockAtHighPriority( _rStim );
	if( bBlocked )
	{
		m_PendingBark = DDH_NOTICE_DEATH_BY_WOL;
	}
	return bBlocked;
}

// 0xafd810
UBOOL UDisBehaviorSoiree::FilterReactionRequest( const FAIStimStruct_ReactionRequest& _rStim )
{
	const UBOOL bBlocked = FilterBlockAtHighPriority( _rStim );
	if( bBlocked )
	{
		m_PendingBark = _rStim.m_eBarkToPlay;
	}
	return bBlocked;
}

// 0xafb1e0
UBOOL UDisBehaviorSoiree::FilterTopAttnProxyUpdated( const FAIStimStruct_TopAttnProxyUpdated& _rStim )
{
	if( _rStim.m_RelationshipToTopAttnProxy == Relationship_Enemy )
	{
		FilterFinishAtLowPriority( _rStim );
	}
	return FALSE;
}

// 0xafd880
UBOOL UDisBehaviorSoiree::FilterWitnessMagic( const FAIStimStruct_WitnessMagic& _rStim )
{
	const UBOOL bBlocked = FilterBlockAtHighPriority( _rStim );
	if( !bBlocked )
	{
		return bBlocked;
	}
	if( _rStim.m_bDisappearance )
	{
		FireDialogHook( DDH_MAGIC_DISAPPEARED, _rStim.m_pMagicUser );
		return bBlocked;
	}
	FDisHookParameters* HookParameters = m_pOwningBrain->m_pOwningPawn->GetHookParameters();
	if( HookParameters )
	{
		HookParameters->m_pLastWitnessedMagic = _rStim.m_pActivePowerClass;
	}
	FireDialogHook( DDH_MAGIC_WITNESSED, _rStim.m_pMagicUser );
	return bBlocked;
}

// 0xac2700 (folded with UDisBehaviorBlockingDialog's)
UBOOL UDisBehaviorSoiree::GetResumingBodyIntentionDesire( const FDisBodyIntention& _rPreviousBodyIntention, FDisBodyIntention& _rResumingBodyIntention ) const
{
	return DisContinuePreviousBodyIntention( m_pOwningBrain->m_pOwningPawn, _rPreviousBodyIntention, _rResumingBodyIntention );
}

// agent JE (PHASE24 JE), 2013 0x6fb2c0 (2012 0x742170): a soiree refuses a blocking conversation that carries flag 1 only at the
// high soiree priority (m_SoireePriority 0).
eDisDialogAvailability UDisBehaviorSoiree::GetDialogAvailability( eDisConvPriority _Priority, INT _StartDialogFlags ) const
{
	if( !( _StartDialogFlags & 2 ) || !( _StartDialogFlags & 1 ) )
	{
		return DDA_AVAILABLE;
	}
	const UDisTweaks_AIBehavior_Soiree* Tweaks = Cast<UDisTweaks_AIBehavior_Soiree>( const_cast<UDisBehaviorSoiree*>( this )->GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AIBehavior_Soiree*)UDisTweaks_AIBehavior_Soiree::StaticClass()->GetDefaultObject();
	}
	return Tweaks->m_SoireePriority != 0 ? DDA_AVAILABLE : DDA_REJECTED_BY_AI_BEHAVIOR;
}
