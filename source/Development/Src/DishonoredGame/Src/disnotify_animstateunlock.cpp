// DishonoredGame/src/disnotify_animstateunlock.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6d0830  public: static void __cdecl UDisNotify_AnimStateUnlock::InitializePrivateStaticClassUDisNotify_AnimStateUnlock(void)
//   0x6dde10  public: static class UClass * __cdecl UDisNotify_AnimStateUnlock::GetPrivateStaticClassUDisNotify_AnimStateUnlock(wchar_t const *)
//   0x6e9df0  public: static class UClass * __cdecl UDisNotify_AnimStateUnlock::StaticClassNoInline(void)
//   0x6f6810  public: virtual void __thiscall UDisNotify_AnimStateUnlock::Notify(class UAnimNodeSequence *)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): 2012 0x6f6810 / 2013 0x69b5e0 (172 B, same bytes). A dominant sequence of an anim state lets its picker in
	again, blending over the time left (at most 0.2 s).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

void DishonoredLinkIM_DisNotifyAnimStateUnlock()
{
}

void UDisNotify_AnimStateUnlock::Notify( UAnimNodeSequence* NodeSeq )
{
	UDishonoredAnimNodeSeq* DisSeq = NodeSeq->SkelComponent ? Cast<UDishonoredAnimNodeSeq>( NodeSeq ) : NULL;
	if( DisSeq == NULL || !DisSeq->IsDominantAnimStateSequence() )
	{
		return;
	}
	AActor* OwnerActor = NodeSeq->SkelComponent->GetOwner();
	IDisAnimStateOwnerInterface* Owner = OwnerActor ? IDisAnimStateOwnerInterface::FromObject( OwnerActor ) : NULL;
	UDisAnimStateComponent* Comp = Owner ? Owner->AnimState_GetAnimStateComp() : NULL;
	if( Comp )
	{
		Comp->UnlockPicker( m_AnimStateType, Min( NodeSeq->GetTimeLeft(), 0.2f ) );
	}
}
