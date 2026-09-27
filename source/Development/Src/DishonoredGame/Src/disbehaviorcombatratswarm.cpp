// DishonoredGame/src/disbehaviorcombatratswarm.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (23):
//   0x745ba0  public: static void __cdecl UDisBehaviorCombatRatSwarm::InitializePrivateStaticClassUDisBehaviorCombatRatSwarm(void)
//   0x745bc0  public: virtual unsigned int __thiscall UDisBehaviorCombatRatSwarm::IsBehaviorFinished(void)const
//   0x745c20  public: virtual unsigned char const * __thiscall UDisBehaviorCombatRatSwarm::BuildFilterStimMask(void)const
//   0x745c50  public: virtual unsigned char const * __thiscall UDisBehaviorCombatRatSwarm::BuildEvaluateStimMask(void)const
//   0x745c80  private: void __thiscall UDisBehaviorCombatRatSwarm::SetupFromHelpFromRats(struct FAIStimStruct_HelpFromRats const &)
//   0x745ca0  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::OnExitCallback_TakeActorPosition(class UDishonoredNativeState *)
//   0x745cb0  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::RequestStateExitCallback_TakeActorPosition(class UDishonoredNativeState *)
//   0x7471f0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorCombatRatSwarm::GetFilterStimDelegate(enum EAIStimID)
//   0x747230  private: unsigned int __thiscall UDisBehaviorCombatRatSwarm::EvaluateSightedRatSwarm(struct FAIStimStruct_SightedRatSwarm const &)const
//   0x747360  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::RefreshCallback_TakeActorPosition(class UDisAISubState *, float)
//   0x749310  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorCombatRatSwarm::GetSetupFromStimDelegate(enum EAIStimID)
//   0x74d4e0  public: virtual unsigned char const * __thiscall UDisBehaviorCombatRatSwarm::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74d560  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorCombatRatSwarm::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x74fa20  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::InitBehavior(class UDishonoredAIBrain * const)
//   0x752190  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::OnBehaviorStop(unsigned int)
//   0x757800  public: static class UClass * __cdecl UDisBehaviorCombatRatSwarm::GetPrivateStaticClassUDisBehaviorCombatRatSwarm(wchar_t const *)
//   0x759de0  public: static class UClass * __cdecl UDisBehaviorCombatRatSwarm::StaticClassNoInline(void)
//   0x75b4e0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_CombatRatSwarm::GetPrivateStaticClassUDisTweaks_AIBehavior_CombatRatSwarm(wchar_t const *)
//   0x75bdd0  public: static void __cdecl UDisTweaks_AIBehavior_CombatRatSwarm::InitializePrivateStaticClassUDisTweaks_AIBehavior_CombatRatSwarm(void)
//   0x75c340  public: static class UClass * __cdecl UDisTweaks_AIBehavior_CombatRatSwarm::StaticClassNoInline(void)
//   0x75d8b0  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::OnBehaviorStart(void)
//   0x75dae0  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x75db20  public: virtual void __thiscall UDisBehaviorCombatRatSwarm::RefreshCallback_Stand(class UDisAISubState *, float)

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x700ac0 (2012 0x745cb0)
void UDisBehaviorCombatRatSwarm::execRequestStateExitCallback_TakeActorPosition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_TakeActorPosition( _pThisState );
}

void UDisBehaviorCombatRatSwarm::RequestStateExitCallback_TakeActorPosition( UDishonoredNativeState* _pThisState )
{
	m_bIsFinished = TRUE;
}

// DISHONORED(port): 2013 rva 0x700ab0 (2012 0x745ca0): the target actor is cleared on the way out, not on the way in,
// so a rat swarm that re-enters the sub-state picks a fresh target.
void UDisBehaviorCombatRatSwarm::execOnExitCallback_TakeActorPosition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	OnExitCallback_TakeActorPosition( _pThisState );
}

void UDisBehaviorCombatRatSwarm::OnExitCallback_TakeActorPosition( UDishonoredNativeState* _pThisState )
{
	m_pTargetActor = NULL;
}

// ---- agent CG natives sweep, round 2 (PHASE9 CG) ----

// DISHONORED(port): 2013 rva 0x70ea50 (2012 0x75dae0): the exec wrapper, over the C++ body below.
void UDisBehaviorCombatRatSwarm::execOnEnterCallback_Stand( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pLastState);
	P_FINISH;
	OnEnterCallback_Stand( _pThisState, _pLastState );
}

// DISHONORED(port): 2013 rva 0x70ea50 (2012 0x75dae0)
void UDisBehaviorCombatRatSwarm::OnEnterCallback_Stand( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pLastState )
{
	// Standing restarts the group-setup clock from the tweaks, which is how a swarm of rats regroups before it
	// charges again. The tweaks go through GetTweaks_Derived, never a base pointer.
	UDisTweaks_AIBehavior_CombatRatSwarm* Tweaks = Cast<UDisTweaks_AIBehavior_CombatRatSwarm>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_AIBehavior_CombatRatSwarm*)UDisTweaks_AIBehavior_CombatRatSwarm::StaticClass()->GetDefaultObject();
	}
	m_fRemainingCombatGroupSetupTime = Tweaks->m_fMaxCombatGroupSetupTime;
}
