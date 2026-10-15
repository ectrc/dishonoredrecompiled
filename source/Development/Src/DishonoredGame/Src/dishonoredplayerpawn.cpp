// DishonoredGame/src/dishonoredplayerpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (165):
//   0x6fa8e0  public: static void __cdecl ADishonoredPlayerPawn::InitializePrivateStaticClassADishonoredPlayerPawn(void)
//   0x6fa900  public: virtual enum ERelationship __thiscall ADishonoredPlayerPawn::DetermineRelationship_Derived(class IDisRelationshipInterface const &, enum ERelationship)const
//   0x6fa950  public: float __thiscall ADishonoredPlayerPawn::GetLastDeltaSeconds(void)const
//   0x6fa960  public: virtual unsigned int __thiscall UDisNPCTravelManager::IsSaveable(enum ESaveLoadLocation)const
//   0x6fa970  private: void __thiscall ADishonoredPlayerPawn::SaveLoadTutorialTrackers(class FArchive &)const
//   0x6fa9a0  public: virtual void __thiscall ADishonoredPlayerPawn::OnAddAttributeModifier(class UDisSeqAct_AddAttributeModifier *)
//   0x6fa9e0  public: virtual void __thiscall ADishonoredPlayerPawn::OnRemoveAttributeModifier(class UDisSeqAct_RemoveAttributeModifier *)
//   0x6faa10  public: static void __cdecl UDisSeqAct_GiveUpgrade::InitializePrivateStaticClassUDisSeqAct_GiveUpgrade(void)
//   0x6faa30  public: virtual void __thiscall ADishonoredPlayerPawn::OnEnterWaterVolume(class ADishonoredWaterVolume *)
//   0x6faa40  public: class UDisDarknessManager * __thiscall ADishonoredPlayerPawn::GetDarknessManager(void)const
//   0x6faa50  public: class UDisKeyRing * __thiscall ADishonoredPlayerPawn::GetKeyRing(void)const
//   0x6faa60  public: void __thiscall ADishonoredPlayerPawn::AffectByPlagueFor(float, class UClass *)
//   0x6faac0  public: void __thiscall ADishonoredPlayerPawn::SetWindBlastOpacity(float)
//   0x6faae0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::ShouldDropItemsOnAnimNotify(void)const
//   0x6faaf0  public: virtual void __thiscall ADishonoredPlayerPawn::OnUsableObjectTransitionComplete(class ADishonoredUsableObject *)
//   0x6fab00  public: virtual void __thiscall ADishonoredPlayerPawn::physCustom(float, int)
//   0x6fab30  public: virtual void __thiscall ADishonoredPlayerPawn::CheatWalk_Native(void)
//   0x6fab50  public: virtual void __thiscall ADishonoredPlayerPawn::CheatGhost_Native(void)
//   0x6fab70  public: virtual void __thiscall ADishonoredPlayerPawn::CheatFly_Native(void)
//   0x6fab80  public: int __thiscall ADishonoredPlayerPawn::FindBoneCharmSlot(int)const
//   0x6fac40  public: virtual void __thiscall ADishonoredPlayerPawn::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x6fac60  public: virtual int __thiscall ADishonoredPlayerPawn::GetMana(void)const
//   0x6fac70  public: virtual int __thiscall ADishonoredPlayerPawn::GetManaMax(void)const
//   0x6fac80  public: virtual void __thiscall ADishonoredPlayerPawn::AddMana(int)
//   0x6faca0  public: int __thiscall ADishonoredPlayerPawn::GetPreviousMana(void)const
//   0x6facb0  protected: virtual void __thiscall ADishonoredPlayerPawn::StopRootMotionMode(void)
//   0x6facc0  public: static void __cdecl UDisSeqAct_EvalAchievement::InitializePrivateStaticClassUDisSeqAct_EvalAchievement(void)
//   0x6face0  public: static void __cdecl UDisSeqAct_GetPlayerStat::InitializePrivateStaticClassUDisSeqAct_GetPlayerStat(void)
//   0x6fad00  public: static void __cdecl UDisSeqAct_ToggleAchievementEval::InitializePrivateStaticClassUDisSeqAct_ToggleAchievementEval(void)
//   0x6fad20  public: static void __cdecl UDisSeqAct_IncrementPlayerStat::InitializePrivateStaticClassUDisSeqAct_IncrementPlayerStat(void)
//   0x6fad40  public: static void __cdecl UDisSeqAct_GivePickup::InitializePrivateStaticClassUDisSeqAct_GivePickup(void)
//   0x6fad60  public: void __thiscall ADishonoredPlayerPawn::SetIsVisible(unsigned int, int)
//   0x6fada0  public: virtual void __thiscall ADishonoredPlayerPawn::Regen(float)
//   0x6fadd0  public: unsigned int __thiscall ADishonoredPlayerPawn::IsTutorialNoteAlreadyDisplayed(enum EDisTutorialNote)const
//   0x6fadf0  public: void __thiscall ADishonoredPlayerPawn::SetTutorialNoteAlreadyDisplayed(enum EDisTutorialNote)
//   0x6fae10  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::DisIsBlinking(void)const
//   0x6fcfe0  public: virtual void __thiscall ADishonoredPlayerPawn::StartCrouch(float)
//   0x6fd080  public: virtual void __thiscall ADishonoredPlayerPawn::EndCrouch(float)
//   0x6fd0e0  public: virtual void __thiscall ADishonoredPlayerPawn::performPhysics(float)
//   0x6fd180  void __cdecl ModifyHitNormalAndDampenVelocityForSlide(struct FCheckResult const &, class FVector const &, class FVector &, class FVector &)
//   0x6fd470  private: unsigned int __thiscall ADishonoredPlayerPawn::moveSmoothSlide(class FVector const &)
//   0x6fd9e0  public: unsigned int __thiscall ADishonoredPlayerPawn::PerformSlidePhysics(float, int)
//   0x6fe0e0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::CanShowScene(void)const
//   0x6fe140  protected: virtual float __thiscall ADishonoredPlayerPawn::MaxSpeedModifier_Derived(void)
//   0x6fe1b0  public: virtual void __thiscall ADishonoredPlayerPawn::OnGivePickup(class UDisSeqAct_GivePickup *)
//   0x6fe220  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsAttacking(class ADishonoredPawn const &)const
//   0x6fe250  public: virtual void __thiscall ADishonoredPlayerPawn::OnSetPlayerHealth(class UDisSeqAct_SetPlayerHealth *)
//   0x6fe2a0  public: virtual void __thiscall ADishonoredPlayerPawn::OnSetPlayerMana(class UDisSeqAct_SetPlayerMana *)
//   0x6fe2e0  public: class UDisWheelTracker * __thiscall ADishonoredPlayerPawn::GetWheelTracker(void)const
//   0x6fe2f0  public: class UDisElixirTracker * __thiscall ADishonoredPlayerPawn::GetElixirTracker(void)const
//   0x6fe300  public: class UDisBlockTracker * __thiscall ADishonoredPlayerPawn::GetBlockTracker(void)const
//   0x6fe310  public: class UDisPerfectBlockTracker * __thiscall ADishonoredPlayerPawn::GetPerfectBlockTracker(void)const
//   0x6fe320  public: class UDisMovableTracker * __thiscall ADishonoredPlayerPawn::GetMovableTracker(void)const
//   0x6fe330  private: virtual void __thiscall ADishonoredPlayerPawn::HandleBendtimeTouch(class AActor *)
//   0x701cf0  public: virtual void __thiscall ADishonoredPlayerPawn::ApplyAttributes(void)
//   0x701e00  private: void __thiscall ADishonoredPlayerPawn::RegenMana(float)
//   0x701f10  void __cdecl ShowCompRecursiveHelper(class UPrimitiveComponent *, unsigned int)
//   0x701fc0  void __cdecl SetCompDepthGroupRecursiveHelper(class UPrimitiveComponent *, enum ESceneDepthPriorityGroup)
//   0x702070  public: void __thiscall ADishonoredPlayerPawn::SetBodyMode(enum ePlayerBodyMode)
//   0x7021d0  public: void __thiscall ADishonoredPlayerPawn::UnCrouch_Down(void)
//   0x702330  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsInvisible(void)
//   0x702380  public: virtual class FVector __thiscall ADishonoredPlayerPawn::GetRatTargetBottomLocation(void)const
//   0x702480  public: virtual void __thiscall ADishonoredPlayerPawn::OnTogglePlayerLeftHand(class UDisSeqAct_TogglePlayerLeftHand *)
//   0x702550  public: void __thiscall ADishonoredPlayerPawn::EnableControllerInput(int, unsigned int, enum DisInputMaskLevel)
//   0x702580  public: unsigned int __thiscall ADishonoredPlayerPawn::IsInputEnabled(int)const
//   0x7025b0  public: virtual void __thiscall ADishonoredPlayerPawn::physLadder(float, int)
//   0x702e80  public: virtual void __thiscall ADishonoredPlayerPawn::OnUsableObjectTransitionStart(class ADishonoredUsableObject *)
//   0x702f00  public: virtual class UObject * __thiscall ADishonoredPlayerPawn::GetStateFiringObject(class UAnimTree *)const
//   0x702f20  public: unsigned int __thiscall ADishonoredPlayerPawn::CheckStoryFlag(class FString const &, class FGuid const &)const
//   0x702fa0  public: void __thiscall ADishonoredPlayerPawn::SetStoryFlag(class FString const &, class FGuid const &, unsigned int)
//   0x7030d0  public: int __thiscall ADishonoredPlayerPawn::FindBoneCharm(int)
//   0x703150  public: int __thiscall ADishonoredPlayerPawn::GetBoneCharmSlotCount(void)const
//   0x7031b0  public: struct FDisWhaleBoneCharmInfo const & __thiscall ADishonoredPlayerPawn::GetBoneCharmInfo(int)const
//   0x703290  public: void __thiscall ADishonoredPlayerPawn::ApplyUpgrades(void)
//   0x7032f0  public: float __thiscall ADishonoredPlayerPawn::GetAimStability(void)const
//   0x7033f0  public: void __thiscall ADishonoredPlayerPawn::FillUIInteractions(struct FDisUIInteractionContext &)const
//   0x703670  public: void __thiscall ADishonoredPlayerPawn::ConsumeMana(int)
//   0x703730  private: void __thiscall ADishonoredPlayerPawn::EvalAchievement(enum EAchievement, unsigned int)
//   0x703910  private: void __thiscall ADishonoredPlayerPawn::EvalAchievements(void)
//   0x703990  public: void __thiscall ADishonoredPlayerPawn::FindMissionStatValue(unsigned char, float &, int &)const
//   0x703a70  private: void __thiscall ADishonoredPlayerPawn::IncrementMissionStatValue(enum EDisPlayerStat, float)
//   0x703b40  private: void __thiscall ADishonoredPlayerPawn::SetMissionStatValueIfGreater(enum EDisPlayerStat, float)
//   0x703c10  private: void __thiscall ADishonoredPlayerPawn::HandleAvoidance(float)
//   0x703df0  public: void __thiscall ADishonoredPlayerPawn::HandleAINoiseMade(class FVector const &, class FVector const &, float)
//   0x703f90  public: virtual void __thiscall ADishonoredPlayerPawn::OnEvalAchievement(class UDisSeqAct_EvalAchievement *)
//   0x703fc0  public: virtual void __thiscall ADishonoredPlayerPawn::OnGetPlayerStat(class UDisSeqAct_GetPlayerStat *)
//   0x704080  public: virtual void __thiscall ADishonoredPlayerPawn::OnToggleAchievementEval(class UDisSeqAct_ToggleAchievementEval *)
//   0x7042b0  public: void __thiscall ADishonoredPlayerPawn::AttemptEndClimb(void)
//   0x704330  public: virtual void __thiscall ADishonoredPlayerPawn::OnAdrenalineToggle(class UDisSeqAct_AdrenalineToggle *)
//   0x704410  public: unsigned int __thiscall ADishonoredPlayerPawn::HasUpgrade(class UDisTweaks_Upgrade const *)const
//   0x704480  public: virtual void __thiscall ADishonoredPlayerPawn::OnToggleChoke(class UDisSeqAct_ToggleChoke *)
//   0x7045d0  public: virtual void __thiscall ADishonoredPlayerPawn::OnCancelPlayerActivePower(class UDisSeqAct_CancelPlayerActivePower *)
//   0x704680  public: virtual void __thiscall ADishonoredPlayerPawn::OnToggleTutorial(class UDisSeqAct_ToggleTutorial *)
//   0x7048c0  public: void __thiscall ADishonoredPlayerPawn::CancelAllActivePowers(void)const
//   0x704940  public: void __thiscall ADishonoredPlayerPawn::OnPreCommitMapChange(void)
//   0x70ad90  private: void __thiscall ADishonoredPlayerPawn::CheckCrouchEvents(void)
//   0x70ae50  public: virtual void __thiscall ADishonoredPlayerPawn::UnCrouch(int)
//   0x70afd0  public: virtual void __thiscall ADishonoredPlayerPawn::FaceRotation(class FRotator, float)
//   0x70b130  public: virtual void __thiscall ADishonoredPlayerPawn::OnTogglePowerWheel(class UDisSeqAct_TogglePowerWheel *)
//   0x70b200  public: virtual void __thiscall ADishonoredPlayerPawn::OnToggleJournal(class UDisSeqAct_ToggleJournal *)
//   0x70b2d0  public: virtual void __thiscall ADishonoredPlayerPawn::OnAddKey(class UDisSeqAct_AddKey *)
//   0x70b300  public: virtual void __thiscall ADishonoredPlayerPawn::OnExitWaterVolume(class ADishonoredWaterVolume *)
//   0x70b340  public: void __thiscall ADishonoredPlayerPawn::SnapRotation(class FRotator const &)
//   0x70b380  public: virtual void __thiscall ADishonoredPlayerPawn::OnSetActivePowerLevel(int, int)
//   0x70b470  public: unsigned int __thiscall ADishonoredPlayerPawn::CheckStoryFlag(class UDisStoryFlagSet const *, class FGuid const &)const
//   0x70b4f0  public: void __thiscall ADishonoredPlayerPawn::SetStoryFlag(class UDisStoryFlagSet const *, class FGuid const &, unsigned int)
//   0x70b570  public: int __thiscall ADishonoredPlayerPawn::CanBuyPowers(int)const
//   0x70b780  public: int __thiscall ADishonoredPlayerPawn::FindFreeBoneCharmSlot(void)const
//   0x70b7c0  public: int __thiscall ADishonoredPlayerPawn::GetMaxBoneCharmSlotCount(void)const
//   0x70b8a0  public: void __thiscall ADishonoredPlayerPawn::DeactivateBoneCharmSlot(int)
//   0x70ba40  public: virtual void __thiscall ADishonoredPlayerPawn::SetMana(int)
//   0x70bb10  public: void __thiscall ADishonoredPlayerPawn::AddTrackedMissionStat(unsigned char, int)
//   0x70ef70  public: virtual void __thiscall ADishonoredPlayerPawn::ShutPawnDown_Derived(void)
//   0x70eff0  public: virtual void __thiscall ADishonoredPlayerPawn::Landed_Native(class FVector, class AActor *)
//   0x70f630  public: virtual void __thiscall ADishonoredPlayerPawn::TakeDamage_Native(int &, class AController *, class FVector, class FVector &, class UClass *, struct FTraceHitInfo, class AActor *)
//   0x70f930  public: virtual void __thiscall ADishonoredPlayerPawn::PlayDying_Native(class AController *, class UClass *, class FVector)
//   0x70faa0  public: void __thiscall ADishonoredPlayerPawn::InitializeBoneCharmPickup(class ADisWhaleBoneCharm *)
//   0x70fb80  public: void __thiscall ADishonoredPlayerPawn::DeactivateBoneCharm(int)
//   0x70fbc0  public: void __thiscall ADishonoredPlayerPawn::ResetTrackedMissionStats(void)
//   0x711300  public: virtual void __thiscall ADishonoredPlayerPawn::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x711630  public: virtual void __thiscall ADishonoredPlayerPawn::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x711940  public: void __thiscall ADishonoredPlayerPawn::RemoveBoneCharm(int)
//   0x711a50  public: void __thiscall ADishonoredPlayerPawn::ActivateBoneCharm(int, int)
//   0x711c60  public: void __thiscall ADishonoredPlayerPawn::OnLoseUpgrade(class UDisTweaks_Upgrade *)
//   0x711c90  public: void __thiscall ADishonoredPlayerPawn::IncrementStat(enum EDisPlayerStat, float, class UDisTweaksBase const *, class UDisAbstractItem const *, class UClass const *)
//   0x712060  public: void __thiscall ADishonoredPlayerPawn::SetStatValue(enum EDisPlayerStat, float)
//   0x712210  public: void __thiscall ADishonoredPlayerPawn::SetStatValueIfGreater(enum EDisPlayerStat, float)
//   0x712420  public: virtual void __thiscall UDisSeqAct_IncrementPlayerStat::Activated(void)
//   0x712460  public: virtual void __thiscall ADishonoredPlayerPawn::OnOverrideAwarenessDisplay(class UDisSeqAct_OverrideAwarenessDisplay *)
//   0x715a00  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::Tick_Begin(float, enum ELevelTick)
//   0x715b10  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::Tick(float, enum ELevelTick)
//   0x715fe0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::DoJump(unsigned int)
//   0x716060  public: virtual void __thiscall ADishonoredPlayerPawn::OnAddPower(class UDisSeqAct_AddPower *)
//   0x7160e0  public: virtual void __thiscall ADishonoredPlayerPawn::OnRemovePower(class UDisSeqAct_RemovePower *)
//   0x716160  public: virtual void __thiscall ADishonoredPlayerPawn::InhibitPowersFor(float)
//   0x7162f0  public: virtual void __thiscall ADishonoredPlayerPawn::OnMaxPowers(class UDisSeqAct_MaxPowers *)
//   0x716310  public: virtual void __thiscall ADishonoredPlayerPawn::OnMinPowers(class UDisSeqAct_MinPowers *)
//   0x716330  public: virtual void __thiscall ADishonoredPlayerPawn::OnSetPassivePowerLevel(int, int)
//   0x716480  private: void __thiscall ADishonoredPlayerPawn::SetupNPCSpawnCommand(void)
//   0x716840  public: struct FDisWhaleBoneCharmLevel const * __thiscall ADishonoredPlayerPawn::AddBoneCharm(int)
//   0x716a40  public: unsigned int __thiscall ADishonoredPlayerPawn::GiveRandomBoneCharm(void)
//   0x716ac0  public: void __thiscall ADishonoredPlayerPawn::OnDropAssassination(void)
//   0x716b50  public: void __thiscall ADishonoredPlayerPawn::OnReceiveUpgrade(class UDisTweaks_Upgrade *)
//   0x716b90  public: virtual void __thiscall ADishonoredPlayerPawn::OnOtherActorTerminated(class AActor const &)
//   0x717cf0  public: virtual void __thiscall ADishonoredPlayerPawn::PostBeginPlay(void)
//   0x7182b0  public: virtual void __thiscall ADishonoredPlayerPawn::OnApplyPlayerLoadout(class UDisSeqAct_ApplyPlayerLoadout *)
//   0x7182e0  public: virtual void __thiscall ADishonoredPlayerPawn::OnRemoveKey(class UDisSeqAct_RemoveKey *)
//   0x718310  public: virtual void __thiscall ADishonoredPlayerPawn::OnGiveUpgrade(class UDisSeqAct_GiveUpgrade *)
//   0x719ca0  public: virtual void __thiscall ADishonoredPlayerPawn::BeginDestroy(void)
//   0x71c0f0  public: static class UClass * __cdecl UDisSeqAct_GiveUpgrade::GetPrivateStaticClassUDisSeqAct_GiveUpgrade(wchar_t const *)
//   0x71c180  public: static class UClass * __cdecl UDisSeqAct_EvalAchievement::GetPrivateStaticClassUDisSeqAct_EvalAchievement(wchar_t const *)
//   0x71c210  public: static class UClass * __cdecl UDisSeqAct_GetPlayerStat::GetPrivateStaticClassUDisSeqAct_GetPlayerStat(wchar_t const *)
//   0x71c2a0  public: static class UClass * __cdecl UDisSeqAct_ToggleAchievementEval::GetPrivateStaticClassUDisSeqAct_ToggleAchievementEval(wchar_t const *)
//   0x71c330  public: static class UClass * __cdecl UDisSeqAct_IncrementPlayerStat::GetPrivateStaticClassUDisSeqAct_IncrementPlayerStat(wchar_t const *)
//   0x71c3c0  public: static class UClass * __cdecl UDisSeqAct_GivePickup::GetPrivateStaticClassUDisSeqAct_GivePickup(wchar_t const *)
//   0x71d1a0  public: static class UClass * __cdecl UDisSeqAct_GiveUpgrade::StaticClassNoInline(void)
//   0x71d1d0  private: virtual void __thiscall ADishonoredPlayerPawn::ApplyTweakChanges_Derived(void)
//   0x71d430  public: static class UClass * __cdecl UDisSeqAct_EvalAchievement::StaticClassNoInline(void)
//   0x71d460  public: static class UClass * __cdecl UDisSeqAct_GetPlayerStat::StaticClassNoInline(void)
//   0x71d490  public: static class UClass * __cdecl UDisSeqAct_ToggleAchievementEval::StaticClassNoInline(void)
//   0x71d4c0  public: static class UClass * __cdecl UDisSeqAct_IncrementPlayerStat::StaticClassNoInline(void)
//   0x71d4f0  public: static class UClass * __cdecl UDisSeqAct_GivePickup::StaticClassNoInline(void)
//   0x71e9d0  private: __thiscall ADishonoredPlayerPawn::ADishonoredPlayerPawn(void)
//   0x71f680  public: static class UClass * __cdecl ADishonoredPlayerPawn::GetPrivateStaticClassADishonoredPlayerPawn(wchar_t const *)
//   0x71fe10  public: static class UClass * __cdecl ADishonoredPlayerPawn::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "gfarmsdiag.h"

ADishonoredPlayerPawn* ADishonoredPlayerPawn::s_pInstance = NULL;

// DISHONORED(written): 2013 rva 0x6bf830 (2012 0x71e9d0). Retail constructs the 81 m_AchievementTrackers elements and the interface
// vtables here as well, which the compiler does for us.
ADishonoredPlayerPawn::ADishonoredPlayerPawn()
{
	for( UObject* Object = this; Object; Object = Object->GetOuter() )
	{
		if( Object->HasAnyFlags( RF_ClassDefaultObject | RF_ArchetypeObject ) )
		{
			return;
		}
	}
	s_pInstance = this;
}

// ---- natives whose retail body is trivial (generated by build/agentAC_work/gen_trivial.py from the 2013 vtables) ----

// DISHONORED(written): 2013 rva 0x5ef960; the retail ADishonoredPlayerPawn vtable slot +2028 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x128ad0) and no retail subclass overrides it
void ADishonoredPlayerPawn::execOnDestroy_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_NAME(_ActionName);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x1c1c40; the retail ADishonoredPlayerPawn vtable slot +364 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x6436a0) and no retail subclass overrides it
void ADishonoredPlayerPawn::execNativePostRenderFor( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(APlayerController, _pPC);
	P_GET_OBJECT(UCanvas, _pCanvas);
	P_GET_STRUCT(FVector, _CameraPosition);
	P_GET_STRUCT(FVector, _CameraDir);
	P_FINISH;
}

// DISHONORED(written): 2013 rva 0x5eeee0; the retail ADishonoredPlayerPawn vtable slot +1912 it dispatches to is `return FALSE`
// (2013 rva 0x2a3630) and no retail subclass overrides it
void ADishonoredPlayerPawn::execDisplayDebug_Cheats( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AHUD, _HUD);
	P_GET_FLOAT_REF(_fOut_YL);
	P_GET_FLOAT_REF(_fOut_YPos);
	P_FINISH;
	*(UBOOL*)Result = FALSE;
}

// DISHONORED(written): 2013 rva 0x5ec2c0; the retail ADishonoredPlayerPawn vtable slot +1312 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x6a1410) and no retail subclass overrides it
void ADishonoredPlayerPawn::execDisplayDebug_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AHUD, _pHUD);
	P_GET_FLOAT_REF(_rfOut_YL);
	P_GET_FLOAT_REF(_rfOut_YPos);
	P_FINISH;
}

// ---- end of trivial natives ----

// ---- agent BF ports (PHASE8 BF): the player's landing override ----

// DISHONORED(written): 2013 rva 0x6b53a0 (2012 0x70eff0), 1,601 bytes, and the shape of it is five things:
//  1. the big-fall branch: when the floor blocks a trace and -pFallState->m_fFall_Stun_Vel > Velocity.Z, request the
//     m_Action_JumpLandBig fullbody action and use UDisContactType_JumpLand_Big;
//  2. otherwise the small-land branch: pick m_Action_JumpLandSmall / _Ready / _Sneak / _LeftHand by the desired stance,
//     the equipped item and IsSneaking, push it onto the upper and left-arm FSMs and invalidate the anim state;
//  3. the contact: trace 1.5 cylinder heights under the feet and cross the chosen JumpLand contact type with the
//     surface, recording m_fTimeOfLastLandedContact;
//  4. the camera: UDishonoredCamera_PhysicalReact::OnImpact with the landing velocity unless the big-fall action already
//     handled it, then clear the second FOV target;
//  5. m_Debug_Player.m_LastJumpLandVel = Velocity.
// DISHONORED(port): 1 to 4 all stand on subsystems that do not exist. m_pPlayerMasterFSM / m_pPlayerUpperFSM /
// m_pPlayerLeftArmFSM are never created (ADishonoredPlayerController::PostBeginPlay does not call
// UDishonoredNativeStateMachine::InitFSM - agent AJ's note) and every UStatePlayerMaster* / UStateSharedActionBase* unit
// is still a comment-only skeleton; UDisTweaks_PlayerPawn_Actions' action structs, ADishonoredPawn::GetDishonoredCamera,
// UDishonoredCamera_PhysicalReact and the whole contact system are unported too. Step 5 is ported for real, and the FSM
// pointers are tested rather than assumed so this stays correct once those packages land.
void ADishonoredPlayerPawn::Landed_Native( FVector HitNormal, AActor* FloorActor )
{
	if( UDisAttributes::IsCensusEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): disattrib land %s: Velocity %.1f %.1f %.1f, floor %s; land action, contact and camera react unported"),
			*GetName(), Velocity.X, Velocity.Y, Velocity.Z, FloorActor ? *FloorActor->GetName() : TEXT("none") );
	}
	m_Debug_Player.m_LastJumpLandVel = Velocity;
}

// DISHONORED(written): the generated exec wrapper; ICF folded it onto ADishonoredPawn::execLanded_Native (2013 rva
// 0x5ec7e0) because the code is identical - it dispatches through the virtual.
void ADishonoredPlayerPawn::execLanded_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FVector, HitNormal);
	P_GET_ACTOR(FloorActor);
	P_FINISH;
	Landed_Native( HitNormal, FloorActor );
}

// ---- agent EL (PHASE12 EL): the story-flag store ----
//
// A story flag is a GUID declared by a UDisStoryFlagSet asset and a boolean recorded on the player pawn.
// m_StoryFlagInstances is CPF_Transient, so a fresh game starts with every flag false and the value only
// exists once something has written it. The Tower's arrival chain stands on exactly this: the menu's
// GoToTowerEmpress sequence sets a flag, and L_Tower_Script's DisSeqCond_CheckStoryFlag reads it.

// DISHONORED(port): agent EL, 2013 rva 0x6b1370 (2012 0x70b470). Retail calls FindStoryFlag_ByGUID and
// discards the result on this overload (only the setter acts on it); the call is kept because it is in the
// binary and it is free.
UBOOL ADishonoredPlayerPawn::CheckStoryFlag( const UDisStoryFlagSet* _pStoryFlagSet, const FGuid& _rGUID ) const
{
	_pStoryFlagSet->FindStoryFlag_ByGUID( _rGUID );
	return CheckStoryFlag( FName( *_pStoryFlagSet->GetPathName() ), _rGUID );
}

// DISHONORED(port): agent EL, 2013 rva 0x6b1430 (2012 0x70b4f0). A GUID the set does not declare is not written.
void ADishonoredPlayerPawn::SetStoryFlag( const UDisStoryFlagSet* _pStoryFlagSet, const FGuid& _rGUID, UBOOL _bValue )
{
	if( _pStoryFlagSet->FindStoryFlag_ByGUID( _rGUID ) )
	{
		SetStoryFlag( FName( *_pStoryFlagSet->GetPathName() ), _rGUID, _bValue );
	}
}

// DISHONORED(port): agent EL, 2013 rva 0x6a8c30 (2012 0x702f20)
UBOOL ADishonoredPlayerPawn::CheckStoryFlag( const FName& _rStoryFlagSetPath, const FGuid& _rGUID ) const
{
	for( INT InstanceIndex = 0; InstanceIndex < m_StoryFlagInstances.Num(); InstanceIndex++ )
	{
		if( m_StoryFlagInstances(InstanceIndex).CheckStoryFlagValue( _rStoryFlagSetPath, _rGUID ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): agent EL, 2013 rva 0x6a8cc0 (2012 0x702fa0)
void ADishonoredPlayerPawn::SetStoryFlag( const FName& _rStoryFlagSetPath, const FGuid& _rGUID, UBOOL _bValue )
{
	FDisStoryFlagInstance* pInstance = NULL;
	for( INT InstanceIndex = 0; InstanceIndex < m_StoryFlagInstances.Num(); InstanceIndex++ )
	{
		if( m_StoryFlagInstances(InstanceIndex).MatchesStoryFlagInstance( _rStoryFlagSetPath, _rGUID ) )
		{
			pInstance = &m_StoryFlagInstances(InstanceIndex);
			break;
		}
	}
	if( pInstance == NULL )
	{
		pInstance = &m_StoryFlagInstances( m_StoryFlagInstances.AddZeroed() );
		pInstance->BuildStoryFlagInstance( _rStoryFlagSetPath, _rGUID );
	}
	pInstance->m_bCurValue = _bValue ? 1 : 0;
}

// DISHONORED(port): agent EQ, 2013 rva 0x6a1a80 (2012 0x6fac40) - thirty-one bytes, one bit. Bit 16 of the
// bitfield at +2208 is m_bCameraRelativeClimbing, counting the seventeen flags the retail SDK declares before it.
void ADishonoredPlayerPawn::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	m_bCameraRelativeClimbing = Parameters->m_bCameraRelativeClimbing ? TRUE : FALSE;
}

// ---- agent FZ (PHASE18 FZ): the player master FSM's life, the body mode, the input mask forward ----

// DISHONORED(bringup): retail's ADishonoredPawn::PreBeginPlay (2012 0x78e220) calls the PreBeginPlay_* virtuals in the order
// NativeComponents, StartAllComponents, Inventory, Attributes, Actions. Agent GA's base (ad4a35d, dishonoredpawn.cpp) does
// NativeComponents and StartAllComponents and names the rest as unported; this override is the stand-in for the one of them
// the player master FSM needs, chained after the base so retail's order holds.
void ADishonoredPlayerPawn::PreBeginPlay()
{
	Super::PreBeginPlay();
	PreBeginPlay_Actions();
}

// DISHONORED(port): agent GF (PHASE19 GF), 2012 0x715b10 / 2013 0x6bb1e0 (1229 / 1257 B): ADishonoredPawn::Tick (whose first phase is
// now Tick_Begin, below), then m_fLastDeltaSeconds. The rest of retail's body is not ported and named: the breathlessness ramp and its
// two Ak events, the ranged weapon's dispersion, the breathlessness RTPC, CheckCrouchEvents (0x7079f0), the distance-travelled stat
// once a second, HandleAvoidance (0x70d9f0), RegenMana (0x7094b0), the four tutorial trackers, the drowning damage and the dead-state
// request, the post-process manager's tick, the tattoo glow ramp on the mesh's first material.
UBOOL ADishonoredPlayerPawn::Tick( FLOAT DeltaTime, ELevelTick TickType )
{
	const UBOOL bResult = Super::Tick( DeltaTime, TickType );
	m_fLastDeltaSeconds = DeltaTime;
	DisGFReportPlayer( this );
	return bResult;
}

// DISHONORED(port): agent GF, 2012 0x715a00 / 2013 0x6b0850 (269 B both). The base phase (a return), Tick_Mantle (0x7129d0 / 0x6dbd00:
// the mantle unit is a skeleton - named, not called), the mesh's transform brought up to the pawn's before anything reads a bone, the
// camera bone written into the audio-log dummy actor (the speaker position of the player's audio logs), Tick_PlayerAction through
// the vtable (the three FSMs), then the deferred corpse drop (two transient bits at +2016: UStatePlayerCarryCorpseIdle::
// StartDroppingCorpse - the carry states are skeletons, the bits are left alone).
UBOOL ADishonoredPlayerPawn::Tick_Begin( FLOAT _fDeltaTime, ELevelTick _TickType )
{
	Super::Tick_Begin( _fDeltaTime, _TickType );
	GFDIAG_ONCE( TEXT("Tick_Begin: Tick_Mantle (2012 0x7129d0) and the deferred corpse drop are not ported") );
	if( Mesh && Mesh->GetOwner() )
	{
		Mesh->ConditionalUpdateTransform( Mesh->GetOwner()->LocalToWorld() );
		if( m_pAudioLogConvActor )
		{
			GetCameraBone( &m_pAudioLogConvActor->Location, &m_pAudioLogConvActor->Rotation );
		}
	}
	Tick_PlayerAction( _fDeltaTime, _TickType );
	return TRUE;
}

// DISHONORED(port): 2012 0x701f10 / 0x701fc0 (unnamed in 2013; the 2013 SetBodyMode inlines both and adds particle systems)
static void DisShowCompRecursiveHelper( UPrimitiveComponent* Comp, UBOOL bShow )
{
	if( !Comp )
	{
		return;
	}
	Comp->SetHiddenGame( !bShow );
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>( Comp );
	if( SkelComp )
	{
		for( INT Index = 0; Index < SkelComp->Attachments.Num(); Index++ )
		{
			UActorComponent* Attached = SkelComp->Attachments(Index).Component;
			UPrimitiveComponent* Prim = ( Attached && Attached->IsAttached() ) ? Cast<UPrimitiveComponent>( Attached ) : NULL;
			DisShowCompRecursiveHelper( Prim, bShow );
		}
	}
}

static void DisSetCompDepthGroupRecursiveHelper( UPrimitiveComponent* Comp, ESceneDepthPriorityGroup DepthGroup )
{
	if( !Comp )
	{
		return;
	}
	Comp->SetDepthPriorityGroup( DepthGroup );
	USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>( Comp );
	if( SkelComp )
	{
		for( INT Index = 0; Index < SkelComp->Attachments.Num(); Index++ )
		{
			UActorComponent* Attached = SkelComp->Attachments(Index).Component;
			UPrimitiveComponent* Prim = ( Attached && Attached->IsAttached() ) ? Cast<UPrimitiveComponent>( Attached ) : NULL;
			DisSetCompDepthGroupRecursiveHelper( Prim, DepthGroup );
		}
	}
}

/**
 * DISHONORED(port): agent HD (PHASE22 HD), 2012 0x70afd0 / 2013 0x6b0d40 (348 B both). While the camera's PlayerControl influence has no
 * weight (a soiree's AnimDriven camera holds the view) or the mesh rotates the actor by root motion, the pawn keeps its own pitch and
 * yaw - so a pawn the matinee detaches keeps the heading the boat gave it instead of snapping to the controller's pre-ride yaw.
 * Otherwise the pitch is dropped when the master FSM's logical state (a base or an action state) ignores it, unless the controller
 * flies by cheat; walking and falling drop it too; the roll is always 0.
 * DISHONORED(written): retail dereferences the camera and its first influence group unchecked; without them the pawn faces as asked.
 */
void ADishonoredPlayerPawn::FaceRotation( FRotator NewRotation, FLOAT DeltaTime )
{
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	UDishonoredCameraInfluence* PlayerControl = ( Camera && Camera->InfluenceGroups().Num() > 0 && Camera->InfluenceGroups()(0) )
		? Camera->InfluenceGroups()(0)->GetInfluence( UDishonoredCamera_PlayerControl::StaticClass() ) : NULL;
	if( ( PlayerControl && PlayerControl->m_Weight <= 1e-8f ) || ( Mesh && Mesh->RootMotionRotationMode == RMRM_RotateActor ) )
	{
		NewRotation.Pitch = Rotation.Pitch;
		NewRotation.Yaw = Rotation.Yaw;
	}
	else
	{
		UDishonoredNativeState* LogicalState = m_pPlayerMasterFSM ? m_pPlayerMasterFSM->GetLogicalState() : NULL;
		UStatePlayerMasterBase* BaseState = Cast<UStatePlayerMasterBase>( LogicalState );
		UStatePlayerMasterAction* ActionState = Cast<UStatePlayerMasterAction>( LogicalState );
		ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
		const UBOOL bCheatOverride = PC && PC->bCheatFlying;
		if( ( ( BaseState && BaseState->ShouldIgnorePitch() ) || ( ActionState && ActionState->m_bIgnorePitch ) ) && !bCheatOverride )
		{
			NewRotation.Pitch = 0;
		}
	}
	if( Physics == PHYS_Walking || Physics == PHYS_Falling )
	{
		NewRotation.Pitch = 0;
	}
	SetRotation( FRotator( NewRotation.Pitch, NewRotation.Yaw, 0 ) );
}

void ADishonoredPlayerPawn::execFaceRotation( FFrame& Stack, RESULT_DECL )
{
	P_GET_ROTATOR(NewRotation);
	P_GET_FLOAT(DeltaTime);
	P_FINISH;
	FaceRotation( NewRotation, DeltaTime );
}

/**
 * DISHONORED(port): 2012 0x702070 (2013 0x6a7c20, which also gives particle-system attachments the depth group). Only when
 * the mode changes and the pawn has a Dishonored camera: BODY_MODE_FULL_BODY shows the mesh in the world depth group at
 * forced LOD 2 (the full body); BODY_MODE_HIDDEN hides it; BODY_MODE_ARMS_ONLY shows it in the foreground depth group at
 * forced LOD 1 (the arms-only LOD). Every skeletal attachment follows the mesh's depth group.
 * This is retail's whole mechanism for hiding the first-person arms: there is no separate arms component.
 */
void ADishonoredPlayerPawn::SetBodyMode( BYTE BodyMode )
{
	if( m_BodyMode == BodyMode || !GetDishonoredCamera() || !Mesh )
	{
		return;
	}
	if( BodyMode == BODY_MODE_FULL_BODY )
	{
		DisShowCompRecursiveHelper( Mesh, TRUE );
		Mesh->SetDepthPriorityGroup( SDPG_World );
		Mesh->ForcedLodModel = 2;
	}
	else
	{
		if( BodyMode == BODY_MODE_HIDDEN )
		{
			DisShowCompRecursiveHelper( Mesh, FALSE );
		}
		else
		{
			DisShowCompRecursiveHelper( Mesh, TRUE );
			Mesh->SetDepthPriorityGroup( SDPG_Foreground );
		}
		Mesh->ForcedLodModel = 1;
	}
	for( INT Index = 0; Index < Mesh->Attachments.Num(); Index++ )
	{
		UActorComponent* Attached = Mesh->Attachments(Index).Component;
		USkeletalMeshComponent* SkelAttached = Cast<USkeletalMeshComponent>( Attached );
		if( SkelAttached )
		{
			DisSetCompDepthGroupRecursiveHelper( SkelAttached, (ESceneDepthPriorityGroup)Mesh->DepthPriorityGroup );
		}
		else
		{
			UParticleSystemComponent* Particles = Cast<UParticleSystemComponent>( Attached );
			if( Particles )
			{
				DisSetCompDepthGroupRecursiveHelper( Particles, (ESceneDepthPriorityGroup)Mesh->DepthPriorityGroup );
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): fz body mode %d -> %d on %s (mesh %s DPG %d LOD %d hidden %d)"), (INT)m_BodyMode, (INT)BodyMode, *GetName(),
		*Mesh->GetName(), (INT)Mesh->DepthPriorityGroup, Mesh->ForcedLodModel, (INT)Mesh->HiddenGame );
	m_BodyMode = BodyMode;
}

// DISHONORED(port): 2012 0x702550: the controller's mask level
void ADishonoredPlayerPawn::EnableControllerInput( INT Mask, UBOOL bEnable, DisInputMaskLevel Level )
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	if( PC )
	{
		PC->EnableControllerInput( Mask, bEnable, Level );
	}
}

// ---- agent HL (PHASE22 HL): the player Kismet toggles. Input links 0 / 1 / 2 are Enable / Disable / Toggle in every one. ----

static FString DisKismetMaskDiag( const ADishonoredPlayerPawn* _pPawn )
{
	const ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( _pPawn->Controller );
	return PC ? FString::Printf( TEXT("kismet mask 0x%05x"), PC->m_InputEnableMask[DisInputMaskLevel_Kismet] ) : FString( TEXT("no controller") );
}

// DISHONORED(port): 2012 0x704680 / 2013 0x6aa300 (574 B both). Retail reads the trackers of s_pInstance, not of this.
void ADishonoredPlayerPawn::OnToggleTutorial( UDisSeqAct_ToggleTutorial* _pAction )
{
	ADishonoredPlayerPawn* Player = s_pInstance;
	const INT Op = _pAction->InputLinks(0).bHasImpulse ? 0 : _pAction->InputLinks(1).bHasImpulse ? 1 : _pAction->InputLinks(2).bHasImpulse ? 2 : -1;
	for( INT TutorialIdx = 0; Op >= 0 && TutorialIdx < _pAction->m_AffectedTutorials.Num(); ++TutorialIdx )
	{
		UDisTutorialTracker* Tracker = Player->m_TutorialTrackers[_pAction->m_AffectedTutorials(TutorialIdx)];
		if( Tracker == NULL )
		{
			debugf( TEXT("DISHONORED(bringup): OnToggleTutorial %s: tracker %d is NULL (retail dereferences it)"), *_pAction->GetName(), (INT)_pAction->m_AffectedTutorials(TutorialIdx) );
			continue;
		}
		Tracker->m_bEnabled = Op == 0 ? TRUE : Op == 1 ? FALSE : !Tracker->m_bEnabled;
		if( UDishonoredObjectivesComponent::IsDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag OnToggleTutorial %s link %d: tracker %d %s enabled %d"), *_pAction->GetName(), Op,
				(INT)_pAction->m_AffectedTutorials(TutorialIdx), *Tracker->GetName(), (INT)Tracker->m_bEnabled );
		}
	}
}

// DISHONORED(port): 2012 0x704080 (559 B) / 2013 0x6a9df0 (561 B). Turning evaluation on resets every stat the achievement reads.
void ADishonoredPlayerPawn::OnToggleAchievementEval( UDisSeqAct_ToggleAchievementEval* _pAction )
{
	FAchievementTracker& Tracker = m_AchievementTrackers[_pAction->m_Achievement];
	const UBOOL bWasEnabled = Tracker.m_bEnabled;
	if( _pAction->InputLinks(0).bHasImpulse )
	{
		Tracker.m_bEnabled = TRUE;
	}
	else if( _pAction->InputLinks(1).bHasImpulse )
	{
		Tracker.m_bEnabled = FALSE;
	}
	else if( _pAction->InputLinks(2).bHasImpulse )
	{
		Tracker.m_bEnabled = !Tracker.m_bEnabled;
	}
	INT ResetCount = 0;
	if( Tracker.m_bEnabled && !bWasEnabled )
	{
		UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( GetTweaks_Derived() );
		if( !Tweaks )
		{
			Tweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
		}
		if( Tweaks->m_pPlayerStats )
		{
			const FAchievementInfo& Info = Tweaks->m_pPlayerStats->m_Achievements[_pAction->m_Achievement];
			for( INT EvalIdx = 0; EvalIdx < Info.m_StatEvals.Num(); ++EvalIdx )
			{
				const INT StatIndex = Info.m_StatEvals(EvalIdx).m_StatIndex;
				if( StatIndex < m_StatValues.Num() )
				{
					m_StatValues(StatIndex) = 0.f;
					++ResetCount;
				}
			}
		}
	}
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag OnToggleAchievementEval %s achievement %d enabled %d -> %d, %d stats reset"), *_pAction->GetName(),
			(INT)_pAction->m_Achievement, (INT)bWasEnabled, (INT)Tracker.m_bEnabled, ResetCount );
	}
}

// DISHONORED(port): 2012 0x704480 (336 B) / 2013 0x6b1bd0 (300 B, EnableControllerInput inlined): input 0x20000 at the Kismet level.
void ADishonoredPlayerPawn::OnToggleChoke( UDisSeqAct_ToggleChoke* _pAction )
{
	if( _pAction->InputLinks(0).bHasImpulse )
	{
		EnableControllerInput( 0x20000, TRUE, DisInputMaskLevel_Kismet );
	}
	else if( _pAction->InputLinks(1).bHasImpulse )
	{
		EnableControllerInput( 0x20000, FALSE, DisInputMaskLevel_Kismet );
	}
	else if( _pAction->InputLinks(2).bHasImpulse )
	{
		ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
		EnableControllerInput( 0x20000, !( PC && PC->IsInputEnabled( 0x20000 ) ), DisInputMaskLevel_Kismet );
	}
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag OnToggleChoke %s: %s"), *_pAction->GetName(), *DisKismetMaskDiag( this ) );
	}
}

// DISHONORED(port): 2012 0x70b200 (204 B) / 2013 0x6b5c40 (220 B): input 0x10000 at the Kismet level; Disable and no impulse both disable.
void ADishonoredPlayerPawn::OnToggleJournal( UDisSeqAct_ToggleJournal* _pAction )
{
	UBOOL bEnable = FALSE;
	if( _pAction->InputLinks(0).bHasImpulse )
	{
		bEnable = TRUE;
	}
	else if( _pAction->InputLinks(2).bHasImpulse )
	{
		ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
		bEnable = !( PC && PC->IsInputEnabled( 0x10000 ) );
	}
	EnableControllerInput( 0x10000, bEnable, DisInputMaskLevel_Kismet );
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag OnToggleJournal %s: %s"), *_pAction->GetName(), *DisKismetMaskDiag( this ) );
	}
}

// DISHONORED(port): 2012 0x70b130 (204 B) / 2013 0x6b5b60 (220 B): OnToggleJournal with input 0x800.
void ADishonoredPlayerPawn::OnTogglePowerWheel( UDisSeqAct_TogglePowerWheel* _pAction )
{
	UBOOL bEnable = FALSE;
	if( _pAction->InputLinks(0).bHasImpulse )
	{
		bEnable = TRUE;
	}
	else if( _pAction->InputLinks(2).bHasImpulse )
	{
		ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
		bEnable = !( PC && PC->IsInputEnabled( 0x800 ) );
	}
	EnableControllerInput( 0x800, bEnable, DisInputMaskLevel_Kismet );
	if( UDishonoredObjectivesComponent::IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): hidiag OnTogglePowerWheel %s: %s"), *_pAction->GetName(), *DisKismetMaskDiag( this ) );
	}
}

// DISHONORED(port): the five execs, 96 B each: 2012 0x635de0 / 0x6359c0 / 0x635d80 / 0x635a80 / 0x635a20, 2013 0x5ef840 / 0x5ef420 /
// 0x5ef7e0 / 0x5ef4e0 / 0x5ef480 (vtable +2016 / +1972 / +2012 / +1980 / +1976)
void ADishonoredPlayerPawn::execOnToggleTutorial( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDisSeqAct_ToggleTutorial, _pAction );
	P_FINISH;
	OnToggleTutorial( _pAction );
}

void ADishonoredPlayerPawn::execOnToggleAchievementEval( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDisSeqAct_ToggleAchievementEval, _pAction );
	P_FINISH;
	OnToggleAchievementEval( _pAction );
}

void ADishonoredPlayerPawn::execOnToggleChoke( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDisSeqAct_ToggleChoke, _pAction );
	P_FINISH;
	OnToggleChoke( _pAction );
}

void ADishonoredPlayerPawn::execOnToggleJournal( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDisSeqAct_ToggleJournal, _pAction );
	P_FINISH;
	OnToggleJournal( _pAction );
}

void ADishonoredPlayerPawn::execOnTogglePowerWheel( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( UDisSeqAct_TogglePowerWheel, _pAction );
	P_FINISH;
	OnTogglePowerWheel( _pAction );
}
