// DishonoredGame/src/disaudiopropagationinfo.cpp
// PDB functions attributed to this file (8):
//   0x7f3d10  public: static void __cdecl UDisAudioPropagationInfo::InitializePrivateStaticClassUDisAudioPropagationInfo(void)
//   0x7f3d30  DisAudioPropagationFindPathInfo
//   0x7f7ea0  class UDisAudioPropagationInfo * __cdecl DisGetAudioPropagationInfo(void)
//   0x7f7f30  public: unsigned int __thiscall UDisAudioPropagationInfo::FindShortestPath(struct FDisAudioPathEndPair const &, float, unsigned int, struct FDisAudioPortalPath &)
//   0x801dd0  public: unsigned int __thiscall UDisAudioPropagationInfo::FinalizeAudioPath(struct FDisAudioPropagationCache &, struct FDisAudioPortalPath const &, int, int, class FVector const &, class FVector const &, float, unsigned int, class TArray<struct FDisAudioDebugPath, class FDefaultAllocator> *)
//   0x807670  public: virtual void __thiscall UDisAudioPropagationInfo::PostLoad(void)
//   0x814980  public: static class UClass * __cdecl UDisAudioPropagationInfo::GetPrivateStaticClassUDisAudioPropagationInfo(wchar_t const *)
//   0x816cb0  public: static class UClass * __cdecl UDisAudioPropagationInfo::StaticClassNoInline(void)
//
// DISHONORED(port): agent IE (PHASE23 IE). 2013 rvas (2012 in parentheses): DisGetAudioPropagationInfo 0x792290 (0x7f7ea0, 143 B),
// PostLoad 0x7a3070 (0x807670, 147 B). The level's audio cells and portals: the first streaming level whose world info carries
// one wins, else the persistent world info's. PostLoad drops a build whose tag is not 2, or one with portals and no path data.
// Not ported (the portal path search, IH3): DisAudioPropagationFindPathInfo, FindShortestPath, FinalizeAudioPath.
// Retail reads UWorld::m_pWorldInfoCheckStreamingPersistent (the cached GetWorldInfo(TRUE)) unchecked; GetWorldInfo(TRUE) here, as
// DishonoredGetMapInfo does, and no world answers no propagation info.

#include "DishonoredGame.h"

UDisAudioPropagationInfo* DisGetAudioPropagationInfo()
{
	AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo( TRUE ) : NULL;
	if( !WorldInfo )
	{
		return NULL;
	}
	for( INT i = 0; i < WorldInfo->StreamingLevels.Num(); i++ )
	{
		ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(i);
		if( StreamingLevel && StreamingLevel->LoadedLevel )
		{
			AWorldInfo* LevelWorldInfo = StreamingLevel->LoadedLevel->GetWorldInfo();
			if( LevelWorldInfo->m_pAudioPropagationInfo )
			{
				return (UDisAudioPropagationInfo*)LevelWorldInfo->m_pAudioPropagationInfo;
			}
		}
	}
	return (UDisAudioPropagationInfo*)WorldInfo->m_pAudioPropagationInfo;
}

void UDisAudioPropagationInfo::PostLoad()
{
	Super::PostLoad();
	if( m_BuildTag != 2 || ( m_Portals.Num() && m_PathData.Num() <= 0 ) )
	{
		m_fDistCoeff = 0.0f;
		m_NumPaths = 0;
		m_PathData.Empty();
		m_Cells.Empty();
		m_Portals.Empty();
		m_BuildTag = -1;
	}
}
