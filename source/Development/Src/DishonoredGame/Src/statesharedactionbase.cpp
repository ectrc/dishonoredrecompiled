// DishonoredGame/src/statesharedactionbase.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x692a20  public: virtual unsigned int __thiscall UStateSharedActionBase::OnResetState(void)
//   0x692a30  public: virtual void __thiscall UStateSharedActionBase::SetFiringObject(class UObject *)
//   0x6959b0  protected: __thiscall FStateSharedActionBase_Param::FStateSharedActionBase_Param(void)
//   0x6959c0  public: virtual void __thiscall FDisUseState_Base_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x6959d0  public: virtual void __thiscall UStateSharedActionBase::RequestStateExit_WithTime(float)
//   0x69a330  public: static class UClass * __cdecl UStateSharedActionBase::GetPrivateStaticClassUStateSharedActionBase(wchar_t const *)
//   0x69a3c0  public: virtual void __thiscall UStateSharedActionBase::OnFiringObjectChanged(class UObject *)
//   0x69a3f0  public: unsigned int __thiscall UStateSharedActionBase::ShouldIgnoreBlocking(class AActor const *)const
//   0x69deb0  public: static void __cdecl UStateSharedActionBase::InitializePrivateStaticClassUStateSharedActionBase(void)
//   0x6a0490  public: static class UClass * __cdecl UStateSharedActionBase::StaticClassNoInline(void)
//   0x6a3da0  public: virtual void __thiscall UStateSharedActionBase::OnEnterState(class UDishonoredNativeState *)

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the action base (Inc/CppText/UStateSharedActionBase.h). 2013 bodies checked against 2012: same flow.
	DISHONORED(bringup), named: OnSetFiringObject (the virtual SetFiringObject calls first; no override is ported) and
	UDisItemContext::ItemActionEnded (OnFiringObjectChanged's callee; the item contexts are skeletons).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "imdiag.h"

void DishonoredLinkIM_StateSharedActionBase()
{
}

// 2012 0x6a3da0 / 2013 0x66e050 (75 B)
void UStateSharedActionBase::OnEnterState( UDishonoredNativeState* LastState )
{
	UStateSharedActionBase* LastAction = Cast<UStateSharedActionBase>( LastState );
	m_fTimeToGo = LastAction ? LastAction->m_fTimeToGo : -1.f;
	DoAction();
}

// 2012 0x692a20 / 2013 0x65fa50
UBOOL UStateSharedActionBase::OnResetState()
{
	return DoAction();
}

// 2012 0x692a30 / 2013 0x65fa60
void UStateSharedActionBase::SetFiringObject( UObject* FiringObject )
{
	m_pFiringObj = FiringObject;
}

// 2012 0x69a3c0 / 2013 0x666ee0
void UStateSharedActionBase::OnFiringObjectChanged( UObject* OldFiringObject )
{
	if( Cast<UDisItemContext>( OldFiringObject ) )
	{
		IMDIAG_ONCE( TEXT("UStateSharedActionBase::OnFiringObjectChanged: UDisItemContext::ItemActionEnded (2012 0x863090) is not ported") );
	}
}

// 2012 0x6959d0 / 2013 0x662880 (76 B): RequestStateExit inlined, then the blend time the next state reads
void UStateSharedActionBase::RequestStateExit_WithTime( FLOAT TimeToGo )
{
	RequestStateExit();
	m_fTimeToGo = TimeToGo;
}
