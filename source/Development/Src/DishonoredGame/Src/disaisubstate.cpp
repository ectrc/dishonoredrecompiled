// DishonoredGame/src/disaisubstate.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (30):
//   0x765250  public: static void __cdecl UDisAISubState::InitializePrivateStaticClassUDisAISubState(void)
//   0x765270  public: static void __cdecl UDisTweaks_AISubState::InitializePrivateStaticClassUDisTweaks_AISubState(void)
//   0x768420  public: __thiscall FDisAISubState_Param::FDisAISubState_Param(void)
//   0x768440  public: virtual void __thiscall FDisAISubState_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x768470  public: unsigned int __thiscall UDisAISubState::ArePreconditionsMet(struct FDisNativeStateParam &)
//   0x7684a0  public: virtual void __thiscall UDisAISubState::TickState(float)
//   0x768530  protected: virtual void __thiscall UDisAISubState::RequestStateExit_Derived(void)
//   0x768560  public: void __thiscall UDisAISubState::OnOwningBehaviorResume(struct FDisBodyIntention const &)
//   0x7685b0  public: void __thiscall UDisAISubState::OnOwningBehaviorPause(unsigned int)
//   0x768600  public: virtual void __thiscall UDisAISubState::RefreshSubState(float)
//   0x768640  protected: virtual void __thiscall UDisAISubState::PostGameLoad(enum ESaveLoadLocation)
//   0x770750  public: void __thiscall UDisAISubState::RegisterDelegate_OnEnterCallback(class UObject *, class FString const &)
//   0x770830  public: void __thiscall UDisAISubState::RegisterDelegate_TickCallback(class UObject *, class FString const &)
//   0x770910  public: void __thiscall UDisAISubState::RegisterDelegate_OnExitCallback(class UObject *, class FString const &)
//   0x7709f0  public: void __thiscall UDisAISubState::RegisterDelegate_OnResetCallback(class UObject *, class FString const &)
//   0x7731a0  public: static class UClass * __cdecl UDisAISubState::GetPrivateStaticClassUDisAISubState(wchar_t const *)
//   0x773230  public: void __thiscall UDisAISubState::RegisterDelegate_RefreshCallback(class UDishonoredAIBehavior * const)
//   0x773370  public: void __thiscall UDisAISubState::RegisterDelegate_RequestStateExitCallback(class UDishonoredAIBehavior * const)
//   0x7734b0  private: virtual void __thiscall UDisAISubState::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7734e0  private: virtual void __thiscall UDisAISubState::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x7750a0  public: static class UClass * __cdecl UDisAISubState::StaticClassNoInline(void)
//   0x7750d0  public: void __thiscall UDisAISubState::InitSubState(class UDishonoredAIBehavior * const, class UDisTweaks_AISubState * const)
//   0x777300  protected: void __thiscall UDisAISubState::SetActionTargetProxy(struct FDisAttentionProxy)
//   0x777390  public: virtual void __thiscall UDisAISubState::BeginDestroy(void)
//   0x7773d0  protected: void __thiscall UDisAISubState::ClearActionTargetProxy(void)
//   0x77b5f0  public: virtual void __thiscall UDisAISubState::OnEnterState(class UDishonoredNativeState *)
//   0x77b6a0  public: virtual void __thiscall UDisAISubState::OnExitState(class UDishonoredNativeState *)
//   0x77b810  public: virtual unsigned int __thiscall UDisAISubState::OnResetState(void)
//   0x782f60  public: static class UClass * __cdecl UDisTweaks_AISubState::GetPrivateStaticClassUDisTweaks_AISubState(wchar_t const *)
//   0x7844d0  public: static class UClass * __cdecl UDisTweaks_AISubState::StaticClassNoInline(void)

// ---- agent CG ports (PHASE9 CG): the AI sub-state ----

#include "DishonoredGame.h"
#include "disaicensus.h"
#include "disdesirestructs.h"
#include "disaisubstate.h"

/*-----------------------------------------------------------------------------
	The six behaviour callbacks.

	DISHONORED(retail): these are ordinary UE3 script delegates, not C++ bound methods. UDisAISubState carries six
	FScriptDelegate members (__OnEnterCallback__Delegate, __OnResetCallback__Delegate, __OnExitCallback__Delegate,
	__TickCallback__Delegate, __RefreshCallback__Delegate, __RequestStateExitCallback__Delegate) and the generated
	delegateOnEnterCallback(...) / delegateTickCallback(...) / ... thunks beside them call ProcessDelegate. Each
	RegisterDelegate_* below composes a function name and stores it with the owner object; ProcessDelegate then resolves
	it on that object at call time, which is how UDisBehavior*::OnEnterCallback_<Suffix> and friends - script functions
	with native exec entries - are reached.

	The name is a plain concatenation with no separator: "OnEnterCallback" + Suffix. The suffix therefore carries its own
	leading underscore, so a sub-state whose m_StateSuffix is "_TakePosition" binds "RefreshCallback_TakePosition".
	Retail composes it into a 128-wchar static scratch buffer with _wcscpy_s / _wcscat_s; ours builds an FString, which
	differs only in that a name longer than 127 characters is not truncated.

	The FName is created with FNAME_Add, so the name is interned whether or not the function exists: binding never
	verifies the target. A name that does not resolve is a no-op at call time, and the only case retail clears the
	delegate for is an empty composed name.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x705a00 (2012 0x770750)
void UDisAISubState::RegisterDelegate_OnEnterCallback( UObject* Owner, const FString& Suffix )
{
	const FString TargetFunctionName = FString( TEXT("OnEnterCallback") ) + Suffix;
	const FName TargetFunction( *TargetFunctionName, FNAME_Add, TRUE );
	if( TargetFunction != NAME_None )
	{
		__OnEnterCallback__Delegate.Object = Owner;
		__OnEnterCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__OnEnterCallback__Delegate.Object = NULL;
		__OnEnterCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2013 rva 0x705ae0 (2012 0x770830)
void UDisAISubState::RegisterDelegate_TickCallback( UObject* Owner, const FString& Suffix )
{
	const FString TargetFunctionName = FString( TEXT("TickCallback") ) + Suffix;
	const FName TargetFunction( *TargetFunctionName, FNAME_Add, TRUE );
	if( TargetFunction != NAME_None )
	{
		__TickCallback__Delegate.Object = Owner;
		__TickCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__TickCallback__Delegate.Object = NULL;
		__TickCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2013 rva 0x705bc0 (2012 0x770910)
void UDisAISubState::RegisterDelegate_OnExitCallback( UObject* Owner, const FString& Suffix )
{
	const FString TargetFunctionName = FString( TEXT("OnExitCallback") ) + Suffix;
	const FName TargetFunction( *TargetFunctionName, FNAME_Add, TRUE );
	if( TargetFunction != NAME_None )
	{
		__OnExitCallback__Delegate.Object = Owner;
		__OnExitCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__OnExitCallback__Delegate.Object = NULL;
		__OnExitCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2013 rva 0x705ca0 (2012 0x7709f0)
void UDisAISubState::RegisterDelegate_OnResetCallback( UObject* Owner, const FString& Suffix )
{
	const FString TargetFunctionName = FString( TEXT("OnResetCallback") ) + Suffix;
	const FName TargetFunction( *TargetFunctionName, FNAME_Add, TRUE );
	if( TargetFunction != NAME_None )
	{
		__OnResetCallback__Delegate.Object = Owner;
		__OnResetCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__OnResetCallback__Delegate.Object = NULL;
		__OnResetCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2013 rva 0x70d350 (2012 0x773230): unlike the four above, the refresh and request-exit hooks are
// only ever bound to the owning behaviour, so they take it directly and compose the suffix from m_StateSuffix themselves.
void UDisAISubState::RegisterDelegate_RefreshCallback( UDishonoredAIBehavior* const OwningBehavior )
{
	const FString TargetFunctionName = FString( TEXT("RefreshCallback") ) + m_StateSuffix.ToString();
	const FName TargetFunction( *TargetFunctionName, FNAME_Add, TRUE );
	if( TargetFunction != NAME_None )
	{
		__RefreshCallback__Delegate.Object = OwningBehavior;
		__RefreshCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__RefreshCallback__Delegate.Object = NULL;
		__RefreshCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2013 rva 0x70d490 (2012 0x773370)
void UDisAISubState::RegisterDelegate_RequestStateExitCallback( UDishonoredAIBehavior* const OwningBehavior )
{
	const FString TargetFunctionName = FString( TEXT("RequestStateExitCallback") ) + m_StateSuffix.ToString();
	const FName TargetFunction( *TargetFunctionName, FNAME_Add, TRUE );
	if( TargetFunction != NAME_None )
	{
		__RequestStateExitCallback__Delegate.Object = OwningBehavior;
		__RequestStateExitCallback__Delegate.FunctionName = TargetFunction;
	}
	else
	{
		__RequestStateExitCallback__Delegate.Object = NULL;
		__RequestStateExitCallback__Delegate.FunctionName = NAME_None;
	}
}

// DISHONORED(port): 2013 rva 0x710540 (2012 0x7750d0): the sub-state is bound to its behaviour and the behaviour's brain,
// takes its tweaks, and starts out paused - OnEnterState is what unpauses it.
void UDisAISubState::InitSubState( UDishonoredAIBehavior* const OwningBehavior, UDisTweaks_AISubState* const SubStateTweaks )
{
	m_pOwningBehavior = OwningBehavior;
	m_pOwningBrain = OwningBehavior->m_pOwningBrain;

	// DISHONORED(port): agent DF - the desires are cached as a script interface so the garbage collector can null them,
	// and initialised immediately (which binds them to the pawn's components at this sub-state's own priority and leaves
	// them paused).
	IDisDesiresInterface* OwnDesires = GetDesires();
	DisSetScriptInterface( m_pDesires, OwnDesires ? OwnDesires->GetUObjectInterfaceDisDesiresInterface() : NULL );
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->InitializeDesires();
	}

	if( GetTweaks_Derived() != SubStateTweaks )
	{
		SetTweaks_Derived( SubStateTweaks );
		ApplyTweakChanges();
	}

	m_bIsSubStatePaused = TRUE;
	m_pFilterStimMask = (FPointer)BuildFilterStimMask();
	InitSubState_Derived();
}

// DISHONORED(port): 2013 rva 0x705850 (2012 0x768470): the parameter's OnPending plants the behaviour and the body
// intention on this state before the preconditions are asked, so a precondition can already read them.
UBOOL UDisAISubState::ArePreconditionsMet( FDisNativeStateParam& Params )
{
	Params.OnPending( this, m_pOwningBehavior );
	return ArePreconditionsMet_Derived();
}

// DISHONORED(port): 2013 rva 0x705880 (2012 0x77b5f0): the entry is split in two. BeginSubState_Derived always runs;
// the unpause half (resume the desires, ResumeSubState_Derived) is skipped for UDisAISubStateInit, which is the machine's
// idle state and must not look like a running action. The behaviour's OnEnterCallback_<Suffix> fires last.
void UDisAISubState::OnEnterState( UDishonoredNativeState* LastState )
{
	GDisAISubStateEnters++;
	DisAINoteSubStateEnter( GetClass(), LastState ? LastState->GetClass() : NULL );
	BeginSubState_Derived();

	if( GetClass() != UDisAISubStateInit::StaticClass() )
	{
		m_bIsSubStatePaused = FALSE;
		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->ResumeDesires( m_BodyIntentionBeforeEnterState );
		}
		ResumeSubState_Derived();
	}

	delegateOnEnterCallback( this, LastState );
}

// DISHONORED(port): 2013 rva 0x724420 (2012 0x77b6a0): a real transition (NextState != NULL) hands the next state the
// body intention the pawn is in now, which is what its OnPending carried and what its OnEnterState resumes against; a
// NULL next state means the machine is being destroyed. The callback fires before the pause, and the pause only runs if
// the state was not already paused.
void UDisAISubState::OnExitState( UDishonoredNativeState* NextState )
{
	const UBOOL bIsBeingTerminated = ( NextState == NULL );

	if( NextState )
	{
		ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
		UDisAISubState* NextSubState = (UDisAISubState*)NextState;
		NextSubState->m_BodyIntentionBeforeEnterState.m_IntendedBodyStance = Pawn->GetBodyStance();
		NextSubState->m_BodyIntentionBeforeEnterState.m_pDesiredPrimaryItemClass = Pawn->GetDesiredPrimaryItem();
		NextSubState->m_BodyIntentionBeforeEnterState.m_pDesiredSecondaryItemClass = Pawn->GetDesiredSecondaryItem();
	}

	delegateOnExitCallback( this, NextState );

	if( !m_bIsSubStatePaused )
	{
		m_bIsSubStatePaused = TRUE;
		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->PauseDesires();
		}
		PauseSubState_Derived( bIsBeingTerminated );
		if( !bIsBeingTerminated )
		{
			m_pOwningBrain->ClearAllMinAttention( DMALT_AISubState );
		}
	}

	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubState::OnOtherActorTerminatedEvent );
	}
	m_ActionTargetProxy.ClearAttnProxy();

	// DISHONORED(port): a sub-state leaving for good unbinds its desires from the components; one merely changing state
	// only withdraws the orders, so the next state can re-issue its own against the same components.
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		if( bIsBeingTerminated )
		{
			Desires->FinalizeDesires();
		}
		else
		{
			Desires->StopDesires();
		}
	}
	EndSubState_Derived( bIsBeingTerminated );
}

// DISHONORED(port): 2013 rva 0x724590 (2012 0x77b810): a reset is an exit and a re-entry of the same state without going
// through the machine, so the state keeps its slot while its action starts over. UDisAISubStateInit is exempt: the idle
// state has nothing to restart, so only its OnResetCallback_<Suffix> fires. The return value is FALSE in the base (the
// folded xor eax,eax at 2012 rva 0xd9220), i.e. the base does not report that it handled the reset itself.
UBOOL UDisAISubState::OnResetState()
{
	if( !IsA( UDisAISubStateInit::StaticClass() ) )
	{
		ADishonoredNPCPawn* Pawn = m_pOwningBrain->m_pOwningPawn;
		FDisBodyIntention PawnExistingBodyIntention;
		PawnExistingBodyIntention.m_IntendedBodyStance = Pawn->GetBodyStance();
		PawnExistingBodyIntention.m_pDesiredPrimaryItemClass = Pawn->GetDesiredPrimaryItem();
		PawnExistingBodyIntention.m_pDesiredSecondaryItemClass = Pawn->GetDesiredSecondaryItem();

		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->PauseDesires();
		}
		PauseSubState_Derived( FALSE );
		m_pOwningBrain->ClearAllMinAttention( DMALT_AISubState );
		ClearActionTargetProxy();
		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->StopDesires();
		}

		delegateOnExitCallback( this, this );
		EndSubState_Derived( FALSE );

		BeginSubState_Derived();
		delegateOnEnterCallback( this, this );

		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->ResumeDesires( PawnExistingBodyIntention );
		}
		ResumeSubState_Derived();
	}

	delegateOnResetCallback( this );
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x705930 (2012 0x7684a0)
void UDisAISubState::TickState( FLOAT DeltaSeconds )
{
	GDisAISubStateTicks++;
	delegateTickCallback( this, DeltaSeconds );
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->TickDesires( DeltaSeconds );
	}
}

// DISHONORED(port): 2013 rva 0x705dd0 (2012 0x768600): the thinking half, driven by the brain's refresh rather than by
// the frame, so the behaviour's RefreshCallback_<Suffix> is where a sub-state decides to leave.
void UDisAISubState::RefreshSubState( const FLOAT TimeSinceLastThought )
{
	delegateRefreshCallback( this, TimeSinceLastThought );
}

// DISHONORED(port): 2013 rva 0x7059c0 (2012 0x768530): the base UDishonoredNativeState demands the machine's default
// state here; a sub-state asks its behaviour instead, through RequestStateExitCallback_<Suffix>, because the behaviour is
// the only thing that knows which sub-state should follow.
void UDisAISubState::RequestStateExit_Derived()
{
	delegateRequestStateExitCallback( this );
}

// DISHONORED(port): 2013 rva 0x705d80 (2012 0x768560)
void UDisAISubState::OnOwningBehaviorResume( const FDisBodyIntention& PreviousBodyIntention )
{
	m_bIsSubStatePaused = FALSE;
	if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
	{
		Desires->ResumeDesires( PreviousBodyIntention );
	}
	ResumeSubState_Derived();
}

// DISHONORED(port): 2013 rva 0x724750 (2012 0x7685b0): note the order - the derived pause runs first and the desires and
// the minimum attention are only released when the behaviour is pausing rather than terminating.
void UDisAISubState::OnOwningBehaviorPause( UBOOL bIsBeingTerminated )
{
	m_bIsSubStatePaused = TRUE;
	PauseSubState_Derived( bIsBeingTerminated );

	if( !bIsBeingTerminated )
	{
		if( IDisDesiresInterface* Desires = DisGetScriptInterface( m_pDesires ) )
		{
			Desires->PauseDesires();
		}
		m_pOwningBrain->ClearAllMinAttention( DMALT_AISubState );
	}
}

// DISHONORED(port): 2013 rva 0x712720 (2012 0x777300)
void UDisAISubState::SetActionTargetProxy( FDisAttentionProxy ActionTarget )
{
	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubState::OnOtherActorTerminatedEvent );
	}

	m_ActionTargetProxy = ActionTarget;

	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->RegisterToEvent( EventType, this, &UDisAISubState::OnOtherActorTerminatedEvent );
	}
}

// DISHONORED(port): 2013 rva 0x7127f0 (2012 0x7773d0)
void UDisAISubState::ClearActionTargetProxy()
{
	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubState::OnOtherActorTerminatedEvent );
	}
	m_ActionTargetProxy.ClearAttnProxy();
}

// DISHONORED(port): 2013 rva 0x739b30 (2012 0x7763b0). Retail's PDB attributes this one to disaisubprocess.cpp beside its
// sub-process twin; it is kept with its class here.
void UDisAISubState::OnOtherActorTerminatedEvent( const FArkGameEvent& Event )
{
	if( m_ActionTargetProxy.IsEqualToActor( *(const AActor*)Event.m_pInstigator ) )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubState::OnOtherActorTerminatedEvent );
		m_ActionTargetProxy.ClearAttnProxy();
	}
}

// DISHONORED(port): 2013 rva 0x7127b0 (2012 0x777390)
void UDisAISubState::BeginDestroy()
{
	if( m_ActionTargetProxy.HasActorReference() )
	{
		const INT EventType = GDisAIEvent_OtherActorTerminated;
		FArkGameEventDispatcher::GetInstance()->UnregisterToEvent( EventType, this, &UDisAISubState::OnOtherActorTerminatedEvent );
	}
	Super::BeginDestroy();
}

// DISHONORED(port): 2013 rva 0x705e10 (2012 0x768640): the filter mask is a raw pointer into per-class static data, so it
// is rebuilt rather than loaded.
// DISHONORED(bringup): as UDisAISubProcess::PostGameLoad_SubProcess - retail reaches this through the DisSaveLoad vtable
// slot (+280), which UObject does not declare in this tree, so nothing calls it yet.
void UDisAISubState::PostGameLoad_SubState()
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

// DISHONORED(port): 2013 rva 0x705820 (2012 0x768440): the state-change parameter's pending hook. The machine passes the
// state it is about to enter and its own managed object, which for a behaviour FSM is the behaviour, so the state gets
// both its behaviour and that behaviour's brain from one call, plus the body intention the requester recorded.
void FDisAISubState_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UDisAISubState* SubState = (UDisAISubState*)PendingState;
	UDishonoredAIBehavior* Behavior = (UDishonoredAIBehavior*)ManagedObject;
	SubState->m_pOwningBehavior = Behavior;
	SubState->m_pOwningBrain = Behavior->m_pOwningBrain;
	SubState->m_BodyIntentionBeforeEnterState = m_BodyIntentionBeforeEnterState;
}

/*-----------------------------------------------------------------------------
	DISHONORED(bringup): GameSave / GameLoad (2013 rvas 0x710620 / 0x7126c0, 2012 0x7734b0 / 0x7734e0) are not ported, for
	the same reason as UDisAISubProcess's: DisSaveLoadObject and DisSaveAISubTweakReference /
	DisLoadAISubTweakReference (2013 0x7312a0 / 0x731310) are unported, and the DisSaveLoad vtable slots do not exist.
	The retail bodies, so landing them is a transcription:

	  GameSave( FArchive& Ar, ESaveLoadLocation Location )
	      DisSaveLoadObject( Ar, this );
	      DisSaveAISubTweakReference<UDisTweaks_AISubState, UDisTweaks_AIBehavior>( Ar, m_pSubStateTweaks,
	          &UDisTweaks_AIBehavior::m_SubStateTweak );

	  GameLoad( FArchive& Ar, ESaveLoadLocation Location )
	      DisSaveLoadObject( Ar, this );
	      DisLoadAISubTweakReference<UDisTweaks_AISubState, UDisTweaks_AIBehavior>( Ar, m_pSubStateTweaks,
	          &UDisTweaks_AIBehavior::m_SubStateTweak );
-----------------------------------------------------------------------------*/

// ---- agent CG: the tweak-interface pair, defined here rather than in the cpptext ----
// DISHONORED(port): 2013 rva 0x7059f0 (2012 0x16f40) and its folded getter; see the note in disaisubprocess.cpp.
UDisTweaksBase* UDisAISubState::GetTweaks_Derived()
{
	return m_pSubStateTweaks;
}

void UDisAISubState::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pSubStateTweaks = (UDisTweaks_AISubState*)Tweaks;
}
