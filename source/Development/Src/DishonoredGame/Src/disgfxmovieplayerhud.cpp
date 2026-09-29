// DishonoredGame/src/disgfxmovieplayerhud.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (157):
//   0x7f46f0  public: static void __cdecl UDisSeqAct_ShowHUDMessage::InitializePrivateStaticClassUDisSeqAct_ShowHUDMessage(void)
//   0x7f4710  public: static void __cdecl UDisSeqAct_SetTutorialMessage::InitializePrivateStaticClassUDisSeqAct_SetTutorialMessage(void)
//   0x7f4730  public: static void __cdecl UDisSeqAct_EnableSystemicTutorials::InitializePrivateStaticClassUDisSeqAct_EnableSystemicTutorials(void)
//   0x7f4750  public: static void __cdecl UDisSeqAct_ShowLocationDiscovery::InitializePrivateStaticClassUDisSeqAct_ShowLocationDiscovery(void)
//   0x7f4770  public: static void __cdecl UDisSeqAct_ShowTargetNotification::InitializePrivateStaticClassUDisSeqAct_ShowTargetNotification(void)
//   0x7f4790  public: unsigned int __thiscall FDisPlayerStatus_Equipment::operator!=(struct FDisPlayerStatus_Equipment const &)const
//   0x7f47f0  public: __thiscall FDisUIInteraction::FDisUIInteraction(void)
//   0x7f4810  public: __thiscall FDisUIInteraction::FDisUIInteraction(enum EDisUIInteraction, class FString const *, unsigned int)
//   0x7f4830  public: void __thiscall FDisUIInteractionGroup::Add(struct FDisUIInteraction const &)
//   0x7f48b0  public: void __thiscall FDisUIInteractionContext::Add(struct FDisUIInteraction const &)
//   0x7f48e0  public: unsigned int __thiscall FDisUIInteractionContext::HasGroupChanged(enum EDisUIInteractionGroup, struct FDisUIInteractionContext const &)const
//   0x7f4950  public: __thiscall FDisHUDDamageInfo::FDisHUDDamageInfo(int, class FVector const &, class AActor *, class ADishonoredPawn *)
//   0x7f4980  public: void __thiscall UDisGFxMoviePlayerHUD::CloseHUD(void)
//   0x7f49b0  public: unsigned int __thiscall UDisGFxMoviePlayerHUD::IsHighlightEnabled(void)const
//   0x7f49c0  public: void __thiscall UDisGFxMoviePlayerHUD::HandleObjectiveEvents(unsigned int)
//   0x7f49e0  public: void __thiscall UDisGFxMoviePlayerHUD::ShowDamage(struct FDisHUDDamageInfo const &)
//   0x7f6330  public: __thiscall FDisUIInteraction::FDisUIInteraction(enum EDisUIInteraction, unsigned int)
//   0x7f6430  public: unsigned int __thiscall FDisUIInteractionProbe::CheckMantle(void)
//   0x7f64a0  public: void __thiscall FDisUIInteractionProbe::FillUIInteractions(struct FDisUIInteractionContext &)const
//   0x7f6570  public: void __thiscall UDisGFxMoviePlayerHUD::StartHUD(void)
//   0x7f65b0  public: unsigned int __thiscall UDisGFxMoviePlayerHUD::ShouldAlwaysShowPlayerInfo(void)const
//   0x7f6610  public: void __thiscall UDisGFxMoviePlayerHUD::OnElixirConsumed(enum EElixirType)
//   0x7f66e0  public: void __thiscall UDisGFxMoviePlayerHUD::ShowKeyhole(void)
//   0x7f6780  public: void __thiscall UDisGFxMoviePlayerHUD::PreHideKeyhole(void)
//   0x7f6820  public: void __thiscall UDisGFxMoviePlayerHUD::HideKeyhole(void)
//   0x7f68c0  public: void __thiscall UDisGFxMoviePlayerHUD::HideChoiceSelection(void)
//   0x7f6990  public: virtual void __thiscall UDisGFxMoviePlayerHUD::OnPlayerChoiceConfirm(int)
//   0x7f69d0  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateStealthShroud(unsigned int)
//   0x7f6a80  protected: void __thiscall UDisGFxMoviePlayerHUD::MoveCrosshairInteractions(class AActor const *, class UCanvas *)
//   0x7f6bf0  protected: void __thiscall UDisGFxMoviePlayerHUD::HideTutorialWindow(void)
//   0x7f6c90  public: float __thiscall FDisHUDMarkerVariation::ComputeValue(float)const
//   0x7fad50  public: __thiscall FDisTutorialInfo::~FDisTutorialInfo(void)
//   0x7fad80  public: void __thiscall UDisGFxMoviePlayerHUD::HideGauge(enum EDisGaugeType, unsigned int)
//   0x7faed0  public: void __thiscall UDisGFxMoviePlayerHUD::UpdateGauge(enum EDisGaugeType, float)
//   0x7fb080  public: void __thiscall UDisGFxMoviePlayerHUD::ShowLocationDiscovery(class FString const &, unsigned int)
//   0x7fb230  public: void __thiscall UDisGFxMoviePlayerHUD::OnRemoveAwarenessOverride(class ADishonoredNPCPawn const *)
//   0x7fb2b0  public: void __thiscall UDisGFxMoviePlayerHUD::AddGrenadeMarker(class AActor const *)
//   0x7fb360  public: void __thiscall UDisGFxMoviePlayerHUD::AddHeartMarker(class ADisPickup_Base const *)
//   0x7fb450  protected: virtual void __thiscall UDisGFxMoviePlayerHUD::PostStart(void)
//   0x7fbd50  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_PlayerState(int)
//   0x7fbed0  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_Stealth(int)
//   0x7fbf60  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_Cinematic(void)
//   0x7fc0d0  protected: void __thiscall UDisGFxMoviePlayerHUD::PreRender_Layout(class UCanvas *)
//   0x7fccf0  protected: void __thiscall UDisGFxMoviePlayerHUD::PreRender_Damage(class UCanvas *)
//   0x7fd720  protected: void __thiscall UDisGFxMoviePlayerHUD::PreRender_Keyhole(void)
//   0x7fd940  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateHealthGauge(struct FDisPlayerStatus_Health const &)
//   0x7fdb80  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateManaGauge(struct FDisPlayerStatus_Mana const &)
//   0x7fddf0  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateEquipmentInfo(struct FDisPlayerStatus_Equipment const &)
//   0x7fe040  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateCrosshairFocusText(class FString const &)
//   0x7fe180  protected: void __thiscall UDisGFxMoviePlayerHUD::AddPickupLogItem(class FString const &, int, class FString const &)
//   0x7fe430  protected: int __thiscall UDisGFxMoviePlayerHUD::FindTutorial(int)const
//   0x7fe4b0  protected: void __thiscall UDisGFxMoviePlayerHUD::SetTutorialMessage(class FString const &)
//   0x7fe620  protected: void __thiscall UDisGFxMoviePlayerHUD::ClearTutorialMessage(void)
//   0x7fe700  protected: void __thiscall UDisGFxMoviePlayerHUD::ShowTutorialWindow(struct FDisTutorialInfo const &)
//   0x7fe8b0  protected: void __thiscall UDisGFxMoviePlayerHUD::ShowObjectivePopup(struct FDisObjectivePopupInfo const &)
//   0x7fef30  protected: void __thiscall UDisGFxMoviePlayerHUD::ShowTargetNotification(class UDisSeqAct_ShowTargetNotification const *)
//   0x7ff2e0  protected: void __thiscall UDisGFxMoviePlayerHUD::SortMarkers(struct FDisHUDMarkerPosInfo *, unsigned int)
//   0x7ff4f0  protected: void __thiscall UDisGFxMoviePlayerHUD::ShowSubtitleText(wchar_t const *, int)
//   0x7ff6a0  public: virtual void __thiscall UDisSeqAct_EnableSystemicTutorials::Activated(void)
//   0x7ff700  public: virtual void __thiscall UDisSeqAct_ShowLocationDiscovery::Activated(void)
//   0x802fd0  public: void __thiscall FDisObjectivePopupInfo::AddTask(struct FDisTaskPopupInfo const &)
//   0x803040  public: void __thiscall UDisGFxMoviePlayerHUD::OnAmmoPickedUp(enum eDisAmmoType, int)
//   0x803140  public: void __thiscall UDisGFxMoviePlayerHUD::OnKeyPickedUp(class FString const &, class UDisTweaks_Key const *)
//   0x803160  public: void __thiscall UDisGFxMoviePlayerHUD::OnUpgradePickedUp(class UDisTweaks_Upgrade const *)
//   0x803180  public: struct FDisTutorialInfo * __thiscall UDisGFxMoviePlayerHUD::FindTutorialInfo(int)
//   0x8031e0  public: unsigned int __thiscall UDisGFxMoviePlayerHUD::HasTutorialWindow(void)const
//   0x803250  public: void __thiscall UDisGFxMoviePlayerHUD::RemoveGrenadeMarker(class AActor const *)
//   0x803360  public: void __thiscall UDisGFxMoviePlayerHUD::RemoveGrenadeMarkers(void)
//   0x803400  public: void __thiscall UDisGFxMoviePlayerHUD::RemoveHeartMarkers(void)
//   0x8034a0  public: void __thiscall UDisGFxMoviePlayerHUD::ShowNextMainSubtitlePart(int)
//   0x803560  public: void __thiscall UDisGFxMoviePlayerHUD::ShowNextSecondarySubtitlePart(int)
//   0x803620  public: void __thiscall UDisGFxMoviePlayerHUD::ShowChoiceSelection(class FString const &, struct TMemStackArray<struct FDisPlayerChoiceInfo> &)
//   0x803950  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_PlayerStatus(int)
//   0x803f70  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_Subtitles(int)
//   0x804080  protected: void __thiscall UDisGFxMoviePlayerHUD::PreRender_Crosshair(class UCanvas *)
//   0x8049d0  protected: void __thiscall UDisGFxMoviePlayerHUD::PreRender_Markers(class UCanvas *)
//   0x804ef0  protected: void __thiscall UDisGFxMoviePlayerHUD::RemoveMarkerInstance(class FDisHUDMarker *)
//   0x804f70  protected: void __thiscall UDisGFxMoviePlayerHUD::ClearMarkerInstances(void)
//   0x805080  public: static struct FDisNoteParams * __cdecl FDisNoteParams::CreateNoteParams(unsigned char)
//   0x808d20  public: unsigned int __thiscall FDisUIInteractionProbe::CheckAssassinate(void)
//   0x808e50  public: unsigned int __thiscall FDisUIInteractionProbe::CheckDropAssassinate(void)
//   0x809010  public: unsigned int __thiscall FDisUIInteractionProbe::CheckAdrenalineKill(void)
//   0x809140  class FArchive & __cdecl operator<<(class FArchive &, struct FDisTutorialInfo &)
//   0x809220  public: void __thiscall UDisGFxMoviePlayerHUD::PreRender(class UCanvas *)
//   0x809370  public: virtual void __thiscall UDisGFxMoviePlayerHUD::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x809470  public: void __thiscall UDisGFxMoviePlayerHUD::ShowGauge(enum EDisGaugeType)
//   0x809690  public: void __thiscall UDisGFxMoviePlayerHUD::OnActorTerminated(class AActor const *)
//   0x809750  public: void __thiscall UDisGFxMoviePlayerHUD::AddHeartMarkers(class TArray<class ADisPickup_Base *, class FDefaultAllocator> const &)
//   0x8097d0  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateCrosshairInteractions(class FString const &, struct FDisUIInteractionGroup const &)
//   0x809a40  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateContextInteractions(struct FDisUIInteractionGroup const &)
//   0x809c10  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateSpecialInteractions(struct FDisUIInteractionGroup const &)
//   0x80a000  protected: int __thiscall UDisGFxMoviePlayerHUD::InsertTutorial(struct FDisTutorialInfo const &)
//   0x80a240  protected: unsigned int __thiscall UDisGFxMoviePlayerHUD::IsBrainEligibleForAwarenessMarker(class UDishonoredAIBrain const *, enum EDisAttentionLevel)const
//   0x80c000  public: void __thiscall FDisUIInteractionProbe::Tick(float)
//   0x80c0f0  public: virtual void __thiscall UDisGFxMoviePlayerHUD::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x80c2b0  public: void __thiscall UDisGFxMoviePlayerHUD::ClearInteractions(void)
//   0x80c3c0  public: void __thiscall UDisGFxMoviePlayerHUD::HideTutorial(int)
//   0x80c3f0  public: void __thiscall UDisGFxMoviePlayerHUD::HideAdrenalineTutorialMessage(void)
//   0x80c430  protected: virtual void __thiscall UDisGFxMoviePlayerHUD::BeginDestroy(void)
//   0x80c450  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_Interaction(class IDisInteractableInterface const *)
//   0x80c6b0  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_Tutorials(float, int)
//   0x80caa0  protected: void __thiscall UDisGFxMoviePlayerHUD::ResolveMarkupString(class FString &)const
//   0x80ccf0  protected: void __thiscall UDisGFxMoviePlayerHUD::RemoveTutorials(void)
//   0x80cd40  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateTaskMarkerInstances(void)
//   0x80d120  protected: void __thiscall UDisGFxMoviePlayerHUD::UpdateAwarenessMarkerInstances(void)
//   0x80d770  protected: void __thiscall UDisGFxMoviePlayerHUD::SplitSubtitleText(wchar_t const *, class TArray<class FString, class FDefaultAllocator> &, class TArray<float, class FDefaultAllocator> &)
//   0x811620  public: virtual void __thiscall UDisGFxMoviePlayerHUD::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x8117c0  public: void __thiscall UDisGFxMoviePlayerHUD::OnCrosshairActorSet(class IDisInteractableInterface *)
//   0x811810  public: int __thiscall UDisGFxMoviePlayerHUD::ShowTutorial(class FString const &, float, unsigned int, unsigned int, unsigned int, unsigned int)
//   0x811990  public: int __thiscall UDisGFxMoviePlayerHUD::ShowTutorialWindow(class FString const &, enum EDisTutorialType, float, unsigned int, enum EDisUINoteType, struct FDisNoteParams *, unsigned int, enum EDisJournalTab)
//   0x811b20  public: void __thiscall UDisGFxMoviePlayerHUD::ShowAdrenalineTutorialMessage(void)
//   0x811bf0  public: void __thiscall UDisGFxMoviePlayerHUD::OnMainSubtitleChanged(wchar_t const *, class TArray<float, class FDefaultAllocator> *)
//   0x811d00  public: void __thiscall UDisGFxMoviePlayerHUD::OnSecondarySubtitleChanged(wchar_t const *, class TArray<float, class FDefaultAllocator> *)
//   0x811e40  protected: virtual void __thiscall UDisGFxMoviePlayerHUD::PreClose(unsigned int)
//   0x812080  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_ObjectivePopup(float)
//   0x8122c0  protected: void __thiscall UDisGFxMoviePlayerHUD::Tick_Markers(float)
//   0x812340  protected: void __thiscall UDisGFxMoviePlayerHUD::SetGameMessage(class FString const &, float)const
//   0x8124f0  public: virtual void __thiscall UDisSeqAct_SetTutorialMessage::PostLoad(void)
//   0x8125a0  public: virtual void __thiscall UDisSeqAct_SetTutorialMessage::Activated(void)
//   0x814ce0  public: virtual void __thiscall UDisGFxMoviePlayerHUD::PreAdvance(float)
//   0x814e80  public: void __thiscall UDisGFxMoviePlayerHUD::OnAbstractItemAdded(class UDisAbstractItem const *, int, unsigned int)
//   0x814ec0  public: void __thiscall UDisGFxMoviePlayerHUD::AddGameMessage(class FString const &)
//   0x814ef0  public: void __thiscall UDisGFxMoviePlayerHUD::AddUseMessage(class FString const &)
//   0x814f20  public: void __thiscall UDisGFxMoviePlayerHUD::AddInteractionMessage(class FString const &)
//   0x814f50  public: void __thiscall UDisGFxMoviePlayerHUD::OnObjectiveAdded(class UDishonoredObjective *)
//   0x814fd0  public: void __thiscall UDisGFxMoviePlayerHUD::OnObjectiveCompleted(class UDishonoredObjective *)
//   0x815130  public: void __thiscall UDisGFxMoviePlayerHUD::OnObjectiveFailed(class UDishonoredObjective *)
//   0x815290  public: void __thiscall UDisGFxMoviePlayerHUD::OnTaskAdded(class UDishonoredTask_Base *)
//   0x8153b0  public: void __thiscall UDisGFxMoviePlayerHUD::OnTaskUpdated(class UDishonoredTask_Base *)
//   0x8154d0  public: void __thiscall UDisGFxMoviePlayerHUD::OnTaskCompleted(class UDishonoredTask_Base *)
//   0x8155f0  public: void __thiscall UDisGFxMoviePlayerHUD::OnTaskFailed(class UDishonoredTask_Base *)
//   0x815720  public: void __thiscall UDisGFxMoviePlayerHUD::QueueTargetNotification(class UDisSeqAct_ShowTargetNotification *)
//   0x815790  public: virtual void __thiscall UDisSeqAct_ShowHUDMessage::Activated(void)
//   0x815800  public: virtual void __thiscall UDisSeqAct_ShowTargetNotification::Activated(void)
//   0x816e60  public: void __thiscall UDisGFxMoviePlayerHUD::OnAbstractItemPickedUp(class UDisAbstractItem const *, int)
//   0x8180c0  public: static class UClass * __cdecl UDisTweaks_GFxMoviePlayerHUD::GetPrivateStaticClassUDisTweaks_GFxMoviePlayerHUD(wchar_t const *)
//   0x81ad60  public: static void __cdecl UDisTweaks_GFxMoviePlayerHUD::InitializePrivateStaticClassUDisTweaks_GFxMoviePlayerHUD(void)
//   0x81b5a0  public: static class UClass * __cdecl UDisTweaks_GFxMoviePlayerHUD::StaticClassNoInline(void)
//   0x81b5d0  public: static class UClass * __cdecl UDisSeqAct_ShowHUDMessage::GetPrivateStaticClassUDisSeqAct_ShowHUDMessage(wchar_t const *)
//   0x81b660  public: static class UClass * __cdecl UDisSeqAct_SetTutorialMessage::GetPrivateStaticClassUDisSeqAct_SetTutorialMessage(wchar_t const *)
//   0x81b6f0  public: static class UClass * __cdecl UDisSeqAct_EnableSystemicTutorials::GetPrivateStaticClassUDisSeqAct_EnableSystemicTutorials(wchar_t const *)
//   0x81b780  public: static class UClass * __cdecl UDisSeqAct_ShowLocationDiscovery::GetPrivateStaticClassUDisSeqAct_ShowLocationDiscovery(wchar_t const *)
//   0x81b810  public: static class UClass * __cdecl UDisSeqAct_ShowTargetNotification::GetPrivateStaticClassUDisSeqAct_ShowTargetNotification(wchar_t const *)
//   0x81d840  public: static class UClass * __cdecl UDisSeqAct_ShowHUDMessage::StaticClassNoInline(void)
//   0x81d870  public: static class UClass * __cdecl UDisSeqAct_SetTutorialMessage::StaticClassNoInline(void)
//   0x81d8a0  public: static class UClass * __cdecl UDisSeqAct_EnableSystemicTutorials::StaticClassNoInline(void)
//   0x81d8d0  public: static class UClass * __cdecl UDisSeqAct_ShowLocationDiscovery::StaticClassNoInline(void)
//   0x81d900  public: static class UClass * __cdecl UDisSeqAct_ShowTargetNotification::StaticClassNoInline(void)
//   0x81ec80  public: static class UClass * __cdecl UDisGFxMoviePlayerHUD::GetPrivateStaticClassUDisGFxMoviePlayerHUD(wchar_t const *)
//   0x81ed10  public: static class UClass * __cdecl UDisGFxMoviePlayerHUDFX::GetPrivateStaticClassUDisGFxMoviePlayerHUDFX(wchar_t const *)
//   0x81eda0  public: void __thiscall UDisGFxMoviePlayerHUD::OnBoneCharmPickedUp(struct FDisCollectedWhaleBoneCharm const *)
//   0x81f060  public: void __thiscall UDisGFxMoviePlayerHUD::OnElixirPickedUp(enum EElixirType)
//   0x81f130  public: unsigned int __thiscall UDisGFxMoviePlayerHUD::UseTutorialWindow(void)
//   0x820da0  public: static void __cdecl UDisGFxMoviePlayerHUD::InitializePrivateStaticClassUDisGFxMoviePlayerHUD(void)
//   0x820dc0  public: static void __cdecl UDisGFxMoviePlayerHUDFX::InitializePrivateStaticClassUDisGFxMoviePlayerHUDFX(void)
//   0x821d70  public: static class UClass * __cdecl UDisGFxMoviePlayerHUD::StaticClassNoInline(void)
//   0x821da0  public: static class UClass * __cdecl UDisGFxMoviePlayerHUDFX::StaticClassNoInline(void)

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "gfxui_gfx3.h"

/*-----------------------------------------------------------------------------
	Agent EK (PHASE11 EK): the in-game HUD.

	The order retail brings this up in is UDisGlobalUIManager::RefreshGlobalUIState ->
	UDisGFxMoviePlayerHUD::StartHUD (2013 rva 0x78cd00) -> UDisGFxMoviePlayerBase::Start (0x7a3f40)
	-> UGFxMoviePlayer::Start -> PostStart (0x795650); then every frame
	UDisGFxMoviePlayerBase::Advance (0x787680) -> PreAdvance (0x7b2280) -> Tick_PlayerStatus
	(0x79f6e0) for the numbers, and ADishonoredPlayerController::PreRender (0x6a0c50) -> PreRender
	(0x7a5840) -> PreRender_Layout (0x796250) for where they sit.

	Everything below the bring-up section is that chain, body for body. The bring-up itself stands in
	for UDisGlobalUIManager, which is not this package: it does for the HUD exactly what
	RefreshGlobalUIState does - construct the two movie players, point them at the cooked movies the
	config movie set names, and call StartHUD - and nothing else that class does.
-----------------------------------------------------------------------------*/


void DisGFxMoviePlayerHUDPostStart( class UDisGFxMoviePlayerHUD* HUD );
void DisGFxMoviePlayerHUDPreAdvance( class UDisGFxMoviePlayerHUD* HUD, FLOAT DeltaTime );
void DisGFxMoviePlayerHUDApplyGameSettings( class UDisGFxMoviePlayerHUD* HUD,
	const class ArkSettingsParameters* Parameters );

/* The 32 clip handles PostStart binds live in m_pMovieClips, indexed by retail's own
   EDisHUDMovieClip (DishonoredGameEngineShims.h): DHMC_PlayerStatus is masterHUD_mc, DHMC_Cinematic
   is blackStripes_mc and DHMC_CrosshairAspect is the one slot PostStart sets to null and never
   binds. */

/** the element bit of ADishonoredHUD::m_ShowFlags the mana gauge's visibility is read from */
#define DIS_HUD_ELEMENT_PLAYERINFO 0x4

static GFxValue* DisHUDClips( const UDisGFxMoviePlayerHUD* HUD )
{
	return (GFxValue*)HUD->m_pMovieClips;
}

static GFxMovieView* DisHUDView( UGFxMoviePlayer* Player )
{
	FGFxMovie* Movie = Player ? Player->GetMovie() : NULL;
	return Movie ? Movie->pView.GetPtr() : NULL;
}

/** DISHONORED(port): the movie-space math of UDisGFxMoviePlayerBase::ComputeMovieSpaceInfo (2013 rva
    0x787860), which PostStart and PreRender both inline: the movie is laid out at its authored size
    and the screen is letterboxed or pillarboxed around it. */
static void DisHUDComputeMovieSpace( FDisMovieSpaceInfo& Info )
{
	const FLOAT ScreenX = Info.m_ScreenSize.X;
	const FLOAT ScreenY = Info.m_ScreenSize.Y;
	const FLOAT MovieX = Info.m_MovieSize.X;
	const FLOAT MovieY = Info.m_MovieSize.Y;
	if( MovieX / MovieY <= ScreenX * ( 1.f / ScreenY ) )
	{
		Info.m_fScreenToMovieScaling = MovieY * ( 1.f / ScreenY );
		Info.m_MovieSpaceSize.X = Info.m_fScreenToMovieScaling * ScreenX;
		Info.m_MovieSpaceSize.Y = MovieY;
		Info.m_EmptySpace.X = ( Info.m_MovieSpaceSize.X - MovieX ) * 0.5f;
		Info.m_EmptySpace.Y = 0.f;
	}
	else
	{
		Info.m_fScreenToMovieScaling = MovieX / ScreenX;
		Info.m_MovieSpaceSize.X = MovieX;
		Info.m_MovieSpaceSize.Y = Info.m_fScreenToMovieScaling * ScreenY;
		Info.m_EmptySpace.X = 0.f;
		Info.m_EmptySpace.Y = ( Info.m_MovieSpaceSize.Y - MovieY ) * 0.5f;
	}
}

/** DISHONORED(port): 2013 rva 0x83d290, new in the 2013 build (no 2012 body, so no PDB name): the
    (EeDisUISelectionType, EeDisAmmoType) -> EDisPowerWheelItem map the equipment slot is indexed by.
    Both tables are the retail byte sequences. 36 is the escape that means "ask the ammo table". */
static BYTE DisGetPowerWheelItem( BYTE UISelection, BYTE AmmoType )
{
	static const BYTE BySelection[28] =
	{
		0, 0, 24, 25, 22, 26, 21, 23, 0, 0, 0, 0, 4, 5,
		36, 36, 36, 0, 0, 13, 27, 28, 29, 0, 10, 11, 36, 30,
	};
	static const BYTE ByAmmo[12] = { 8, 9, 1, 2, 3, 0, 6, 7, 10, 0, 12, 0 };
	if( UISelection >= ARRAY_COUNT(BySelection) )
	{
		return 0;
	}
	const BYTE Item = BySelection[UISelection];
	if( Item != 36 )
	{
		return Item;
	}
	return AmmoType < ARRAY_COUNT(ByAmmo) ? ByAmmo[AmmoType] : 0;
}

/*-----------------------------------------------------------------------------
	The gauges and the equipment: what the numbers on screen actually are.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x797d40 (2012 0x7fd940). Five arguments to masterHUD_mc.FillHealthGauge:
// the percentage, the two upgrade bits, whether the player is under the low-health threshold, and the
// percentage the pending regen will reach.
void UDisGFxMoviePlayerHUD::UpdateHealthGauge( const FDisPlayerStatus_Health& _rHealth )
{
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	const UBOOL bLowOnHealth = Tweaks != NULL && _rHealth.m_Health <= Tweaks->m_LowHealthThreshold;
	const DOUBLE Denominator = _rHealth.m_HealthMax != 0 ? (DOUBLE)_rHealth.m_HealthMax : 1.0;

	GFxValue Args[5];
	Args[0].SetNumber( (DOUBLE)( 100 * _rHealth.m_Health ) / Denominator );
	Args[1].SetBoolean( _rHealth.m_bHasSmallUpgrade != 0 );
	Args[2].SetBoolean( _rHealth.m_bHasLargeUpgrade != 0 );
	Args[3].SetBoolean( bLowOnHealth != 0 );
	Args[4].SetNumber( (DOUBLE)( 100 * ( _rHealth.m_Health + _rHealth.m_HealthRegen ) ) / Denominator );

	GFxValue Result;
	const UBOOL bInvoked = DisHUDClips( this )[DHMC_PlayerStatus].Invoke( "FillHealthGauge", &Result, Args, 5 );
	Result.ReleaseManaged();
	m_bPlayerLowOnHealth = bLowOnHealth ? TRUE : FALSE;
	debugf( TEXT("DISHONORED(bringup): HUD gauge census: FillHealthGauge(%.1f, %d, %d, %d, %.1f) -> %s ")
		TEXT("[health %d/%d regen %d]"),
		Args[0].GetNumber(), (INT)Args[1].GetBool(), (INT)Args[2].GetBool(), (INT)Args[3].GetBool(),
		Args[4].GetNumber(), bInvoked ? TEXT("ok") : TEXT("NOT FOUND"),
		_rHealth.m_Health, _rHealth.m_HealthMax, _rHealth.m_HealthRegen );
}

// DISHONORED(port): 2013 rva 0x797f80 (2012 0x7fdb80). Six arguments to masterHUD_mc.FillManaGauge;
// argument 3 is the NEGATION of m_bVisible, which is what greys the bar out rather than removing it.
void UDisGFxMoviePlayerHUD::UpdateManaGauge( const FDisPlayerStatus_Mana& _rMana )
{
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	const DOUBLE Denominator = _rMana.m_ManaMax != 0 ? (DOUBLE)_rMana.m_ManaMax : 1.0;

	GFxValue Args[6];
	Args[0].SetNumber( (DOUBLE)( 100 * _rMana.m_Mana ) / Denominator );
	Args[1].SetBoolean( _rMana.m_bHasUpgrade != 0 );
	Args[2].SetBoolean( _rMana.m_bAvailable != 0 );
	Args[3].SetBoolean( _rMana.m_bVisible == 0 );
	Args[4].SetBoolean( Tweaks != NULL && _rMana.m_Mana <= Tweaks->m_LowManaThreshold );
	Args[5].SetNumber( (DOUBLE)( 100 * ( _rMana.m_Mana + _rMana.m_ManaRegen ) ) / Denominator );

	GFxValue Result;
	const UBOOL bInvoked = DisHUDClips( this )[DHMC_PlayerStatus].Invoke( "FillManaGauge", &Result, Args, 6 );
	Result.ReleaseManaged();
	debugf( TEXT("DISHONORED(bringup): HUD gauge census: FillManaGauge(%.1f, %d, %d, %d, %d, %.1f) -> %s ")
		TEXT("[mana %d/%d regen %d]"),
		Args[0].GetNumber(), (INT)Args[1].GetBool(), (INT)Args[2].GetBool(), (INT)Args[3].GetBool(),
		(INT)Args[4].GetBool(), Args[5].GetNumber(), bInvoked ? TEXT("ok") : TEXT("NOT FOUND"),
		_rMana.m_Mana, _rMana.m_ManaMax, _rMana.m_ManaRegen );
}

// DISHONORED(port): 2013 rva 0x7981f0 (2012 0x7fddf0). One of three calls on masterHUD_mc: a power
// slot, a weapon slot with its two ammo counts, or an empty hand.
void UDisGFxMoviePlayerHUD::UpdateEquipmentInfo( const FDisPlayerStatus_Equipment& _rEquipment )
{
	GFxValue* Clips = DisHUDClips( this );
	GFxValue Result;
	// DISHONORED(port): UDisGlobalEnums::PowerWheelItemIsPower, 2013 rva 0x83d4a0, is `Item > 20`.
	if( _rEquipment.m_Item > 20 )
	{
		GFxValue Args[3];
		Args[0].SetNumber( (DOUBLE)_rEquipment.m_Item );
		Args[1].SetBoolean( _rEquipment.m_bPowerActive != 0 );
		Args[2].SetBoolean( _rEquipment.m_bPowerActive == 0 && _rEquipment.m_bPowerHasMana != 0 );
		Clips[DHMC_PlayerStatus].Invoke( "SetPower", &Result, Args, 3 );
	}
	else if( _rEquipment.m_Item != 0 )
	{
		GFxValue Args[3];
		Args[0].SetNumber( (DOUBLE)_rEquipment.m_Item );
		if( _rEquipment.m_Item == 4 )
		{
			// the sword: no counts at all
			Args[1].SetNull();
			Args[2].SetNull();
		}
		else
		{
			// DISHONORED(bringup): retail asks ADishonoredGameInfo (vtable +1060) whether ammo is
			// infinite, shows "--" instead of the total when it is and then replays _equipment_mc from
			// its "default" frame. That accessor is not declared in this tree, so the count is always
			// the real one and the replay never runs.
			Args[1].SetNumber( (DOUBLE)_rEquipment.m_AmmoTotal );
			Args[2].SetNumber( (DOUBLE)_rEquipment.m_LoadedAmmoCount );
		}
		Clips[DHMC_PlayerStatus].Invoke( "SetWeapon", &Result, Args, 3 );
	}
	else
	{
		Clips[DHMC_PlayerStatus].Invoke( "SetEmptyEquipment", &Result, NULL, 0 );
	}
	Result.ReleaseManaged();
	debugf( TEXT("DISHONORED(bringup): HUD gauge census: equipment item %d, ammo %d/%d, power %d/%d"),
		(INT)_rEquipment.m_Item, _rEquipment.m_LoadedAmmoCount, _rEquipment.m_AmmoTotal,
		(INT)_rEquipment.m_bPowerActive, (INT)_rEquipment.m_bPowerHasMana );
}

// DISHONORED(port): 2013 rva 0x794c10 (2012 0x7fad80). Only the gauge that is up answers.
void UDisGFxMoviePlayerHUD::HideGauge( BYTE _Gauge, UBOOL _bSuccess )
{
	if( _Gauge != m_ActiveGauge )
	{
		return;
	}
	GFxValue* Clips = DisHUDClips( this );
	GFxValue Result;
	switch( _Gauge )
	{
	case 1:
	case 2:
		Clips[DHMC_InteractionGauge].Invoke( "Close", &Result, NULL, 0 );
		break;
	case 3:
		{
			GFxValue Arg;
			Arg.SetBoolean( _bSuccess == 0 );
			Clips[DHMC_GrenadeCookingGauge].Invoke( "Close", &Result, &Arg, 1 );
		}
		break;
	case 4:
		Clips[DHMC_SkipCutSceneGauge].Invoke( "Close", &Result, NULL, 0 );
		break;
	default:
		break;
	}
	Result.ReleaseManaged();
	m_ActiveGauge = 0;
}

// DISHONORED(port): 2013 rva 0x794d40 (2012 0x7faed0). The grenade gauge is scaled by 300 and the
// other two by 100; that is the retail constant, not a rounding of one.
void UDisGFxMoviePlayerHUD::UpdateGauge( BYTE _Gauge, FLOAT _fValue )
{
	if( _Gauge != m_ActiveGauge )
	{
		return;
	}
	GFxValue* Clips = DisHUDClips( this );
	GFxValue Arg;
	GFxValue Result;
	switch( _Gauge )
	{
	case 1:
	case 2:
		Arg.SetNumber( _fValue * 100.0 );
		Clips[DHMC_InteractionGauge].Invoke( "SetGauge", &Result, &Arg, 1 );
		break;
	case 3:
		Arg.SetNumber( _fValue * 300.0 );
		Clips[DHMC_GrenadeCookingGauge].Invoke( "SetGauge", &Result, &Arg, 1 );
		break;
	case 4:
		Arg.SetNumber( _fValue * 100.0 );
		Clips[DHMC_SkipCutSceneGauge].Invoke( "SetGauge", &Result, &Arg, 1 );
		break;
	default:
		break;
	}
	Result.ReleaseManaged();
}

// DISHONORED(port): 2013 rva 0x78cd40 (2012 0x7f65b0). With the HUD set to always-on the answer is
// yes outright; in contextual mode it is yes only while the player is low on health or on mana.
UBOOL UDisGFxMoviePlayerHUD::ShouldAlwaysShowPlayerInfo() const
{
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	ADishonoredPlayerPawn* Pawn = ADishonoredPlayerPawn::s_pInstance;
	if( m_Settings.m_HUDVisibility == 0 || Tweaks == NULL || Pawn == NULL )
	{
		return FALSE;
	}
	if( m_Settings.m_HUDVisibility == 2 )
	{
		return TRUE;
	}
	if( Pawn->Health <= Tweaks->m_LowHealthThreshold && Tweaks->m_bAlwaysShowPlayerInfoWhenLowOnHealth )
	{
		return TRUE;
	}
	return Pawn->m_Mana <= Tweaks->m_LowManaThreshold && Tweaks->m_bAlwaysShowPlayerInfoWhenLowOnMana
		? TRUE : FALSE;
}

/*-----------------------------------------------------------------------------
	The per-frame feed.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x795ed0 (2012 0x7fbd50). playerStatesIcon_mc.SetIcon(<state>), where the
// state is null / "sneak" / "crouch" / "slide", and only a change is sent.
void UDisGFxMoviePlayerHUD::Tick_PlayerState( INT _ShowFlags )
{
	ADishonoredPlayerPawn* Pawn = ADishonoredPlayerPawn::s_pInstance;
	BYTE State = 0;
	// DISHONORED(bringup): retail gates this on ADishonoredPawn::IsPossessing (2013 0x74a550) and on
	// the pawn's vtable slot +1660, and reads the slide state from ADishonoredPlayerPawn::IsSliding.
	// None of the three is declared in this tree, so the stance is the crouch bit alone and state 3
	// (slide) is never reached.
	if( Pawn != NULL && ( _ShowFlags & 0xE00 ) != 0 && m_Settings.m_bShowPlayerStance )
	{
		State = Pawn->bIsCrouched ? 2 : 1;
	}
	if( State == m_PlayerState )
	{
		return;
	}
	GFxValue Arg;
	switch( State )
	{
	case 0:  Arg.SetNull();             break;
	case 1:  Arg.SetString( "sneak" );  break;
	case 2:  Arg.SetString( "crouch" ); break;
	case 3:  Arg.SetString( "slide" );  break;
	default: break;
	}
	GFxValue Result;
	DisHUDClips( this )[DHMC_PlayerState].Invoke( "SetIcon", &Result, &Arg, 1 );
	Result.ReleaseManaged();
	Arg.ReleaseManaged();
	m_PlayerState = State;
}

/** DISHONORED(port): the equipment half of Tick_PlayerStatus (2013 rva 0x79f6e0), split out so the
    tick body reads as retail's three status blocks. */
static void DisHUDBuildEquipmentStatus( ADishonoredPawn* Pawn, ADishonoredPlayerPawn* PlayerPawn,
	FDisPlayerStatus_Equipment& OutEquipment )
{
	UDishonoredInventoryItem* Item = Pawn->m_pInventory != NULL
		? Pawn->m_pInventory->GetEquippedItem( EDisEquipUsage_Secondary ) : NULL;
	// DISHONORED(bringup): when the equipped item is the bare hand and the pawn is the player's own,
	// retail looks the last-equipped power back up through UDishonoredInventory::GetReequipItem
	// (2013 0x8055e0) and FindItemByClass (0x80b4a0) so the wheel keeps showing it. Neither is
	// declared in this tree; the effect is that an empty hand reads as empty rather than as the power
	// it would be re-equipped with.
	OutEquipment.m_pItem = Item;
	if( Item == NULL )
	{
		return;
	}

	if( UDisItemPowers* Powers = Cast<UDisItemPowers>( Item ) )
	{
		UDishonoredActivePowerComponent* Power = Powers->m_pCurrentActivePower;
		if( Power != NULL )
		{
			OutEquipment.m_Item = DisGetPowerWheelItem( Power->m_eUISelection, 255 );
			// DISHONORED(bringup): the two power bits are retail's vtable slots +352 (is the power
			// running) and +396 (does the player have the mana for it) on
			// UDishonoredActivePowerComponent. Neither slot is declared in this tree, so a power reads
			// as idle and unaffordable and the icon draws in its resting state.
		}
		return;
	}

	if( UDishonoredWeapon_Ranged* Weapon = Cast<UDishonoredWeapon_Ranged>( Item ) )
	{
		// DISHONORED(port): UDishonoredWeapon_Ranged::GetLoadedAmmoCount 2013 rva 0x81ca90 and
		// GetNotLoadedAmmoCount 0x81caa0 (the inventory's reserve of the loaded type, less what is in
		// the weapon).
		const INT Loaded = Weapon->m_LoadedAmmoInfo.m_AmmoCount;
		INT Reserve = 0;
		if( PlayerPawn->m_pInventory != NULL )
		{
			if( const FDisAmmoInfo* AmmoInfo = PlayerPawn->m_pInventory->GetAmmoInfo( Weapon->m_CurAmmoType ) )
			{
				Reserve = AmmoInfo->m_AmmoCount - Loaded;
			}
		}
		OutEquipment.m_Item = DisGetPowerWheelItem( Weapon->m_eUISelection, Weapon->m_CurAmmoType );
		OutEquipment.m_AmmoTotal = Loaded + Reserve;
		OutEquipment.m_LoadedAmmoCount = Loaded;
		return;
	}

	OutEquipment.m_Item = DisGetPowerWheelItem( Item->m_eUISelection, 255 );
	if( PlayerPawn->m_pInventory != NULL )
	{
		// the three thrown items whose count is an ammo pool rather than a magazine
		BYTE AmmoType = 255;
		switch( Item->m_eUISelection )
		{
		case 13: AmmoType = 5;  break;
		case 25: AmmoType = 9;  break;
		case 19: AmmoType = 11; break;
		default: break;
		}
		if( AmmoType != 255 )
		{
			if( const FDisAmmoInfo* AmmoInfo = PlayerPawn->m_pInventory->GetAmmoInfo( AmmoType ) )
			{
				OutEquipment.m_AmmoTotal = AmmoInfo->m_AmmoCount;
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x79f6e0 (2012 0x803950). The whole of the player's status: three structs
// rebuilt every frame, pushed at the movie only when one of them changed, then masterHUD_mc.Open or
// .Close when the HUD's own visibility changed.
void UDisGFxMoviePlayerHUD::Tick_PlayerStatus( INT _ShowFlags )
{
	static const FName NAME_AttributeHealthMax( TEXT("Attribute_HealthMax") );
	static const FName NAME_BoneCharmHealthMax( TEXT("Whale Bone Charm Attribute_HealthMax") );
	static const FName NAME_Vitality1( TEXT("Vitality 1 (0)") );
	static const FName NAME_Vitality2( TEXT("Vitality 2 (0)") );
	static const FName NAME_AttributeManaMax( TEXT("Attribute_ManaMax") );
	static const FName NAME_BoneCharmManaMax( TEXT("Whale Bone Charm Attribute_ManaMax") );

	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	ADishonoredPlayerPawn* PlayerPawn = ADishonoredPlayerPawn::s_pInstance;
	if( PC == NULL || PlayerPawn == NULL || m_pMovieClips == NULL )
	{
		return;
	}
	// retail: the possessed pawn while a possession is running, the player's own otherwise
	ADishonoredPawn* Pawn = Cast<ADishonoredPawn>( PC->Pawn );
	if( Pawn == NULL )
	{
		Pawn = PlayerPawn;
	}
	ADishonoredHUD* HUDActor = Cast<ADishonoredHUD>( PC->myHUD );
	const UBOOL bShowFromFlags = ( _ShowFlags & 0xE ) != 0;
	UBOOL bBecameValid = FALSE;

	// --- health ---
	FDisPlayerStatus_Health Health(EC_EventParm);
	Health.m_Health = Max<INT>( Pawn->Health, 0 );
	Health.m_HealthRegen = Pawn->m_HealthRegenAmount;
	Health.m_HealthMax = Pawn->HealthMax;
	Health.m_bHasSmallUpgrade = Pawn->HasAttributeModifier( NAME_AttributeHealthMax, NAME_BoneCharmHealthMax )
		? TRUE : FALSE;
	Health.m_bHasLargeUpgrade = ( Pawn->HasAttributeModifier( NAME_AttributeHealthMax, NAME_Vitality1 )
		|| Pawn->HasAttributeModifier( NAME_AttributeHealthMax, NAME_Vitality2 ) ) ? TRUE : FALSE;
	if( Health.m_Health != m_Health.m_Health
		|| Health.m_HealthRegen != m_Health.m_HealthRegen
		|| Health.m_HealthMax != m_Health.m_HealthMax
		|| Health.m_bHasSmallUpgrade != m_Health.m_bHasSmallUpgrade
		|| Health.m_bHasLargeUpgrade != m_Health.m_bHasLargeUpgrade )
	{
		bBecameValid = m_Health.m_HealthMax >= 0;
		UpdateHealthGauge( Health );
		m_Health = Health;
	}

	// --- mana ---
	FDisPlayerStatus_Mana Mana(EC_EventParm);
	Mana.m_Mana = Max<INT>( PlayerPawn->m_Mana, 0 );
	Mana.m_ManaRegen = Max<INT>( PlayerPawn->m_ManaRegenAmount, 0 );
	Mana.m_ManaMax = PlayerPawn->m_ManaMax;
	Mana.m_bVisible = ( HUDActor != NULL && HUDActor->IsHUDElementEnabled( 4, DIS_HUD_ELEMENT_PLAYERINFO ) )
		? TRUE : FALSE;
	// DISHONORED(bringup): retail's second bit is `Pawn == PlayerPawn && !ArePowersInhibited()`
	// (2013 0x749020, a bend-time comparison against ADishonoredPawn::m_fPowersInhibitedUntil). That
	// property is not declared in this tree, so only the possession half is tested.
	Mana.m_bAvailable = ( Pawn == PlayerPawn ) ? TRUE : FALSE;
	Mana.m_bHasUpgrade = PlayerPawn->HasAttributeModifier( NAME_AttributeManaMax, NAME_BoneCharmManaMax )
		? TRUE : FALSE;
	if( Mana.m_Mana != m_Mana.m_Mana
		|| Mana.m_ManaRegen != m_Mana.m_ManaRegen
		|| Mana.m_ManaMax != m_Mana.m_ManaMax
		|| Mana.m_bVisible != m_Mana.m_bVisible
		|| Mana.m_bAvailable != m_Mana.m_bAvailable
		|| Mana.m_bHasUpgrade != m_Mana.m_bHasUpgrade )
	{
		bBecameValid = bBecameValid || m_Mana.m_ManaMax >= 0;
		UpdateManaGauge( Mana );
		m_Mana = Mana;
	}

	// --- the equipped item ---
	FDisPlayerStatus_Equipment Equipment(EC_EventParm);
	DisHUDBuildEquipmentStatus( Pawn, PlayerPawn, Equipment );
	if( Equipment.m_Item != m_Equipment.m_Item
		|| Equipment.m_bPowerActive != m_Equipment.m_bPowerActive
		|| Equipment.m_bPowerHasMana != m_Equipment.m_bPowerHasMana
		|| Equipment.m_AmmoTotal != m_Equipment.m_AmmoTotal
		|| Equipment.m_LoadedAmmoCount != m_Equipment.m_LoadedAmmoCount )
	{
		bBecameValid = bBecameValid || m_Equipment.m_pItem != NULL;
		UpdateEquipmentInfo( Equipment );
		m_Equipment = Equipment;
	}
	else
	{
		m_Equipment.m_pItem = Equipment.m_pItem;
	}

	// --- masterHUD_mc.Open / .Close ---
	UBOOL bWantVisible = bShowFromFlags;
	if( bBecameValid && m_Settings.m_HUDVisibility == 1 )
	{
		bWantVisible = TRUE;
		if( HUDActor != NULL )
		{
			HUDActor->RequestPlayerInfoDisplay( TRUE );
		}
	}
	if( bWantVisible != ( m_bPlayerStatusVisible != 0 ) )
	{
		// retail never opens on the first tick: m_bTickedOnce is what makes the movie advance once
		// before it is asked to play its opening tween.
		const UBOOL bOpen = m_bTickedOnce && bWantVisible;
		GFxValue Result;
		const UBOOL bInvoked = DisHUDClips( this )[DHMC_PlayerStatus].Invoke(
			bOpen ? "Open" : "Close", &Result, NULL, 0 );
		debugf( TEXT("DISHONORED(bringup): HUD masterHUD_mc.%s -> %s"),
			bOpen ? TEXT("Open") : TEXT("Close"), bInvoked ? TEXT("ok") : TEXT("NOT FOUND") );
		Result.ReleaseManaged();
		m_bPlayerStatusVisible = bOpen ? TRUE : FALSE;
	}
	m_bTickedOnce = TRUE;

	// DISHONORED(bringup): the census line this package is measured on - what the HUD is being fed and
	// what it did with it, once a second.
	static INT Ticks = 0;
	static DOUBLE LastReport = 0.0;
	Ticks++;
	if( appSeconds() - LastReport > 1.0 )
	{
		LastReport = appSeconds();
		debugf( TEXT("DISHONORED(bringup): HUD status census: %d ticks, show flags 0x%04X, health %d/%d ")
			TEXT("(+%d), mana %d/%d (+%d), item %d ammo %d/%d, player status %s, gauges %d/%d"),
			Ticks, _ShowFlags, m_Health.m_Health, m_Health.m_HealthMax, m_Health.m_HealthRegen,
			m_Mana.m_Mana, m_Mana.m_ManaMax, m_Mana.m_ManaRegen, (INT)m_Equipment.m_Item,
			m_Equipment.m_LoadedAmmoCount, m_Equipment.m_AmmoTotal,
			m_bPlayerStatusVisible ? TEXT("open") : TEXT("closed"),
			(INT)m_Mana.m_bVisible, (INT)m_Mana.m_bAvailable );
	}
}

/*-----------------------------------------------------------------------------
	Where it all sits: PreRender and PreRender_Layout.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x796250 (2012 0x7fc0d0). Every element of the HUD is anchored to one of
// the four corners of the safe area, and this is the only place any of them is positioned. It runs
// once per viewport size, which is why m_ViewportSizeX/Y are written at the end.
void UDisGFxMoviePlayerHUD::PreRender_Layout( UCanvas* _pCanvas )
{
	if( _pCanvas->SizeX == m_ViewportSizeX && _pCanvas->SizeY == m_ViewportSizeY )
	{
		return;
	}
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	GFxValue* Clips = DisHUDClips( this );
	if( Clips == NULL || Tweaks == NULL )
	{
		return;
	}

	const FDisMovieSpaceInfo& Space = m_MovieSpaceInfoOnLastPreRender;
	const FLOAT SafeRatio = Tweaks->m_fSafeAreaRatio_PC;
	const FLOAT HalfMovieX = Space.m_MovieSize.X * 0.5f;
	const FLOAT HalfMovieY = Space.m_MovieSize.Y * 0.5f;
	const FLOAT MarginX = ( ( 1.f - SafeRatio ) * Space.m_MovieSpaceSize.X ) * 0.5f;
	const FLOAT MarginY = ( Space.m_MovieSpaceSize.Y * ( 1.f - SafeRatio ) ) * 0.5f;
	const FLOAT Left = -Space.m_EmptySpace.X + MarginX;
	const FLOAT Top = -Space.m_EmptySpace.Y + MarginY;
	const FLOAT Right = ( Space.m_MovieSpaceSize.X - Space.m_EmptySpace.X ) - MarginX;
	const FLOAT Bottom = ( Space.m_MovieSpaceSize.Y - Space.m_EmptySpace.Y ) - MarginY;

	// the shroud is drawn over the whole movie space; under triple screen retail recomputes that
	// space for the single centre screen instead.
	FDisMovieSpaceInfo ShroudSpace = Space;
	if( m_bTripleScreenMode )
	{
		ShroudSpace.m_ScreenSize.X = (FLOAT)_pCanvas->SizeX;
		ShroudSpace.m_ScreenSize.Y = (FLOAT)_pCanvas->SizeY;
		DisHUDComputeMovieSpace( ShroudSpace );
	}

	// blackStripes_mc._stripeH is the cinematic bar height the movie itself publishes; every element
	// anchored to the bottom of the 720-high authored frame is offset by it.
	FLOAT StripeH = 0.f;
	GFxValue StripeHValue;
	if( Clips[DHMC_Cinematic].GetMember( "_stripeH", &StripeHValue ) )
	{
		StripeH = (FLOAT)StripeHValue.GetNumber();
	}
	StripeHValue.ReleaseManaged();
	const FLOAT StripeBottom = 720.f - StripeH;

	GFxValue::DisplayInfo Info;

	Info.Clear(); Info.SetPosition( Left, Top );
	Clips[DHMC_PlayerStatus].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( Left, Bottom );
	Clips[DHMC_PlayerState].SetDisplayInfo( Info );

	GFxValue Number;
	Number.SetNumber( ShroudSpace.m_MovieSpaceSize.X );
	Clips[DHMC_StealthShroud].SetMember( "_width", Number );
	Number.SetNumber( ShroudSpace.m_MovieSpaceSize.Y );
	Clips[DHMC_StealthShroud].SetMember( "_height", Number );

	Info.Clear(); Info.SetPosition( Left + m_ContextInteractionsOffset.X, Top + m_ContextInteractionsOffset.Y );
	Clips[DHMC_ContextInteractions].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( Right, Bottom );
	Clips[DHMC_SpecialInteractions].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( Left, StripeBottom );
	Clips[DHMC_SkipCutSceneGauge].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( m_PickupLogOffset.X + Right - 54.f, m_PickupLogOffset.Y + Top );
	Clips[DHMC_PickupLog].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( HalfMovieX, StripeBottom );
	Clips[DHMC_GameMessages].SetDisplayInfo( Info );
	Clips[DHMC_Tutorials].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( Left + 8.f, Bottom );
	Clips[DHMC_TutorialWindow].SetDisplayInfo( Info );

	Info.Clear();
	Info.SetPosition( HalfMovieX,
		m_fLocationDiscoveryVerticalAlignment * Space.m_MovieSpaceSize.Y - Space.m_EmptySpace.Y );
	Clips[DHMC_LocationDiscovery].SetDisplayInfo( Info );

	Info.Clear(); Info.SetPosition( Right, Top );
	Clips[DHMC_ObjectivePopup].SetDisplayInfo( Info );
	Clips[DHMC_SpecialNotification].SetDisplayInfo( Info );

	// the four damage-direction vignette strips, pushed out to the edges of the whole screen
	const FLOAT VignetteEmptyX = m_bTripleScreenMode ? ShroudSpace.m_EmptySpace.X : Space.m_EmptySpace.X;
	const FLOAT VignetteEmptyY = m_bTripleScreenMode ? ShroudSpace.m_EmptySpace.Y : Space.m_EmptySpace.Y;
	Info.Clear(); Info.SetPosition( 0.f, -HalfMovieY - VignetteEmptyY );
	Clips[DHMC_DamageVignetteUp].SetDisplayInfo( Info );
	Info.Clear(); Info.SetY( VignetteEmptyY + HalfMovieY );
	Clips[DHMC_DamageVignetteDown].SetDisplayInfo( Info );
	Info.Clear(); Info.SetPosition( -HalfMovieX - VignetteEmptyX, 0.f );
	Clips[DHMC_DamageVignetteLeft].SetDisplayInfo( Info );
	Info.Clear(); Info.SetX( VignetteEmptyX + HalfMovieX );
	Clips[DHMC_DamageVignetteRight].SetDisplayInfo( Info );

	Number.SetNumber( Max<FLOAT>( ShroudSpace.m_MovieSpaceSize.X, 1280.f ) );
	Clips[DHMC_DamageVignetteUp].SetMember( "_width", Number );
	Clips[DHMC_DamageVignetteDown].SetMember( "_width", Number );
	Number.SetNumber( Max<FLOAT>( ShroudSpace.m_MovieSpaceSize.Y, 1280.f ) );
	Clips[DHMC_DamageVignetteLeft].SetMember( "_height", Number );
	Clips[DHMC_DamageVignetteRight].SetMember( "_height", Number );

	// the two black bars that fill the pillarbox or the letterbox
	FLOAT RectW = 1.f;
	FLOAT RectH = 1.f;
	if( Space.m_EmptySpace.X != 0.f )
	{
		Info.Clear(); Info.SetPosition( -Space.m_EmptySpace.X, 0.f );
		Clips[DHMC_KeyholeRectangle1].SetDisplayInfo( Info );
		Info.Clear(); Info.SetPosition( Space.m_MovieSize.X, 0.f );
		Clips[DHMC_KeyholeRectangle2].SetDisplayInfo( Info );
		RectW = Space.m_EmptySpace.X;
		RectH = Space.m_MovieSize.Y;
	}
	else if( Space.m_EmptySpace.Y != 0.f )
	{
		Info.Clear(); Info.SetPosition( 0.f, -Space.m_EmptySpace.Y );
		Clips[DHMC_KeyholeRectangle1].SetDisplayInfo( Info );
		Info.Clear(); Info.SetPosition( 0.f, Space.m_MovieSize.Y );
		Clips[DHMC_KeyholeRectangle2].SetDisplayInfo( Info );
		RectW = Space.m_MovieSize.X;
		RectH = Space.m_EmptySpace.Y;
	}
	else
	{
		Info.Clear(); Info.SetPosition( -2.f, -2.f );
		Clips[DHMC_KeyholeRectangle1].SetDisplayInfo( Info );
		Clips[DHMC_KeyholeRectangle2].SetDisplayInfo( Info );
	}
	Number.SetNumber( RectW );
	Clips[DHMC_KeyholeRectangle1].SetMember( "_width", Number );
	Clips[DHMC_KeyholeRectangle2].SetMember( "_width", Number );
	Number.SetNumber( RectH );
	Clips[DHMC_KeyholeRectangle1].SetMember( "_height", Number );
	Clips[DHMC_KeyholeRectangle2].SetMember( "_height", Number );

	Info.Clear(); Info.SetX( -Space.m_EmptySpace.X );
	GFxValue Stripe;
	if( Clips[DHMC_Cinematic].GetMember( "_stripeUp_mc", &Stripe ) )
	{
		Stripe.SetDisplayInfo( Info );
	}
	Stripe.ReleaseManaged();
	if( Clips[DHMC_Cinematic].GetMember( "_stripeDown_mc", &Stripe ) )
	{
		Stripe.SetDisplayInfo( Info );
	}
	Stripe.ReleaseManaged();

	Info.Clear(); Info.SetPosition( HalfMovieX, StripeBottom );
	Clips[DHMC_Subtitles].SetDisplayInfo( Info );
	Info.Clear(); Info.SetY( StripeBottom );
	Clips[DHMC_Choice].SetDisplayInfo( Info );

	m_ViewportSizeX = _pCanvas->SizeX;
	m_ViewportSizeY = _pCanvas->SizeY;
	debugf( TEXT("DISHONORED(bringup): HUD layout census: canvas %dx%d, safe %.3f, left %.1f top %.1f ")
		TEXT("right %.1f bottom %.1f, stripeH %.1f, movie %.0fx%.0f, space %.0fx%.0f, empty %.1f/%.1f"),
		_pCanvas->SizeX, _pCanvas->SizeY, SafeRatio, Left, Top, Right, Bottom, StripeH,
		Space.m_MovieSize.X, Space.m_MovieSize.Y, Space.m_MovieSpaceSize.X, Space.m_MovieSpaceSize.Y,
		Space.m_EmptySpace.X, Space.m_EmptySpace.Y );
}

// DISHONORED(port): 2013 rva 0x7a5840. Called from ADishonoredPlayerController::PreRender (0x6a0c50)
// once a frame, before the world is drawn. It recomputes the movie space against the canvas the frame
// is being drawn into, re-lays the interface out when that changed, and then runs the four passes that
// need the canvas to project with.
void UDisGFxMoviePlayerHUD::PreRender( UCanvas* _pCanvas )
{
	GFxMovieView* View = DisHUDView( this );
	FGFxMovie* Movie = GetMovie();
	if( View == NULL || Movie == NULL || m_pMovieClips == NULL )
	{
		return;
	}

	INT ScreenX = _pCanvas->SizeX;
	const INT ScreenY = _pCanvas->SizeY;
	if( ScreenX != m_ViewportSizeX || ScreenY != m_ViewportSizeY )
	{
		GViewport Vp;
		View->GetViewport( &Vp );
		// three monitors side by side: the movie is laid out on the centre one only
		if( (FLOAT)ScreenX / (FLOAT)ScreenY < 3.6500001f )
		{
			m_bTripleScreenMode = FALSE;
			Vp.Left = 0;
			Vp.Width = ScreenX;
		}
		else
		{
			m_bTripleScreenMode = TRUE;
			Vp.Left = ScreenX / 3;
			Vp.Width = ScreenX / 3;
		}
		View->SetViewport( Vp );
		debugf( TEXT("DISHONORED(bringup): HUD viewport census: canvas %dx%d -> buffer %dx%d, rect %d,%d %dx%d, ")
			TEXT("scissor %d,%d %dx%d, scale %.3f aspect %.3f flags 0x%X"),
			ScreenX, ScreenY, Vp.BufferWidth, Vp.BufferHeight, Vp.Left, Vp.Top, Vp.Width, Vp.Height,
			Vp.ScissorLeft, Vp.ScissorTop, Vp.ScissorWidth, Vp.ScissorHeight, Vp.Scale, Vp.AspectRatio,
			Vp.Flags );
		GRect<FLOAT> Frame = View->GetVisibleFrameRect();
		debugf( TEXT("DISHONORED(bringup): HUD frame rect %.1f,%.1f .. %.1f,%.1f"),
			Frame.Left, Frame.Top, Frame.Right, Frame.Bottom );
	}
	if( m_bTripleScreenMode )
	{
		ScreenX /= 3;
	}

	m_MovieSpaceInfoOnLastPreRender.m_ScreenSize.X = (FLOAT)ScreenX;
	m_MovieSpaceInfoOnLastPreRender.m_ScreenSize.Y = (FLOAT)ScreenY;
	m_MovieSpaceInfoOnLastPreRender.m_MovieSize.X = (FLOAT)Movie->Info.Width;
	m_MovieSpaceInfoOnLastPreRender.m_MovieSize.Y = (FLOAT)Movie->Info.Height;
	DisHUDComputeMovieSpace( m_MovieSpaceInfoOnLastPreRender );

	PreRender_Layout( _pCanvas );

	// DISHONORED(bringup): retail then runs PreRender_Crosshair (2013 0x79ff30), the crosshair's
	// interaction pass (0x78d540 through the player controller's current interactable),
	// PreRender_Markers (0x7a0890), PreRender_Damage (0x7970f0) and PreRender_Keyhole (0x797b20).
	// All four project world positions through the canvas and none of them is ported by this package.
}

/*-----------------------------------------------------------------------------
	Start, PostStart, PreAdvance, Close: the movie's own life.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x78cd00 (2012 0x7f6570). The effects movie is pointed at the tweaks'
// own SwfMovie and started first, because PostStart binds eight of its clips out of it.
void UDisGFxMoviePlayerHUD::StartHUD()
{
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	if( m_pHudFX != NULL && Tweaks != NULL )
	{
		m_pHudFX->MovieInfo = Tweaks->m_pFXMovieInfo;
		m_pHudFX->Start( FALSE );
	}
	Start( FALSE );
}

// DISHONORED(port): 2013 rva 0x787e30 (2012 0x7f4980)
void UDisGFxMoviePlayerHUD::CloseHUD()
{
	if( m_pHudFX != NULL )
	{
		m_pHudFX->Close( FALSE );
	}
	Close( FALSE );
}

/** the clip a PostStart binding comes out of: eight of the 32 live in the effects movie */
static UBOOL DisHUDBindClip( GFxMovieView* View, GFxValue& OutClip, const char* Path )
{
	if( View == NULL )
	{
		return FALSE;
	}
	return View->GetVariable( &OutClip, Path ) ? TRUE : FALSE;
}

// DISHONORED(port): 2013 rva 0x795650 (2012 0x7fb450), retail vtable slot 117 (+468). Both movies are
// advanced by zero first so their frame-1 ActionScript has built the clips, then the 32 clip handles
// are bound by path and the two offsets the layout pass needs are measured off the movie itself.
void DisGFxMoviePlayerHUDPostStart( UDisGFxMoviePlayerHUD* HUD )
{
	GFxMovieView* View = DisHUDView( HUD );
	GFxMovieView* FXView = DisHUDView( HUD->m_pHudFX );
	if( View == NULL )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): HUD PostStart: the HUD movie has no view") );
		return;
	}
	if( FXView != NULL )
	{
		FXView->Advance( 0.f, 2 );
	}
	View->Advance( 0.f, 2 );

	// the movie space at the resolution the engine came up at; PreRender redoes this every time the
	// canvas changes size.
	FDisMovieSpaceInfo& Space = HUD->m_MovieSpaceInfoOnLastPreRender;
	Space.m_ScreenSize.X = (FLOAT)GSystemSettings.ResX;
	Space.m_ScreenSize.Y = (FLOAT)GSystemSettings.ResY;
	Space.m_MovieSize.X = (FLOAT)HUD->GetMovie()->Info.Width;
	Space.m_MovieSize.Y = (FLOAT)HUD->GetMovie()->Info.Height;
	DisHUDComputeMovieSpace( Space );

	GFxValue* Clips = new GFxValue[DHMC_MAX];
	HUD->m_pMovieClips = Clips;

	INT Bound = 0;
	INT Missing = 0;
	struct FDisHUDClipBinding { INT Index; const char* Path; UBOOL bFX; };
	static const FDisHUDClipBinding Bindings[] =
	{
		{ DHMC_CrosshairDot,       "_root._dot_mc",                                FALSE },
		{ DHMC_CrosshairInfo,  "_root.crosshairInfosTxt_mc",                   FALSE },
		{ DHMC_PlayerStatus,          "_root.masterHUD_mc",                           FALSE },
		{ DHMC_PlayerState,   "_root.playerStatesIcon_mc",                    FALSE },
		{ DHMC_StealthShroud,        "_root.blackShroud_mc",                         TRUE  },
		{ DHMC_CrosshairInteractions, "_root.interactions_window_mc",                 FALSE },
		{ DHMC_ContextInteractions,    "_root.interactions_txt_mc",                    FALSE },
		{ DHMC_SpecialInteractions,     "_root.interactions_ic_mc",                     FALSE },
		{ DHMC_SpecialInteractionsQTE,  "_root.interactions_icQTE_mc",                  FALSE },
		{ DHMC_BreathGauge,        "_root.oxygenGauge_mc",                         FALSE },
		{ DHMC_InteractionGauge,   "_root.interactionGauge_mc",                    FALSE },
		{ DHMC_GrenadeCookingGauge,     "_root.grenadeCooking_mc",                      FALSE },
		{ DHMC_PickupLog,          "_root.pickupLog_mc",                           FALSE },
		{ DHMC_GameMessages,            "_root.gameMsg_mc",                             FALSE },
		{ DHMC_Tutorials,        "_root.tutorialMsg_mc",                         FALSE },
		{ DHMC_TutorialWindow,     "_root.tutorialWindow_mc",                      FALSE },
		{ DHMC_LocationDiscovery,           "_root.location_mc",                            FALSE },
		{ DHMC_ObjectivePopup,         "_root.objectives_mc",                          FALSE },
		{ DHMC_SpecialNotification, "_root.targetNotification_mc",                  FALSE },
		{ DHMC_Damage, "_root.directionalDamages_mc",                  TRUE  },
		{ DHMC_DamageVignetteUp,          "_root.directionalDamages_mc._vignette_mc.U_mc", TRUE },
		{ DHMC_DamageVignetteDown,          "_root.directionalDamages_mc._vignette_mc.D_mc", TRUE },
		{ DHMC_DamageVignetteLeft,          "_root.directionalDamages_mc._vignette_mc.L_mc", TRUE },
		{ DHMC_DamageVignetteRight,          "_root.directionalDamages_mc._vignette_mc.R_mc", TRUE },
		{ DHMC_Keyhole,            "_root.keyHole_mc",                             TRUE  },
		{ DHMC_KeyholeRectangle1,         "_root._black_rectangle0_mc",                   TRUE  },
		{ DHMC_KeyholeRectangle2,         "_root._black_rectangle1_mc",                   TRUE  },
		{ DHMC_Cinematic,       "_root.blackStripes_mc",                        FALSE },
		{ DHMC_Subtitles,          "_root.subtitles_mc",                           FALSE },
		{ DHMC_Choice,       "_root.playerChoice_mc",                        FALSE },
		{ DHMC_SkipCutSceneGauge,       "_root.skipCutScene_mc",                        FALSE },
	};
	for( INT Index = 0; Index < ARRAY_COUNT(Bindings); Index++ )
	{
		const FDisHUDClipBinding& Binding = Bindings[Index];
		GFxMovieView* From = Binding.bFX ? FXView : View;
		if( DisHUDBindClip( From, Clips[Binding.Index], Binding.Path ) && !Clips[Binding.Index].IsUndefined() )
		{
			Bound++;
		}
		else
		{
			Missing++;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): HUD PostStart: no %s"),
				ANSI_TO_TCHAR(Binding.Path) );
		}
	}
	// retail's own unused slot
	Clips[DHMC_CrosshairAspect].SetNull();

	HUD->m_bPlayerLowOnHealth = FALSE;
	HUD->m_bBlockInteractionWindow = FALSE;
	HUD->m_bPlayerStatusVisible = FALSE;
	HUD->m_CrosshairName = NAME_None;
	HUD->m_CrosshairCurPos.X = Space.m_MovieSize.X * 0.5f;
	HUD->m_CrosshairCurPos.Y = Space.m_MovieSize.Y * 0.5f;
	HUD->m_CrosshairTargetPos = HUD->m_CrosshairCurPos;
	HUD->m_ViewportSizeX = -1;
	HUD->m_ViewportSizeY = -1;
	HUD->m_CrosshairDotState = 0;
	HUD->m_CrosshairAspectState = 0;
	HUD->m_fCrosshairDispersion = 3.4028235e38f;

	// the crosshair dot starts hidden, and the focus text's authored Y is measured off the movie
	GFxValue::DisplayInfo Hidden;
	Hidden.Clear();
	Hidden.SetVisible( false );
	Clips[DHMC_CrosshairDot].SetDisplayInfo( Hidden );
	GFxValue::DisplayInfo Measured;
	if( Clips[DHMC_CrosshairInfo].GetDisplayInfo( &Measured ) )
	{
		HUD->m_fCrosshairFocusTextPosY = (FLOAT)Measured.GetY();
	}
	HUD->m_bShowHighlightNextFrame = TRUE;
	HUD->m_bGameMessagesVisible = TRUE;
	HUD->m_CurrentTutorialID = -1;
	HUD->m_NextTutorialID = Max<INT>( HUD->m_NextTutorialID, 1 );
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)HUD->m_pTweaks;
	HUD->m_fPopupTimer = Tweaks ? Tweaks->m_fPopupDelayOnGameStart : 0.f;
	HUD->m_CurrentMainSubtitlePart = -1;
	HUD->m_CurrentSecondarySubtitlePart = -1;
	HUD->m_CurrentSubtitleType = -1;

	// the two offsets the layout pass anchors the context interactions and the pickup log with are
	// the distance between the authored positions of two of the movie's own clips.
	GFxValue::DisplayInfo A;
	GFxValue::DisplayInfo B;
	if( Clips[DHMC_PlayerStatus].GetDisplayInfo( &A ) && Clips[DHMC_ContextInteractions].GetDisplayInfo( &B ) )
	{
		HUD->m_ContextInteractionsOffset.X = (FLOAT)( B.GetX() - A.GetX() );
		HUD->m_ContextInteractionsOffset.Y = (FLOAT)( B.GetY() - A.GetY() );
	}
	if( Clips[DHMC_ObjectivePopup].GetDisplayInfo( &A ) && Clips[DHMC_PickupLog].GetDisplayInfo( &B ) )
	{
		HUD->m_PickupLogOffset.X = (FLOAT)( B.GetX() - A.GetX() );
		HUD->m_PickupLogOffset.Y = (FLOAT)( B.GetY() - A.GetY() );
	}
	if( Clips[DHMC_LocationDiscovery].GetDisplayInfo( &A ) && Space.m_ScreenSize.Y != 0.f )
	{
		HUD->m_fLocationDiscoveryVerticalAlignment = (FLOAT)A.GetY() / Space.m_ScreenSize.Y;
	}

	DisGFxMoviePlayerHUDApplyGameSettings( HUD, &ArkSettings::GetParameters() );

	debugf( TEXT("DISHONORED(bringup): HUD census: %d/%d clips bound, %d missing, movie %.0fx%.0f, ")
		TEXT("screen %.0fx%.0f, movie space %.0fx%.0f, empty %.1f/%.1f, scale %.4f, visibility %d"),
		Bound, ARRAY_COUNT(Bindings), Missing, Space.m_MovieSize.X, Space.m_MovieSize.Y,
		Space.m_ScreenSize.X, Space.m_ScreenSize.Y, Space.m_MovieSpaceSize.X, Space.m_MovieSpaceSize.Y,
		Space.m_EmptySpace.X, Space.m_EmptySpace.Y, Space.m_fScreenToMovieScaling,
		(INT)HUD->m_Settings.m_HUDVisibility );
}

// DISHONORED(port): 2013 rva 0x7af370 (2012 0x811620), the IArkSettingsListenerInterface override.
// DISHONORED(bringup): retail reaches it as a virtual; declaring the override here would mean adding
// it to CppText/UDisGFxMoviePlayerHUD.h and regenerating DishonoredGameUIClasses.h, which is agent
// DG's deviation 5 exactly (GFxUI/Inc/gfxuiengine.h), so it is called directly at the one retail call
// site instead - the ArkSettings::ApplyCurrentSettings at the end of PostStart.
void DisGFxMoviePlayerHUDApplyGameSettings( UDisGFxMoviePlayerHUD* HUD, const ArkSettingsParameters* Parameters )
{
	const UBOOL bWasHighlight = HUD->m_Settings.m_bShowHighlight != 0;

	HUD->m_Settings.m_HUDVisibility = (BYTE)Clamp<INT>( Parameters->m_HUDVisibility, 0, 2 );
	HUD->m_Settings.m_bShowObjectivePopups = Parameters->m_bShowObjectivePopups;
	HUD->m_Settings.m_bShowTutorialNotifications = Parameters->m_bShowTutorialNotifications;
	HUD->m_Settings.m_bShowInteractions = Parameters->m_bShowInteractions;
	HUD->m_Settings.m_bShowHighlight = Parameters->m_bShowHighlight;
	HUD->m_Settings.m_bShowPickupLog = Parameters->m_bShowPickupLog;
	HUD->m_Settings.m_bShowContextualIcons = Parameters->m_bShowContextualIcons;
	HUD->m_Settings.m_bShowPlayerStance = Parameters->m_bShowPlayerStance;
	HUD->m_Settings.m_bShowObjectiveMarkers = Parameters->m_bShowObjectiveMarkers;
	HUD->m_Settings.m_bShowGrenadeMarkers = Parameters->m_bShowGrenadeMarkers;
	HUD->m_Settings.m_bShowAwarenessMarkers = Parameters->m_bShowAwarenessMarkers;
	HUD->m_Settings.m_bShowHeartTargetMarkers = Parameters->m_bShowHeartTargetMarkers;
	HUD->m_Settings.m_CrosshairStyle = (BYTE)Clamp<INT>( Parameters->m_CrosshairStyle, 0, 2 );
	HUD->m_Settings.m_bCrosshairMovement = Parameters->m_bCrosshairMovement;
	HUD->m_Settings.m_fCrosshairOpacity = (FLOAT)Parameters->m_CrosshairOpacity;

	// DISHONORED(bringup): retail then takes down whatever the new settings turned off - the player
	// info (0x78cf30), the objective popups (0x7ad010), the pickup log (0x79f0d0) and the marker
	// instances (0x79f170) - and re-arms the heart's highlight when m_bShowHeartTargetMarkers came on.
	// Those five bodies are not ported, so a setting that changes at runtime is only obeyed by what
	// reads m_Settings on its next tick.
	if( bWasHighlight != ( HUD->m_Settings.m_bShowHighlight != 0 ) )
	{
		HUD->m_bShowHighlightNextFrame = HUD->m_Settings.m_bShowHighlight ? TRUE : FALSE;
		HUD->m_bHideHighlightNextFrame = HUD->m_Settings.m_bShowHighlight ? FALSE : TRUE;
	}
}

// DISHONORED(port): 2013 rva 0x7b2280 (2012 0x814ce0), retail vtable slot 119 (+476). The show-flag
// masks of the six levels are ANDed once here and every tick body downstream is gated on the result.
void DisGFxMoviePlayerHUDPreAdvance( UDisGFxMoviePlayerHUD* HUD, FLOAT DeltaTime )
{
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	ADishonoredHUD* HUDActor = PC ? Cast<ADishonoredHUD>( PC->myHUD ) : NULL;
	if( HUDActor == NULL || HUD->m_pMovieClips == NULL )
	{
		return;
	}
	INT ShowFlags = HUDActor->m_ShowFlags[0];
	for( INT MaskLevel = 1; MaskLevel < ARRAY_COUNT(HUDActor->m_ShowFlags); MaskLevel++ )
	{
		ShowFlags &= HUDActor->m_ShowFlags[MaskLevel];
	}
	HUD->m_ShowFlagsOnLastTick = ShowFlags;

	// the crosshair chases its target at the tweaked speed
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)HUD->m_pTweaks;
	const FLOAT Alpha = Tweaks ? Clamp<FLOAT>( Tweaks->m_fCrosshairInterpolationSpeed * DeltaTime, 0.f, 1.f ) : 1.f;
	HUD->m_CrosshairCurPos.X += ( HUD->m_CrosshairTargetPos.X - HUD->m_CrosshairCurPos.X ) * Alpha;
	HUD->m_CrosshairCurPos.Y += ( HUD->m_CrosshairTargetPos.Y - HUD->m_CrosshairCurPos.Y ) * Alpha;

	HUD->Tick_PlayerStatus( ShowFlags );
	HUD->Tick_PlayerState( ShowFlags );

	// DISHONORED(bringup): retail then runs Tick_Stealth (2013 0x796050), the interaction probe's five
	// checks, Tick_Interaction (0x78d3b0), Tick_Tutorials (0x7afa60), Tick_ObjectivePopup (0x7afe70),
	// Tick_Markers (0x7b00c0), Tick_Subtitles (0x79fe20) and Tick_Cinematic (0x7960e0). None of the
	// eight is ported by this package.
}

/*-----------------------------------------------------------------------------
	The bring-up: what UDisGlobalUIManager does for the HUD and nothing else.

	Retail's UDisGlobalUIManager::Init (2012 0x8c45d0) constructs one movie player per entry of the
	config movie set and RefreshGlobalUIState (2012 0x8b75e0) opens and closes them as the game moves
	between the menu and a level; UDisGlobalUIManager is not this package. What is reproduced here is
	only the HUD's share of that: find the movie set's HUD path, construct UDisGFxMoviePlayerHUD and
	UDisGFxMoviePlayerHUDFX, load the cooked packages the way
	UDisGFxMoviePlayerBase::LoadMoviePackage (2013 rva 0x7a3fd0) loads them, and call StartHUD.
	Everything from StartHUD onwards is the real path.

	It is behind -dishud so the regression harness's command lines, whose thresholds are calibrated
	without a movie open, are unchanged - the same reason -gfxuimenu is a switch (Core/Src/UnMisc.cpp).
-----------------------------------------------------------------------------*/

/** DISHONORED(port): the package/object split of UDisGFxMoviePlayerBase::LoadMoviePackage, 2013 rva
    0x7a3fd0: the file name gets the seek-free suffix, the object path does not. */
static USwfMovie* DisHUDLoadMovie( const FString& MoviePath )
{
	const INT Dot = MoviePath.InStr( TEXT(".") );
	FString PackageName = Dot > 0 ? MoviePath.Left( Dot ) : MoviePath;
	if( GUseSeekFreeLoading )
	{
		PackageName += TEXT("_SF");
	}
	FString Unused;
	if( GPackageFileCache->FindPackageFile( *PackageName, NULL, Unused ) )
	{
		UObject::LoadPackage( NULL, *PackageName, LOAD_None );
	}
	if( USwfMovie* Movie = FindObject<USwfMovie>( ANY_PACKAGE, *MoviePath ) )
	{
		return Movie;
	}
	// The cook can put the movie inside a group, so the object path carries one more level than the
	// config does. Every resident SwfMovie is logged once, which is what makes a wrong path visible.
	const INT LastDot = MoviePath.InStr( TEXT("."), TRUE );
	const FString WantName = LastDot >= 0 ? MoviePath.Mid( LastDot + 1 ) : MoviePath;
	USwfMovie* Found = NULL;
	for( TObjectIterator<USwfMovie> It; It; ++It )
	{
		if( It->RawData.Num() == 0 )
		{
			continue;
		}
		debugf( TEXT("DISHONORED(bringup): HUD: resident movie %s (%d bytes)"), *It->GetPathName(),
			It->RawData.Num() );
		if( Found == NULL && It->GetName() == WantName )
		{
			Found = *It;
		}
	}
	return Found;
}

static UGFxMoviePlayer* DisHUDConstructPlayer( UClass* PlayerClass, const TCHAR* Name, USwfMovie* Movie )
{
	UGFxMoviePlayer* Player = ConstructObject<UGFxMoviePlayer>( PlayerClass,
		UObject::GetTransientPackage(), FName( Name ) );
	Player->AddToRoot();
	Player->MovieInfo = Movie;
	// Retail's HUD is not focusable and takes no input: it is drawn over the game and the game keeps
	// the keyboard. bDisplayWithHudOff is what keeps it drawn while the engine's own HUD flag is off.
	Player->bAllowFocus = FALSE;
	Player->bAllowInput = FALSE;
	Player->bCaptureInput = FALSE;
	Player->bDisplayWithHudOff = TRUE;
	Player->TimingMode = TM_Game;
	Player->LocalPlayerOwnerIndex = 0;
	if( Player->ExternalInterface == NULL )
	{
		Player->ExternalInterface = Player;
	}
	return Player;
}

/** the one thing UDisGlobalUIManager owns that this package needs: the manager object itself, so
    DisGetGFxHUD() (2013 rva 0x7bf730, dishonoredutilities_accessors.cpp) answers and every body that
    reaches the HUD through it - here and in ADishonoredHUD::Tick - is retail's own lookup. */
static UDisGlobalUIManager* DisHUDGetOrCreateUIManager()
{
	UDisGlobalUIManager* UIManager = DisGetGlobalUIManager();
	if( UIManager != NULL )
	{
		return UIManager;
	}
	ADishonoredGameInfo* GameInfo = DisGetGameInfo();
	if( GameInfo == NULL )
	{
		return NULL;
	}
	// DISHONORED(bringup): retail constructs this in ADishonoredGameInfo's own start-up and calls
	// UDisGlobalUIManager::Init (2012 0x8c45d0) on it. Neither is ported; the object itself is a
	// config class, so its movie set comes from DisUI.ini exactly as retail's does.
	GameInfo->m_pGlobalUIManager = ConstructObject<UDisGlobalUIManager>(
		UDisGlobalUIManager::StaticClass(), UObject::GetTransientPackage(), TEXT("DisGlobalUIManager") );
	GameInfo->m_pGlobalUIManager->AddToRoot();
	return GameInfo->m_pGlobalUIManager;
}

/** -dishud: open the HUD once a world, a viewport and a player pawn exist. Returns TRUE once the
    movie is up, so the caller stops asking. */
static UBOOL DisHUDBringUp()
{
	static INT State = -2;      // -2 unread, -1 off or done, 0 waiting, 1 open
	if( State == -2 )
	{
		State = ParseParam( appCmdLine(), TEXT("dishud") ) ? 0 : -1;
	}
	if( State != 0 )
	{
		return State == 1;
	}
	if( GWorld == NULL || GEngine == NULL || GEngine->GameViewport == NULL
		|| ADishonoredPlayerController::s_pInstance == NULL
		|| ADishonoredPlayerPawn::s_pInstance == NULL )
	{
		return FALSE;
	}

	UDisGlobalUIManager* UIManager = DisHUDGetOrCreateUIManager();
	if( UIManager == NULL )
	{
		return FALSE;
	}
	if( UIManager->m_pHUD != NULL )
	{
		State = 1;
		return TRUE;
	}

	const FString HUDMoviePath = UIManager->m_DefaultUI.m_HUDMoviePath.Len() > 0
		? UIManager->m_DefaultUI.m_HUDMoviePath : FString( TEXT("UI_HUD.HUD") );
	USwfMovie* HUDMovie = DisHUDLoadMovie( HUDMoviePath );
	if( HUDMovie == NULL )
	{
		static UBOOL bComplained = FALSE;
		if( !bComplained )
		{
			bComplained = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): HUD: no SwfMovie '%s'"), *HUDMoviePath );
		}
		return FALSE;
	}

	UClass* HUDClass = UIManager->m_DefaultUI.m_HUDClassName.Len() > 0
		? FindObject<UClass>( ANY_PACKAGE, *UIManager->m_DefaultUI.m_HUDClassName ) : NULL;
	if( HUDClass == NULL || !HUDClass->IsChildOf( UDisGFxMoviePlayerHUD::StaticClass() ) )
	{
		HUDClass = UDisGFxMoviePlayerHUD::StaticClass();
	}
	UDisGFxMoviePlayerHUD* HUD = (UDisGFxMoviePlayerHUD*)DisHUDConstructPlayer( HUDClass,
		TEXT("DisGFxMoviePlayerHUD"), HUDMovie );

	// DISHONORED(bringup): retail's movie players are handed their tweaks out of the SF tweaks
	// package the movie set names (m_SFTweaksPackageName, "Twk_UI"), through IDisTweaksInterface.
	// Nothing in this build loads that package, so the tweak values come from the class default
	// object - which is the same fallback UDisGFxMoviePlayerBase::LoadMoviePackage itself uses when
	// GetTweaks_Derived answers null (2013 rva 0x7a3fd0).
	if( HUD->m_pTweaks == NULL )
	{
		HUD->m_pTweaks = (UDisTweaks_GFxMoviePlayerBase*)UDisTweaks_GFxMoviePlayerHUD::StaticClass()->GetDefaultObject();
	}
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)HUD->m_pTweaks;

	// the effects movie, whose eight clips PostStart binds
	USwfMovie* FXMovie = Tweaks ? Tweaks->m_pFXMovieInfo : NULL;
	if( FXMovie == NULL )
	{
		FXMovie = DisHUDLoadMovie( TEXT("UI_HUDFX.HUDFX") );
	}
	if( FXMovie != NULL )
	{
		HUD->m_pHudFX = (UDisGFxMoviePlayerHUDFX*)DisHUDConstructPlayer(
			UDisGFxMoviePlayerHUDFX::StaticClass(), TEXT("DisGFxMoviePlayerHUDFX"), FXMovie );
		if( HUD->m_pHudFX->m_pTweaks == NULL )
		{
			HUD->m_pHudFX->m_pTweaks = HUD->m_pTweaks;
		}
		if( Tweaks != NULL && Tweaks->m_pFXMovieInfo == NULL )
		{
			Tweaks->m_pFXMovieInfo = FXMovie;
		}
	}

	UIManager->m_pHUD = HUD;
	HUD->StartHUD();
	debugf( TEXT("DISHONORED(bringup): HUD: %s %s through %s (fx %s), tweaks %s safe area %.3f, ")
		TEXT("low health %d low mana %d"),
		HUD->bMovieIsOpen ? TEXT("opened") : TEXT("FAILED to open"), *HUDMovie->GetPathName(),
		*HUDClass->GetName(), FXMovie ? *FXMovie->GetPathName() : TEXT("none"),
		Tweaks ? *Tweaks->GetName() : TEXT("none"), Tweaks ? Tweaks->m_fSafeAreaRatio_PC : 0.f,
		Tweaks ? Tweaks->m_LowHealthThreshold : 0, Tweaks ? Tweaks->m_LowManaThreshold : 0 );
	if( !HUD->bMovieIsOpen )
	{
		UIManager->m_pHUD = NULL;
		State = -1;
		return FALSE;
	}
	State = 1;
	return TRUE;
}

// DISHONORED(port): ADishonoredPlayerController::PreRender, 2013 rva 0x6a0c50. Four instructions in
// retail: the UI manager, its HUD, the movie's open bit, and the HUD's PreRender.
// DISHONORED(bringup): retail reaches it as a native virtual of ADishonoredPlayerController that
// UGameViewportClient::Draw calls where Engine/Src/UnPlayer.cpp now has its
// `PreRender is not a 2013 event` line. Engine cannot call into DishonoredGame, so it arrives through
// the same function-pointer seam GDisEngineTickHook already uses (Engine/Inc/UnWorld.h).
static void DisPlayerControllerPreRender( UCanvas* Canvas )
{
	if( !DisHUDBringUp() )
	{
		return;
	}
	UDisGlobalUIManager* UIManager = DisGetGlobalUIManager();
	if( UIManager != NULL && UIManager->m_pHUD != NULL && UIManager->m_pHUD->bMovieIsOpen )
	{
		UIManager->m_pHUD->PreRender( Canvas );
	}
}

/** installs the PreRender seam. A file-scope constructor is safe here for the reason agent DG's
    FDisGFxMoviePlayerSeam is safe: it writes one zero-initialised pointer in another translation unit
    and reads nothing, in particular not appCmdLine(). */
static struct FDisHUDPreRenderSeam
{
	FDisHUDPreRenderSeam()
	{
		GDisPlayerPreRenderHook = &DisPlayerControllerPreRender;
	}
} GDisHUDPreRenderSeam;
