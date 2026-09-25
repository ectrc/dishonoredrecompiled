// DishonoredGame/src/dissavegame.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (30):
//   0x6300c0  DisSaveLoad::IsSubLevelUnshared
//   0x6300f0  public: static unsigned int __cdecl DisSaveLoad::FGameState::LoadMapName(class FArchive &, class FString &)
//   0x643ff0  public: void __thiscall DisSaveLoad::FStringDictionary::Save(class FArchive &)
//   0x644160  public: void __thiscall DisSaveLoad::FStringDictionary::LoadFName(class FArchive &, class FName &)
//   0x6441e0  public: virtual class FArchive & __thiscall DisSaveLoad::FLevelLoader::operator<<(class FName &)
//   0x644200  private: int __thiscall DisSaveLoad::FGameState::findLevelIndex(class FName const &)const
//   0x644280  public: unsigned int __thiscall DisSaveLoad::FGameState::ContainsObjectState(class UObject *)const
//   0x649130  DisSaveLoad::InitializeSequencePostLoad
//   0x6492e0  public: void __thiscall DisSaveLoad::FGameState::SaveGameState(void)
//   0x64d0b0  private: unsigned int __thiscall DisSaveLoad::FLevelSaver::ShouldSaveObject(class UObject *)
//   0x64d120  private: unsigned int __thiscall DisSaveLoad::FLevelLoader::ShouldLoadObject(class UObject *)
//   0x64ff80  private: void __thiscall DisSaveLoad::FLevelSaver::SerializeLevel(class ULevel *, unsigned int)
//   0x6516f0  public: void __thiscall DisSaveLoad::FLevelLoader::NotifyPostGameLoad(void)const
//   0x653700  public: virtual class FArchive & __thiscall DisSaveLoad::FLevelLoader::operator<<(class UObject * &)
//   0x6562a0  public: unsigned short __thiscall DisSaveLoad::FStringDictionary::GetStringIndex(class FName const &)
//   0x656320  public: virtual class FArchive & __thiscall DisSaveLoad::FLevelSaver::operator<<(class FName &)
//   0x656370  private: unsigned short __thiscall DisSaveLoad::FLevelSaver::GetObjectIndex(class UObject *)
//   0x6573f0  private: void __thiscall DisSaveLoad::FStringDictionary::Init(int)
//   0x657460  public: void __thiscall DisSaveLoad::FStringDictionary::Load(class FArchive &)
//   0x6575e0  public: void __thiscall DisSaveLoad::FStringDictionary::Clear(void)
//   0x6576c0  public: virtual class FArchive & __thiscall DisSaveLoad::FLevelSaver::operator<<(class UObject * &)
//   0x6577f0  private: void __thiscall DisSaveLoad::FGameState::discardLevelState(int)
//   0x657870  public: void __thiscall DisSaveLoad::FGameState::Save(class FArchive &, class FString &)
//   0x658630  public: __thiscall DisSaveLoad::FStringDictionary::FStringDictionary(void)
//   0x6586d0  public: __thiscall DisSaveLoad::FLevelSaver::FLevelSaver(class DisSaveLoad::FStringDictionary &, class TArray<unsigned char, class FDefaultAllocator> &, class TArray<unsigned char, class FDefaultAllocator> &, class TArray<class DisSaveLoad::FDeletedActor, class FDefaultAllocator> &, class ULevel *, enum ESaveLoadLocation)
//   0x658d80  public: __thiscall DisSaveLoad::FLevelLoader::FLevelLoader(class DisSaveLoad::FStringDictionary &, class TArray<unsigned char, class FDefaultAllocator> &, class TArray<unsigned char, class FDefaultAllocator> &, class ULevel *, class FName const &, enum ESaveLoadLocation, unsigned short, unsigned short, unsigned int, int)
//   0x659a60  public: void __thiscall DisSaveLoad::FGameState::SaveLevel(class ULevel *, enum ESaveLoadLocation)
//   0x659b50  public: void __thiscall DisSaveLoad::FGameState::LoadLevel(class ULevel *)
//   0x659cb0  public: void __thiscall DisSaveLoad::FGameState::DiscardLevelState(class FName const &)
//   0x659db0  public: unsigned int __thiscall DisSaveLoad::FGameState::Load(class FArchive &)
