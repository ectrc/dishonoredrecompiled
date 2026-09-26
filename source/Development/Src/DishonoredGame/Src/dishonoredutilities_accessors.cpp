// DishonoredGame/src/dishonoredutilities_accessors.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (41):
//   0x823980  float __cdecl DisGetAppropriateWorldTime(class AActor const * const)
//   0x8239b0  class FVector const __cdecl DisGetPawnFeet(class APawn const *)
//   0x823a10  class FVector const __cdecl DisGetPawnTop(class APawn const *)
//   0x827020  class UDishonoredMapInfo * __cdecl DishonoredGetMapInfo(void)
//   0x827040  class ADishonoredGameInfo * __cdecl DisGetGameInfo(void)
//   0x827060  class UDishonoredContactSystem * __cdecl DisGetContactSystem(void)
//   0x827090  class UDisPostProcessManager & __cdecl DisGetPpManager(void)
//   0x8270c0  unsigned int __cdecl DisIsBendTimeOn(void)
//   0x8270f0  unsigned int __cdecl DisIsBendTimeFrozen(void)
//   0x827120  unsigned int __cdecl DisIsActorAffectedByBendTimePower(class AActor const * const)
//   0x827170  unsigned int __cdecl DisIsActorFrozenByBendTimePower(class AActor const * const)
//   0x8271c0  class UDishonoredAudioSystem * __cdecl DisGetAudioSystem(void)
//   0x8271e0  class UDisConvGlobalMan * __cdecl DisGetConvGlobalMan(void)
//   0x8271f0  class UStatePlayerCarryCorpseIdle * __cdecl DisGetCarryCorpseState(void)
//   0x827220  class UDisWaterVolumeManager * __cdecl DisGetWaterVolumeManager(void)
//   0x827250  class AActor * __cdecl DisGetActorUnderCrosshair(void)
//   0x827380  class UDisGlobalCombatManager * __cdecl DisGetGlobalCombatManager(void)
//   0x8273b0  class UDisGameCrowdPopulationManager * __cdecl DisGetGameCrowdPopulationManager(void)
//   0x8273d0  class UDisRatSwarmGlobalManager * __cdecl DisGetRatSwarmGlobalManager(void)
//   0x827400  class UDisGlobalAIDangerManager * __cdecl DisGetGlobalAIDangerManager(void)
//   0x827430  class UDisNPCTravelManager * __cdecl DisGetNPCTravelManager(void)
//   0x827460  class UDisGlobalUIManager * __cdecl DisGetGlobalUIManager(void)
//   0x827490  class UDisGFxMoviePlayerHUD * __cdecl DisGetGFxHUD(void)
//   0x8274d0  class UDisGFxMoviePlayerPowerWheel * __cdecl DisGetPowerWheel(void)
//   0x82d7e0  class UDisLocalPlayer * __cdecl DisGetLocalPlayer(void)
//   0x82d840  class UArkPpNode * __cdecl DisGetArkPpNode(class FName const &)
//   0x82d900  class UArkPpNodeMaterial * __cdecl DisGetArkPpNodeMaterial(class FName const &, unsigned int)
//   0x82d990  class UDisDistractionGlobalManager * __cdecl DisGetDistractionGlobalManagerUnchecked(void)
//   0x82d9d0  class UDisAlarmGlobalManager * __cdecl DisGetAlarmGlobalManager(void)
//   0x82da00  class UDisAmbushGlobalManager * __cdecl DisGetAmbushGlobalManager(void)
//   0x82da30  class UStatePlayerMasterMantle * __cdecl DisGetMantleState(class UAnimNodeSequence *)
//   0x82da80  class FVector const __cdecl DisGetDamageCenter(class AActor const *)
//   0x82db30  class UDisTweaks_PlayerVisSettings const & __cdecl DisGetGlobalVisSettings(void)
//   0x82dc00  class UDisGlobalAIAttentionManager * __cdecl DisGetGlobalAIAttentionManager(void)
//   0x82dc30  class UDisGlobalProjectileManager * __cdecl DisGetGlobalProjectileManager(void)
//   0x832240  class UDishonoredGlobalAIManager * __cdecl DisGetGlobalAIManagerUnchecked(void)
//   0x832270  class UDisAIBlackboard * __cdecl DisGetGlobalAIBlackboard(void)
//   0x8322a0  class UDisAINoiseManager * __cdecl DisGetAINoiseManagerUnchecked(void)
//   0x8322d0  class UDisTweaks_PlayerVisSettings const & __cdecl DisGetPlayerVisSettings(void)
//   0x839fd0  class UDisContactPhysicalMaterial * __cdecl DisGetPhysicalMaterial(class AActor const *, class FVector const &, class FVector const &, int, struct FCheckResult *)
//   0x83a090  class UClass * __cdecl DisGetContactType(class UPhysicalMaterial const *, class AActor const *)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x7bf0b0 (2012 0x827040, same bytes). Retail returns GetGameInfo() unchecked: the game info
// class is always DishonoredGameInfo or a child.
ADishonoredGameInfo* DisGetGameInfo()
{
	if( !GWorld )
	{
		return NULL;
	}
	return (ADishonoredGameInfo*)GWorld->GetGameInfo();
}

// DISHONORED(written): 2013 rva 0x7bf090 (2012 0x827020, same bytes). Retail reads UWorld::m_pWorldInfoCheckStreamingPersistent
// (@708, the cached GetWorldInfo(TRUE)); our Engine declares that member but never fills it, so the lookup is done here.
UDishonoredMapInfo* DishonoredGetMapInfo()
{
	if( !GWorld )
	{
		return NULL;
	}
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo( TRUE );
	return WorldInfo ? (UDishonoredMapInfo*)WorldInfo->GetMapInfo() : NULL;
}

// DISHONORED(written): 2013 rva 0x7bf700 (2012 0x827460)
UDisGlobalUIManager* DisGetGlobalUIManager()
{
	ADishonoredGameInfo* GameInfo = DisGetGameInfo();
	return GameInfo ? GameInfo->m_pGlobalUIManager : NULL;
}

// DISHONORED(written): 2013 rva 0x7c8b20 (2012 0x82d7e0)
UDisLocalPlayer* DisGetLocalPlayer()
{
	if( !GEngine || GEngine->GamePlayers.Num() <= 0 )
	{
		return NULL;
	}
	return Cast<UDisLocalPlayer>( GEngine->GamePlayers(0) );
}

// DISHONORED(written): 2013 rva 0x7bf3c0 (2012 0x8271c0): UWorld::m_pAudioSystem @688, unchecked cast like retail
UDishonoredAudioSystem* DisGetAudioSystem()
{
	return GWorld ? (UDishonoredAudioSystem*)GWorld->m_pAudioSystem : NULL;
}

// DISHONORED(written): 2013 rva 0x7c8be0 (2012 0x82d840): the node of the first local player's post-process graph by EffectName
UArkPpNode* DisGetArkPpNode( const FName& EffectName )
{
	ULocalPlayer* LocalPlayer = ( GEngine && GEngine->GamePlayers.Num() > 0 ) ? GEngine->GamePlayers(0) : NULL;
	if( !LocalPlayer || !LocalPlayer->PlayerPostProcess )
	{
		return NULL;
	}
	UPostProcessChain* Chain = LocalPlayer->PlayerPostProcess;
	for( INT NodeIndex = 0; NodeIndex < Chain->m_AllNodes.Num(); NodeIndex++ )
	{
		if( Chain->m_AllNodes(NodeIndex)->EffectName == EffectName )
		{
			return Chain->m_AllNodes(NodeIndex);
		}
	}
	return NULL;
}

// DISHONORED(written): 2013 rva 0x7c8ca0 (2012 0x82d900): with bMakeUnique the node gets its own MIC parented to its material
UArkPpNodeMaterial* DisGetArkPpNodeMaterial( const FName& EffectName, UBOOL bMakeUnique )
{
	UArkPpNodeMaterial* Node = Cast<UArkPpNodeMaterial>( DisGetArkPpNode( EffectName ) );
	if( Node && bMakeUnique && !Cast<UMaterialInstanceConstant>( Node->m_Material ) )
	{
		UMaterialInstanceConstant* UniqueMaterial = ConstructObject<UMaterialInstanceConstant>( UMaterialInstanceConstant::StaticClass(), UObject::GetTransientPackage() );
		UniqueMaterial->SetParent( Node->m_Material );
		Node->m_Material = UniqueMaterial;
	}
	return Node;
}

// ---- agent AU ports (PHASE7 AU) ----

// DISHONORED(written): 2013 rva 0x7bf730 (2012 0x827490): the game info's UI manager holds the HUD movie player, and
// only an open movie answers.
UDisGFxMoviePlayerHUD* DisGetGFxHUD()
{
	UDisGlobalUIManager* UIManager = DisGetGlobalUIManager();
	if( !UIManager || !UIManager->m_pHUD )
	{
		return NULL;
	}
	return UIManager->m_pHUD->bMovieIsOpen ? UIManager->m_pHUD : NULL;
}

// DISHONORED(written): 2013 rva 0x7bf2c0 (2012 0x8270c0).
// DISHONORED(bringup): ADishonoredGameInfo's bend-time accessors (vtable +980 IsBendTimeOn, +1080 IsBendTimeFrozen) are
// not ported, so bend time reads as off and every bend-time branch of the ported bodies is inert.
UBOOL DisIsBendTimeOn()
{
	return FALSE;
}

// DISHONORED(written): 2013 rva 0x7bf2f0 (2012 0x8270f0); same hand-over as DisIsBendTimeOn.
UBOOL DisIsBendTimeFrozen()
{
	return FALSE;
}
