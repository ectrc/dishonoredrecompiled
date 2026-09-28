// DishonoredGame/src/disbehaviornotice.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (24):
//   0x7238c0  public: static void __cdecl UDisTweaks_AIBehavior_Notice::InitializePrivateStaticClassUDisTweaks_AIBehavior_Notice(void)
//   0x7238e0  private: virtual unsigned int __thiscall UDisBehaviorNotice::IsBehaviorFinished(void)const
//   0x7238f0  private: virtual unsigned char const * __thiscall UDisBehaviorNotice::BuildEvaluateStimMask(void)const
//   0x723930  private: void __thiscall UDisBehaviorNotice::SetupFromNoticeBegin(struct FAIStimStruct_NoticeBegin const &)
//   0x723970  private: virtual unsigned char const * __thiscall UDisBehaviorNotice::BuildFilterStimMask(void)const
//   0x7239c0  private: virtual unsigned char const * __thiscall UDisBehaviorNotice::BuildShouldFinishWhileDormantStimMask(void)const
//   0x723a00  private: unsigned int __thiscall UDisBehaviorNotice::FilterNoticeEnd(struct FAIStimStruct_NoticeEnd const &)
//   0x723a50  public: virtual void __thiscall UDisBehaviorNotice::OnEnterCallback_Init(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x723a80  public: virtual void __thiscall UDisBehaviorNotice::OnExitCallback_Init(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x728890  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorNotice::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x7288c0  private: virtual unsigned char const * __thiscall UDisBehaviorNotice::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x728960  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorNotice::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x72bc80  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorNotice::GetSetupFromStimDelegate(enum EAIStimID)
//   0x72e1a0  private: virtual void __thiscall UDisBehaviorNotice::InitBehavior(class UDishonoredAIBrain * const)
//   0x736700  public: static class UClass * __cdecl UDisBehaviorNotice::GetPrivateStaticClassUDisBehaviorNotice(wchar_t const *)
//   0x738aa0  public: static void __cdecl UDisBehaviorNotice::InitializePrivateStaticClassUDisBehaviorNotice(void)
//   0x73a1d0  public: static class UClass * __cdecl UDisBehaviorNotice::StaticClassNoInline(void)
//   0x73a200  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Notice::GetPrivateStaticClassUDisTweaks_AIBehavior_Notice(wchar_t const *)
//   0x73ae00  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Notice::StaticClassNoInline(void)
//   0x73dbe0  private: virtual void __thiscall UDisBehaviorNotice::OnBehaviorStart(void)
//   0x73dd00  private: unsigned int __thiscall UDisBehaviorNotice::FilterRotationReached(struct FAIStimStruct_RotationReached const &)
//   0x73dd50  private: virtual unsigned int __thiscall UDisBehaviorNotice::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x73ddb0  public: virtual void __thiscall UDisBehaviorNotice::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x742c30  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorNotice::GetFilterStimDelegate(enum EAIStimID)

#include "DishonoredGame.h"
#include "disdesirestructs.h"
#include "disaisubstate.h"

/*-----------------------------------------------------------------------------
	agent DF: the three callbacks the desire layer and FDisAISubStateInit_Param unblocked
-----------------------------------------------------------------------------*/

void UDisBehaviorNotice::execOnEnterCallback_Init( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pLastState);
	P_FINISH;
	OnEnterCallback_Init( _pThisState, _pLastState );
}

// DISHONORED(port): 2012 rva 0x723a50: a procedural look-at on the noticed proxy with the look-target pattern - the head
// and torso turn towards it without the NPC changing what it is doing.
void UDisBehaviorNotice::OnEnterCallback_Init( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pLastState )
{
	// DISHONORED(retail): the proxy is UDisBehaviorAttentionBase::m_AttentionTargetProxy, not a member of this class.
	// UDisBehaviorAttentionBase is NEW IN 2013 (the generated header says so, and the 2012 PDB has no such type): retail
	// hoisted the per-behaviour attention proxy - 2012's UDisBehaviorNotice::m_NoticedProxy,
	// UDisBehaviorSearch::m_SearchTargetProxy and UDisBehaviorCombat's m_EnemyProxy, all at offset 160 of their own class -
	// into one member on a shared base. Porting the 2012 names would have added three members retail does not have.
	SetLookAtProceduralProxyDesire( m_AttentionTargetProxy, DisLookAtProceduralPattern_LookTarget, FDisLookAtInfluence::Torso, -1.f );
}

void UDisBehaviorNotice::execOnExitCallback_Init( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pNextState);
	P_FINISH;
	OnExitCallback_Init( _pThisState, _pNextState );
}

// DISHONORED(port): 2012 rva 0x723a80: a real transition drops the look-at; a NULL next state is the machine being
// destroyed, and the desire layer is being finalised anyway.
void UDisBehaviorNotice::OnExitCallback_Init( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pNextState )
{
	if( _pNextState )
	{
		ClearLookAtDesire();
	}
}

void UDisBehaviorNotice::execRequestStateExitCallback_GenericAction( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_GenericAction( _pThisState );
}

// DISHONORED(port): 2012 rva 0x73ddb0: the notice animation is over, so go back to slot 0 - the idle state.
void UDisBehaviorNotice::RequestStateExitCallback_GenericAction( UDishonoredNativeState* _pThisState )
{
	FDisAISubStateInit_Param Param;
	RequestSubStateChange< UDisTweaks_AIBehavior_Notice, UDisTweaks_AISubState_Init >( 0, Param );
}
