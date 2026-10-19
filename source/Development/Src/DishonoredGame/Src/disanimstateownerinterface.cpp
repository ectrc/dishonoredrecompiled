// DishonoredGame/src/disanimstateownerinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x6b1af0  public: unsigned int __thiscall IDisAnimStateOwnerInterface::AnimState_PlayAnim(class IDisAnimStateFiringInterface *, class UAnimNodeSequence *)
//   0x6b1b30  public: unsigned int __thiscall IDisAnimStateOwnerInterface::AnimState_TriggerExit(class IDisAnimStateFiringInterface *, int, class UDishonoredNativeState *, float)
//   0x6b1b80  public: unsigned int __thiscall IDisAnimStateOwnerInterface::AnimState_Advance(class IDisAnimStateFiringInterface *, float)
//   0x6b1bc0  public: unsigned int __thiscall IDisAnimStateOwnerInterface::AnimState_AnimNotFound(class IDisAnimStateFiringInterface *, class FString const &)
//   0x6bdc30  public: static class UClass * __cdecl UDisAnimStateOwnerInterface::GetPrivateStaticClassUDisAnimStateOwnerInterface(wchar_t const *)
//   0x6c0d20  public: static void __cdecl UDisAnimStateOwnerInterface::InitializePrivateStaticClassUDisAnimStateOwnerInterface(void)
//   0x6c0d40  public: void __thiscall IDisAnimStateOwnerInterface::AnimState_LockoutPicker(int, float, unsigned int)
//   0x6c3500  public: static class UClass * __cdecl UDisAnimStateOwnerInterface::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the owner's dispatchers (Inc/CppText/IDisAnimStateOwnerInterface.h). The firing object answers first
	(unless it is the owner itself); FALSE falls back to the owner's own _Derived. 2013 bodies are byte-identical to 2012.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "imdiag.h"

void DishonoredLinkIM_DisAnimStateOwnerInterface()
{
}

// 2012 0x6b1af0 / 2013 0x67d560 (49 B)
UBOOL IDisAnimStateOwnerInterface::AnimState_PlayAnim( IDisAnimStateFiringInterface* FiringObj, UAnimNodeSequence* Seq )
{
	if( FiringObj && FiringObj != this && FiringObj->AnimState_PlayAnim_Derived( Seq ) )
	{
		return TRUE;
	}
	return AnimState_PlayAnim_Derived( Seq );
}

// 2012 0x6b1b30 / 2013 0x67d5a0 (70 B)
UBOOL IDisAnimStateOwnerInterface::AnimState_TriggerExit( IDisAnimStateFiringInterface* FiringObj, INT StateType, UDishonoredNativeState* AttachedState, FLOAT TimeToGo )
{
	DisIMNoteExit( TEXT("exit"), StateType, AttachedState, TimeToGo );
	if( FiringObj && FiringObj != this && FiringObj->AnimState_TriggerExit_Derived( StateType, AttachedState, TimeToGo ) )
	{
		return TRUE;
	}
	return AnimState_TriggerExit_Derived( StateType, AttachedState, TimeToGo );
}

// 2012 0x6b1b80 / 2013 0x67d5f0 (56 B)
UBOOL IDisAnimStateOwnerInterface::AnimState_Advance( IDisAnimStateFiringInterface* FiringObj, FLOAT TimeToGo )
{
	DisIMNoteExit( TEXT("advance"), -1, NULL, TimeToGo );
	if( FiringObj && FiringObj != this && FiringObj->AnimState_Advance_Derived( TimeToGo ) )
	{
		return TRUE;
	}
	return AnimState_Advance_Derived( TimeToGo );
}

// 2012 0x6b1bc0 / 2013 0x67d630 (49 B)
UBOOL IDisAnimStateOwnerInterface::AnimState_AnimNotFound( IDisAnimStateFiringInterface* FiringObj, const FString& SeqName )
{
	IMDIAG( TEXT("anim not found: '%s'"), *SeqName );
	if( FiringObj && FiringObj != this && FiringObj->AnimState_AnimNotFound_Derived( SeqName ) )
	{
		return TRUE;
	}
	return AnimState_AnimNotFound_Derived( SeqName );
}

// 2012 0x6c0d40 / 2013 0x691830 (141 B): the picker's lockout bit, the owner's reaction, a locked picker cleared
void IDisAnimStateOwnerInterface::AnimState_LockoutPicker( INT StateType, FLOAT TimeToGo, UBOOL bLocked )
{
	UDisAnimStateComponent* Comp = AnimState_GetAnimStateComp();
	if( Comp == NULL )
	{
		return;
	}
	Comp->m_StatePickers( StateType ).m_bLockedOut = bLocked ? TRUE : FALSE;
	AnimState_LockoutPicker_Derived( StateType, TimeToGo, bLocked );
	if( bLocked )
	{
		Comp->ClearAnimState( StateType, TimeToGo );
	}
}

IDisAnimStateFiringInterface* IDisAnimStateFiringInterface::FromObject( UObject* Object )
{
	if( Object == NULL )
	{
		return NULL;
	}
	if( Object->IsA( ADishonoredPlayerPawn::StaticClass() ) )
	{
		return (ADishonoredPlayerPawn*)Object;
	}
	if( Object->IsA( UStateSharedActionBase_PlayerOnly::StaticClass() ) )
	{
		return (UStateSharedActionBase_PlayerOnly*)Object;
	}
	if( Object->IsA( UStatePlayerUpperBase::StaticClass() ) )
	{
		return (UStatePlayerUpperBase*)Object;
	}
	return (IDisAnimStateFiringInterface*)Object->GetInterfaceAddress( UDisAnimStateFiringInterface::StaticClass() );
}

IDisAnimStateOwnerInterface* IDisAnimStateOwnerInterface::FromObject( UObject* Object )
{
	if( Object == NULL )
	{
		return NULL;
	}
	if( Object->IsA( ADishonoredPlayerPawn::StaticClass() ) )
	{
		return (ADishonoredPlayerPawn*)Object;
	}
	return (IDisAnimStateOwnerInterface*)Object->GetInterfaceAddress( UDisAnimStateOwnerInterface::StaticClass() );
}
