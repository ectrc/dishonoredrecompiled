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
