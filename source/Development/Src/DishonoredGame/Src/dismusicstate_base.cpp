// DishonoredGame/src/dismusicstate_base.cpp
// PDB functions attributed to this file (7):
//   0x8af200  public: static void __cdecl UDisMusicState_Base::InitializePrivateStaticClassUDisMusicState_Base(void)
//   0x8af220  public: virtual void __thiscall UDisMusicState_Base::TickState(float)
//   0x8af270  public: virtual void __thiscall UDisMusicState_Base::OnExitState(class UDishonoredNativeState *)
//   0x8b1720  public: void __thiscall UDisMusicState_Base::RegisterDelegate_TickCallback(class UObject *, class FString const &)
//   0x8b8a70  public: static class UClass * __cdecl UDisMusicState_Base::GetPrivateStaticClassUDisMusicState_Base(wchar_t const *)
//   0x8b8b00  public: virtual void __thiscall FDisMusicState_Base_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x8bc730  public: static class UClass * __cdecl UDisMusicState_Base::StaticClassNoInline(void)
//
// DISHONORED(port): agent IH4 (PHASE23 IH4), from the 2012 decompile; 2013 twins on each body. The retail bodies open with a
// call to the (empty, ICF-folded) base-class slot; those calls are kept as Super:: calls.

#include "DishonoredGame.h"
#include "dismusicstateparams.h"

void DishonoredLinkDisMusicStateBase()
{
}

/** 2012 0x8af220 / 2013 0x83dfa0: the manager's TickCallback<suffix> through the state's delegate. */
void UDisMusicState_Base::TickState( FLOAT DeltaSeconds )
{
	Super::TickState( DeltaSeconds );
	delegateTickCallback( this, DeltaSeconds );
}

/** 2012 0x8af270 / 2013 0x83dff0. */
void UDisMusicState_Base::OnExitState( UDishonoredNativeState* NextState )
{
	Super::OnExitState( NextState );
	m_pMusicEvents = NULL;
}

/** 2012 0x8b1720 / 2013 0x841920: the delegate is bound to "TickCallback" + _Suffix on _pOwner, or cleared for a none name. */
void UDisMusicState_Base::RegisterDelegate_TickCallback( UObject* _pOwner, const FString& _Suffix )
{
	static TCHAR TargetFunctionNameBuffer[128];
	appStrcpy( TargetFunctionNameBuffer, TEXT("TickCallback") );
	appStrcat( TargetFunctionNameBuffer, *_Suffix );
	const FName FunctionName( TargetFunctionNameBuffer, FNAME_Add, TRUE );
	if( FunctionName != NAME_None )
	{
		__TickCallback__Delegate.Object = _pOwner;
		__TickCallback__Delegate.FunctionName = FunctionName;
	}
	else
	{
		__TickCallback__Delegate.Object = NULL;
		__TickCallback__Delegate.FunctionName = NAME_None;
	}
}

/** 2012 0x8b8b00 / 2013 0x8484e0: the pending state reads the manager's music events and binds its tick delegate. */
void FDisMusicState_Base_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UDisMusicState_Base* pState = (UDisMusicState_Base*)PendingState;
	pState->m_pMusicEvents = ( (UDisGlobalMusicManager*)ManagedObject )->GetMusicEventsTweaks();
	pState->RegisterDelegate_TickCallback( ManagedObject, pState->m_StateSuffix.ToString() );
}

FDisMusicState_Exploration_Param::FDisMusicState_Exploration_Param() : FDisMusicState_Base_Param( UDisMusicState_Exploration::StaticClass() ) {}
FDisMusicState_Suspense_Param::FDisMusicState_Suspense_Param() : FDisMusicState_Base_Param( UDisMusicState_Suspense::StaticClass() ) {}
FDisMusicState_RatAttack_Param::FDisMusicState_RatAttack_Param() : FDisMusicState_Base_Param( UDisMusicState_RatAttack::StaticClass() ) {}
