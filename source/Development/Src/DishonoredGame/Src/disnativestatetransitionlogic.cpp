// DishonoredGame/src/disnativestatetransitionlogic.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x692a00  public: static void __cdecl UDisNativeStateTransitionLogic::InitializePrivateStaticClassUDisNativeStateTransitionLogic(void)
//   0x6a3c90  public: unsigned int __thiscall UDisNativeStateTransitionLogic::CanTransition(class UDishonoredNativeState const *, class UClass *)const
//   0x6a9380  public: virtual void __thiscall UDisNativeStateTransitionLogic::InitTransitionLogic(class TArray<class UDishonoredNativeState *, class FDefaultAllocator> const &)
//   0x6ac600  public: static class UClass * __cdecl UDisNativeStateTransitionLogic::GetPrivateStaticClassUDisNativeStateTransitionLogic(wchar_t const *)
//   0x6ad330  public: static class UClass * __cdecl UDisNativeStateTransitionLogic::StaticClassNoInline(void)

// ---- agent AJ ports ----

#include "DishonoredGame.h"

// DISHONORED(written): FStateTransitionLogicKey is the (from class, to class) pair the 2012 PDB TMap uses; the generated struct
// has no hash/equality, both live here next to the only user
static inline UBOOL operator==( const FStateTransitionLogicKey& A, const FStateTransitionLogicKey& B )
{
	return A.m_pFromClass == B.m_pFromClass && A.m_pToClass == B.m_pToClass;
}

static inline DWORD GetTypeHash( const FStateTransitionLogicKey& Key )
{
	return PointerHash( Key.m_pFromClass ) ^ PointerHash( Key.m_pToClass, 7 );
}

static FStateTransitionLogicKey DisTransitionKey( UClass* From, UClass* To )
{
	FStateTransitionLogicKey Key(EC_EventParm);
	Key.m_pFromClass = From;
	Key.m_pToClass = To;
	return Key;
}

// DISHONORED(written): 2013 rva 0x67bbe0 (2012 0x6a9380): the designer's m_Transitions become the class-pair map, then every
// (from, to) pair of the machine's states is resolved by walking both class chains up to UObject (the first defined entry
// wins, EStateTransitionLogicResult_Call when none is), so the runtime lookup never walks
void UDisNativeStateTransitionLogic::InitTransitionLogic( const TArray<UDishonoredNativeState*>& States )
{
	if( m_bInitialized )
	{
		return;
	}
	TMap<FStateTransitionLogicKey,EStateTransitionLogicResult> Defined;
	for( INT Index = 0; Index < m_Transitions.Num(); Index++ )
	{
		const FStateTransitionLogic& Transition = m_Transitions(Index);
		Defined.Set( DisTransitionKey( Transition.m_pFromClass, Transition.m_pToClass ), (EStateTransitionLogicResult)Transition.m_Result );
	}
	for( INT FromIndex = 0; FromIndex < States.Num(); FromIndex++ )
	{
		for( INT ToIndex = 0; ToIndex < States.Num(); ToIndex++ )
		{
			UClass* FromClass = States(FromIndex)->GetClass();
			UClass* ToClass = States(ToIndex)->GetClass();
			EStateTransitionLogicResult Result = EStateTransitionLogicResult_Call;
			UBOOL bFound = FALSE;
			for( UClass* From = FromClass; From && From != UObject::StaticClass() && !bFound; From = From->GetSuperClass() )
			{
				for( UClass* To = ToClass; To && To != UObject::StaticClass(); To = To->GetSuperClass() )
				{
					const EStateTransitionLogicResult* Entry = Defined.Find( DisTransitionKey( From, To ) );
					if( Entry )
					{
						Result = *Entry;
						bFound = TRUE;
						break;
					}
				}
			}
			m_TransitionMap.Set( DisTransitionKey( FromClass, ToClass ), Result );
		}
	}
	m_bInitialized = TRUE;
}

// DISHONORED(written): 2013 rva 0x672720 (2012 0x6a3c90): an unmapped pair or a _Call entry asks the state
// (CanTransitionExternal); otherwise only _Allowed passes
UBOOL UDisNativeStateTransitionLogic::CanTransition( const UDishonoredNativeState* FromState, UClass* ToStateID ) const
{
	const EStateTransitionLogicResult* Entry = m_TransitionMap.Find( DisTransitionKey( FromState->GetClass(), ToStateID ) );
	if( !Entry || *Entry == EStateTransitionLogicResult_Call )
	{
		return FromState->CanTransitionExternal( ToStateID );
	}
	return *Entry == EStateTransitionLogicResult_Allowed;
}
