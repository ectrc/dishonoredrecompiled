// DishonoredGame/src/disbehaviorratstomp.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (22):
//   0x723e90  public: static void __cdecl UDisTweaks_AIBehavior_RatStomp::InitializePrivateStaticClassUDisTweaks_AIBehavior_RatStomp(void)
//   0x723ec0  private: virtual void __thiscall UDisBehaviorRatStomp::OnBehaviorStop(unsigned int)
//   0x723ed0  private: virtual void __thiscall UDisBehaviorRatStomp::OnPostGameLoad(unsigned int, unsigned int)
//   0x723ee0  private: virtual unsigned char const * __thiscall UDisBehaviorRatStomp::BuildFilterStimMask(void)const
//   0x723f70  private: virtual unsigned char const * __thiscall UDisBehaviorRatStomp::BuildEvaluateStimMask(void)const
//   0x723fb0  private: void __thiscall UDisBehaviorRatStomp::SetupFromDocileRatIsNear(struct FAIStimStruct_DocileRatIsNear const &)
//   0x723fd0  private: virtual struct FDisBodyIntentionRequest * __thiscall UDisBehaviorRatStomp::GetDesiresBodyIntentionRequest(void)
//   0x723fe0  public: virtual void __thiscall UDisBehaviorRatStomp::RequestStateExitCallback_TakeActorPosition(class UDishonoredNativeState *)
//   0x728b80  private: virtual unsigned char const * __thiscall UDisBehaviorRatStomp::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x72bf70  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorRatStomp::GetFilterStimDelegate(enum EAIStimID)
//   0x72c060  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorRatStomp::GetSetupFromStimDelegate(enum EAIStimID)
//   0x72e340  private: virtual void __thiscall UDisBehaviorRatStomp::InitBehavior(class UDishonoredAIBrain * const)
//   0x7369e0  public: static class UClass * __cdecl UDisBehaviorRatStomp::GetPrivateStaticClassUDisBehaviorRatStomp(wchar_t const *)
//   0x738b90  public: static void __cdecl UDisBehaviorRatStomp::InitializePrivateStaticClassUDisBehaviorRatStomp(void)
//   0x73a4e0  public: static class UClass * __cdecl UDisBehaviorRatStomp::StaticClassNoInline(void)
//   0x73a510  public: static class UClass * __cdecl UDisTweaks_AIBehavior_RatStomp::GetPrivateStaticClassUDisTweaks_AIBehavior_RatStomp(wchar_t const *)
//   0x73af10  public: static class UClass * __cdecl UDisTweaks_AIBehavior_RatStomp::StaticClassNoInline(void)
//   0x73ec50  private: void __thiscall UDisBehaviorRatStomp::StartNewRatInteraction(void)
//   0x73ed70  private: virtual void __thiscall UDisBehaviorRatStomp::OnBehaviorStart(void)
//   0x73edd0  private: virtual void __thiscall UDisBehaviorRatStomp::RefreshThoughts(float)
//   0x73efa0  private: unsigned int __thiscall UDisBehaviorRatStomp::EvaluateDocileRatIsNear(struct FAIStimStruct_DocileRatIsNear const &)const
//   0x742e90  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorRatStomp::GetEvaluateStimDelegate(enum EAIStimID)const

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x6e40a0 (2012 0x723fe0, exec 0x63bf80): arriving at the rat is what the
// take-actor-position sub-state reports by asking to leave, so the flag it sets is "reached", not "finished".
void UDisBehaviorRatStomp::execRequestStateExitCallback_TakeActorPosition( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	RequestStateExitCallback_TakeActorPosition( _pThisState );
}

void UDisBehaviorRatStomp::RequestStateExitCallback_TakeActorPosition( UDishonoredNativeState* _pThisState )
{
	m_bRatReached = TRUE;
}
