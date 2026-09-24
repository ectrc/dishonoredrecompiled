// DishonoredGame/src/distweaksbase.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (37):
//   0x8d94d0  public: static void __cdecl UDisTweaksBase::InitializePrivateStaticClassUDisTweaksBase(void)
//   0x8d94f0  public: static void __cdecl UDisTweaksInterface::InitializePrivateStaticClassUDisTweaksInterface(void)
//   0x8d9510  public: static void __cdecl UDisTweaksList::InitializePrivateStaticClassUDisTweaksList(void)
//   0x8d9530  public: void __thiscall FDisTweakChildInfo::SetTweakChildInfo(class FName const &, class UDisTweaksBase *, int, class FName const &)
//   0x8d9570  public: void __thiscall FDisTweakChildInfo::SetTweakChildInfo(struct FDisTweakChildInfo_ChildName const &, class UDisTweaksBase *, int)
//   0x8d95a0  public: __thiscall FDisTweaksGenericThumbnailItem::FDisTweaksGenericThumbnailItem(class FName const &, class USkeletalMesh *, int)
//   0x8d95e0  public: __thiscall FDisTweaksGenericThumbnailItem::FDisTweaksGenericThumbnailItem(class FName const &, class UStaticMesh *, int)
//   0x8d9620  public: virtual class UClass * __thiscall UDisTweaksBase::GetSpawnedObjectClass(enum eDisTweaksSpawnType)const
//   0x8d9640  public: class AActor * __thiscall UDisTweaksBase::SpawnActor(enum eDisTweaksSpawnType, class FName, class FVector const &, class FRotator const &, class AActor *, unsigned int, unsigned int, class AActor *, class APawn *, unsigned int, unsigned int)const
//   0x8dcbc0  public: static class UClass * __cdecl UDisTweaksInterface::GetPrivateStaticClassUDisTweaksInterface(wchar_t const *)
//   0x8dcc50  protected: int __thiscall UDisTweaksBase::FindFallbackSkip(class FString const &)
//   0x8dccd0  public: unsigned int __thiscall UDisTweaksBase::IsFallbackDerivedFrom(class UDisTweaksBase const * const)const
//   0x8dcdc0  public: unsigned int __thiscall UDisTweaksBase::IsFallbackDerivedFrom(class FString const &)const
//   0x8dcf80  public: unsigned int __thiscall UDisTweaksBase::IsFallbackRelated(class UDisTweaksBase const * const)const
//   0x8dcfc0  public: unsigned int __thiscall UDisTweaksBase::CanSpawnObjectOfClass(class UClass const *, enum eDisTweaksSpawnType)
//   0x8dd000  protected: virtual class AActor * __thiscall UDisTweaksBase::SpawnActor_Derived(enum eDisTweaksSpawnType, class FName, class FVector const &, class FRotator const &, class AActor *, unsigned int, unsigned int, class AActor *, class APawn *, unsigned int, unsigned int)
//   0x8e0400  public: static class UClass * __cdecl UDisTweaksInterface::StaticClassNoInline(void)
//   0x8e0430  public: virtual unsigned int __thiscall UDisTweaksBase::ComparePropForPerObjectLocalizationExport(class UProperty *, unsigned char *, int, wchar_t const *)
//   0x8e0500  protected: void __thiscall UDisTweaksBase::ApplyFallbackChain_Struct(class UStruct const *, class UProperty const *, class FString const &, int)
//   0x8e0870  void __cdecl TweakBuildPropPath(class FEditPropertyChain *, class FString &)
//   0x8e0a00  public: virtual unsigned int __thiscall UDisTweaksBase::EditConditionAskObj_IsConditionMet(class FEditPropertyChain *, wchar_t const *)
//   0x8e48c0  protected: void __thiscall UDisTweaksBase::InvalidateFallback_Recurse(void)
//   0x8e6fc0  public: void __thiscall UDisTweaksBase::ApplyFallbackChain(void)
//   0x8e7650  public: void __thiscall UDisTweaksBase::SkipFallback(class FName const &)
//   0x8ea0c0  public: static class UClass * __cdecl UDisTweaksList::GetPrivateStaticClassUDisTweaksList(wchar_t const *)
//   0x8ea150  public: void __thiscall IDisTweaksInterface::ApplyTweakChanges(void)
//   0x8ea190  public: virtual void __thiscall UDisTweaksBase::PostLoad(void)
//   0x8ea1c0  public: virtual void __thiscall UDisTweaksBase::EditConditionAskObj_SetCondition(class FEditPropertyChain *, wchar_t const *, unsigned int)
//   0x8ec4d0  public: static class UClass * __cdecl UDisTweaksList::StaticClassNoInline(void)
//   0x8ec500  public: virtual void __thiscall FSpawnActor_TweakObj::DoInit(class AActor *)
//   0x8ec550  public: virtual void __thiscall FSpawnNPCPawn_TweakObj::DoInit(class AActor *)
//   0x8ee320  public: static class UClass * __cdecl UDisTweaksBase::GetPrivateStaticClassUDisTweaksBase(wchar_t const *)
//   0x8ef5a0  public: static class UClass * __cdecl UDisTweaksBase::StaticClassNoInline(void)
//   0x8f0600  public: virtual void __thiscall UDisTweaksBase::Serialize(class FArchive &)
//   0x8f1010  private: virtual unsigned int __thiscall IDisTweaksInterface::HasTweaks_Derived(class UDisEngineTweaksBase const &)const
//   0x8f1040  public: void __thiscall UDisTweaksBase::PostEditChangeChainProperty(struct FPropertyChangedChainEvent &)
//   0x8f12d0  public: static class FString __cdecl UDisTweaksBase::GetTweaksOwnerDisplayName(class UObject const *)
