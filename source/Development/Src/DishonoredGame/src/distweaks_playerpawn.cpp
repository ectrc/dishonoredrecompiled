// DishonoredGame/src/distweaks_playerpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (49):
//   0x7221c0  public: static void __cdecl UDisTweaks_PlayerPawn::InitializePrivateStaticClassUDisTweaks_PlayerPawn(void)
//   0x7221e0  public: static void __cdecl UDisTweaks_PlayerStats::InitializePrivateStaticClassUDisTweaks_PlayerStats(void)
//   0x7274d0  public: unsigned int __thiscall UDisTweaks_PlayerStats::IsNPCOfType(class UDisTweaksBase const *, struct FNPCTypeInfo const &)const
//   0x7275a0  public: unsigned int __thiscall UDisTweaks_PlayerStats::IsHostileNPC(class UDisTweaksBase const *)const
//   0x7275c0  public: unsigned int __thiscall UDisTweaks_PlayerStats::IsCivilianNPC(class UDisTweaksBase const *)const
//   0x7275e0  private: virtual void __thiscall UDisTweaks_PlayerPawn::ApplyFallbackChain_Derived(void)
//   0x72d9a0  void __cdecl SetTweaksNames(class TArray<class FString, class FDefaultAllocator> &, class TArray<class UDisTweaksBase *, class FDefaultAllocator> const &)
//   0x72fb60  public: virtual void __thiscall UDisTweaks_PlayerStats::Serialize(class FArchive &)
//   0x72fd10  public: virtual void __thiscall UDisTweaks_PlayerPawn::Serialize(class FArchive &)
//   0x731580  public: void __thiscall UDisTweaks_PlayerStats::GetAffectedStats(struct TMemStackArray<int> &, enum EDisPlayerStat, class UDisTweaksBase const *, class UDisAbstractItem const *, class UClass const *)const
//   0x731730  private: virtual void __thiscall UDisTweaks_PlayerPawn::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x731a40  private: virtual void __thiscall UDisTweaks_PlayerPawn_Actions::GatherTweakChildren_Derived(struct TMemStackArray<struct FDisTweakChildInfo> &)const
//   0x739240  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Actions::GetPrivateStaticClassUDisTweaks_PlayerPawn_Actions(wchar_t const *)
//   0x7392d0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Animation::GetPrivateStaticClassUDisTweaks_PlayerPawn_Animation(wchar_t const *)
//   0x739360  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Attributes::GetPrivateStaticClassUDisTweaks_PlayerPawn_Attributes(wchar_t const *)
//   0x7393f0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Camera::GetPrivateStaticClassUDisTweaks_PlayerPawn_Camera(wchar_t const *)
//   0x739480  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_DebugActions::GetPrivateStaticClassUDisTweaks_PlayerPawn_DebugActions(wchar_t const *)
//   0x739510  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_ItemActions::GetPrivateStaticClassUDisTweaks_PlayerPawn_ItemActions(wchar_t const *)
//   0x7395a0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_MiscellaneousActions::GetPrivateStaticClassUDisTweaks_PlayerPawn_MiscellaneousActions(wchar_t const *)
//   0x739630  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_NavigationActions::GetPrivateStaticClassUDisTweaks_PlayerPawn_NavigationActions(wchar_t const *)
//   0x73c190  public: static class UClass * __cdecl UDisTweaks_PlayerPawn::GetPrivateStaticClassUDisTweaks_PlayerPawn(wchar_t const *)
//   0x73c220  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_FullBodyAnimation::GetPrivateStaticClassUDisTweaks_PlayerPawn_FullBodyAnimation(wchar_t const *)
//   0x73c2b0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_SpecialAnimation::GetPrivateStaticClassUDisTweaks_PlayerPawn_SpecialAnimation(wchar_t const *)
//   0x73c340  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_UpperBodyAnimation::GetPrivateStaticClassUDisTweaks_PlayerPawn_UpperBodyAnimation(wchar_t const *)
//   0x73c3d0  public: static class UClass * __cdecl UDisTweaks_PlayerStats::GetPrivateStaticClassUDisTweaks_PlayerStats(wchar_t const *)
//   0x7406a0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn::StaticClassNoInline(void)
//   0x7406d0  public: static void __cdecl UDisTweaks_PlayerPawn_Actions::InitializePrivateStaticClassUDisTweaks_PlayerPawn_Actions(void)
//   0x7406f0  public: static void __cdecl UDisTweaks_PlayerPawn_Animation::InitializePrivateStaticClassUDisTweaks_PlayerPawn_Animation(void)
//   0x740710  public: static void __cdecl UDisTweaks_PlayerPawn_Attributes::InitializePrivateStaticClassUDisTweaks_PlayerPawn_Attributes(void)
//   0x740730  public: static void __cdecl UDisTweaks_PlayerPawn_Camera::InitializePrivateStaticClassUDisTweaks_PlayerPawn_Camera(void)
//   0x740750  public: static class UClass * __cdecl UDisTweaks_PlayerStats::StaticClassNoInline(void)
//   0x742430  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Actions::StaticClassNoInline(void)
//   0x742460  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Animation::StaticClassNoInline(void)
//   0x742490  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Attributes::StaticClassNoInline(void)
//   0x7424c0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_Camera::StaticClassNoInline(void)
//   0x7424f0  public: static void __cdecl UDisTweaks_PlayerPawn_DebugActions::InitializePrivateStaticClassUDisTweaks_PlayerPawn_DebugActions(void)
//   0x742510  public: static void __cdecl UDisTweaks_PlayerPawn_FullBodyAnimation::InitializePrivateStaticClassUDisTweaks_PlayerPawn_FullBodyAnimation(void)
//   0x742530  public: static void __cdecl UDisTweaks_PlayerPawn_ItemActions::InitializePrivateStaticClassUDisTweaks_PlayerPawn_ItemActions(void)
//   0x742550  public: static void __cdecl UDisTweaks_PlayerPawn_MiscellaneousActions::InitializePrivateStaticClassUDisTweaks_PlayerPawn_MiscellaneousActions(void)
//   0x742570  public: static void __cdecl UDisTweaks_PlayerPawn_NavigationActions::InitializePrivateStaticClassUDisTweaks_PlayerPawn_NavigationActions(void)
//   0x742590  public: static void __cdecl UDisTweaks_PlayerPawn_SpecialAnimation::InitializePrivateStaticClassUDisTweaks_PlayerPawn_SpecialAnimation(void)
//   0x7425b0  public: static void __cdecl UDisTweaks_PlayerPawn_UpperBodyAnimation::InitializePrivateStaticClassUDisTweaks_PlayerPawn_UpperBodyAnimation(void)
//   0x743360  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_DebugActions::StaticClassNoInline(void)
//   0x743390  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_FullBodyAnimation::StaticClassNoInline(void)
//   0x7433c0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_ItemActions::StaticClassNoInline(void)
//   0x7433f0  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_MiscellaneousActions::StaticClassNoInline(void)
//   0x743420  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_NavigationActions::StaticClassNoInline(void)
//   0x743450  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_SpecialAnimation::StaticClassNoInline(void)
//   0x743480  public: static class UClass * __cdecl UDisTweaks_PlayerPawn_UpperBodyAnimation::StaticClassNoInline(void)
