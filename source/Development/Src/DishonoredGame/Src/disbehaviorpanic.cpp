// DishonoredGame/src/disbehaviorpanic.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (49):
//   0x723aa0  public: static void __cdecl UDisTweaks_AIBehavior_Panic::InitializePrivateStaticClassUDisTweaks_AIBehavior_Panic(void)
//   0x723ac0  public: unsigned int __thiscall FAIPanicThreat::IsThreatDisturbingCower(class ADishonoredNPCPawn *, float)const
//   0x723b40  private: virtual unsigned char const * __thiscall UDisBehaviorPanic::BuildFilterStimMask(void)const
//   0x723bc0  private: virtual unsigned char const * __thiscall UDisBehaviorPanic::BuildEvaluateStimMask(void)const
//   0x723c10  private: unsigned int __thiscall UDisBehaviorPanic::FilterGoToRequest(struct FAIStimStruct_GoToRequest const &)
//   0x723c40  private: virtual unsigned int __thiscall UDisBehaviorPanic::IsBehaviorFinished(void)const
//   0x723c50  private: virtual enum EAIAwareness __thiscall UDisBehaviorPanic::GetAwarenessLevel(void)const
//   0x728990  private: virtual unsigned char const * __thiscall UDisBehaviorPanic::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x728a00  public: enum EDisPanicReason __thiscall UDisBehaviorPanic::RequestPanicReason(void)const
//   0x728a10  private: virtual class IDisAttentionTargetInterface * __thiscall UDisBehaviorPanic::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x72bcb0  public: void __thiscall FAIPanicThreat::SetupPanicThreat(enum EDisPanicReason, class AActor *, class UDishonoredAIBrain *)
//   0x72bd50  public: void __thiscall FAIPanicThreat::SwitchThreatTo(class AActor *, class UDishonoredAIBrain *)
//   0x72bdb0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPanic::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x72bde0  private: virtual unsigned int __thiscall UDisBehaviorPanic::GetPathGoals(class FVector const &, class TArray<class UNavMeshPathGoalEvaluator *, class FDefaultAllocator> &)const
//   0x72e220  private: struct FDisAttentionProxy __thiscall UDisBehaviorPanic::GetPanicProxy(void)const
//   0x730a80  private: void __thiscall UDisBehaviorPanic::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x730af0  private: virtual void __thiscall UDisBehaviorPanic::BeginDestroy(void)
//   0x730b30  public: virtual void __thiscall UDisBehaviorPanic::OnEnterCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x730c00  private: void __thiscall UDisBehaviorPanic::DoCorpseBark(void)
//   0x7328a0  private: void __thiscall UDisBehaviorPanic::SetupFromPanicked(struct FAIStimStruct_Panicked const &)
//   0x736790  public: static class UClass * __cdecl UDisBehaviorPanic::GetPrivateStaticClassUDisBehaviorPanic(wchar_t const *)
//   0x736820  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorPanic::GetSetupFromStimDelegate(enum EAIStimID)
//   0x738ac0  public: static void __cdecl UDisBehaviorPanic::InitializePrivateStaticClassUDisBehaviorPanic(void)
//   0x73a290  public: static class UClass * __cdecl UDisBehaviorPanic::StaticClassNoInline(void)
//   0x73a2c0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Panic::GetPrivateStaticClassUDisTweaks_AIBehavior_Panic(wchar_t const *)
//   0x73ae30  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Panic::StaticClassNoInline(void)
//   0x73ae60  private: unsigned int __thiscall UDisBehaviorPanic::FilterBehaviorAbort(struct FAIStimStruct_BehaviorAbort const &)
//   0x73dde0  private: virtual void __thiscall UDisBehaviorPanic::InitBehavior(class UDishonoredAIBrain * const)
//   0x73dea0  private: unsigned int __thiscall UDisBehaviorPanic::FilterPathingSuccess(struct FAIStimStruct_PathingSuccess const &)
//   0x73df60  private: virtual void __thiscall UDisBehaviorPanic::OnBehaviorStart(void)
//   0x73e030  public: virtual void __thiscall UDisBehaviorPanic::RefreshCallback_GenericAction(class UDisAISubState *, float)
//   0x73e100  public: virtual void __thiscall UDisBehaviorPanic::OnEnterCallback_Flee(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x73e1b0  public: virtual void __thiscall UDisBehaviorPanic::RequestStateExitCallback_Flee(class UDishonoredNativeState *)
//   0x73e1f0  private: void __thiscall UDisBehaviorPanic::DoFlee(void)
//   0x73e220  private: void __thiscall UDisBehaviorPanic::DoBeg(void)
//   0x73e260  private: void __thiscall UDisBehaviorPanic::CheckIfFleePointNeedsUnClaiming(unsigned int)
//   0x73e310  private: void __thiscall UDisBehaviorPanic::BeCaught(class AActor *)
//   0x73e400  private: void __thiscall UDisBehaviorPanic::CountPanickingNPC(unsigned int)const
//   0x7415b0  private: unsigned int __thiscall UDisBehaviorPanic::FilterPathingFail(struct FAIStimStruct_PathingFail const &)
//   0x741640  private: unsigned int __thiscall UDisBehaviorPanic::FilterTouchedEnemy(struct FAIStimStruct_TouchedEnemy const &)
//   0x741670  private: unsigned int __thiscall UDisBehaviorPanic::FilterGettingCaught(struct FAIStimStruct const &)
//   0x741690  private: virtual void __thiscall UDisBehaviorPanic::OnBehaviorResume(void)
//   0x7416b0  private: virtual void __thiscall UDisBehaviorPanic::OnBehaviorPause(unsigned int)
//   0x7416c0  private: virtual void __thiscall UDisBehaviorPanic::OnBehaviorStop(unsigned int)
//   0x7416d0  public: virtual void __thiscall UDisBehaviorPanic::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x741720  public: virtual void __thiscall UDisBehaviorPanic::RefreshCallback_Flee(class UDisAISubState *, float)
//   0x7417b0  public: virtual void __thiscall UDisBehaviorPanic::RefreshCallback_Cower(class UDisAISubState *, float)
//   0x741840  public: virtual void __thiscall UDisBehaviorPanic::RequestStateExitCallback_Cower(class UDishonoredNativeState *)
//   0x743940  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorPanic::GetFilterStimDelegate(enum EAIStimID)

#include "DishonoredGame.h"
#include "disdesirestructs.h"
#include "disaisubstate.h"

// ---- natives whose retail body is trivial (generated by build/agentAC_work/gen_trivial.py from the 2013 vtables) ----

// DISHONORED(written): 2013 rva 0x5f57b0; the retail UDisBehaviorPanic vtable slot +452 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x128ad0) and no retail subclass overrides it
void UDisBehaviorPanic::execRefreshCallback_Cower( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDisAISubState, _pThisState);
	P_GET_FLOAT(_fDeltaSeconds);
	P_FINISH;
}

// ---- end of trivial natives ----

/*-----------------------------------------------------------------------------
	agent DF: the callback that needed FDisAISubStateFlee_Param
-----------------------------------------------------------------------------*/

void UDisBehaviorPanic::execRequestStateExitCallback_GenericAction( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_GenericAction( _pThisState );
}

// DISHONORED(port): 2012 rva 0x7416d0: the panic animation is over, so run. Slot 3 is the cowering generic action, and
// leaving it releases the facing the cower was holding.
void UDisBehaviorPanic::RequestStateExitCallback_GenericAction( UDishonoredNativeState* _pThisState )
{
	if( GetActiveSubStateIndex() == 3 )
	{
		ClearFaceToDesire();
	}
	FDisAISubStateFlee_Param Param( m_PanicThreat.m_pPanicActor, TRUE );
	RequestSubStateChange< UDisTweaks_AIBehavior_Panic, UDisTweaks_AISubState_GenericAction >( 1, Param );
}
