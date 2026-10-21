// DishonoredGame/src/dishonoredpowerscomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (14):
//   0x6fb9b0  public: static void __cdecl UDishonoredPowersComponent::InitializePrivateStaticClassUDishonoredPowersComponent(void)
//   0x707aa0  FindPowerIndexByName
//   0x707b20  public: int __thiscall UDishonoredPowersComponent::GetPowerLevel(class FName)const
//   0x70d7b0  MakePowerModName
//   0x710080  public: virtual void __thiscall UDishonoredPowersComponent::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x712bc0  public: void __thiscall UDishonoredPowersComponent::SetPowerLevel(class FName, int)
//   0x712f30  public: void __thiscall UDishonoredPowersComponent::RemovePower(class FName)
//   0x712f50  public: void __thiscall UDishonoredPowersComponent::InhibitPassivePowers(void)
//   0x713050  public: void __thiscall UDishonoredPowersComponent::RestorePassivePowers(void)
//   0x713110  public: void __thiscall UDishonoredPowersComponent::MaxOutPowers(void)
//   0x713290  public: void __thiscall UDishonoredPowersComponent::MinOutPowers(void)
//   0x7133c0  public: virtual void __thiscall UDishonoredPowersComponent::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x71ecc0  public: static class UClass * __cdecl UDishonoredPowersComponent::GetPrivateStaticClassUDishonoredPowersComponent(wchar_t const *)
//   0x71f710  public: static class UClass * __cdecl UDishonoredPowersComponent::StaticClassNoInline(void)

// ---- agent JH (PHASE24 JH) ----

#include "DishonoredGame.h"

void DishonoredLinkUDishonoredPowersComponent()
{
}

// DISHONORED(port): agent JH. 2013 rva 0x6c8a00 (2012 0x707b20, 192 B): the current level of the named power, -1 when the player
// does not have it (the last entry of that name wins, as retail's loop runs to the end)
INT UDishonoredPowersComponent::GetPowerLevel( FName _PowerName ) const
{
	INT CurrentLevel = -1;
	for( INT Index = 0; Index < m_Powers.Num(); Index++ )
	{
		if( m_Powers(Index).m_Name == _PowerName )
		{
			CurrentLevel = m_Powers(Index).m_CurrentLevel;
		}
	}
	return CurrentLevel;
}
