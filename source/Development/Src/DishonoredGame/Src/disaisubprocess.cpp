// DishonoredGame/src/disaisubprocess.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (24):
//   0x765d00  public: static void __cdecl UDisAISubProcess::InitializePrivateStaticClassUDisAISubProcess(void)
//   0x765d20  public: static void __cdecl UDisTweaks_AISubProcess::InitializePrivateStaticClassUDisTweaks_AISubProcess(void)
//   0x765d40  private: void __thiscall UDisAISubProcess::RefreshSubProcess(float)
//   0x765d60  public: unsigned int __thiscall UDisAISubProcess::IsSubProcessEnabled(void)const
//   0x769810  private: void __thiscall UDisAISubProcess::BeginSubProcess(struct FDisBodyIntention const &)
//   0x769850  private: void __thiscall UDisAISubProcess::OnOwningBehaviorResume(struct FDisBodyIntention const &)
//   0x7698a0  private: void __thiscall UDisAISubProcess::TickSubProcess(float)
//   0x7698f0  private: void __thiscall UDisAISubProcess::EnableSubProcess_Internal(unsigned int)
//   0x769960  protected: virtual void __thiscall UDisAISubProcess::PostGameLoad(enum ESaveLoadLocation)
//   0x774760  public: static class UClass * __cdecl UDisAISubProcess::GetPrivateStaticClassUDisAISubProcess(wchar_t const *)
//   0x7747f0  public: struct FDisAttentionProxy __thiscall UDisAISubProcess::GetSubProcessActionTargetProxy(void)const
//   0x774840  protected: virtual void __thiscall UDisAISubProcess::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x774870  protected: virtual void __thiscall UDisAISubProcess::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x776180  public: static class UClass * __cdecl UDisAISubProcess::StaticClassNoInline(void)
//   0x7761b0  private: void __thiscall UDisAISubProcess::InitSubProcess(class UDishonoredAIBehavior * const, class UDisTweaks_AISubProcess * const)
//   0x776290  private: void __thiscall UDisAISubProcess::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x7762e0  protected: virtual void __thiscall UDisAISubProcess::BeginDestroy(void)
//   0x776320  protected: void __thiscall UDisAISubProcess::SetActionTargetProxy(struct FDisAttentionProxy)
//   0x7763b0  public: void __thiscall UDisAISubState::OnOtherActorTerminatedEvent(class FArkGameEvent const &)
//   0x777b80  private: void __thiscall UDisAISubProcess::EndSubProcess(unsigned int)
//   0x777c30  private: void __thiscall UDisAISubProcess::OnOwningBehaviorPause(unsigned int)
//   0x777c50  private: void __thiscall UDisAISubProcess::DisableSubProcess_Internal(unsigned int)
//   0x783ea0  public: static class UClass * __cdecl UDisTweaks_AISubProcess::GetPrivateStaticClassUDisTweaks_AISubProcess(wchar_t const *)
//   0x784810  public: static class UClass * __cdecl UDisTweaks_AISubProcess::StaticClassNoInline(void)

// ---- agent CG ports (PHASE9 CG): the AI sub-process ----

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "disdesirestructs.h"
#include "disaisubstate.h"
#include "dishonoredutilities_saveload.h"
#include "dishonoredutilities_saveload_ai.h"

/*-----------------------------------------------------------------------------
	The desires half.

	DISHONORED(port): agent DF landed IDisDesiresInterface (Src/disdesiresinterface.cpp), so every transition of a
	sub-process and a sub-state now drives the owning object's desires for real - InitializeDesires / ResumeDesires /
	PauseDesires / StopDesires / FinalizeDesires / TickDesires / PostGameLoad_Desires, 2013 rvas 0x8ba1f0, 0x8bbf50,
	0x8ba360, 0x8b22d0, 0x8b3cb0, 0x8bc080, 0x8bc0e0. Agent CG's DisAINoteDesiresGap placeholder is gone with them.
	m_pDesires is filled only for a class whose GetDesires() override answers non-NULL, i.e. UDisAISubStateWithDesires /
	UDisAISubProcessWithDesires / UDisAIBehaviorWithDesires and their subclasses; the base answers NULL, so a sub-state
	with no desires of its own still takes none of these branches, exactly as retail.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x739320 (2012 0x7761b0): the sub-process is bound to its behaviour and the behaviour's
// brain, given its tweaks through the tweak interface, and starts out enabled but not begun - the behaviour's first
// resume is what calls BeginSubProcess.
void UDisAISubProcess::InitSubProcess( UDishonoredAIBehavior* const OwningBehavior, UDisTweaks_AISubProcess* const SubProcessTweaks )
{
	// DISHONORED(port): agent DF, as UDisAISubState::InitSubState.
	IDisDesiresInterface* OwnDesires = GetDesires();
	DisSetScriptInterface( m_pDesires, OwnDesires ? OwnDesires->GetUObjectInterfaceDisDesiresInterface() : NULL );
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->InitializeDesires();
	}

	if( GetTweaks_Derived() != SubProcessTweaks )
	{
		SetTweaks_Derived( SubProcessTweaks );
		ApplyTweakChanges();
	}
	m_pOwningBehavior = OwningBehavior;
	m_pOwningBrain = OwningBehavior->m_pOwningBrain;

	m_pFilterStimMask = (FPointer)BuildFilterStimMask();
	InitSubProcess_Derived();

	m_bIsSubProcessEnabled = TRUE;
	m_bSubProcessHasBegun = FALSE;
}

// DISHONORED(port): 2013 rva 0x72acf0 (2012 0x769810)
void UDisAISubProcess::BeginSubProcess( const FDisBodyIntention& PreviousBodyIntention )
{
	m_bSubProcessHasBegun = TRUE;
	GDisAISubProcessBegins++;
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->ResumeDesires( PreviousBodyIntention );
	}
	BeginSubProcess_Derived();
}

// DISHONORED(port): 2013 rva 0x73d480 (2012 0x777b80): the action target is dropped and its termination subscription with
// it. A sub-process that is ending for good finalises its desires; one that is only pausing pauses and then stops them.
void UDisAISubProcess::EndSubProcess( UBOOL bIsBeingTerminated )
{
	m_bSubProcessHasBegun = FALSE;

	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubProcess::OnOtherActorTerminatedEvent );
	}
	m_ActionTargetProxy.ClearAttnProxy();

	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		if( bIsBeingTerminated )
		{
			Desires->FinalizeDesires();
		}
		else
		{
			Desires->PauseDesires();
			Desires->StopDesires();
		}
	}
	EndSubProcess_Derived( bIsBeingTerminated );
}

// DISHONORED(port): 2013 rva 0x72ad80 (2012 0x7698a0)
void UDisAISubProcess::TickSubProcess( FLOAT DeltaTime )
{
	TickSubProcess_Derived( DeltaTime );
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->TickDesires( DeltaTime );
	}
}

// DISHONORED(port): 2013 rva 0x728450 (2012 0x765d40)
void UDisAISubProcess::RefreshSubProcess( FLOAT TimeSinceLastThought )
{
	RefreshSubProcess_Derived( TimeSinceLastThought );
}

// DISHONORED(port): 2013 rva 0x72add0 (2012 0x7698f0): enabling a sub-process while its behaviour is running begins it
// immediately, against the body intention the pawn is in right now; enabling it while the behaviour is paused only sets
// the flag, and the behaviour's resume begins it.
void UDisAISubProcess::EnableSubProcess_Internal( UBOOL bIsBehaviorPaused )
{
	if( m_bIsSubProcessEnabled )
	{
		return;
	}
	m_bIsSubProcessEnabled = TRUE;
	if( !bIsBehaviorPaused )
	{
		ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
		FDisBodyIntention PawnExistingBodyIntention;
		PawnExistingBodyIntention.m_IntendedBodyStance = Pawn->GetBodyStance();
		PawnExistingBodyIntention.m_pDesiredPrimaryItemClass = Pawn->GetDesiredPrimaryItem();
		PawnExistingBodyIntention.m_pDesiredSecondaryItemClass = Pawn->GetDesiredSecondaryItem();
		BeginSubProcess( PawnExistingBodyIntention );
	}
}

// DISHONORED(port): 2013 rva 0x73d550 (2012 0x777c50)
void UDisAISubProcess::DisableSubProcess_Internal( UBOOL bIsBehaviorPaused )
{
	if( !m_bIsSubProcessEnabled )
	{
		return;
	}
	m_bIsSubProcessEnabled = FALSE;
	if( !bIsBehaviorPaused )
	{
		EndSubProcess( FALSE );
	}
}

// DISHONORED(port): 2013 rva 0x72ad30 (2012 0x769850): an enabled sub-process resumes with its behaviour. Note that this
// sets m_bSubProcessHasBegun and calls BeginSubProcess_Derived directly rather than going through BeginSubProcess.
void UDisAISubProcess::OnOwningBehaviorResume( const FDisBodyIntention& PreviousBodyIntention )
{
	if( !m_bIsSubProcessEnabled )
	{
		return;
	}
	m_bSubProcessHasBegun = TRUE;
	GDisAISubProcessBegins++;
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->ResumeDesires( PreviousBodyIntention );
	}
	BeginSubProcess_Derived();
}

// DISHONORED(port): 2013 rva 0x73d530 (2012 0x777c30): only a sub-process that is both enabled and begun has anything to
// end.
void UDisAISubProcess::OnOwningBehaviorPause( UBOOL bIsBeingTerminated )
{
	if( m_bIsSubProcessEnabled && m_bSubProcessHasBegun )
	{
		EndSubProcess( bIsBeingTerminated );
	}
}

// DISHONORED(port): 2013 rva 0x734d30 (2012 0x7747f0)
FDisAttentionProxy UDisAISubProcess::GetSubProcessActionTargetProxy() const
{
	return m_ActionTargetProxy;
}

// DISHONORED(port): 2013 rva 0x739490 (2012 0x776320): the subscription follows the proxy, so the sub-process is told
// when the actor it is acting on is destroyed and only while it holds one.
void UDisAISubProcess::SetActionTargetProxy( FDisAttentionProxy ActionTarget )
{
	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubProcess::OnOtherActorTerminatedEvent );
	}

	m_ActionTargetProxy = ActionTarget;

	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->RegisterToEvent( EventType, this, &UDisAISubProcess::OnOtherActorTerminatedEvent );
	}
}

// DISHONORED(port): 2013 rva 0x739400 (2012 0x776290): the event is broadcast for every terminated actor, so the
// instigator has to be compared against our own target before the proxy is dropped.
void UDisAISubProcess::OnOtherActorTerminatedEvent( const FArkGameEvent& Event )
{
	if( m_ActionTargetProxy.IsEqualToActor( *(const AActor*)Event.m_pInstigator ) )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubProcess::OnOtherActorTerminatedEvent );
		m_ActionTargetProxy.ClearAttnProxy();
	}
}

// DISHONORED(port): 2013 rva 0x739450 (2012 0x7762e0): the subscription must not outlive the object.
void UDisAISubProcess::BeginDestroy()
{
	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubProcess::OnOtherActorTerminatedEvent );
	}
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x72ae40 (2012 0x769960): the stim mask is rebuilt after a load because it is a raw pointer
// into per-class static data that the save file cannot carry.
// DISHONORED(bringup): retail reaches this through the DisSaveLoad vtable slot UDisAISubProcess_vtbl+280; UObject
// declares no such slot in this tree, so this is a plain member and nothing calls it until agent CF's save package
// lands the slots. Named PostGameLoad_SubProcess so it cannot be mistaken for an override.
void UDisAISubProcess::PostGameLoad_SubProcess()
{
	if( m_pOwningBrain && m_pOwningBrain->IsBrainInitialized() )
	{
		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->PostGameLoad_Desires();
		}
	}
	m_pFilterStimMask = (FPointer)BuildFilterStimMask();
}

// ---- agent CG: the tweak-interface pair, defined here rather than in the cpptext ----
// DISHONORED(port): 2012 vtable UDisAISubProcess{IDisTweaksInterface} slots 4 and 5. They are one load and one store at
// m_pSubProcessTweaks. The bodies live in the unit because inside the generated class body UDisTweaks_AISubProcess is
// still only forward-declared, so the derived-to-base conversion is not visible there.
UDisTweaksBase* UDisAISubProcess::GetTweaks_Derived()
{
	return m_pSubProcessTweaks;
}

void UDisAISubProcess::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pSubProcessTweaks = (UDisTweaks_AISubProcess*)Tweaks;
}
// DISHONORED(port): 2013 rva 0x73d580 (2012 0x774870), retail vtable slot 70 (vftable rva 0xd39468). The
// process's own script properties, then which of the owning behaviour tweaks' sub-process tweaks it runs on.
// DISHONORED(bringup): as UDisAISubState::GameLoad, retail's tail re-registers the other-actor-terminated
// event and reads no stream byte.
void UDisAISubProcess::GameLoad( FArchive& _rArchive, ESaveLoadLocation _Location )
{
	DisSaveLoadObject( _rArchive, this );
	DisLoadAISubTweakReference< UDisTweaks_AISubProcess, UDisTweaks_AIBehavior >( _rArchive, m_pSubProcessTweaks,
		&UDisTweaks_AIBehavior::m_SubProcessTweaks );
}

// DISHONORED(port): 2013 rva 0x72ae40, retail vtable slot 71. Reads no stream byte; the whole body is agent
// CG's PostGameLoad_SubProcess, which is now reachable.
void UDisAISubProcess::PostGameLoad( ESaveLoadLocation _Location )
{
	PostGameLoad_SubProcess();
}
