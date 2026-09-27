// DishonoredGame/src/disbehaviorshoot.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (29):
//   0x724920  public: static void __cdecl UDisTweaks_AIBehavior_Shoot::InitializePrivateStaticClassUDisTweaks_AIBehavior_Shoot(void)
//   0x724940  protected: virtual unsigned char const * __thiscall UDisBehaviorShoot::BuildFilterStimMask(void)const
//   0x7249b0  protected: virtual unsigned char const * __thiscall UDisBehaviorShoot::BuildEvaluateStimMask(void)const
//   0x7249f0  protected: virtual unsigned int __thiscall UDisBehaviorShoot::FilterShootRequest(struct FAIStimStruct_ShootRequest const &)
//   0x724a10  private: unsigned int __thiscall UDisBehaviorShoot::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x724a20  protected: virtual unsigned char const * __thiscall UDisBehaviorShoot::BuildShouldFinishWhileDormantStimMask(void)const
//   0x724a70  public: virtual unsigned int __thiscall UDisBehaviorShoot::IsWillingToStartRangedAction(void)const
//   0x724a90  private: virtual struct FDisBodyIntentionRequest * __thiscall UDisBehaviorShoot::GetDesiresBodyIntentionRequest(void)
//   0x728d90  protected: virtual unsigned char const * __thiscall UDisBehaviorShoot::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x728e00  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorShoot::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x728e30  public: virtual void __thiscall UDisBehaviorShoot::RequestStateExitCallback_FirePistol(class UDishonoredNativeState *)
//   0x728e60  protected: virtual void __thiscall UDisBehaviorShoot::OnBehaviorPause(unsigned int)
//   0x728ec0  protected: virtual void __thiscall UDisBehaviorShoot::OnBehaviorResume(void)
//   0x72c2f0  protected: virtual unsigned int __thiscall UDisBehaviorShoot::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x72e700  private: virtual void __thiscall UDisBehaviorShoot::InitBehavior(class UDishonoredAIBrain * const)
//   0x730f20  protected: void __thiscall UDisBehaviorShoot::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x730f70  protected: virtual void __thiscall UDisBehaviorShoot::BeginDestroy(void)
//   0x736c20  public: static class UClass * __cdecl UDisBehaviorShoot::GetPrivateStaticClassUDisBehaviorShoot(wchar_t const *)
//   0x738c60  public: static void __cdecl UDisBehaviorShoot::InitializePrivateStaticClassUDisBehaviorShoot(void)
//   0x73a7e0  public: static class UClass * __cdecl UDisBehaviorShoot::StaticClassNoInline(void)
//   0x73a810  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Shoot::GetPrivateStaticClassUDisTweaks_AIBehavior_Shoot(wchar_t const *)
//   0x73afd0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Shoot::StaticClassNoInline(void)
//   0x73b000  private: unsigned int __thiscall UDisBehaviorShoot::FilterBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)
//   0x73b030  private: unsigned int __thiscall UDisBehaviorShoot::ShouldFinishWhileDormantBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)const
//   0x73f990  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorShoot::GetFilterStimDelegate(enum EAIStimID)
//   0x73fad0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorShoot::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x741d50  private: virtual void __thiscall UDisBehaviorShoot::SetupFromShootRequest(struct FAIStimStruct_ShootRequest const &)
//   0x741e40  public: virtual void __thiscall UDisBehaviorShoot::TickCallback_Stand(class UDishonoredNativeState *, float)
//   0x743060  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorShoot::GetSetupFromStimDelegate(enum EAIStimID)

// ---- agent CG natives sweep, round 2 (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x6e7770 (2012 0x728e30): the exec wrapper, over the C++ body below.
void UDisBehaviorShoot::execRequestStateExitCallback_FirePistol( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_FirePistol( _pThisState );
}

// DISHONORED(port): 2013 rva 0x6e7770 (2012 0x728e30)
void UDisBehaviorShoot::RequestStateExitCallback_FirePistol( class UDishonoredNativeState* _pThisState )
{
	// The shot is over, so the Kismet Shoot action is told how it ended and the behaviour finishes.
	// DISHONORED(bringup): retail reads the outcome out of the FirePistol sub-state's own status byte
	// (UDisAISubStateFirePistol, unported - see agentCG.md's hand-over 1), so the action is always told the default
	// outcome rather than "out of ammo" or "target lost".
	if( m_pOwningBrain && m_pOwningBrain->GetOwningController() )
	{
		m_pOwningBrain->GetOwningController()->SetShootActionStatus( 0 );
	}
	m_bDone = TRUE;
}
