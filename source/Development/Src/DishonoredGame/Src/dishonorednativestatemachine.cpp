// DishonoredGame/src/dishonorednativestatemachine.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x6927b0  public: static void __cdecl UDishonoredNativeStateMachine::InitializePrivateStaticClassUDishonoredNativeStateMachine(void)
//   0x6927d0  public: __thiscall FDisNativeFSMRejectedInfo::FDisNativeFSMRejectedInfo(class UClass *, class UClass *, enum eDisNativeFSMRejectReason)
//   0x692810  public: unsigned int __thiscall FDisNativeFSMRejectedInfo::operator!=(struct FDisNativeFSMRejectedInfo const &)const
//   0x692850  public: void __thiscall UDishonoredNativeStateMachine::DoStateChange(void)
//   0x6928c0  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::GetCurrentlyActiveState(void)const
//   0x6928e0  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::GetPendingState(void)const
//   0x692900  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::GetLogicalState(void)const
//   0x692920  public: class UClass const * __thiscall UDishonoredNativeStateMachine::GetPendingStateID(void)const
//   0x692940  public: void __thiscall UDishonoredNativeStateMachine::LockFSM(unsigned int)
//   0x692960  public: void __thiscall UDishonoredNativeStateMachine::SavePartialState(class FArchive &, enum ESaveLoadLocation)
//   0x6929b0  public: void __thiscall UDishonoredNativeStateMachine::PostLoadPartialState(void)
//   0x6929e0  private: void __thiscall UDishonoredNativeStateMachine::ClearPendingState(void)
//   0x695960  public: unsigned int __thiscall UDishonoredNativeStateMachine::IsCurState(class UClass const *, unsigned int)const
//   0x69a240  private: void __thiscall UDishonoredNativeStateMachine::DebugStoreRejectedStateInfo(struct FDisNativeFSMRejectedInfo const &)
//   0x69a2c0  public: void __thiscall UDishonoredNativeStateMachine::OnPawnShutDown(class ADishonoredPawn const &)
//   0x6a0330  public: void __thiscall UDishonoredNativeStateMachine::TickStateMachine(float)
//   0x6a1d80  public: void __thiscall UDishonoredNativeStateMachine::GetAllStateIDs(struct TMemStackArray<class UClass *> &)const
//   0x6a3970  private: void __thiscall UDishonoredNativeStateMachine::DemandStateChange(class UDishonoredNativeState *, struct FDisNativeStateParam &)
//   0x6a3be0  public: void __thiscall UDishonoredNativeStateMachine::LoadPartialState(class FArchive &, enum ESaveLoadLocation)
//   0x6a3c50  public: class UDishonoredNativeState * __thiscall UDishonoredNativeStateMachine::FindState(class UClass *)const
//   0x6a59b0  public: unsigned int __thiscall UDishonoredNativeStateMachine::CanTransitionTo(class UClass *, class UDishonoredNativeState * *)const
//   0x6a5a30  public: unsigned int __thiscall UDishonoredNativeStateMachine::RequestStateChange(struct FDisNativeStateParam &, class UObject * const, unsigned int)
//   0x6a89e0  public: void __thiscall UDishonoredNativeStateMachine::DestroyFSM(void)
//   0x6a9270  public: void __thiscall UDishonoredNativeStateMachine::BuildNativeStateMap(void)
//   0x6a9360  public: virtual void __thiscall UDishonoredNativeStateMachine::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6ac450  public: static class UClass * __cdecl UDishonoredNativeStateMachine::GetPrivateStaticClassUDishonoredNativeStateMachine(wchar_t const *)
//   0x6ac4e0  public: void __thiscall UDishonoredNativeStateMachine::InitFSM(class UObject * const, struct FDisNativeStateParam &, struct TMemStackArray<class UDishonoredNativeState *> *)
//   0x6ad300  public: static class UClass * __cdecl UDishonoredNativeStateMachine::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(written): 2013 rva 0x65f930 (2012 0x6928c0, same bytes): nothing is active while DestroyFSM runs
UDishonoredNativeState* UDishonoredNativeStateMachine::GetCurrentlyActiveState() const
{
	return m_bDestroyFSMCalled ? NULL : m_pCurrentState;
}

// DISHONORED(written): 2013 rva 0x65f950 (2012 0x6928e0)
UDishonoredNativeState* UDishonoredNativeStateMachine::GetPendingState() const
{
	return m_bDestroyFSMCalled ? NULL : m_pPendingState;
}

// DISHONORED(written): 2013 rva 0x65f970 (2012 0x692900, same bytes): the state the machine is about to be in
UDishonoredNativeState* UDishonoredNativeStateMachine::GetLogicalState() const
{
	if( m_bDestroyFSMCalled )
	{
		return NULL;
	}
	return m_pPendingState ? m_pPendingState : m_pCurrentState;
}

// DISHONORED(written): 2013 rva 0x65f990 (2012 0x692920, same bytes)
const UClass* UDishonoredNativeStateMachine::GetPendingStateID() const
{
	return m_bDestroyFSMCalled ? NULL : m_pPendingStateID;
}

// DISHONORED(written): 2013 rva 0x6726e0 (2012 0x6a3c50, same bytes): m_NativeStateMap is keyed by the state class
UDishonoredNativeState* UDishonoredNativeStateMachine::FindState( UClass* StateID ) const
{
	UDishonoredNativeState* const* State = m_NativeStateMap.Find( StateID );
	return State ? *State : NULL;
}
