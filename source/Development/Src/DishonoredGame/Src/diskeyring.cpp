// DishonoredGame/src/diskeyring.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (11):
//   0x6fba60  public: static void __cdecl UDisKeyRing::InitializePrivateStaticClassUDisKeyRing(void)
//   0x708220  public: void __thiscall UDisKeyRing::AddKey(class FString const &, class UDisTweaks_Key const *)
//   0x708400  public: void __thiscall UDisKeyRing::UseKey(class FString const &)
//   0x708530  private: int __thiscall UDisKeyRing::FindKey(class FString const &)const
//   0x716eb0  public: void __thiscall UDisKeyRing::FlushAllKeys(void)
//   0x716ec0  public: void __thiscall UDisKeyRing::RestoreFromBackup(void)
//   0x718490  public: virtual void __thiscall UDisKeyRing::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7184e0  public: virtual void __thiscall UDisKeyRing::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x718f30  public: void __thiscall UDisKeyRing::BackupAndClear(void)
//   0x719d40  public: static class UClass * __cdecl UDisKeyRing::GetPrivateStaticClassUDisKeyRing(wchar_t const *)
//   0x71af70  public: static class UClass * __cdecl UDisKeyRing::StaticClassNoInline(void)

// ---- agent JH (PHASE24 JH) ----

#include "DishonoredGame.h"

void DishonoredLinkUDisKeyRing()
{
}

// DISHONORED(port): agent JH. 2012 0x708530: the index of the key of that name, INDEX_NONE when the ring does not hold it
INT UDisKeyRing::FindKey( const FString& _rKey ) const
{
	for( INT Index = 0; Index < m_Keys.Num(); Index++ )
	{
		if( m_Keys(Index).m_Name == _rKey )
		{
			return Index;
		}
	}
	return INDEX_NONE;
}
