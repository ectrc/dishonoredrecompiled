// DishonoredGame/src/dishonoredgameinfo.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (67):
//   0x62f7a0  public: static void __cdecl ADishonoredGameInfo::InitializePrivateStaticClassADishonoredGameInfo(void)
//   0x62f7c0  public: static void __cdecl UDisGameplayEventsWriter::InitializePrivateStaticClassUDisGameplayEventsWriter(void)
//   0x62f7e0  public: static void __cdecl UDisTweaks_ChapterInfo::InitializePrivateStaticClassUDisTweaks_ChapterInfo(void)
//   0x62f800  public: static void __cdecl UDisTweaks_ChapterInfoList::InitializePrivateStaticClassUDisTweaks_ChapterInfoList(void)
//   0x62f820  public: static void __cdecl UDisTweaks_ChapterTarget::InitializePrivateStaticClassUDisTweaks_ChapterTarget(void)
//   0x62f840  public: virtual void __thiscall ADishonoredGameInfo::OnOtherActorTerminated(class AActor const &)
//   0x62f8b0  public: void __thiscall ADishonoredGameInfo::InitGlobalManagers(void)
//   0x62f950  public: void __thiscall ADishonoredGameInfo::TermGlobalManagers(void)
//   0x62fa30  public: virtual void __thiscall ADishonoredGameInfo::BeginDestroy(void)
//   0x62fa40  public: virtual void __thiscall ADishonoredGameInfo::SetupPathfindingParams(struct FNavMeshPathParams &)const
//   0x62fa60  public: void __thiscall ADishonoredGameInfo::GetBendTime(enum EBendTimeChannel, float &, float &, float &)
//   0x62fa90  public: void __thiscall ADishonoredGameInfo::SetBendTimeChannelExclusive(enum EBendTimeChannel, unsigned int)
//   0x62fab0  public: float __thiscall ADishonoredGameInfo::GetBendTimePowerRealTimeSeconds(void)const
//   0x62fac0  public: virtual float __thiscall ADishonoredGameInfo::GetBendTimeDilation(unsigned int)const
//   0x62fb00  public: virtual float __thiscall ADishonoredGameInfo::GetBendTimeInputDilation(void)const
//   0x62fb10  public: virtual unsigned int __thiscall ADishonoredGameInfo::IsBendTimeOn(void)const
//   0x62fb40  public: virtual unsigned int __thiscall ADishonoredGameInfo::IsBendTimeFrozen(void)const
//   0x62fb70  public: virtual unsigned int __thiscall ADishonoredGameInfo::CanStartMatch(void)
//   0x62fb90  public: class UDisGameplayEventsWriter * __thiscall ADishonoredGameInfo::GetGameplayEventsWriter(void)const
//   0x62fba0  public: void __thiscall ADishonoredGameInfo::SetDifficulty(enum EDifficulty)
//   0x62fbf0  public: enum EDifficulty __thiscall ADishonoredGameInfo::GetDifficulty(void)const
//   0x62fc00  public: void __thiscall ADishonoredGameInfo::EnableGore(unsigned int)
//   0x62fc20  public: unsigned int __thiscall ADishonoredGameInfo::IsGoreEnabled(void)const
//   0x62fc30  public: unsigned int __thiscall ADishonoredGameInfo::IsGameOverPending(void)const
//   0x62fc40  public: virtual class UGameCrowdPopulationManager * __thiscall ADishonoredGameInfo::GetCrowdPopulationManager(void)
//   0x62fc50  public: virtual void __thiscall ADishonoredGameInfo::FlushCrowdAgents(void)
//   0x63fab0  private: void __thiscall ADishonoredGameInfo::Tick_PendingGameOver(float)
//   0x63fb20  public: virtual void __thiscall ADishonoredGameInfo::GameEnding(void)
//   0x63fc00  public: virtual void __thiscall ADishonoredGameInfo::OnCleanupWorld(void)
//   0x63fc10  public: virtual void __thiscall ADishonoredGameInfo::PostBeginPlay(void)
//   0x63fcf0  public: void __thiscall ADishonoredGameInfo::BendTime(enum EBendTimeChannel, float, float, float, float, enum EBendTimeEffectType)
//   0x63fe30  public: virtual void __thiscall ADishonoredGameInfo::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x640020  public: virtual void __thiscall ADishonoredGameInfo::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x640240  public: struct FDisChapterTarget const * __thiscall ADishonoredGameInfo::GetCurrentChapterTarget(int)const
//   0x640270  public: void __thiscall ADishonoredGameInfo::GameOver(class FString const &, float)
//   0x642b50  public: class UDisTweaks_PlayerPawn * __thiscall ADishonoredGameInfo::LoadDefaultPlayerTweaks(unsigned int)const
//   0x642c40  public: virtual class APawn * __thiscall ADishonoredGameInfo::SpawnPlayer(class UClass *, class FVector, class FRotator)
//   0x642c90  public: void __thiscall ADishonoredGameInfo::TickBendTimeParticleEffects(float, float)
//   0x642db0  public: virtual void __thiscall ADishonoredGameInfo::TickBendTime(float)
//   0x6432d0  public: class UDisTweaks_ChapterInfo const * __thiscall ADishonoredGameInfo::GetCurrentChapter(void)const
//   0x6483f0  public: void __thiscall ADishonoredGameInfo::Tick_PendingDestroyList(void)
//   0x6484a0  public: void __thiscall ADishonoredGameInfo::AddToPendingDestroyList(class AActor *)
//   0x6484c0  public: void __thiscall ADishonoredGameInfo::SetCurrentChapter(class FName)
//   0x6486f0  public: void __thiscall ADishonoredGameInfo::OnTargetNotification(class UDisSeqAct_ShowTargetNotification *)
//   0x64b480  public: virtual unsigned int __thiscall ADishonoredGameInfo::Tick(float, enum ELevelTick)
//   0x64fee0  public: virtual float __thiscall ADishonoredGameInfo::GetBendTimeDuration(class AActor const *)const
//   0x6534a0  public: virtual void __thiscall ADishonoredGameInfo::CancelAutoReturnToBendTime(class AActor *)
//   0x6534b0  public: void __thiscall ADishonoredGameInfo::ReallowAutoReturnToBendTime(class AActor *)
//   0x654c60  public: static class UClass * __cdecl UDisGameplayEventsWriter::GetPrivateStaticClassUDisGameplayEventsWriter(wchar_t const *)
//   0x654cf0  public: static class UClass * __cdecl UDisTweaks_ChapterInfo::GetPrivateStaticClassUDisTweaks_ChapterInfo(wchar_t const *)
//   0x654d80  public: static class UClass * __cdecl UDisTweaks_ChapterInfoList::GetPrivateStaticClassUDisTweaks_ChapterInfoList(wchar_t const *)
//   0x654e10  public: static class UClass * __cdecl UDisTweaks_ChapterTarget::GetPrivateStaticClassUDisTweaks_ChapterTarget(wchar_t const *)
//   0x655ae0  public: static class UClass * __cdecl UDisGameplayEventsWriter::StaticClassNoInline(void)
//   0x655b10  public: static class UClass * __cdecl UDisTweaks_ChapterInfo::StaticClassNoInline(void)
//   0x655b40  public: static class UClass * __cdecl UDisTweaks_ChapterInfoList::StaticClassNoInline(void)
//   0x655b70  public: static class UClass * __cdecl UDisTweaks_ChapterTarget::StaticClassNoInline(void)
//   0x655ba0  public: virtual void __thiscall ADishonoredGameInfo::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x655ca0  public: virtual void __thiscall ADishonoredGameInfo::Serialize(class FArchive &)
//   0x655cd0  public: virtual void __thiscall ADishonoredGameInfo::PostTickBendTime(float)
//   0x655e00  public: virtual void __thiscall ADishonoredGameInfo::AutoReturnToBendTime(class AActor *, float, unsigned int)
//   0x655ec0  public: void __thiscall ADishonoredGameInfo::DisallowAutoReturnToBendTime(class AActor *)
//   0x658510  public: static class UClass * __cdecl ADishonoredGameInfo::GetPrivateStaticClassADishonoredGameInfo(wchar_t const *)
//   0x65d860  public: static class UClass * __cdecl ADishonoredGameInfo::StaticClassNoInline(void)
//   0x65f400  public: virtual void __thiscall ADishonoredGameInfo::PostCommitMapChange(void)
//   0x65f500  public: virtual void __thiscall ADishonoredGameInfo::PostGameLoad(enum ESaveLoadLocation)
//   0x65f570  public: void __thiscall ADishonoredGameInfo::SetCurrentMission(int)
//   0x661260  public: virtual void __thiscall ADishonoredGameInfo::PreCommitMapChange(class FString const &, class FString const &)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x5e9f40 (2012 0x62fb70, same bytes; the GetGameInfo call's result is unused)
UBOOL ADishonoredGameInfo::CanStartMatch()
{
	GWorld->GetGameInfo();
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x5fc7d0 (2012 0x642b50). The tweak name length test reads m_DefaultPawnTweak for both cases, as
// retail does; with seek-free loading a missing tweak loads its package and retries once.
UDisTweaks_PlayerPawn* ADishonoredGameInfo::LoadDefaultPlayerTweaks( UBOOL bCampaign )
{
	if( m_DefaultPawnTweak.Len() == 0 )
	{
		return NULL;
	}
	const FString& TweakName = bCampaign ? m_CampaignPawnTweak : m_DefaultPawnTweak;
	UObject* Tweaks = UObject::StaticLoadObject( UDisTweaks_PlayerPawn::StaticClass(), NULL, *TweakName, NULL, LOAD_None, NULL, TRUE );
	if( !Tweaks && GUseSeekFreeLoading )
	{
		UObject::LoadPackage( NULL, *( bCampaign ? m_CampaignPawnTweakPackage : m_DefaultPawnTweakPackage ), LOAD_None );
		Tweaks = UObject::StaticLoadObject( UDisTweaks_PlayerPawn::StaticClass(), NULL, *TweakName, NULL, LOAD_None, NULL, TRUE );
	}
	return (UDisTweaks_PlayerPawn*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x5fc900 (2012 0x642c40, same bytes): the player pawn is spawned by the default (or campaign) player
// tweak, named after the requested class
APawn* ADishonoredGameInfo::SpawnPlayer( UClass* SpawnClass, FVector SpawnLocation, FRotator SpawnRotation )
{
	UDishonoredMapInfo* MapInfo = DishonoredGetMapInfo();
	UDisTweaks_PlayerPawn* PlayerTweaks = LoadDefaultPlayerTweaks( MapInfo && MapInfo->m_bIsCampaignMap );
	if( !PlayerTweaks )
	{
		// DISHONORED(bringup): retail dereferences both without a test (a missing map info or player tweak is fatal there)
		return NULL;
	}
	return (APawn*)PlayerTweaks->SpawnActor( eDisTweaksSpawnType_InGame, SpawnClass->GetFName(), SpawnLocation, SpawnRotation, NULL, GIsEditor, FALSE, NULL, NULL, FALSE );
}

// DISHONORED(written): the 2013 vtable slot +1032 is `return FALSE` in ADishonoredGameInfo (2013 rva 0x5be60; no 2012 equivalent)
UBOOL ADishonoredGameInfo::PreventDeath_Native( APawn* KilledPawn, AController* Killer, UClass* DamageType, FVector HitLocation )
{
	return FALSE;
}

// DISHONORED(written): 2013 rva 0x5ed020 (2012 0x633a80)
void ADishonoredGameInfo::execCanStartMatch( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = CanStartMatch();
}

// DISHONORED(written): 2013 rva 0x5ed060 (2012 0x633ac0)
void ADishonoredGameInfo::execSpawnPlayer( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass, SpawnClass);
	P_GET_STRUCT(FVector, SpawnLocation);
	P_GET_STRUCT(FRotator, SpawnRotation);
	P_FINISH;
	*(APawn**)Result = SpawnPlayer( SpawnClass, SpawnLocation, SpawnRotation );
}

// DISHONORED(written): 2013 rva 0x5ed140 (2012 0x633ba0)
void ADishonoredGameInfo::execPreventDeath_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(APawn, KilledPawn);
	P_GET_OBJECT(AController, Killer);
	P_GET_OBJECT(UClass, DamageType);
	P_GET_STRUCT(FVector, HitLocation);
	P_FINISH;
	*(UBOOL*)Result = PreventDeath_Native( KilledPawn, Killer, DamageType, HitLocation );
}

// DISHONORED(written): 2013 rva 0xcf20 (2012 0xcf90, same bytes)
void ADishonoredGameInfo::execGetChangelist( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(INT*)Result = GBuiltFromChangeList;
}

// DISHONORED(written): 2013 rva 0xcee0 = UObject::execGetEngineVersion (same code in the retail table)
void ADishonoredGameInfo::execGetDishonoredEngineVersion( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(INT*)Result = GEngineVersion;
}

// ---- agent AU ports (PHASE7 AU) ----

// DISHONORED(written): 2013 rva 0x5f9f60 (2012 0x63fb20): the end of a mission tears down what the player pawn is in
// the middle of - the possession power, the water volume it is swimming in - and stops the audio system and the
// gameplay-event log.
// DISHONORED(bringup): retail resolves the pawn's m_ExitWaterPPName / m_EnterWaterPPName post-process names and then
// discards both results (the shipped build's post-process reset is compiled out); UDisItemPowers::Cancel (the possess
// power, retail vtable +364), ADishonoredPlayerPawn::OnChangeWaterVolume (0x6d4120) and UDishonoredAudioSystem::CleanUp
// are not ported. What the port does run is the null-safe shape and the gameplay-event writer, and retail's own
// unguarded ADishonoredPlayerPawn::s_pInstance dereference is guarded here (the native fires on the walking path with
// no possessed pawn in a -startmap run).
void ADishonoredGameInfo::GameEnding()
{
	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	debugf( TEXT("DISHONORED(bringup): ADishonoredGameInfo::GameEnding (pawn %s)"), PlayerPawn ? *PlayerPawn->GetName() : TEXT("None") );
	// DISHONORED(bringup): m_pGamePlayEventsWriter is a native-only member of ADishonoredGameInfo (not in the retail SDK
	// reflection dump), so the generated class does not have it and UGameplayEventsWriter::EndLogging is not reached.
}

void ADishonoredGameInfo::execGameEnding( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	GameEnding();
}
