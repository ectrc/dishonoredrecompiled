// DishonoredGame/src/disbehaviorsearch.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (55):
//   0x724410  public: static void __cdecl UDisTweaks_AIBehavior_Search::InitializePrivateStaticClassUDisTweaks_AIBehavior_Search(void)
//   0x724430  private: virtual void __thiscall UDisBehaviorSearch::RefreshThoughts(float)
//   0x724450  private: virtual unsigned int __thiscall UDisBehaviorSearch::IsBehaviorFinished(void)const
//   0x724460  private: virtual unsigned char const * __thiscall UDisBehaviorSearch::BuildShouldFinishWhileDormantStimMask(void)const
//   0x7244a0  private: virtual unsigned char const * __thiscall UDisBehaviorSearch::BuildEvaluateStimMask(void)const
//   0x7244e0  private: unsigned int __thiscall UDisBehaviorSearch::ShouldDoNewInvestigate(struct FDisAttentionChangeReason const &)const
//   0x724570  private: virtual unsigned char const * __thiscall UDisBehaviorSearch::BuildFilterStimMask(void)const
//   0x724600  private: unsigned int __thiscall UDisBehaviorSearch::FilterSearchEnd(struct FAIStimStruct_SearchEnd const &)
//   0x724750  private: unsigned int __thiscall UDisBehaviorSearch::FilterAlarm(struct FAIStimStruct_Alarm const &)
//   0x724770  private: unsigned int __thiscall UDisBehaviorSearch::FilterDocileRatIsNear(struct FAIStimStruct_DocileRatIsNear const &)
//   0x724790  private: unsigned int __thiscall UDisBehaviorSearch::FilterEnemyBusted(struct FAIStimStruct_EnemyBusted const &)
//   0x7247e0  private: unsigned int __thiscall UDisBehaviorSearch::FilterPathingSuccess(struct FAIStimStruct_PathingSuccess const &)
//   0x724810  private: virtual unsigned int __thiscall UDisBehaviorStationarySearch::CanReactToNoise(struct FDisAINoiseInfo const &)const
//   0x724840  public: virtual void __thiscall UDisBehaviorSearch::OnExitCallback_Investigate(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x724850  public: virtual void __thiscall UDisBehaviorSearch::OnEnterCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x7248a0  public: virtual void __thiscall UDisBehaviorSearch::TickCallback_Stand(class UDishonoredNativeState *, float)
//   0x7248d0  private: virtual struct FDisBodyIntentionRequest * __thiscall UDisBehaviorSearch::GetDesiresBodyIntentionRequest(void)
//   0x7248e0  private: virtual class IDisAttentionTargetInterface * __thiscall UDisBehaviorStationarySearch::GetAllyReactionTarget(enum EDisAttentionChangeReasonType &)const
//   0x728cf0  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorSearch::GetEvaluateStimDelegate(enum EAIStimID)const
//   0x728d20  private: virtual unsigned char const * __thiscall UDisBehaviorSearch::BuildBehaviorFilterStimMasks(unsigned char const * &, unsigned char const * &)const
//   0x729090  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorStationarySearch::GetShouldFinishWhileDormantDelegate(enum EAIStimID)const
//   0x72c1a0  private: virtual void __thiscall UDisBehaviorSearch::OnBehaviorStop(unsigned int)
//   0x72c200  public: virtual void __thiscall UDisBehaviorSearch::OnEnterCallback_TrackTarget(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x72e510  private: virtual void __thiscall UDisBehaviorSearch::InitBehavior(class UDishonoredAIBrain * const)
//   0x72e5f0  public: virtual void __thiscall UDisBehaviorSearch::OnEnterCallback_Investigate(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x72e670  public: virtual void __thiscall UDisBehaviorSearch::OnExitCallback_TrackTarget(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x730e60  private: void __thiscall UDisBehaviorSearch::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x730ed0  private: virtual void __thiscall UDisBehaviorSearch::BeginDestroy(void)
//   0x732980  private: unsigned int __thiscall UDisBehaviorSearch::FilterDoorUsedByPlayer(struct FAIStimStruct_DoorUsedByPlayer const &)
//   0x736b90  public: static class UClass * __cdecl UDisBehaviorSearch::GetPrivateStaticClassUDisBehaviorSearch(wchar_t const *)
//   0x738c40  public: static void __cdecl UDisBehaviorSearch::InitializePrivateStaticClassUDisBehaviorSearch(void)
//   0x73a720  public: static class UClass * __cdecl UDisBehaviorSearch::StaticClassNoInline(void)
//   0x73a750  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Search::GetPrivateStaticClassUDisTweaks_AIBehavior_Search(wchar_t const *)
//   0x73afa0  public: static class UClass * __cdecl UDisTweaks_AIBehavior_Search::StaticClassNoInline(void)
//   0x73f040  private: void __thiscall UDisBehaviorSearch::GetBodyIntention(enum eDisNPCBodyStance &, class UClass * &, class UClass * &)const
//   0x73f140  private: virtual void __thiscall UDisBehaviorSearch::OnBehaviorStart(void)
//   0x73f200  private: virtual void __thiscall UDisBehaviorSearch::TickBehavior(float)
//   0x73f320  private: unsigned int __thiscall UDisBehaviorSearch::ShouldInvestigateAtARun(struct FDisAttentionChangeReason const &)const
//   0x73f3c0  private: void __thiscall UDisBehaviorSearch::RequestTrackTargetSubState(void)
//   0x73f420  private: unsigned int __thiscall UDisBehaviorSearch::FilterTetherRequest(struct FAIStimStruct_TetherRequest const &)
//   0x73f440  public: virtual void __thiscall UDisBehaviorSearch::RequestStateExitCallback_Investigate(class UDishonoredNativeState *)
//   0x73f760  public: virtual void __thiscall UDisBehaviorSearch::RequestStateExitCallback_GenericAction(class UDishonoredNativeState *)
//   0x73f770  public: virtual void __thiscall UDisBehaviorSearch::OnExitCallback_GenericAction(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x73f7d0  public: virtual void __thiscall UDisBehaviorSearch::OnEnterCallback_Stand(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x73f880  public: virtual void __thiscall UDisBehaviorSearch::OnEnterCallback_StareAtUnreachable(class UDishonoredNativeState *, class UDishonoredNativeState *)
//   0x73f8f0  public: virtual void __thiscall UDisBehaviorSearch::TickCallback_StareAtUnreachable(class UDishonoredNativeState *, float)
//   0x73f940  private: virtual unsigned int __thiscall UDisBehaviorSearch::GetResumingBodyIntentionDesire(struct FDisBodyIntention const &, struct FDisBodyIntention &)const
//   0x741bf0  private: void __thiscall UDisBehaviorSearch::RequestInvestigateSubState(enum eDisHookInvestigate, struct FDisAttentionChangeReason const &)
//   0x742ec0  private: void __thiscall UDisBehaviorSearch::RequestResponseSubState(enum eDisHookInvestigate, struct FDisAttentionChangeReason const &)
//   0x742fa0  private: unsigned int __thiscall UDisBehaviorSearch::FilterTopAttnProxyUpdated(struct FAIStimStruct_TopAttnProxyUpdated const &)
//   0x742fd0  private: unsigned int __thiscall UDisBehaviorSearch::FilterHelp(struct FAIStimStruct_Help const &)
//   0x743010  private: unsigned int __thiscall UDisBehaviorSearch::FilterCombatToSearch(struct FAIStimStruct_CombatToSearch const &)
//   0x743aa0  private: void __thiscall UDisBehaviorSearch::SetupFromSearchBegin(struct FAIStimStruct_SearchBegin const &)
//   0x744790  private: virtual class DisDelegate<unsigned int, struct FAIStimStruct> __thiscall UDisBehaviorSearch::GetFilterStimDelegate(enum EAIStimID)
//   0x744fc0  private: virtual class DisDelegate<void, struct FAIStimStruct> __thiscall UDisBehaviorSearch::GetSetupFromStimDelegate(enum EAIStimID)

// ---- agent CG ports (PHASE9 CG) ----

#include "DishonoredGame.h"
#include "disdesirestructs.h"
#include "disaisubstate.h"

// DISHONORED(port): 2013 rva 0x6e4410 (2012 0x724840, exec 0x63f8a0): leaving the investigate sub-state resets the
// reason to the enumeration's MAX sentinel (EDisAttentionChangeReasonType DACRT_MAX = 21), which is how retail spells
// "no reason recorded".
void UDisBehaviorSearch::execOnExitCallback_Investigate( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pNextState);
	P_FINISH;
	OnExitCallback_Investigate( _pThisState, _pNextState );
}

void UDisBehaviorSearch::OnExitCallback_Investigate( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pNextState )
{
	m_CurrentInvestigateReason = DACRT_MAX;
}

// DISHONORED(port): 2013 rva 0x6e4470 (2012 0x7248a0): standing during a search runs the cancel-investigate timer down
// and leaves the sub-state when it expires. Retail does not clamp the timer, so it keeps going negative once it fired.
void UDisBehaviorSearch::execTickCallback_Stand( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_FLOAT(_fDeltaSeconds);
	P_FINISH;
	TickCallback_Stand( _pThisState, _fDeltaSeconds );
}

void UDisBehaviorSearch::TickCallback_Stand( UDishonoredNativeState* _pThisState, FLOAT _fDeltaSeconds )
{
	m_fCancelInvestigateTimer -= _fDeltaSeconds;
	if( m_fCancelInvestigateTimer <= 0.f )
	{
		_pThisState->RequestStateExit();
	}
}

// ---- agent CG natives sweep, round 2 (PHASE9 CG) ----

// DISHONORED(port): 2013 rva 0x6f4a20 (2012 0x72e670): the exec wrapper, over the C++ body below.
void UDisBehaviorSearch::execOnExitCallback_TrackTarget( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pNextState);
	P_FINISH;
	OnExitCallback_TrackTarget( _pThisState, _pNextState );
}

// DISHONORED(port): 2013 rva 0x6f4a20 (2012 0x72e670)
void UDisBehaviorSearch::OnExitCallback_TrackTarget( class UDishonoredNativeState* _pThisState, class UDishonoredNativeState* _pNextState )
{
	// Leaving the track-target state silences the distraction and bark sub-processes, and hands the search target on
	// to whatever state comes next so it keeps looking in the same place.
	UDisAISubProcess* Distractions = GetSubProcess( UDisAISubProcessDistractions::StaticClass() );
	if( Distractions )
	{
		Distractions->DisableSubProcess_Internal( m_bIsPaused );
	}
	UDisAISubProcess* Barks = GetSubProcess( UDisAISubProcessGenericBark::StaticClass() );
	if( Barks )
	{
		Barks->DisableSubProcess_Internal( m_bIsPaused );
	}
	if( _pNextState )
	{
		SetActionTargetProxy( m_AttentionTargetProxy );
	}
}

/*-----------------------------------------------------------------------------
	agent DF: the two callbacks the desire layer unblocked
-----------------------------------------------------------------------------*/

void UDisBehaviorSearch::execOnEnterCallback_GenericAction( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pLastState);
	P_FINISH;
	OnEnterCallback_GenericAction( _pThisState, _pLastState );
}

// DISHONORED(port): 2012 rva 0x724850: face what is being searched for while the search animation plays, and clear the
// "target is unreachable" flag because this is a fresh look.
void UDisBehaviorSearch::OnEnterCallback_GenericAction( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pLastState )
{
	// DISHONORED(retail): m_AttentionTargetProxy on the 2013-only UDisBehaviorAttentionBase; see UDisBehaviorNotice.
	SetFaceToProxyDesire( m_AttentionTargetProxy, -100.f, FALSE );
	m_bTargetIsUnreachable = FALSE;
}

void UDisBehaviorSearch::execOnExitCallback_GenericAction( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_GET_OBJECT(UDishonoredNativeState, _pNextState);
	P_FINISH;
	OnExitCallback_GenericAction( _pThisState, _pNextState );
}

// DISHONORED(port): 2012 rva 0x73f770: the search animation is over, so the facing is released and the cooldown starts -
// which is what stops an NPC playing the same search animation twice in a row.
void UDisBehaviorSearch::OnExitCallback_GenericAction( UDishonoredNativeState* _pThisState, UDishonoredNativeState* _pNextState )
{
	if( _pNextState )
	{
		ClearFaceToDesire();
	}
	const UDisTweaks_AIBehavior_Search* Tweaks = Cast<UDisTweaks_AIBehavior_Search>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AIBehavior_Search*)UDisTweaks_AIBehavior_Search::StaticClass()->GetDefaultObject();
	}
	m_fSearchAnimCooldownTimer = Tweaks->m_fSearchAnimCooldown;
}
