// DishonoredGame/src/stateplayeruppernav.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6b9770  public: static class UClass * __cdecl UStatePlayerUpperNav::GetPrivateStaticClassUStatePlayerUpperNav(wchar_t const *)
//   0x6bf2f0  public: static void __cdecl UStatePlayerUpperNav::InitializePrivateStaticClassUStatePlayerUpperNav(void)
//   0x6c2380  public: static class UClass * __cdecl UStatePlayerUpperNav::StaticClassNoInline(void)
//   0x6c6400  public: virtual void __thiscall UStatePlayerUpperNav::OnEnterState(class UDishonoredNativeState *)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the item-bound machine's navigation state (Inc/CppText/UStatePlayerUpperNav.h), 2013 body (2012 also
	called the base OnEnterState first, an empty body). DISHONORED(written): the item is NULL-tested (retail reads it directly).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

void DishonoredLinkIM_StatePlayerUpperNav()
{
}

// 2013 0x674030 (280 B): the item's contexts are cancelled; a physically equipped item sets its nav anim state on the picker (the
// item follows) and is brought in; otherwise the picker is cleared
void UStatePlayerUpperNav::OnEnterState( UDishonoredNativeState* LastState )
{
	FLOAT BlendTime = 0.2f;
	UStatePlayerAction* LastAction = Cast<UStatePlayerAction>( LastState );
	if( LastAction )
	{
		BlendTime = LastAction->m_fTimeToGo;
	}
	UDishonoredInventoryItem* Item = m_pPawn->GetEquippedItem( m_pActionFSM->m_BoundToItemUsage );
	if( Item == NULL )
	{
		return;
	}
	Item->CancelActiveContexts( FALSE );
	if( Item->ItemIsPhysicallyEquipped() )
	{
		const FName AnimState = Item->GetNavAnimState();
		if( m_pPawn->SetAnimState( AnimState, m_pActionFSM->m_BoundAnimStatePicker, this, BlendTime, this ) )
		{
			Item->SyncPawnAnimState( AnimState, this, BlendTime, this );
		}
		m_pPawn->TransitionItemIn( m_pActionFSM->m_BoundToItemUsage, m_pActionFSM->m_ActionUsage, FALSE, FALSE );
	}
	else
	{
		m_pPawn->ClearAnimState( m_pActionFSM->m_BoundAnimStatePicker, BlendTime );
		Item->SyncPawnAnimState( NAME_None, NULL, BlendTime, NULL );
	}
}
