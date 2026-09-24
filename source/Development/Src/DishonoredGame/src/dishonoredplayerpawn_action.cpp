// DishonoredGame/src/dishonoredplayerpawn_action.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x6fae20  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::Tick_PlayerAction(float, enum ELevelTick)
//   0x6fae70  public: virtual void __thiscall ADishonoredPlayerPawn::ChangeAnimStateMotionModes(enum ERootMotionMode, enum ERootMotionRotationMode)
//   0x6faf50  public: virtual void __thiscall ADishonoredPlayerPawn::StopRagdolling(void)
//   0x6fb030  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::MAT_IsUnequipped(void)const
//   0x6fe380  public: virtual void __thiscall ADishonoredPlayerPawn::setPhysics(unsigned char, class AActor *, class FVector, class USkeletalMeshComponent *, class FName)
//   0x6fe530  public: void __thiscall ADishonoredPlayerPawn::Look(class FVector, class FRotator, class ADisDoor *)
//   0x6fe570  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_SetForceInventory(unsigned int)
//   0x704a00  protected: void __thiscall ADishonoredPlayerPawn::GatherAnimStateArrays(enum eAnimState_Picker, struct FDisPawnAnimState_PickerInfo &)
//   0x704ad0  protected: unsigned int __thiscall ADishonoredPlayerPawn::RequestActionHelper(class UDishonoredNativeStateMachine *, unsigned int, struct FStateSharedActionBase_PlayerOnly_Param &, class UObject *, unsigned int)
//   0x704b30  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::AnimState_TriggerExit_Derived(int, class UDishonoredNativeState *, float)
//   0x704be0  protected: virtual void __thiscall ADishonoredPlayerPawn::AnimState_LockoutPicker_Derived(int, float, unsigned int)
//   0x704d00  public: virtual void __thiscall ADishonoredPlayerPawn::AnimState_SyncEnd(enum eAnimState_Picker)
//   0x704d50  public: void __thiscall ADishonoredPlayerPawn::DisableArmFollow(enum EDisEquipUsage, unsigned int, float)
//   0x704db0  public: void __thiscall ADishonoredPlayerPawn::DisableArmOffset(enum EDisEquipUsage, unsigned int, float)
//   0x704e10  public: void __thiscall ADishonoredPlayerPawn::BreakOutOfLean(void)
//   0x704e30  public: void __thiscall ADishonoredPlayerPawn::BreakOutOfLook(void)
//   0x704e50  public: unsigned int __thiscall ADishonoredPlayerPawn::CanApplyWalkFlags(void)const
//   0x704e80  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IgnoreBlockingBy(class AActor const *)const
//   0x704f80  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::InSyncedAction(void)const
//   0x704fd0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::StartRagdolling(void)
//   0x705120  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_BlendOut(class UInterpGroupInst *)
//   0x7051f0  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_FinishPlayerGroup(class UInterpGroupPlayer *, class UInterpGroupInstPlayer *)
//   0x70bb90  public: unsigned int __thiscall ADishonoredPlayerPawn::RequestAction(enum eDisPlayerActionUsage, struct FStateSharedActionBase_PlayerOnly_Param &, class UObject *, unsigned int)
//   0x70bc90  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::RequestItemAction(class UDisItemContext *, struct FStateSharedActionBase_Param &, class UStateSharedActionBase * &, unsigned int)
//   0x70bce0  public: void __thiscall ADishonoredPlayerPawn::Lean(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int)
//   0x70fc00  protected: virtual void __thiscall ADishonoredPlayerPawn::OnSprintChange_Derived(void)
//   0x70fcc0  public: virtual void __thiscall ADishonoredPlayerPawn::MAT_BeginPlayerGroup(class UInterpGroupPlayer *, class UInterpGroupInstPlayer *)
//   0x720260  public: virtual void __thiscall ADishonoredPlayerPawn::PreBeginPlay_Actions(void)
