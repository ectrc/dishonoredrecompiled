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
