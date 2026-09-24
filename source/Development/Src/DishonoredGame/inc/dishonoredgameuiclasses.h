#pragma once
// DishonoredGame/inc/dishonoredgameuiclasses.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (223):
//   0x63d010  public: void __thiscall UDisGFxMoviePlayerBase::execCaptureAnalogInput(struct FFrame &, void * const)
//   0x63d080  public: void __thiscall UDisGFxMoviePlayerBase::execHasFinishedAsyncLoading(struct FFrame &, void * const)
//   0x63d0c0  public: void __thiscall UDisGFxMoviePlayerBase::execWidgetInitialized_Native(struct FFrame &, void * const)
//   0x63d1e0  public: void __thiscall UDisGFxMoviePlayerBase::execAddMessageBoxTimer(struct FFrame &, void * const)
//   0x63d240  public: void __thiscall UDisGFxMoviePlayerBase::execHideMessageBox(struct FFrame &, void * const)
//   0x63d270  public: void __thiscall UDisGFxMoviePlayerGlobal::execOnLoginChange(struct FFrame &, void * const)
//   0x63d2d0  public: void __thiscall UDisGFxMoviePlayerGlobal::execOnWriteProfileSettingsComplete(struct FFrame &, void * const)
//   0x63d360  public: void __thiscall UDisGFxMoviePlayerGlobal::execOnControllerDisconnected(struct FFrame &, void * const)
//   0x63d3c0  public: void __thiscall UDisGFxMoviePlayerJournal::execReq_FlashingTabsBitfield(struct FFrame &, void * const)
//   0x63d440  public: void __thiscall UDisGFxMoviePlayerJournal::execReq_MissionItemInfo(struct FFrame &, void * const)
//   0x63d4a0  public: void __thiscall UDisGFxMoviePlayerJournal::execReq_Inventory_Gadgets(struct FFrame &, void * const)
//   0x63d4e0  public: void __thiscall UDisGFxMoviePlayerJournal::execReq_InventoryItemInfo(struct FFrame &, void * const)
//   0x63d540  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOpenMissionStats(struct FFrame &, void * const)
//   0x63d5a0  public: void __thiscall UDisGFxMoviePlayerJournal::execStopAudioLog(struct FFrame &, void * const)
//   0x63d600  public: void __thiscall UDisGFxMoviePlayerJournal::execEquipBoneCharm(struct FFrame &, void * const)
//   0x63d660  public: void __thiscall UDisGFxMoviePlayerJournal::execRemoveBoneCharm(struct FFrame &, void * const)
//   0x63d6c0  public: void __thiscall UDisGFxMoviePlayerJournal::execCloseJournal(struct FFrame &, void * const)
//   0x63d700  public: void __thiscall UDisGFxMoviePlayerMenuBase::execOnDeleteSaveConfirm(struct FFrame &, void * const)
//   0x63d7a0  public: void __thiscall UDisGFxMoviePlayerMenuBase::execOpenGammaImage(struct FFrame &, void * const)
//   0x63d7e0  public: void __thiscall UDisGFxMoviePlayerMenuBase::execOnSettingChange(struct FFrame &, void * const)
//   0x63d870  public: void __thiscall UDisGFxMoviePlayerMenuBase::execOnResetOptions(struct FFrame &, void * const)
//   0x63d900  public: void __thiscall UDisGFxMoviePlayerJournal::execReq_Inventory_All(struct FFrame &, void * const)
//   0x63d940  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOnLoginChange(struct FFrame &, void * const)
//   0x63d9a0  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOnLoginCancelled(struct FFrame &, void * const)
//   0x63da90  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOnNewGameConfirm(struct FFrame &, void * const)
//   0x63db70  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOnMissionSelected(struct FFrame &, void * const)
//   0x63dbd0  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOnLoadGameClicked(struct FFrame &, void * const)
//   0x63dc10  public: void __thiscall UDisGFxMoviePlayerMainMenu::execReq_SaveSlotInfos(struct FFrame &, void * const)
//   0x63dc70  public: void __thiscall UDisGFxMoviePlayerMainMenu::execOnDLCClicked(struct FFrame &, void * const)
//   0x63dd30  public: void __thiscall UDisGFxMoviePlayerPauseMenu::execOnSaveGameConfirm(struct FFrame &, void * const)
//   0x63dd90  public: void __thiscall UDisGFxMoviePlayerPauseMenu::execOnLoadLastSaveClicked(struct FFrame &, void * const)
//   0x63de10  public: void __thiscall UDisGFxMoviePlayerPauseMenu::execOnTutorialReadRequest(struct FFrame &, void * const)
//   0x63de70  public: void __thiscall UDisGFxMoviePlayerPauseMenu::execBackToWindows(struct FFrame &, void * const)
//   0x63deb0  public: void __thiscall UDisGFxMoviePlayerPauseMenu::execOnMenuClosed(struct FFrame &, void * const)
//   0x63e0b0  public: void __thiscall ADishonoredHUD::execRenderNonHUD_Native(struct FFrame &, void * const)
//   0x63e120  public: void __thiscall ADishonoredHUD::execPlayerDisplayDebug_Native(struct FFrame &, void * const)
//   0x63e210  public: void __thiscall ADishonoredHUD::execShowDebugInfo_Native(struct FFrame &, void * const)
//   0x63e300  public: void __thiscall ADishonoredHUD::execDebugEnable(struct FFrame &, void * const)
//   0x63e380  public: void __thiscall ADishonoredHUD::execDebugDisable(struct FFrame &, void * const)
//   0x63e440  public: void __thiscall ADishonoredHUD::execDebugOnly(struct FFrame &, void * const)
//   0x647be0  public: void __thiscall UDisGFxMoviePlayerBase::execFormatText(struct FFrame &, void * const)
//   0x647c80  public: void __thiscall UDisGFxMoviePlayerBase::execShowMessageBox(struct FFrame &, void * const)
//   0x647dd0  public: void __thiscall UDisGFxMoviePlayerBase::execReq_EquipmentIconImage(struct FFrame &, void * const)
//   0x647e80  public: void __thiscall UDisGFxMoviePlayerJournal::execReq_PowerDetails(struct FFrame &, void * const)
//   0x647f30  public: void __thiscall UDisGFxMoviePlayerJournal::execOnPowerInfosRequest(struct FFrame &, void * const)
//   0x647fe0  public: void __thiscall UDisGFxMoviePlayerJournal::execPowerBuy(struct FFrame &, void * const)
//   0x6535b0  protected: virtual __thiscall ADishonoredHUD::~ADishonoredHUD(void)
//   0x654580  public: static void __cdecl ADishonoredHUD::InternalConstructor(void *)
//   0x655930  protected: virtual __thiscall UDisSeqAct_ShowMissionStats::~UDisSeqAct_ShowMissionStats(void)
//   0x657130  public: static void __cdecl UDisSeqAct_ShowMissionStats::InternalConstructor(void *)
//   0x7e0b30  protected: virtual __thiscall UDisTweaks_Store::~UDisTweaks_Store(void)
//   0x7e4f20  protected: virtual __thiscall UDisSeqAct_GameOver::~UDisSeqAct_GameOver(void)
//   0x7e4fc0  public: static void __cdecl UDisTweaks_Store::InternalConstructor(void *)
//   0x7e6640  public: static void __cdecl UDisSeqAct_GameOver::InternalConstructor(void *)
//   0x7f3b20  public: void __thiscall UDisGFxMoviePlayerBase::eventSetPauseDuringAsyncLoading(void)
//   0x8016a0  public: __thiscall FDisMsgBoxInfo::FDisMsgBoxInfo(class FString const &, class FString const &)
//   0x801750  public: __thiscall FDisMsgBoxInfo::FDisMsgBoxInfo(class FString const &, class FString const &, class FString const &, class FString const &)
//   0x8167d0  protected: __thiscall UDisTweaks_GFxMoviePlayerHUD::UDisTweaks_GFxMoviePlayerHUD(void)
//   0x8168d0  protected: virtual __thiscall UDisTweaks_GFxMoviePlayerHUD::~UDisTweaks_GFxMoviePlayerHUD(void)
//   0x8179b0  public: static void __cdecl UDisTweaks_GFxMoviePlayerHUD::InternalConstructor(void *)
//   0x8179d0  protected: virtual __thiscall UDisUISoundTheme::~UDisUISoundTheme(void)
//   0x818710  protected: virtual __thiscall UDisSeqAct_EnableSystemicTutorials::~UDisSeqAct_EnableSystemicTutorials(void)
//   0x8187a0  protected: virtual __thiscall UDisSeqAct_OpenJournal::~UDisSeqAct_OpenJournal(void)
//   0x818830  protected: virtual __thiscall UDisSeqAct_SetTutorialMessage::~UDisSeqAct_SetTutorialMessage(void)
//   0x8188e0  protected: virtual __thiscall UDisSeqAct_ShowHUDMessage::~UDisSeqAct_ShowHUDMessage(void)
//   0x818980  protected: virtual __thiscall UDisSeqAct_ShowLocationDiscovery::~UDisSeqAct_ShowLocationDiscovery(void)
//   0x818a20  protected: virtual __thiscall UDisSeqAct_ShowTargetNotification::~UDisSeqAct_ShowTargetNotification(void)
//   0x818ab0  public: static void __cdecl UDisUISoundTheme::InternalConstructor(void *)
//   0x81b080  public: static void __cdecl UDisSeqAct_EnableSystemicTutorials::InternalConstructor(void *)
//   0x81b0a0  public: static void __cdecl UDisSeqAct_OpenJournal::InternalConstructor(void *)
//   0x81b0c0  public: static void __cdecl UDisSeqAct_SetTutorialMessage::InternalConstructor(void *)
//   0x81b0e0  public: static void __cdecl UDisSeqAct_ShowHUDMessage::InternalConstructor(void *)
//   0x81b100  public: static void __cdecl UDisSeqAct_ShowLocationDiscovery::InternalConstructor(void *)
//   0x81b120  public: static void __cdecl UDisSeqAct_ShowTargetNotification::InternalConstructor(void *)
//   0x81c430  protected: virtual __thiscall UDisGFxMoviePlayerBase::~UDisGFxMoviePlayerBase(void)
//   0x81c4b0  public: virtual class UObject * __thiscall UDisGFxMoviePlayerBase::GetUObjectInterfaceDisTweaksInterface(void)
//   0x81c510  protected: virtual __thiscall UDisGFxMoviePlayerGamma::~UDisGFxMoviePlayerGamma(void)
//   0x81c5d0  protected: virtual __thiscall UDisGFxMoviePlayerGlobal::~UDisGFxMoviePlayerGlobal(void)
//   0x81c8b0  protected: virtual __thiscall UDisGFxMoviePlayerJournal::~UDisGFxMoviePlayerJournal(void)
//   0x81cb20  public: virtual class UObject * __thiscall UDisGFxMoviePlayerHUD::GetUObjectInterfaceArkSettingsListenerInterface(void)
//   0x81cb40  protected: virtual __thiscall UDisGFxMoviePlayerHUD::~UDisGFxMoviePlayerHUD(void)
//   0x81ccb0  protected: virtual __thiscall UDisGFxMoviePlayerHUDFX::~UDisGFxMoviePlayerHUDFX(void)
//   0x81cd50  protected: virtual __thiscall UDisGFxMoviePlayerMenuBase::~UDisGFxMoviePlayerMenuBase(void)
//   0x81ce30  protected: virtual __thiscall UDisGFxMoviePlayerMainMenu::~UDisGFxMoviePlayerMainMenu(void)
//   0x81cef0  protected: virtual __thiscall UDisGFxMoviePlayerMissionStats::~UDisGFxMoviePlayerMissionStats(void)
//   0x81df10  public: static void __cdecl UDisGFxMoviePlayerBase::InternalConstructor(void *)
//   0x81df40  public: static void __cdecl UDisGFxMoviePlayerGamma::InternalConstructor(void *)
//   0x81df70  public: static void __cdecl UDisGFxMoviePlayerGlobal::InternalConstructor(void *)
//   0x81dfa0  public: static void __cdecl UDisGFxMoviePlayerJournal::InternalConstructor(void *)
//   0x81dfc0  public: static void __cdecl UDisGFxMoviePlayerHUD::InternalConstructor(void *)
//   0x81dfe0  public: static void __cdecl UDisGFxMoviePlayerHUDFX::InternalConstructor(void *)
//   0x81e010  public: static void __cdecl UDisGFxMoviePlayerMenuBase::InternalConstructor(void *)
//   0x81e040  public: static void __cdecl UDisGFxMoviePlayerMainMenu::InternalConstructor(void *)
//   0x81e070  public: static void __cdecl UDisGFxMoviePlayerMissionStats::InternalConstructor(void *)
//   0x83f830  protected: virtual __thiscall UDisTweaks_GFxMoviePlayerGlobal::~UDisTweaks_GFxMoviePlayerGlobal(void)
//   0x83f8c0  protected: virtual __thiscall UDisTweaks_GFxMoviePlayerPowerWheel::~UDisTweaks_GFxMoviePlayerPowerWheel(void)
//   0x840240  public: static void __cdecl UDisTweaks_GFxMoviePlayerGlobal::InternalConstructor(void *)
//   0x840260  public: static void __cdecl UDisTweaks_GFxMoviePlayerPowerWheel::InternalConstructor(void *)
//   0x841520  protected: virtual __thiscall UDisGFxMoviePlayerNote::~UDisGFxMoviePlayerNote(void)
//   0x841600  protected: virtual __thiscall UDisGFxMoviePlayerPowerWheel::~UDisGFxMoviePlayerPowerWheel(void)
//   0x8416b0  protected: virtual __thiscall UDisGFxMoviePlayerStore::~UDisGFxMoviePlayerStore(void)
//   0x844210  public: static void __cdecl UDisGFxMoviePlayerNote::InternalConstructor(void *)
//   0x844240  protected: virtual __thiscall UDisGFxMoviePlayerPauseMenu::~UDisGFxMoviePlayerPauseMenu(void)
//   0x844300  public: static void __cdecl UDisGFxMoviePlayerPowerWheel::InternalConstructor(void *)
//   0x844330  public: static void __cdecl UDisGFxMoviePlayerStore::InternalConstructor(void *)
//   0x846010  public: static void __cdecl UDisGFxMoviePlayerPauseMenu::InternalConstructor(void *)
//   0x8cb180  protected: virtual __thiscall UDisGlobalUIManager::~UDisGlobalUIManager(void)
//   0x8cc8b0  public: static void __cdecl UDisGlobalUIManager::InternalConstructor(void *)
//   0xba3ef0  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecReq_EquipmentIconImageTemp__
//   0xba3f10  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecHideMessageBoxTemp__
//   0xba3f30  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecAddMessageBoxTimerTemp__
//   0xba3f50  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecShowMessageBoxTemp__
//   0xba3f70  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecFormatTextTemp__
//   0xba3f90  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecOnFocusLostTemp__
//   0xba3fb0  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecOnFocusGainedTemp__
//   0xba3fd0  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecWidgetInitialized_NativeTemp__
//   0xba3ff0  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecHasFinishedAsyncLoadingTemp__
//   0xba4010  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecCaptureAnalogInputTemp__
//   0xba4030  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecAdvanceTemp__
//   0xba4050  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecCloseTemp__
//   0xba4070  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecStartTemp__
//   0xba4090  _dynamic_initializer_for__UDisGFxMoviePlayerBaseexecPreLoadTemp__
//   0xba40b0  _dynamic_initializer_for__UDisGFxMoviePlayerGammaexecOnGammaImageClosedTemp__
//   0xba40d0  _dynamic_initializer_for__UDisGFxMoviePlayerGlobalexecOnMessageBoxConfirmTemp__
//   0xba40f0  _dynamic_initializer_for__UDisGFxMoviePlayerGlobalexecOnControllerDisconnectedTemp__
//   0xba4110  _dynamic_initializer_for__UDisGFxMoviePlayerGlobalexecOnWriteProfileSettingsCompleteTemp__
//   0xba4130  _dynamic_initializer_for__UDisGFxMoviePlayerGlobalexecOnLoginChangeTemp__
//   0xba4150  _dynamic_initializer_for__UDisGFxMoviePlayerHUDexecOnPlayerChoiceConfirmTemp__
//   0xba4170  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecCloseJournalTemp__
//   0xba4190  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecRemoveBoneCharmTemp__
//   0xba41b0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecEquipBoneCharmTemp__
//   0xba41d0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_CharmsListDetailsTemp__
//   0xba41f0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_CharmsTabContentTemp__
//   0xba4210  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReadNoteTemp__
//   0xba4230  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecStopAudioLogTemp__
//   0xba4250  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecPlayAudioLogTemp__
//   0xba4270  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_LogInfoTemp__
//   0xba4290  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Logs_NotesTemp__
//   0xba42b0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Logs_AudioTemp__
//   0xba42d0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Logs_AllTemp__
//   0xba42f0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_InventoryItemInfoTemp__
//   0xba4310  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Inventory_UpgradesTemp__
//   0xba4330  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Inventory_AmmoTemp__
//   0xba4350  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Inventory_GadgetsTemp__
//   0xba4370  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Inventory_ResourcesTemp__
//   0xba4390  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Inventory_Key_RingTemp__
//   0xba43b0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_Inventory_AllTemp__
//   0xba43d0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecPowerBuyTemp__
//   0xba43f0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecOnPowerInfosRequestTemp__
//   0xba4410  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_PowerDetailsTemp__
//   0xba4430  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_PowersTabContentTemp__
//   0xba4450  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_MissionItemInfoTemp__
//   0xba4470  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_MissionItemsTemp__
//   0xba4490  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_ChapterNotesTemp__
//   0xba44b0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecOnToggleHUDMarkerTemp__
//   0xba44d0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecOnToggleObjectivesListHistoryTemp__
//   0xba44f0  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_ObjectivesListTemp__
//   0xba4510  _dynamic_initializer_for__UDisGFxMoviePlayerJournalexecReq_FlashingTabsBitfieldTemp__
//   0xba4530  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnLeaveOptionsTemp__
//   0xba4550  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnResetOptionsTemp__
//   0xba4570  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnSettingChangeTemp__
//   0xba4590  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecDisplayStorageDeviceSelectionTemp__
//   0xba45b0  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecCloseGammaImageTemp__
//   0xba45d0  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOpenGammaImageTemp__
//   0xba45f0  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnApplyVideoSettingsTemp__
//   0xba4610  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecReq_VideoSettingsScreenTemp__
//   0xba4630  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecReq_GamepadMappingScreenTemp__
//   0xba4650  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnKeyListeningTemp__
//   0xba4670  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnDeleteSaveConfirmTemp__
//   0xba4690  _dynamic_initializer_for__UDisGFxMoviePlayerMenuBaseexecOnLoadGameConfirmTemp__
//   0xba46b0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecBackToStartScreenTemp__
//   0xba46d0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnQuitGameConfirmTemp__
//   0xba46f0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnDLCClickedTemp__
//   0xba4710  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnOptionsClickedTemp__
//   0xba4730  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecReq_SaveSlotInfosTemp__
//   0xba4750  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnLoadGameClickedTemp__
//   0xba4770  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOpenMissionStatsTemp__
//   0xba4790  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnMissionSelectedTemp__
//   0xba47b0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnDLCTabSelectedTemp__
//   0xba47d0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnCampaignTabSelectedTemp__
//   0xba47f0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnMissionsClickedTemp__
//   0xba4810  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnContinueClickedTemp__
//   0xba4830  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnNewGameConfirmTemp__
//   0xba4850  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnNewGameClickedTemp__
//   0xba4870  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnDeviceSelectionCompleteTemp__
//   0xba4890  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnLoginCancelledTemp__
//   0xba48b0  _dynamic_initializer_for__UDisGFxMoviePlayerMainMenuexecOnLoginChangeTemp__
//   0xba48d0  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnMenuClosedTemp__
//   0xba48f0  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecBackToWindowsTemp__
//   0xba4910  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnQuitGameConfirmTemp__
//   0xba4930  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnTutorialReadRequestTemp__
//   0xba4950  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnTutorialsClickedTemp__
//   0xba4970  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnOptionsClickedTemp__
//   0xba4990  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnDeleteSaveConfirmTemp__
//   0xba49b0  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnLoadLastSaveClickedTemp__
//   0xba49d0  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnLoadGameClickedTemp__
//   0xba49f0  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnSaveGameConfirmTemp__
//   0xba4a10  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnSaveGameClickedTemp__
//   0xba4a30  _dynamic_initializer_for__UDisGFxMoviePlayerPauseMenuexecOnResumeGameClickedTemp__
//   0xba4a50  _dynamic_initializer_for__UDisGFxMoviePlayerMissionStatsexecOnClosedTemp__
//   ... 23 more, see resources/docs/symbols/functions.csv
