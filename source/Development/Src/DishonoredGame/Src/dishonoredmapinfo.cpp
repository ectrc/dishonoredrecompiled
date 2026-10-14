// DishonoredGame/src/dishonoredmapinfo.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x62fe60  public: static void __cdecl UDishonoredMapInfo::InitializePrivateStaticClassUDishonoredMapInfo(void)
//   0x62fe80  public: class UDisTweaks_PlayerLoadout const * __thiscall UDishonoredMapInfo::GetMapSpecificPlayerLoadout(void)
//   0x62fea0  public: void __thiscall UDishonoredMapInfo::OnPawnShutDown_MapInfo(class ADishonoredPawn const &)
//   0x643760  public: struct FDisSquadInfo * __thiscall UDishonoredMapInfo::FindSquadInfo(class FName)
//   0x643810  public: struct FFishAttractor * __thiscall UDishonoredMapInfo::CreateAttractor(float)
//   0x64cff0  public: void __thiscall UDishonoredMapInfo::DestroyAttractor(struct FFishAttractor *)
//   0x651400  public: static class UClass * __cdecl UDishonoredMapInfo::GetPrivateStaticClassUDishonoredMapInfo(wchar_t const *)
//   0x651490  public: virtual void __thiscall UDishonoredMapInfo::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x653670  public: static class UClass * __cdecl UDishonoredMapInfo::StaticClassNoInline(void)
//   0x655f50  public: virtual void __thiscall UDishonoredMapInfo::GameSave(class FArchive &, enum ESaveLoadLocation)

// Agent GJ (PHASE20 GJ): the player loadout accessor. The squad info, the fish attractors and the save pair stay where they are
// (dissavegame.cpp has GameSave / GameLoad).

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkGJ_MapInfo()
{
}

const UDisTweaks_PlayerLoadout* UDishonoredMapInfo::GetMapSpecificPlayerLoadout()
{
	if( !m_pPlayerMapSpecificLoadout )
	{
		return NULL;
	}
	m_pPlayerMapSpecificLoadout->ApplyFallbackChain();
	return m_pPlayerMapSpecificLoadout;
}
