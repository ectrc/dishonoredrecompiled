// DishonoredGame/src/disbehaviorweep.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (50):
//   0x7452e0  public: static void __cdecl UDisBehaviorWeep::InitializePrivateStaticClassUDisBehaviorWeep(void)
//   0x745300  private: virtual unsigned int __thiscall UDisBehaviorWeep::IsBehaviorFinished(void)const
//   0x745330  private: virtual unsigned int __thiscall UDisBehaviorWeep::IsAttacking(class ADishonoredPawn const &)const
//   0x746a10  private: virtual unsigned char const * __thiscall UDisBehaviorWeep::BuildFilterStimMask(void)const
//   0x746a60  private: virtual unsigned char const * __thiscall UDisBehaviorWeep::BuildEvaluateStimMask(void)const
//   0x746a80  private: virtual void __thiscall UDisBehaviorWeep::TickBehavior(float)
//   0x746ad0  private: virtual unsigned int __thiscall UDisBehaviorWeep::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x746ea0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorWeep::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x7485d0  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorWeep::GetSetupFromStimDelegate(enum EAIStimID)
//   0x74c2a0  public: virtual void __thiscall UDisBehaviorWeep::RequestStateExitCallback_Stand(class UDishonoredNativeState *)
//   0x74c2e0  private: void __thiscall UDisBehaviorWeep::MarkWeeperAsRunning(unsigned char)
//   0x74c410  private: void __thiscall UDisBehaviorWeep::StartWeepAnimation(void)
//   0x74efe0  private: virtual unsigned char const * __thiscall UDisBehaviorWeep::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x74f080  private: unsigned int __thiscall UDisBehaviorWeep::FilterItemContext_Start(struct FAIStimStruct_ItemContext_Start const &)
//   0x74f0f0  private: virtual void __thiscall UDisBehaviorWeep::OnBehaviorResume(void)
//   0x750660  private: virtual void __thiscall UDisBehaviorWeep::InitBehavior(class UDishonoredAIBrain * const)
//   0x750920  private: virtual void __thiscall UDisBehaviorWeep::OnBehaviorPause(unsigned int)
//   0x750a20  public: virtual void __thiscall UDisBehaviorWeep::OnExitCallback_TakePosition(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x750a60  public: virtual void __thiscall UDisBehaviorWeep::OnEnterCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x750ad0  private: int __thiscall UDisBehaviorWeep::WakeRunners(int, int)
//   0x756d60  public: static class UClass * __cdecl UDisBehaviorWeep::GetPrivateStaticClassUDisBehaviorWeep(wchar_t const *)
//   0x759be0  public: static class UClass * __cdecl UDisBehaviorWeep::StaticClassNoInline(void)
//   0x75ab80  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Weep::GetPrivateStaticClassUDisTweaks_AIBehavior_Weep(wchar_t const *)
//   0x75c100  public: static void __cdecl UDisTweaks_AIBehavior_Weep::InitializePrivateStaticClassUDisTweaks_AIBehavior_Weep(void)
//   0x75c6a0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Weep::StaticClassNoInline(void)
//   0x75f270  private: void __thiscall UDisBehaviorWeep::OnBeginReachTowardTarget(class UDishonoredNativeState *)
//   0x75f370  private: unsigned int __thiscall UDisBehaviorWeep::IsAllowedToRun(class UDisTweaks_AIBehavior_Weep const *)
//   0x75f4e0  private: void __thiscall UDisBehaviorWeep::ResetCoughingTimer(void)
//   0x75f530  private: void __thiscall UDisBehaviorWeep::OnNpcHActionEnded(class FArkGameEvent const &)
//   0x75f5b0  private: void __thiscall UDisBehaviorWeep::DoReachWeepTarget(class ADishonoredPawn * const, unsigned int, enum eDisReachability)
//   0x75f710  private: void __thiscall UDisBehaviorWeep::DoStandStill(enum EDisWeepSubStates)
//   0x75f850  private: unsigned int __thiscall UDisBehaviorWeep::IsPawnWithinHomeRadius(class ADishonoredPawn const *)const
//   0x75f8f0  private: enum eDisWeepVariation __thiscall UDisBehaviorWeep::ComputeWeepVariation(void)const
//   0x75f970  private: unsigned int __thiscall UDisBehaviorWeep::CheckForArmGrab(unsigned int)
//   0x760950  private: unsigned int __thiscall UDisBehaviorWeep::FilterItemContext_End(struct FAIStimStruct_ItemContext_End const &)
//   0x7609a0  private: unsigned int __thiscall UDisBehaviorWeep::FilterDestinationReached(struct FAIStimStruct_DestinationReached const &)
//   0x7609e0  private: unsigned int __thiscall UDisBehaviorWeep::FilterReachabilityChange(struct FAIStimStruct_ReachabilityChange const &)
//   0x760a60  private: virtual void __thiscall UDisBehaviorWeep::OnBehaviorStart(void)
//   0x760b80  private: virtual void __thiscall UDisBehaviorWeep::OnBehaviorStop(unsigned int)
//   0x760c00  public: virtual void __thiscall UDisBehaviorWeep::OnEnterCallback_TakeActorPosition(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x760c10  public: virtual void __thiscall UDisBehaviorWeep::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x760c50  public: virtual void __thiscall UDisBehaviorWeep::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x760d30  public: virtual void __thiscall UDisBehaviorWeep::RequestStateExitCallback_DoWeaponManoeuver(class UDishonoredNativeState *)
//   0x760d40  private: void __thiscall UDisBehaviorWeep::DoCoughingAnimation(unsigned int)
//   0x760e60  private: void __thiscall UDisBehaviorWeep::TickCoughingTimer(float, unsigned int)
//   0x762350  public: virtual void __thiscall UDisBehaviorWeep::TickCallback_Stand(class UDisAISubState *, float)
//   0x762580  private: void __thiscall UDisBehaviorWeep::TickReachTowardTarget(class UDishonoredNativeState *, float)
//   0x763650  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorWeep::GetFilterStimDelegate(enum EAIStimID)
//   0x763730  public: virtual void __thiscall UDisBehaviorWeep::TickCallback_TakeActorPosition(class UDisAISubState *, float)
//   0x763820  public: virtual void __thiscall UDisBehaviorWeep::TickCallback_TakePosition(class UDisAISubState *, float)

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x6f6580 (2012 0x750a20) and 0x6ed6f0 (2012 0x74c2a0, exec 0x5f6060): both callbacks have
// the same body, which is why identical-code folding gave them one address in the 2013 exe. The weeper's
// ambient-animation sub-process is switched off, and the "is the owning behaviour paused" argument is m_bIsPaused
// (UDishonoredAIBehavior @92 mask 0x2), so ending it while the behaviour is paused does not raise EndSubProcess twice.
static void DisWeepDisableAmbientAnims( UDisBehaviorWeep* _pBehavior )
{
	UDisAISubProcess* pSubProcess = _pBehavior->GetSubProcess( UDisAISubProcessAmbientAnims::StaticClass() );
	if( pSubProcess )
	{
		pSubProcess->DisableSubProcess_Internal( _pBehavior->m_bIsPaused );
	}
}

void UDisBehaviorWeep::execOnExitCallback_TakePosition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pLastState);
	P_FINISH;
	OnExitCallback_TakePosition( _pThisState, _pLastState );
}

void UDisBehaviorWeep::OnExitCallback_TakePosition( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pLastState )
{
	DisWeepDisableAmbientAnims( this );
}

void UDisBehaviorWeep::execRequestStateExitCallback_Stand( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_Stand( _pThisState );
}

void UDisBehaviorWeep::RequestStateExitCallback_Stand( UDishonoredNativeState* _pThisState )
{
	DisWeepDisableAmbientAnims( this );
}
