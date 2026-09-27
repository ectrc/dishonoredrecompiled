// DishonoredGame/src/disbehaviorescapeexplosion.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (30):
//   0x722cd0  public: static void __cdecl UDisTweaks_AIBehavior_EscapeExplosion::InitializePrivateStaticClassUDisTweaks_AIBehavior_EscapeExplosion(void)
//   0x722cf0  public: static void __cdecl UStateNPCEscapeExplosion::InitializePrivateStaticClassUStateNPCEscapeExplosion(void)
//   0x722d10  public: virtual unsigned char const * __thiscall UDisBehaviorEscapeExplosion::BuildFilterStimMask(void)const
//   0x722d60  public: virtual unsigned char const * __thiscall UDisBehaviorEscapeExplosion::BuildEvaluateStimMask(void)const
//   0x722da0  public: unsigned int __thiscall UDisBehaviorEscapeExplosion::FilterExplosion(struct FAIStimStruct_Explosion const &)
//   0x722db0  public: void __thiscall UDisBehaviorEscapeExplosion::SetupFromImpendingExplosion(struct FAIStimStruct_ImpendingExplosion const &)
//   0x722e90  public: virtual void __thiscall UDisBehaviorEscapePlague::OnBehaviorResume(void)
//   0x7242e0  private: virtual struct FDisFaceToRequest * __thiscall UDisBehaviorReact::GetDesiresFaceToRequest(void)
//   0x7280f0  public: virtual unsigned char const * __thiscall UDisBehaviorEscapeExplosion::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x728190  private: virtual unsigned int __thiscall UDisBehaviorEscapeExplosion::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x72b5c0  public: static class UClass * __cdecl UStateNPCEscapeExplosion::GetPrivateStaticClassUStateNPCEscapeExplosion(wchar_t const *)
//   0x72b650  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorEscapeExplosion::GetFilterStimDelegate(enum EAIStimID)
//   0x72b6b0  public: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorEscapeExplosion::GetSetupFromStimDelegate(enum EAIStimID)
//   0x72de90  public: static class UClass * __cdecl UStateNPCEscapeExplosion::StaticClassNoInline(void)
//   0x72dec0  public: virtual void __thiscall UDisBehaviorEscapeExplosion::InitBehavior(class UDishonoredAIBrain * const)
//   0x730350  public: virtual void __thiscall UDisBehaviorEscapeExplosion::OnBehaviorPause(unsigned int)
//   0x7303a0  public: virtual void __thiscall UDisBehaviorEscapeExplosion::RequestStateExitCallback_Flee(class UDishonoredNativeState *)
//   0x7361c0  public: static class UClass * __cdecl UDisBehaviorEscapeExplosion::GetPrivateStaticClassUDisBehaviorEscapeExplosion(wchar_t const *)
//   0x738980  public: static void __cdecl UDisBehaviorEscapeExplosion::InitializePrivateStaticClassUDisBehaviorEscapeExplosion(void)
//   0x739c00  public: static class UClass * __cdecl UDisBehaviorEscapeExplosion::StaticClassNoInline(void)
//   0x739c30  public: static class UClass * __cdecl UDisTweaks_AIBehavior_EscapeExplosion::GetPrivateStaticClassUDisTweaks_AIBehavior_EscapeExplosion(wchar_t const *)
//   0x73ac80  public: static class UClass * __cdecl UDisTweaks_AIBehavior_EscapeExplosion::StaticClassNoInline(void)
//   0x73cb80  public: unsigned int __thiscall UDisBehaviorEscapeExplosion::EvaluateImpendingExplosion(struct FAIStimStruct_ImpendingExplosion const &)const
//   0x73cc30  public: void __thiscall UDisBehaviorEscapeExplosion::DoFleeFromExplosion(void)
//   0x73cc90  public: virtual void __thiscall UDisBehaviorEscapeExplosion::OnEnterCallback_Flee(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x73cd50  public: virtual void __thiscall UDisBehaviorEscapeExplosion::RefreshCallback_Flee(class UDisAISubState *, float)
//   0x73cfa0  public: virtual void __thiscall UDisBehaviorEscapeExplosion::ThreatTerminatedCallback_Flee(class UDisAISubStateFlee *)
//   0x73cfe0  public: virtual void __thiscall UDisBehaviorEscapeExplosion::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x740c10  public: virtual void __thiscall UDisBehaviorEscapeExplosion::OnBehaviorStart(void)
//   0x742af0  public: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorEscapeExplosion::GetEvaluateStimDelegate(enum EAIStimID)const

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x6e90d0 (2012 0x7303a0, exec 0x63b940): the flee sub-state asking to leave finishes the
// behaviour, and the pawn's master FSM is pulled out of its escape-explosion state only when that is the state it is
// logically in. Retail tests the LOGICAL state, not the active one, so a state stacked on top of it is left alone.
void UDisBehaviorEscapeExplosion::execRequestStateExitCallback_Flee( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_Flee( _pThisState );
}

void UDisBehaviorEscapeExplosion::RequestStateExitCallback_Flee( UDishonoredNativeState* _pThisState )
{
	m_bFinished = TRUE;

	if( !m_pOwningBrain || !m_pOwningBrain->m_pOwningPawn || !m_pOwningBrain->m_pOwningPawn->m_pNPCMasterFSM )
	{
		return;
	}
	UDishonoredNativeState* pLogicalState = m_pOwningBrain->m_pOwningPawn->m_pNPCMasterFSM->GetLogicalState();
	if( pLogicalState && pLogicalState->IsA( UStateNPCEscapeExplosion::StaticClass() ) )
	{
		pLogicalState->RequestStateExit();
	}
}

// ---- agent CG natives sweep, round 2 (PHASE9 CG) ----

// DISHONORED(port): 2013 rva 0x6f1970 (2012 0x73cfa0): the exec wrapper, over the C++ body below.
void UDisBehaviorEscapeExplosion::execThreatTerminatedCallback_Flee( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDisAISubStateFlee, _pSubSate);
	P_FINISH;
	ThreatTerminatedCallback_Flee( _pSubSate );
}

// DISHONORED(port): 2013 rva 0x6f1970 (2012 0x73cfa0)
void UDisBehaviorEscapeExplosion::ThreatTerminatedCallback_Flee( class UDisAISubStateFlee* _pSubSate )
{
	// The thing being fled from is gone, so the escape gets its full duration from the tweaks rather than ending at
	// once - an NPC keeps running for a moment after the explosion stops existing.
	UDisTweaks_AIBehavior_EscapeExplosion* Tweaks = Cast<UDisTweaks_AIBehavior_EscapeExplosion>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_AIBehavior_EscapeExplosion*)UDisTweaks_AIBehavior_EscapeExplosion::StaticClass()->GetDefaultObject();
	}
	m_fEscapeEndTimer = Tweaks->m_fEscapeEndTimer;
}
