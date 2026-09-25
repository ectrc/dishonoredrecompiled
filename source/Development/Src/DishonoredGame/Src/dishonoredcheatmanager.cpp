// DishonoredGame/src/dishonoredcheatmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (81):
//   0x7c9b50  public: static void __cdecl UDishonoredCheatManager::InitializePrivateStaticClassUDishonoredCheatManager(void)
//   0x7c9b70  public: virtual void __thiscall UDishonoredCheatManager::InitCheatManager_Native(void)
//   0x7c9b80  public: virtual void __thiscall UDishonoredCheatManager::BeginDestroy(void)
//   0x7c9b90  public: virtual void __thiscall UDishonoredCheatManager::DebugMenuLeft(void)
//   0x7c9bb0  public: virtual void __thiscall UDishonoredCheatManager::DebugMenuRight(void)
//   0x7c9bd0  public: virtual void __thiscall UDishonoredCheatManager::DebugToggleVerbose(void)
//   0x7c9bf0  public: virtual void __thiscall UDishonoredCheatManager::BendTimeDisplay(void)
//   0x7c9c10  public: virtual void __thiscall UDishonoredCheatManager::DebugDialog(void)
//   0x7c9c30  public: virtual void __thiscall UDishonoredCheatManager::DialogSkipGlobalRules(void)
//   0x7c9c50  public: virtual void __thiscall UDishonoredCheatManager::ListComponents(void)
//   0x7c9c70  public: virtual void __thiscall UDishonoredCheatManager::DisNoteToggle(void)
//   0x7c9ca0  public: virtual void __thiscall UDishonoredCheatManager::ListActorTickTime(void)
//   0x7c9cb0  public: virtual void __thiscall UDishonoredCheatManager::KillAllRats(void)
//   0x7c9cd0  public: virtual void __thiscall UDishonoredCheatManager::SpawnRandomRatSwarm(void)
//   0x7c9ce0  public: virtual void __thiscall UDishonoredCheatManager::ToggleRatProbingForGround(void)
//   0x7c9d00  public: virtual void __thiscall UDishonoredCheatManager::DormantDebugToggle(void)
//   0x7c9d20  public: virtual void __thiscall UDishonoredCheatManager::DormantDebugEnable(unsigned int)
//   0x7c9d40  public: virtual void __thiscall UDishonoredCheatManager::ToggleUsableHighlight(void)
//   0x7c9d60  public: virtual void __thiscall UDishonoredCheatManager::DoorPoltergeistMode(int)
//   0x7c9db0  public: virtual void __thiscall UDishonoredCheatManager::SetDifficulty(int)
//   0x7c9de0  public: virtual void __thiscall UDishonoredCheatManager::GetDifficulty(void)
//   0x7c9df0  public: virtual void __thiscall UDishonoredCheatManager::StartVisSettingsMode(void)
//   0x7c9e00  public: virtual void __thiscall UDishonoredCheatManager::StopVisSettingsMode(void)
//   0x7c9e10  public: virtual void __thiscall UDishonoredCheatManager::DisSlomoFull(float, float)
//   0x7c9e50  public: virtual void __thiscall UDishonoredCheatManager::ForceCorpseDropTypeOff(void)
//   0x7c9e60  public: virtual void __thiscall UDishonoredCheatManager::DisSlomo(float)
//   0x7c9e90  public: virtual void __thiscall UDishonoredCheatManager::NPCToggleSteeringAvoidable(void)
//   0x7c9ec0  public: virtual void __thiscall UDishonoredCheatManager::NPCToggleSteeringDanger(void)
//   0x7c9ee0  public: virtual void __thiscall UDishonoredCheatManager::NPCToggleSteeringCombat(void)
//   0x7c9f00  public: virtual void __thiscall UDishonoredCheatManager::NPCToggleSteeringEnemyPush(void)
//   0x7c9f20  public: virtual void __thiscall UDishonoredCheatManager::NPCToggleSteeringWall(void)
//   0x7c9f40  public: virtual void __thiscall UDishonoredCheatManager::GoreEnable(unsigned int)
//   0x7cc5d0  public: virtual void __thiscall UDishonoredCheatManager::DebugMenuEnter(void)
//   0x7cc610  public: virtual void __thiscall UDishonoredCheatManager::DebugMenuUp(void)
//   0x7cc650  public: virtual void __thiscall UDishonoredCheatManager::DebugMenuDown(void)
//   0x7cc690  public: virtual void __thiscall UDishonoredCheatManager::DebugRats(void)
//   0x7cc6f0  public: virtual void __thiscall UDishonoredCheatManager::DebugDarkness(void)
//   0x7cc740  public: virtual void __thiscall UDishonoredCheatManager::GiveBoneCharm(void)
//   0x7cc750  public: virtual void __thiscall UDishonoredCheatManager::SetVisSettingsHighVis(void)
//   0x7cc790  public: virtual void __thiscall UDishonoredCheatManager::SetVisSettingsLowVis(void)
//   0x7cc7d0  public: virtual void __thiscall UDishonoredCheatManager::NormalStepMantleEdgeFinder(void)
//   0x7cc800  public: virtual void __thiscall UDishonoredCheatManager::SetDebugMantleCheckTime(float)
//   0x7cc830  public: virtual void __thiscall UDishonoredCheatManager::SetDebugMantleStepSize(float)
//   0x7cc860  public: virtual void __thiscall UDishonoredCheatManager::ShowPowerMenu(void)
//   0x7cc880  public: virtual void __thiscall UDishonoredCheatManager::GiveRunes(int)
//   0x7cc8c0  public: virtual void __thiscall UDishonoredCheatManager::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7ccd20  public: virtual void __thiscall UDishonoredCheatManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x7cea50  public: virtual void __thiscall UDishonoredCheatManager::SetTargetedActor(class AActor *)
//   0x7cea80  public: virtual void __thiscall UDishonoredCheatManager::AddMod(class FName, class FName, unsigned char, float, float)
//   0x7ceaf0  public: virtual void __thiscall UDishonoredCheatManager::RemoveMod(class FName, class FName)
//   0x7ceb40  public: virtual void __thiscall UDishonoredCheatManager::AddPower(class FName, int)
//   0x7ceb90  public: virtual void __thiscall UDishonoredCheatManager::AddAttribute(class FName, float)
//   0x7cebf0  public: virtual void __thiscall UDishonoredCheatManager::RemovePower(class FName)
//   0x7cec40  public: virtual void __thiscall UDishonoredCheatManager::MaxPowers(void)
//   0x7ced90  public: virtual void __thiscall UDishonoredCheatManager::MinPowers(void)
//   0x7cedd0  public: virtual void __thiscall UDishonoredCheatManager::DebugRat(void)
//   0x7cf0b0  public: virtual void __thiscall UDishonoredCheatManager::AddDarkness(int)
//   0x7cf0f0  public: virtual void __thiscall UDishonoredCheatManager::AddUpgrade(class FString const &)
//   0x7cf230  public: virtual void __thiscall UDishonoredCheatManager::RemoveUpgrade(class FString const &)
//   0x7cf370  public: virtual void __thiscall UDishonoredCheatManager::MaxUpgrades(void)
//   0x7cf410  public: virtual void __thiscall UDishonoredCheatManager::MinUpgrades(void)
//   0x7cf4b0  public: virtual void __thiscall UDishonoredCheatManager::SingleStepMantleEdgeFinder(void)
//   0x7cf540  public: virtual void __thiscall UDishonoredCheatManager::GiveMoney(int)
//   0x7cf5a0  public: virtual void __thiscall UDishonoredCheatManager::ForceCorpseDropType(unsigned char)
//   0x7cf5f0  public: virtual void __thiscall UDishonoredCheatManager::BlinkShowRange(float)
//   0x7cfaa0  public: virtual void __thiscall UDishonoredCheatManager::SwitchDLCLock(int)
//   0x7cfb90  public: virtual void __thiscall UDishonoredCheatManager::ShowStorePage(class FString const &)
//   0x7d4f60  public: virtual void __thiscall UDishonoredCheatManager::BendTimeReport(void)
//   0x7d4fc0  public: virtual void __thiscall UDishonoredCheatManager::TestNailing(void)
//   0x7d5330  ToggleLightEnvironmentsInList
//   0x7d53f0  public: virtual void __thiscall UDishonoredCheatManager::ToggleActorLightEnvironment(void)
//   0x7d5470  public: virtual void __thiscall UDishonoredCheatManager::DisSlomoCommand(class FName)
//   0x7d58d0  public: virtual void __thiscall UDishonoredCheatManager::GiveAllUsableRequiredItems(void)
//   0x7d8420  public: virtual void __thiscall UDishonoredCheatManager::SingleStep_Native(void)
//   0x7d84c0  public: virtual void __thiscall UDishonoredCheatManager::ListTickableActors(void)
//   0x7dac90  public: virtual void __thiscall UDishonoredCheatManager::DisNoteShow(unsigned int)
//   0x7dace0  public: virtual void __thiscall UDishonoredCheatManager::LogAnimSequences(void)
//   0x7db730  public: virtual void __thiscall UDishonoredCheatManager::LogDuplicateAnimSequences(void)
//   0x7dbbe0  public: virtual void __thiscall UDishonoredCheatManager::BlinkShowRangeFull(float)
//   0x7de440  public: static class UClass * __cdecl UDishonoredCheatManager::GetPrivateStaticClassUDishonoredCheatManager(wchar_t const *)
//   0x7df250  public: static class UClass * __cdecl UDishonoredCheatManager::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(written): exec 2013 rva 0x5f3bb0 -> vtable +1244 = 2013 rva 0x76f3d0 (`m_bVisSettingsMode = TRUE`)
void UDishonoredCheatManager::execStartVisSettingsMode( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	m_bVisSettingsMode = TRUE;
}

// DISHONORED(written): exec 2013 rva 0x5f3bf0 -> vtable +1248 = 2013 rva 0x76f3e0 (`m_bVisSettingsMode = FALSE`)
void UDishonoredCheatManager::execStopVisSettingsMode( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	m_bVisSettingsMode = FALSE;
}

// DISHONORED(written): exec 2013 rva 0x5f3e70 (2012 0x63a0a0) -> vtable +1280 = 2013 rva 0x76f430 (`m_bOverrideCorpseDropType = FALSE`)
void UDishonoredCheatManager::execForceCorpseDropTypeOff( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	m_bOverrideCorpseDropType = FALSE;
}

// ---- natives whose retail body is trivial (generated by build/agentAC_work/gen_trivial.py from the 2013 vtables) ----

// DISHONORED(written): 2013 rva 0x5f45c0; the retail UDishonoredCheatManager vtable slot +1332 it dispatches to is `return TRUE`
// (2013 rva 0x5ea9d0) and no retail subclass overrides it
void UDishonoredCheatManager::execIsAttentionTypeAllowed( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_Type);
	P_FINISH;
	*(UBOOL*)Result = TRUE;
}

// DISHONORED(written): 2013 rva 0x5ee180; the retail UDishonoredCheatManager vtable slot +1328 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAttentionClearExclusiveType( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f44b0; the retail UDishonoredCheatManager vtable slot +1324 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8715d0) and no retail subclass overrides it
void UDishonoredCheatManager::execAttentionSetExclusiveType( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_Type1);
	P_GET_BYTE_OPTX(_Type2, 0);
	P_GET_BYTE_OPTX(_Type3, 0);
	P_GET_BYTE_OPTX(_Type4, 0);
	P_GET_BYTE_OPTX(_Type5, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f43a0; the retail UDishonoredCheatManager vtable slot +1320 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8715d0) and no retail subclass overrides it
void UDishonoredCheatManager::execAttentionAddExclusiveType( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_Type1);
	P_GET_BYTE_OPTX(_Type2, 0);
	P_GET_BYTE_OPTX(_Type3, 0);
	P_GET_BYTE_OPTX(_Type4, 0);
	P_GET_BYTE_OPTX(_Type5, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f4360; the retail UDishonoredCheatManager vtable slot +1316 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAttentionClearIgnoreType( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f4250; the retail UDishonoredCheatManager vtable slot +1312 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8715d0) and no retail subclass overrides it
void UDishonoredCheatManager::execAttentionSetIgnoreType( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_Type1);
	P_GET_BYTE_OPTX(_Type2, 0);
	P_GET_BYTE_OPTX(_Type3, 0);
	P_GET_BYTE_OPTX(_Type4, 0);
	P_GET_BYTE_OPTX(_Type5, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f4140; the retail UDishonoredCheatManager vtable slot +1308 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8715d0) and no retail subclass overrides it
void UDishonoredCheatManager::execAttentionAddIgnoreType( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_Type1);
	P_GET_BYTE_OPTX(_Type2, 0);
	P_GET_BYTE_OPTX(_Type3, 0);
	P_GET_BYTE_OPTX(_Type4, 0);
	P_GET_BYTE_OPTX(_Type5, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f39a0; the retail UDishonoredCheatManager vtable slot +1216 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execReportDanglingPointers( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bReportShutdownPawns, FALSE);
	P_GET_UBOOL_OPTX(_bReportDestroyedActors, FALSE);
	P_GET_UBOOL_OPTX(_bReportUnreportedProperties, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2e70; the retail UDishonoredCheatManager vtable slot +1072 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execEndLiveSession( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2e30; the retail UDishonoredCheatManager vtable slot +1068 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execCreateLiveSession( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2da0; the retail UDishonoredCheatManager vtable slot +1060 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x76f550) and no retail subclass overrides it
void UDishonoredCheatManager::execWriteScoreToLeaderboard_ID( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(nLeaderboardID);
	P_GET_INT(nScore);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2c90; the retail UDishonoredCheatManager vtable slot +1052 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x76f540) and no retail subclass overrides it
void UDishonoredCheatManager::execGetLeaderboardsInfo_ID( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(nLeaderboardID);
	P_GET_INT(nStartIndex);
	P_GET_INT(nNbEntriesToRetrieve);
	P_GET_UBOOL_OPTX(bFriends, FALSE);
	P_GET_UBOOL_OPTX(bAroundUser, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2bb0; the retail UDishonoredCheatManager vtable slot +1040 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execDLC05AddScore( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(I);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2b70; the retail UDishonoredCheatManager vtable slot +1036 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execDLC05ShowChallenge( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2b30; the retail UDishonoredCheatManager vtable slot +1028 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execSlack( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2860; the retail UDishonoredCheatManager vtable slot +996 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execDestroyActor( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f27b0; the retail UDishonoredCheatManager vtable slot +984 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCLogSteeringFlags( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1d4d20; the retail UDishonoredCheatManager vtable slot +960 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCToggleLookAtBlending( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2680; the retail UDishonoredCheatManager vtable slot +956 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCDelayTravel( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT_OPTX(_fTravelDelay, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2610; the retail UDishonoredCheatManager vtable slot +952 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x128ad0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCSpawn( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(_TweaksName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1d0360; the retail UDishonoredCheatManager vtable slot +948 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCPlayerVisibilityForLOD( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_iNPCVisibility);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f6340; the retail UDishonoredCheatManager vtable slot +944 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCToggleLODSystem( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f25d0; the retail UDishonoredCheatManager vtable slot +940 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCClearForcedLOD( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2510; the retail UDishonoredCheatManager vtable slot +936 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCForceLODForAll( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_LodStatus);
	P_GET_INT_OPTX(_iLodDistance, 0);
	P_GET_INT_OPTX(_iNPCForcedRendered, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2450; the retail UDishonoredCheatManager vtable slot +932 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCForceLOD( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE(_LodStatus);
	P_GET_INT_OPTX(_iLodDistance, 0);
	P_GET_INT_OPTX(_iNPCForcedRendered, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1c26e0; the retail UDishonoredCheatManager vtable slot +928 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCActionSpineBendingToggle( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2410; the retail UDishonoredCheatManager vtable slot +924 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCTogglePrediction( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f23a0; the retail UDishonoredCheatManager vtable slot +920 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCSetFreeze( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bFreeze);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2360; the retail UDishonoredCheatManager vtable slot +916 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCFreeze( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2300; the retail UDishonoredCheatManager vtable slot +912 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCForceSkelMeshLOD( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(iDesiredLevel);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f22c0; the retail UDishonoredCheatManager vtable slot +908 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCSlowDeath( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2280; the retail UDishonoredCheatManager vtable slot +904 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCToggleSlopeOffset( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2210; the retail UDishonoredCheatManager vtable slot +900 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCSlap( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(fDesiredDamage);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2170; the retail UDishonoredCheatManager vtable slot +896 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x128ad0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCImpulse( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(_fImpulseMagnitude);
	P_GET_FLOAT_OPTX(_fTeleportHeight, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2130; the retail UDishonoredCheatManager vtable slot +892 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCKnockdown( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f20f0; the retail UDishonoredCheatManager vtable slot +888 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCBreakLinkedAction( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2080; the retail UDishonoredCheatManager vtable slot +884 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCSetDamage( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(fDesiredDamage);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2040; the retail UDishonoredCheatManager vtable slot +880 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCEasyDismemberment( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f2000; the retail UDishonoredCheatManager vtable slot +876 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCForceFatality( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1fc0; the retail UDishonoredCheatManager vtable slot +872 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCTestVersus( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1f80; the retail UDishonoredCheatManager vtable slot +868 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCTestParry( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1f40; the retail UDishonoredCheatManager vtable slot +864 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCDodgeNothing( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1f00; the retail UDishonoredCheatManager vtable slot +860 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCDodgeEverything( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1ec0; the retail UDishonoredCheatManager vtable slot +856 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCDestroyNonVisible( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1e80; the retail UDishonoredCheatManager vtable slot +852 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCDestroyNonSelected( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1e40; the retail UDishonoredCheatManager vtable slot +848 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNPCDestroy( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1e00; the retail UDishonoredCheatManager vtable slot +844 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execGodNPC( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1dc0; the retail UDishonoredCheatManager vtable slot +840 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execNavMeshCheck( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1d80; the retail UDishonoredCheatManager vtable slot +836 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execMatGoBackward( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1d40; the retail UDishonoredCheatManager vtable slot +832 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execMatGoForward( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1d00; the retail UDishonoredCheatManager vtable slot +828 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoTeleport( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1cc0; the retail UDishonoredCheatManager vtable slot +824 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoShowPathAndStraightPath( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1c80; the retail UDishonoredCheatManager vtable slot +820 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoShowAvoidance( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1c40; the retail UDishonoredCheatManager vtable slot +816 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoNewPhysWalking( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1c00; the retail UDishonoredCheatManager vtable slot +812 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoNewIK( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1bc0; the retail UDishonoredCheatManager vtable slot +808 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoNewAvoidance( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1b80; the retail UDishonoredCheatManager vtable slot +804 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoForceUnaccuracy( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1b20; the retail UDishonoredCheatManager vtable slot +800 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoForceModifier( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_Modifier);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1ac0; the retail UDishonoredCheatManager vtable slot +796 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoForceLookAtMode( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_LookAtMode);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1a80; the retail UDishonoredCheatManager vtable slot +792 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoForceIdleAnim( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1a40; the retail UDishonoredCheatManager vtable slot +788 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoDontRotate( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1a00; the retail UDishonoredCheatManager vtable slot +784 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execLocoDisableStartAnims( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f19c0; the retail UDishonoredCheatManager vtable slot +780 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIUseSimpleAttentionForPlayer( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1980; the retail UDishonoredCheatManager vtable slot +776 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIEnableAllSubStates( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x6002f0; the retail UDishonoredCheatManager vtable slot +772 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDisableSubState( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rSubStateName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x600240; the retail UDishonoredCheatManager vtable slot +768 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIEnableSubState( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rSubStateName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1940; the retail UDishonoredCheatManager vtable slot +764 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIEnableAllBehaviors( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x600190; the retail UDishonoredCheatManager vtable slot +760 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDisableBehavior( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rBehaviorName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x6000e0; the retail UDishonoredCheatManager vtable slot +756 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIEnableBehavior( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rBehaviorName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f18d0; the retail UDishonoredCheatManager vtable slot +752 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIShowThoughts( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT_OPTX(_fRadius, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1860; the retail UDishonoredCheatManager vtable slot +748 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetNumbToAIs( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bNumb);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1820; the retail UDishonoredCheatManager vtable slot +744 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAINumbToAIs( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f17b0; the retail UDishonoredCheatManager vtable slot +740 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetDeafToAIs( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bDeaf);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1770; the retail UDishonoredCheatManager vtable slot +736 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDeafToAIs( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1700; the retail UDishonoredCheatManager vtable slot +732 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetBlindToAIs( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bBlind);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f16c0; the retail UDishonoredCheatManager vtable slot +728 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIBlindToAIs( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1650; the retail UDishonoredCheatManager vtable slot +724 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetDumbToAIs( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bDumb);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1610; the retail UDishonoredCheatManager vtable slot +720 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDumbToAIs( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f15a0; the retail UDishonoredCheatManager vtable slot +716 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetNumbToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bNumb);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1560; the retail UDishonoredCheatManager vtable slot +712 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAINumbToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f14f0; the retail UDishonoredCheatManager vtable slot +708 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetDeafToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bDeaf);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f14b0; the retail UDishonoredCheatManager vtable slot +704 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDeafToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1440; the retail UDishonoredCheatManager vtable slot +700 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetBlindToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bBlind);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1400; the retail UDishonoredCheatManager vtable slot +696 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIBlindToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1390; the retail UDishonoredCheatManager vtable slot +692 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetDumbToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bDumb);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5e51c0; the retail UDishonoredCheatManager vtable slot +688 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDumbToPlayer( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1350; the retail UDishonoredCheatManager vtable slot +684 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIForceAttentionLevelOff( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f12e0; the retail UDishonoredCheatManager vtable slot +680 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIForceAttentionLevel( FFrame& Stack, RESULT_DECL )
{
	P_GET_BYTE_OPTX(_AttentionLevel, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f12a0; the retail UDishonoredCheatManager vtable slot +676 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIClearAttention( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1260; the retail UDishonoredCheatManager vtable slot +672 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIForcePanicNow( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1220; the retail UDishonoredCheatManager vtable slot +668 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIForcePanic( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f8070; the retail UDishonoredCheatManager vtable slot +664 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIRegisterAllHideouts( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f11e0; the retail UDishonoredCheatManager vtable slot +660 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDebugHideout( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f8030; the retail UDishonoredCheatManager vtable slot +656 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetLocoDebug( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f11a0; the retail UDishonoredCheatManager vtable slot +652 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIGoToReset( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1160; the retail UDishonoredCheatManager vtable slot +648 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIGoToCrosshair( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f7f90; the retail UDishonoredCheatManager vtable slot +644 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIPauseOnPathFailure( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1120; the retail UDishonoredCheatManager vtable slot +640 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIVerbosePathDebug( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f10b0; the retail UDishonoredCheatManager vtable slot +636 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetBeepWhenSeePlayer( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bBeep);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1070; the retail UDishonoredCheatManager vtable slot +632 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIBeepWhenSeePlayer( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f1030; the retail UDishonoredCheatManager vtable slot +628 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIAlwaysRingAlarm( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f9520; the retail UDishonoredCheatManager vtable slot +624 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAINeverRingAlarm( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f8210; the retail UDishonoredCheatManager vtable slot +620 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDontRotate( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0fc0; the retail UDishonoredCheatManager vtable slot +616 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIForceTransitSpeed( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(TransitSpeedEnum, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0f80; the retail UDishonoredCheatManager vtable slot +612 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIToggleMagic( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f7dd0; the retail UDishonoredCheatManager vtable slot +608 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAITestGuns( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0f10; the retail UDishonoredCheatManager vtable slot +604 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIEnable( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bEnable);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0ed0; the retail UDishonoredCheatManager vtable slot +600 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIToggle( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f8190; the retail UDishonoredCheatManager vtable slot +596 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDontAttack( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f7740; the retail UDishonoredCheatManager vtable slot +592 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetSuspicion( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(DesiredSuspicion);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0e90; the retail UDishonoredCheatManager vtable slot +588 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIForget( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0e10; the retail UDishonoredCheatManager vtable slot +584 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDumb( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bOnSingleAI, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0d90; the retail UDishonoredCheatManager vtable slot +580 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAINumb( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bOnSingleAI, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0d10; the retail UDishonoredCheatManager vtable slot +576 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIBlind( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bOnSingleAI, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0c90; the retail UDishonoredCheatManager vtable slot +572 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAIDeaf( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bOnSingleAI, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0c50; the retail UDishonoredCheatManager vtable slot +568 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIGibAll( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f7b40; the retail UDishonoredCheatManager vtable slot +564 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIGib( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f7580; the retail UDishonoredCheatManager vtable slot +560 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAIClearDebug( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f93a0; the retail UDishonoredCheatManager vtable slot +556 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAICycleDebug( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f7540; the retail UDishonoredCheatManager vtable slot +552 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAISetDebug( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0b30; the retail UDishonoredCheatManager vtable slot +548 it dispatches to is `return FALSE`
// (2013 rva 0x2a3630) and no retail subclass overrides it
void UDishonoredCheatManager::execDisplayDebug_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AHUD, _pHUD);
	P_GET_FLOAT_REF(_rfOut_YL);
	P_GET_FLOAT_REF(_rfOut_YPos);
	P_FINISH;
	*(UBOOL*)Result = FALSE;
}

// DISHONORED(written): 2013 rva 0x5f0ac0; the retail UDishonoredCheatManager vtable slot +540 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerGhostChestView( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bChestView);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0a00; the retail UDishonoredCheatManager vtable slot +536 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerTestCamRumble( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(_fAmplitude);
	P_GET_FLOAT(_fFrequency);
	P_GET_FLOAT(_fDuration);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f9460; the retail UDishonoredCheatManager vtable slot +532 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execEnableIronSight( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0960; the retail UDishonoredCheatManager vtable slot +528 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x128ad0) and no retail subclass overrides it
void UDishonoredCheatManager::execDoLineCheckStressTest( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_NumRadialSegs, 0);
	P_GET_INT_OPTX(_NumHeightSegs, 0);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f94e0; the retail UDishonoredCheatManager vtable slot +524 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceDramaticFatalities( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f94a0; the retail UDishonoredCheatManager vtable slot +520 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceKillCam( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f73e0; the retail UDishonoredCheatManager vtable slot +516 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerToggleHomingArrow( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0920; the retail UDishonoredCheatManager vtable slot +512 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerToggleAimAssist( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f08a0; the retail UDishonoredCheatManager vtable slot +508 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerShowNavMeshStatus( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(_bShow, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f8250; the retail UDishonoredCheatManager vtable slot +504 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerFillAdrenaline( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f73a0; the retail UDishonoredCheatManager vtable slot +500 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerToggleAdrenalineCooldown( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f6ba0; the retail UDishonoredCheatManager vtable slot +496 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerInfiniteAdrenaline( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0860; the retail UDishonoredCheatManager vtable slot +492 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execAbstractInventoryList( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0820; the retail UDishonoredCheatManager vtable slot +488 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerInfiniteAmmo( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f07e0; the retail UDishonoredCheatManager vtable slot +484 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDisableKnockdown( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f07a0; the retail UDishonoredCheatManager vtable slot +480 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerEnableKnockdown( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0760; the retail UDishonoredCheatManager vtable slot +476 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDisableStrongHitReaction( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0720; the retail UDishonoredCheatManager vtable slot +472 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerEnableStrongHitReaction( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f06e0; the retail UDishonoredCheatManager vtable slot +468 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDisableTakeDamage( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f06a0; the retail UDishonoredCheatManager vtable slot +464 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerEnableTakeDamage( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f05d0; the retail UDishonoredCheatManager vtable slot +460 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageProjectile_Stealth( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0500; the retail UDishonoredCheatManager vtable slot +456 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageProjectile( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0430; the retail UDishonoredCheatManager vtable slot +452 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageKnockdown( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0360; the retail UDishonoredCheatManager vtable slot +448 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageImpact( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0290; the retail UDishonoredCheatManager vtable slot +444 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageFastHit( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f01c0; the retail UDishonoredCheatManager vtable slot +440 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageElectricity( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f00f0; the retail UDishonoredCheatManager vtable slot +436 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageBullet( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f0020; the retail UDishonoredCheatManager vtable slot +432 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamageBash( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5eff50; the retail UDishonoredCheatManager vtable slot +428 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDamage( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT_OPTX(_Amount, 0);
	P_GET_FLOAT_OPTX(_fMomentum, 0.f);
	P_GET_FLOAT_OPTX(_fAngle, 0.f);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5e47a0; the retail UDishonoredCheatManager vtable slot +424 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execHealthAdd( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_Amount);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5eff10; the retail UDishonoredCheatManager vtable slot +420 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execBuddha( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efed0; the retail UDishonoredCheatManager vtable slot +416 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execManaToggle( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efe60; the retail UDishonoredCheatManager vtable slot +412 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execManaEnable( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bEnable);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efe00; the retail UDishonoredCheatManager vtable slot +408 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execManaAdd( FFrame& Stack, RESULT_DECL )
{
	P_GET_INT(_Amount);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1cd3e0; the retail UDishonoredCheatManager vtable slot +404 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceDeathDir( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1cd3a0; the retail UDishonoredCheatManager vtable slot +400 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execClearObjectives( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x600030; the retail UDishonoredCheatManager vtable slot +396 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execRemoveObjective( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_ObjectiveName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5fff80; the retail UDishonoredCheatManager vtable slot +392 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execAddObjective( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_ObjectiveName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5ffe90; the retail UDishonoredCheatManager vtable slot +388 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x128ad0) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerTestAnim( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR_OPTX(_AnimName, TEXT(""));
	P_GET_UBOOL_OPTX(_bFullBody, FALSE);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1cfa30; the retail UDishonoredCheatManager vtable slot +384 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDisableMeleeAssist( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efdc0; the retail UDishonoredCheatManager vtable slot +380 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceLongFinishers( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efd80; the retail UDishonoredCheatManager vtable slot +376 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceFastFinishers( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efd40; the retail UDishonoredCheatManager vtable slot +372 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDisableAssassinate( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f4800; the retail UDishonoredCheatManager vtable slot +368 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDisableFatality( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5f47c0; the retail UDishonoredCheatManager vtable slot +364 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerSetFixedFatality( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efd00; the retail UDishonoredCheatManager vtable slot +360 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceReadyStance( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1c7fe0; the retail UDishonoredCheatManager vtable slot +356 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerSetDamage( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT(fDesiredDamage);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efcc0; the retail UDishonoredCheatManager vtable slot +352 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerForceVisible( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efc80; the retail UDishonoredCheatManager vtable slot +348 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDodgeNothing( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1c1ac0; the retail UDishonoredCheatManager vtable slot +344 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerDodgeEverything( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efc40; the retail UDishonoredCheatManager vtable slot +340 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerTestChain( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1d4070; the retail UDishonoredCheatManager vtable slot +336 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execidkfa( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efc00; the retail UDishonoredCheatManager vtable slot +332 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execMaxItems( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5ffde0; the retail UDishonoredCheatManager vtable slot +328 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execGivePlayerItem( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(_rTweakString);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efba0; the retail UDishonoredCheatManager vtable slot +324 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execTogglePlayerBodyMode_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(APawn, _pPawn);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efb30; the retail UDishonoredCheatManager vtable slot +320 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x1cb0c0) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerArmsEnable( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL(_bEnable);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x570fc0; the retail UDishonoredCheatManager vtable slot +316 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x59d10) and no retail subclass overrides it
void UDishonoredCheatManager::execPlayerArmsToggle( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5efa70; the retail UDishonoredCheatManager vtable slot +312 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void UDishonoredCheatManager::execSetCameraBob_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(APawn, _pPawn);
	P_GET_FLOAT(_fBob);
	P_GET_FLOAT(_fRoll);
	P_FINISH;
}

// ---- end of trivial natives ----
