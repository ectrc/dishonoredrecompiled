// DishonoredGame/src/disconv_conditions.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (35):
//   0x8f87c0  public: static void __cdecl UDisConv_Condition::InitializePrivateStaticClassUDisConv_Condition(void)
//   0x8f87e0  protected: virtual void __thiscall UDisConv_Condition::GetClassVars_Derived(struct FDisConvClassVars &)const
//   0x8f8800  public: virtual void __thiscall UDisConv_ConversationFired::PostLoad(void)
//   0x8f9a10  protected: virtual int __thiscall UDisConv_IsObjectiveComplete::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x8f9a60  protected: virtual int __thiscall UDisConv_HasObjective::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x8fb0d0  public: virtual void __thiscall UDisConv_Condition::GetConditionText(class FString &)const
//   0x8fdf30  public: virtual void __thiscall UDisConv_Condition::DrawExtraClientInfo(class FCanvas *, struct FIntPoint const &, int, int, unsigned int, enum eDisDialogZoomLevel)const
//   0x8fe140  public: virtual void __thiscall UDisConv_HasInventoryItem::GetConditionText(class FString &)const
//   0x8fe2b0  public: virtual void __thiscall UDisConv_HasInventoryItemEquipped::GetConditionText(class FString &)const
//   0x8fe390  public: virtual void __thiscall UDisConv_SeenDialogLabel::GetConditionText(class FString &)const
//   0x8fe530  public: virtual void __thiscall UDisConv_IsObjectiveComplete::GetConditionText(class FString &)const
//   0x8fe610  public: virtual void __thiscall UDisConv_HasObjective::GetConditionText(class FString &)const
//   0x906150  protected: virtual int __thiscall UDisConv_HasInventoryItem::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x9061f0  protected: virtual int __thiscall UDisConv_HasInventoryItemEquipped::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x9062a0  protected: virtual int __thiscall UDisConv_SeenDialogLabel::PlayNode_Derived(unsigned int &, struct FDisDialogRunningInstance *)const
//   0x90b970  public: static class UClass * __cdecl UDisConv_Condition::GetPrivateStaticClassUDisConv_Condition(wchar_t const *)
//   0x90ba00  public: static class UClass * __cdecl UDisConv_Comparison::GetPrivateStaticClassUDisConv_Comparison(wchar_t const *)
//   0x90ba90  public: static class UClass * __cdecl UDisConv_HasInventoryItem::GetPrivateStaticClassUDisConv_HasInventoryItem(wchar_t const *)
//   0x90bb20  public: static class UClass * __cdecl UDisConv_HasInventoryItemEquipped::GetPrivateStaticClassUDisConv_HasInventoryItemEquipped(wchar_t const *)
//   0x90bbb0  public: static class UClass * __cdecl UDisConv_SeenDialogLabel::GetPrivateStaticClassUDisConv_SeenDialogLabel(wchar_t const *)
//   0x90bc40  public: static class UClass * __cdecl UDisConv_IsObjectiveComplete::GetPrivateStaticClassUDisConv_IsObjectiveComplete(wchar_t const *)
//   0x90bcd0  public: static class UClass * __cdecl UDisConv_HasObjective::GetPrivateStaticClassUDisConv_HasObjective(wchar_t const *)
//   0x90db20  public: static class UClass * __cdecl UDisConv_Condition::StaticClassNoInline(void)
//   0x90db50  public: static void __cdecl UDisConv_Comparison::InitializePrivateStaticClassUDisConv_Comparison(void)
//   0x90db70  public: static void __cdecl UDisConv_HasInventoryItem::InitializePrivateStaticClassUDisConv_HasInventoryItem(void)
//   0x90db90  public: static void __cdecl UDisConv_HasInventoryItemEquipped::InitializePrivateStaticClassUDisConv_HasInventoryItemEquipped(void)
//   0x90dbb0  public: static void __cdecl UDisConv_SeenDialogLabel::InitializePrivateStaticClassUDisConv_SeenDialogLabel(void)
//   0x90dbd0  public: static void __cdecl UDisConv_IsObjectiveComplete::InitializePrivateStaticClassUDisConv_IsObjectiveComplete(void)
//   0x90dbf0  public: static void __cdecl UDisConv_HasObjective::InitializePrivateStaticClassUDisConv_HasObjective(void)
//   0x90e400  public: static class UClass * __cdecl UDisConv_Comparison::StaticClassNoInline(void)
//   0x90e430  public: static class UClass * __cdecl UDisConv_HasInventoryItem::StaticClassNoInline(void)
//   0x90e460  public: static class UClass * __cdecl UDisConv_HasInventoryItemEquipped::StaticClassNoInline(void)
//   0x90e490  public: static class UClass * __cdecl UDisConv_SeenDialogLabel::StaticClassNoInline(void)
//   0x90e4c0  public: static class UClass * __cdecl UDisConv_IsObjectiveComplete::StaticClassNoInline(void)
//   0x90e4f0  public: static class UClass * __cdecl UDisConv_HasObjective::StaticClassNoInline(void)
