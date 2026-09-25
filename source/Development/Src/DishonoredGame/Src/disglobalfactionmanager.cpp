// DishonoredGame/src/disglobalfactionmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (37):
//   0x8aecf0  public: static void __cdecl UDisGlobalFactionManager::InitializePrivateStaticClassUDisGlobalFactionManager(void)
//   0x8b6f50  public: virtual void __thiscall UDisGlobalFactionManager::BeginDestroy(void)
//   0x8b6f80  private: enum ERelationship __thiscall UDisGlobalFactionManager::GetFactionRelationshipToFaction(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)const
//   0x8c7d00  public: struct FDisRelationshipOverrideInfo const * __thiscall UDisGlobalFactionManager::GetRelationships(class UDisTweaks_Faction const &)const
//   0x8c7d40  private: unsigned int __thiscall UDisGlobalFactionManager::GetGlobalIndividualOverride(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &, enum ERelationship &)const
//   0x8cb0b0  private: void __thiscall UDisGlobalFactionManager::ClearIndividualCache(class IDisRelationshipInterface const &, class IDisRelationshipInterface const &)const
//   0x8ce0e0  public: void __thiscall FDisDetermineRelationshipFactionMap::AddReferencedObjects(class UObject &, class TArray<class UObject *, class FDefaultAllocator> &)const
//   0x8ce160  public: void __thiscall FDisDetermineRelationshipFactionMap::Serialize(class FArchive &)
//   0x8ce250  public: virtual void __thiscall UDisGlobalFactionManager::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x8ce3d0  public: virtual void __thiscall UDisGlobalFactionManager::Serialize(class FArchive &)
//   0x8ce6b0  public: virtual void __thiscall UDisGlobalFactionManager::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x8ce7d0  private: unsigned int __thiscall UDisGlobalFactionManager::CachedIsFallbackDerivedFrom(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)const
//   0x8ce860  private: void __thiscall UDisGlobalFactionManager::ClearDerivedFactions(struct FDisDetermineRelationshipIndividualMap &, class UDisTweaks_Faction const &)const
//   0x8ce970  private: void __thiscall UDisGlobalFactionManager::ClearDerivedFactions(struct FDisDetermineRelationshipFactionMap &, class UDisTweaks_Faction const &)const
//   0x8cea70  private: unsigned int __thiscall UDisGlobalFactionManager::IsTrespassing(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)const
//   0x8ceaf0  private: enum ERelationship __thiscall UDisGlobalFactionManager::GetFactionRelationshipToIndividual(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)const
//   0x8d0570  private: void __thiscall UDisGlobalFactionManager::ClearIndividualCache(class IDisRelationshipInterface const &, class UDisTweaks_Faction const &)const
//   0x8d05c0  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class IDisRelationshipInterface const &, class UDisTweaks_Faction const &)const
//   0x8d0710  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)const
//   0x8d1c60  public: void __thiscall UDisGlobalFactionManager::AddTrespasserForFaction(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8d1c90  public: void __thiscall UDisGlobalFactionManager::RemoveTrespasserForFaction(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8d7730  public: void __thiscall UDisGlobalFactionManager::RebuildCaches(void)
//   0x8d7810  private: enum ERelationship __thiscall UDisGlobalFactionManager::DetermineRelationship(class IDisRelationshipInterface const &, class IDisRelationshipInterface const &)const
//   0x8d7960  public: void __thiscall UDisGlobalFactionManager::OverrideRelationToFaction(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &, enum ERelationship)
//   0x8d7ba0  public: void __thiscall UDisGlobalFactionManager::OverrideRelationToIndividual(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &, enum ERelationship)
//   0x8d7de0  public: void __thiscall UDisGlobalFactionManager::ClearRelationToFactionOverride(class UDisTweaks_Faction const &, class UDisTweaks_Faction const &)
//   0x8d7e60  public: void __thiscall UDisGlobalFactionManager::ClearRelationToIndividualOverride(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)
//   0x8d7ee0  public: void __thiscall UDisGlobalFactionManager::ClearAllRelationOverrides(void)
//   0x8d7f10  public: virtual void __thiscall UDisGlobalFactionManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8d8090  private: void __thiscall UDisGlobalFactionManager::ClearIndividualCache(class IDisRelationshipInterface const &)const
//   0x8d80a0  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class UDisTweaks_Faction const &)const
//   0x8d82e0  public: enum ERelationship __thiscall UDisGlobalFactionManager::DetermineRelationship(class UDisTweaks_Faction const &, class IDisRelationshipInterface const &)const
//   0x8d8470  private: void __thiscall UDisGlobalFactionManager::ClearFactionCache(class IDisRelationshipInterface const &)const
//   0x8d88d0  public: void __thiscall UDisGlobalFactionManager::OnIndividualDestroyed(class IDisRelationshipInterface const &)
//   0x8d8a30  public: __thiscall UDisGlobalFactionManager::UDisGlobalFactionManager(void)
//   0x8d8c00  public: static class UClass * __cdecl UDisGlobalFactionManager::GetPrivateStaticClassUDisGlobalFactionManager(wchar_t const *)
//   0x8d8c90  public: static class UClass * __cdecl UDisGlobalFactionManager::StaticClassNoInline(void)
