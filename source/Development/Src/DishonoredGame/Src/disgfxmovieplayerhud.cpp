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
#include "arkgameeventdispatcher.h"
#include "dishudmarkers.h"

/*-----------------------------------------------------------------------------
	Agent EK (PHASE11 EK): the in-game HUD.

	The order retail brings this up in is UDisGlobalUIManager::RefreshGlobalUIState ->
	UDisGFxMoviePlayerHUD::StartHUD (2013 rva 0x78cd00) -> UDisGFxMoviePlayerBase::Start (0x7a3f40)
	-> UGFxMoviePlayer::Start -> PostStart (0x795650); then every frame
	UDisGFxMoviePlayerBase::Advance (0x787680) -> PreAdvance (0x7b2280) -> Tick_PlayerStatus
	(0x79f6e0) for the numbers, and ADishonoredPlayerController::PreRender (0x6a0c50) -> PreRender
	(0x7a5840) -> PreRender_Layout (0x796250) for where they sit.

	Everything below is that chain, body for body. The HUD is constructed and started by
	UDisGlobalUIManager::Init (disglobaluimanager.cpp, agent FP); the bring-up that used to stand in for it
	here is gone, and the banner above DisPlayerControllerPreRender says why.
-----------------------------------------------------------------------------*/


void DisGFxMoviePlayerHUDPostStart( class UDisGFxMoviePlayerHUD* HUD );
void DisGFxMoviePlayerHUDPreAdvance( class UDisGFxMoviePlayerHUD* HUD, FLOAT DeltaTime );

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
	// DISHONORED(port): agent HE. Retail's four gates in order: not possessing (2013 0x74a550, APawn virtual +1188),
	// sneaking (vtable +1660, 2012 names it ADishonoredPlayerPawn::IsSneaking), element 0xE00 and the stance setting;
	// then slide (IsSliding) over crouch (APawn+640 & 0xC: bWantsToCrouch | bIsCrouched) over sneak. Retail does not
	// test the pawn for NULL.
	if( Pawn != NULL && !Pawn->DisIsPossessing() && Pawn->IsSneaking() && ( _ShowFlags & 0xE00 ) != 0
		&& m_Settings.m_bShowPlayerStance )
	{
		if( Pawn->IsSliding() )
		{
			State = 3;
		}
		else
		{
			State = ( Pawn->bWantsToCrouch || Pawn->bIsCrouched ) ? 2 : 1;
		}
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
	if( IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): phuddiag SetIcon(%d) flags 0x%04X t=%.2f"), (INT)State, _ShowFlags,
			GWorld ? GWorld->GetTimeSeconds() : 0.f );
	}
	Result.ReleaseManaged();
	Arg.ReleaseManaged();
	m_PlayerState = State;
}

/**
 * DISHONORED(port): 2013 rva 0x7960e0 (358 bytes). The letterbox bars. Element 0x10 is the one bit
 * ADishonoredPlayerController::SetCinematicMode_Native turns on that the non-cinematic set does not carry,
 * IsHUDElementEnabledOnce answers "on at any of the six mask levels", and the movie's own
 * _root.blackStripes_mc.SetBlackStripes( bShow, TRUE ) draws them. m_bCinematicMode (this+508 bit 0x40000)
 * is the edge latch, so the method is invoked only when the bit changes - retail's body is that `if` and
 * nothing else.
 *
 * The clip is Clips[DHMC_Cinematic]: retail reads the GFxValue at m_pMovieClips + 448, DHMC_Cinematic is 28
 * and a GFxValue is 16 bytes. SetBlackStripes is in the cooked movie (two occurrences in
 * UI_HUD_SF.UI_HUD.HUD.gfx).
 */
void UDisGFxMoviePlayerHUD::Tick_Cinematic()
{
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	ADishonoredHUD* HUDActor = PC ? Cast<ADishonoredHUD>( PC->myHUD ) : NULL;
	if( HUDActor == NULL || m_pMovieClips == NULL )
	{
		return;
	}
	const UBOOL bStripes = HUDActor->IsHUDElementEnabledOnce( 0x10 );
	if( bStripes == ( m_bCinematicMode != 0 ) )
	{
		return;
	}
	GFxValue Args[2];
	Args[0].SetBoolean( bStripes != 0 );
	Args[1].SetBoolean( TRUE );
	GFxValue Result;
	const UBOOL bInvoked = DisHUDClips( this )[DHMC_Cinematic].Invoke( "SetBlackStripes", &Result, Args, 2 );
	Result.ReleaseManaged();
	Args[0].ReleaseManaged();
	Args[1].ReleaseManaged();
	m_bCinematicMode = bStripes ? TRUE : FALSE;
	debugf( TEXT("DISHONORED(bringup): HUD: SetBlackStripes(%d) %s"), (INT)bStripes,
		bInvoked ? TEXT("ok") : TEXT("NOT FOUND") );
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
	// DISHONORED(port): agent HE. Retail (2013 0x79f6e0, 2012 0x803950) assigns this flag in each of the three blocks
	// below rather than OR-ing it, so the last block that changed decides.
	UBOOL bBecameValid = FALSE;
	if( HUDActor != NULL && IsDiagEnabled() )
	{
		static INT LastLevels[ARRAY_COUNT(HUDActor->m_ShowFlags)] = { -1, -1, -1, -1, -1, -1 };
		if( appMemcmp( LastLevels, HUDActor->m_ShowFlags, sizeof(LastLevels) ) != 0 )
		{
			appMemcpy( LastLevels, HUDActor->m_ShowFlags, sizeof(LastLevels) );
			debugf( TEXT("DISHONORED(bringup): phuddiag masks %04X/%04X/%04X/%04X/%04X/%04X and 0x%04X, player status %d, t=%.2f"),
				LastLevels[0], LastLevels[1], LastLevels[2], LastLevels[3], LastLevels[4], LastLevels[5], _ShowFlags,
				(INT)m_bPlayerStatusVisible, GWorld ? GWorld->GetTimeSeconds() : 0.f );
		}
	}

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
		bBecameValid = m_Mana.m_ManaMax >= 0;
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
		bBecameValid = m_Equipment.m_pItem != NULL;
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
	// All four project world positions through the canvas; of them only PreRender_Markers is ported (agent HI).
	PreRender_Markers( _pCanvas );
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

	// DISHONORED(port): agent HE. 2013 hides masterHUD_mc here with a DisplayInfo whose only set var is
	// V_visible = false (0x795650: SetDisplayInfo on m_pMovieClips + 48, VarsSet 0x40, Visible at +48 = 0), where
	// 2012 (0x7fb450) invoked masterHUD_mc.Close instead. The movie's own start state is open, so without this the
	// vials of a freshly started HUD stay drawn until a Close is sent; PlayerGauges.Open sets _visible back.
	GFxValue::DisplayInfo Hidden;
	Hidden.Clear();
	Hidden.SetVisible( false );
	Clips[DHMC_PlayerStatus].SetDisplayInfo( Hidden );
	// DISHONORED(bringup): retail does not hide the dot here; PreRender_Crosshair (2013 0x79ff30) owns it and is
	// unported, so the dot stays hidden instead.
	Clips[DHMC_CrosshairDot].SetDisplayInfo( Hidden );
	GFxValue::DisplayInfo Measured;
	if( Clips[DHMC_CrosshairInfo].GetDisplayInfo( &Measured ) )
	{
		HUD->m_fCrosshairFocusTextPosY = (FLOAT)Measured.GetY();
	}
	// DISHONORED(port): agent HE. 2013 `this+508 = bits & 0xFFFFF2FF | 0x500`: m_bGameMessagesVisible and
	// m_bSystemicTutorialsEnabled set, m_bTutorialWindowSet cleared; m_bShowHighlightNextFrame is not touched.
	HUD->m_bGameMessagesVisible = TRUE;
	HUD->m_bSystemicTutorialsEnabled = TRUE;
	HUD->m_bTutorialWindowSet = FALSE;
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

	ArkSettings::ApplyCurrentSettings( HUD );

	debugf( TEXT("DISHONORED(bringup): HUD census: %d/%d clips bound, %d missing, movie %.0fx%.0f, ")
		TEXT("screen %.0fx%.0f, movie space %.0fx%.0f, empty %.1f/%.1f, scale %.4f, visibility %d"),
		Bound, ARRAY_COUNT(Bindings), Missing, Space.m_MovieSize.X, Space.m_MovieSize.Y,
		Space.m_ScreenSize.X, Space.m_ScreenSize.Y, Space.m_MovieSpaceSize.X, Space.m_MovieSpaceSize.Y,
		Space.m_EmptySpace.X, Space.m_EmptySpace.Y, Space.m_fScreenToMovieScaling,
		(INT)HUD->m_Settings.m_HUDVisibility );
}

// DISHONORED(port): agent HE, 2013 rva 0x7af820 (574 B; 2012 0x811e40), the vtable slot after PostStart. The clip
// handles are freed while the view is still alive and every cache the ticks compare against goes back to "never sent"
// (-1 / NULL), so the first tick of the next start counts nothing as a change. Retail keeps m_bTickedOnce and
// m_bPlayerStatusVisible here (PostStart clears the latter).
void UDisGFxMoviePlayerHUD::PreClose( UBOOL _bUnload )
{
	m_ShowFlagsOnLastTick = 0;
	delete[] (GFxValue*)m_pMovieClips;
	m_pMovieClips = NULL;
	m_bStealthShroudVisible = FALSE;

	m_Health.m_Health = -1;
	m_Health.m_HealthRegen = -1;
	m_Health.m_HealthMax = -1;
	m_Health.m_bHasSmallUpgrade = FALSE;
	m_Health.m_bHasLargeUpgrade = FALSE;
	m_Mana.m_Mana = -1;
	m_Mana.m_ManaRegen = -1;
	m_Mana.m_ManaMax = -1;
	m_Mana.m_bVisible = FALSE;
	m_Mana.m_bAvailable = FALSE;
	m_Mana.m_bHasUpgrade = FALSE;
	m_Equipment.m_pItem = NULL;
	// 2013 writes 36 (2012: 37)
	m_Equipment.m_Item = DPWI_MAX;
	m_Equipment.m_bPowerActive = FALSE;
	m_Equipment.m_bPowerHasMana = FALSE;
	m_Equipment.m_AmmoTotal = 0;
	m_Equipment.m_LoadedAmmoCount = 0;
	m_PlayerState = 0;

	m_InteractableName.Empty();
	for( INT GroupIndex = 0; GroupIndex < ARRAY_COUNT(m_InteractionContext.m_Groups); GroupIndex++ )
	{
		m_InteractionContext.m_Groups[GroupIndex].m_InteractionCount = 0;
	}
	m_InteractionProbe.m_bCanAssassinate = 0;
	m_InteractionProbe.m_bCanDropAssassinate = FALSE;
	m_InteractionProbe.m_bCanAdrenalineKill = 0;
	m_InteractionProbe.m_bCanMantle = 0;
	m_ActiveGauge = 0;
	m_fCurrentCyclingTutorialTime = 0.f;
	m_ActivePopup.m_pObjective = NULL;
	m_ActivePopup.m_Event = 0;
	m_ActivePopup.m_Tasks.Empty();
	m_ActivePopup.m_pTargetNotifSeq = NULL;
	m_PopupQueue.Reset();
	// DISHONORED(port): agent HI - retail's ClearMarkerInstances (2012 0x804f70, a vtable call at +516 in 2013)
	ClearMarkerInstances();
	m_bPlayerBustedThisFrame = FALSE;
	m_bPlayerBustedLastFrame = FALSE;
	m_LastDamageInfo = FDisHUDDamageInfo(EC_EventParm);
	m_KeyholeRotation = FRotator(0, 0, 0);
	m_MainSubtitleParts.Reset();
	m_SecondarySubtitleParts.Reset();
	m_bCinematicMode = FALSE;
	m_bChoiceSelection = FALSE;
	if( IsDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): phuddiag PreClose(%d): caches reset, player status %d, ticked once %d"),
			(INT)_bUnload, (INT)m_bPlayerStatusVisible, (INT)m_bTickedOnce );
	}
}

UBOOL UDisGFxMoviePlayerHUD::IsDiagEnabled()
{
	static INT State = -1;
	if( State < 0 )
	{
		State = ParseParam( appCmdLine(), TEXT("phuddiag") ) ? 1 : 0;
	}
	return State == 1;
}

// DISHONORED(port): 2013 rva 0x7af370 (2012 0x811620), the IArkSettingsListenerInterface override. Agent EQ
// made it the virtual retail has - the declaration is in CppText/UDisGFxMoviePlayerHUD.h and the one call site
// is retail's own ArkSettings::ApplyCurrentSettings at the end of PostStart - and added retail's reason test:
// the whole body is skipped for ASLI_ModifiedByUser, so moving a slider does not disturb the HUD and only
// leaving the options screen (ASLI_ValidatedByUser) or a profile read does.
void UDisGFxMoviePlayerHUD::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	if( Reason == ASLI_ModifiedByUser )
	{
		return;
	}
	UDisGFxMoviePlayerHUD* HUD = this;
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
	// DISHONORED(bringup): retail's order (2013 0x7b2280, read by agent HF) is Tick_PlayerStatus, Tick_PlayerState,
	// Tick_Stealth (0x796050), the interaction probe's five checks inlined (CheckDropAssassinate, CheckAdrenalineKill,
	// 0x7a50f0, 0x7a5250, CheckMantle), Tick_GameMessages (0x78d3b0), Tick_Tutorials (0x7afa60), Tick_ObjectivePopup
	// (0x7afe70), Tick_Markers (0x7b00c0), Tick_Subtitles (0x79fe20), Tick_Cinematic (0x7960e0). Tick_Stealth, the
	// probe, Tick_Markers and Tick_Subtitles are not ported; the rest run here in that order. 0x78d3b0 is not the
	// Tick_Interaction the earlier comment named: it is gameMsg_mc's _visible from show bit 0x2000. Tick_Interaction
	// (2013 0x7acb10, 2012 0x80c450) is not ticked at all - OnCrosshairActorSet calls it.
	HUD->Tick_GameMessages( ShowFlags );
	HUD->Tick_Tutorials( DeltaTime, ShowFlags );
	HUD->Tick_ObjectivePopup( DeltaTime );
	// DISHONORED(port): agent HI - Tick_Markers (2013 0x7b00c0) in retail's place; Tick_Subtitles is still not ported
	HUD->Tick_Markers( DeltaTime );
	HUD->Tick_Cinematic();
}

/*-----------------------------------------------------------------------------
	The location banner, the objective popup and the tutorial line. Agent HF, wave 25.

	The three are fed from Kismet or the objective system and drawn by the movie's own clips: location_mc's
	SetLocation, objectives_mc's SetObjectives and tutorialMsg_mc's SetTutorialMessage. Every call into the
	movie goes through DisHUDInvoke, which refuses an unbound clip (these run every frame of every map) and
	is what -puidiag logs.
-----------------------------------------------------------------------------*/

/** -puidiag: every HUD Invoke this section makes, with its first argument, and every Kismet feeder */
static UBOOL DisHUDUIDiag()
{
	static const UBOOL bDiag = ParseParam( appCmdLine(), TEXT("puidiag") );
	return bDiag;
}

static UBOOL DisHUDInvoke( UDisGFxMoviePlayerHUD* HUD, INT ClipIndex, const char* Method, const GFxValue* Args, UINT ArgCount )
{
	GFxValue* Clips = DisHUDClips( HUD );
	const UBOOL bBound = Clips != NULL && !Clips[ClipIndex].IsUndefined() && !Clips[ClipIndex].IsNull();
	UBOOL bInvoked = FALSE;
	if( bBound )
	{
		GFxValue Result;
		bInvoked = Clips[ClipIndex].Invoke( Method, &Result, Args, ArgCount );
	}
	if( DisHUDUIDiag() )
	{
		const TCHAR* FirstArg = ( ArgCount > 0 && Args[0].IsStringW() && Args[0].GetStringW() ) ? Args[0].GetStringW() : TEXT("");
		debugf( TEXT("DISHONORED(bringup): puidiag clip %d.%s('%s', %d args) -> %s"), ClipIndex, ANSI_TO_TCHAR(Method),
			FirstArg, (INT)ArgCount, !bBound ? TEXT("CLIP NOT BOUND") : bInvoked ? TEXT("ok") : TEXT("NOT FOUND") );
	}
	return bInvoked;
}

// DISHONORED(port): 2013 rva 0x795160 (2012 0x7fb080, same 425 bytes): location_mc.SetLocation( <name>, the tweaks'
// m_fLocationDiscoveryDuration ). The movie draws, holds and fades the card itself; where it sits on the screen is
// PreRender_Layout's (m_fLocationDiscoveryVerticalAlignment, measured off the clip in PostStart).
// DISHONORED(bringup): retail then posts m_LocationDiscoverySoundEvent through UAkAudioDevice::PostEvent when
// _bPlaySound; audio is deferred (PHASE22), so the event is only named.
void UDisGFxMoviePlayerHUD::ShowLocationDiscovery( const FString& _rLocationName, UBOOL _bPlaySound )
{
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	if( Tweaks == NULL )
	{
		return;
	}
	GFxValue Args[2];
	Args[0].SetStringW( *_rLocationName );
	Args[1].SetNumber( Tweaks->m_fLocationDiscoveryDuration );
	const UBOOL bInvoked = DisHUDInvoke( this, DHMC_LocationDiscovery, "SetLocation", Args, 2 );
	debugf( TEXT("HUD: ShowLocationDiscovery '%s' sound %d"), *_rLocationName, _bPlaySound ? 1 : 0 );
	debugf( TEXT("DISHONORED(bringup): HUD location card: duration %.2f, event %s, SetLocation %s"),
		Tweaks->m_fLocationDiscoveryDuration,
		Tweaks->m_LocationDiscoverySoundEvent ? *((UObject*)Tweaks->m_LocationDiscoverySoundEvent)->GetName() : TEXT("none"),
		bInvoked ? TEXT("ok") : TEXT("failed") );
}

// DISHONORED(port): 2013 rva 0x7998a0 (2012 0x7ff700, same 42 bytes)
// DISHONORED(bringup): retail dereferences DisGetGFxHUD() unguarded; under -nodishud this tree has no HUD.
void UDisSeqAct_ShowLocationDiscovery::Activated()
{
	if( UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD() )
	{
		HUD->ShowLocationDiscovery( m_LocationName, !m_bNoSound );
	}
	USequenceAction::Activated();
}

// DISHONORED(port): 2013 rva 0x78d3b0, new in 2013 (no 2012 body, so no PDB name): gameMsg_mc._visible follows show bit
// 0x2000, sent on an edge of m_bGameMessagesVisible only.
void UDisGFxMoviePlayerHUD::Tick_GameMessages( INT _ShowFlags )
{
	const UBOOL bVisible = ( _ShowFlags >> 13 ) & 1;
	GFxValue* Clips = DisHUDClips( this );
	if( bVisible == ( m_bGameMessagesVisible != 0 ) || Clips[DHMC_GameMessages].IsUndefined() )
	{
		return;
	}
	GFxValue Visible;
	Visible.SetBoolean( bVisible != 0 );
	Clips[DHMC_GameMessages].SetMember( "_visible", Visible );
	m_bGameMessagesVisible = bVisible;
}

/*-----------------------------------------------------------------------------
	The objective popup.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x79ee60 (2012 0x802fd0): a task already in the popup is moved to its end
void FDisObjectivePopupInfo::AddTask( const FDisTaskPopupInfo& _rTask )
{
	for( INT Index = m_Tasks.Num() - 1; Index >= 0; Index-- )
	{
		if( m_Tasks(Index).m_pTask == _rTask.m_pTask )
		{
			m_Tasks.Remove( Index );
		}
	}
	m_Tasks.AddItem( _rTask );
}

// DISHONORED(port): 2013 rva 0x7b2480 (2012 0x814f50). Queued only while m_bHandleObjectiveEvents. The one caller is
// UDishonoredObjectivesComponent::OnGotObjectiveNotify (2013 0x6cc050).
void UDisGFxMoviePlayerHUD::OnObjectiveAdded( UDishonoredObjective* _pObjective )
{
	if( !m_bHandleObjectiveEvents )
	{
		return;
	}
	FDisObjectivePopupInfo Popup( EC_EventParm );
	Popup.m_pObjective = _pObjective;
	Popup.m_Event = 0;
	m_PopupQueue.AddItem( Popup );
}

// DISHONORED(port): 2013 rva 0x7b27c0 (2012 0x815290). A task joins its objective's queued "new"/"updated" popup, or
// queues an "updated" popup of its own. The objective is the task's Outer.
void UDisGFxMoviePlayerHUD::OnTaskAdded( UDishonoredTask_Base* _pTask )
{
	if( !m_bHandleObjectiveEvents )
	{
		return;
	}
	UDishonoredObjective* Objective = (UDishonoredObjective*)_pTask->GetOuter();
	FDisTaskPopupInfo Task;
	Task.m_pTask = _pTask;
	Task.m_Event = 0;
	for( INT Index = 0; Index < m_PopupQueue.Num(); Index++ )
	{
		FDisObjectivePopupInfo& Queued = m_PopupQueue(Index);
		if( Queued.m_pObjective == Objective && Queued.m_Event < 2 )
		{
			Queued.AddTask( Task );
			return;
		}
	}
	FDisObjectivePopupInfo Popup( EC_EventParm );
	Popup.m_pObjective = Objective;
	Popup.m_Event = 1;
	Popup.AddTask( Task );
	m_PopupQueue.AddItem( Popup );
}

// ---- agent HI (PHASE22 HI): the rest of the objective notifications ----

// DISHONORED(port): 2013 rva 0x787ec0 (2012 0x7f49c0). An objective action that asks for no HUD notification turns the
// popup feeders off while it runs.
void UDisGFxMoviePlayerHUD::HandleObjectiveEvents( UBOOL _bHandle )
{
	m_bHandleObjectiveEvents = _bHandle ? 1 : 0;
}

/** OnObjectiveCompleted / OnObjectiveFailed: the objective's queued "updated" popups fold into one finished popup that keeps
	the tasks that finished the same way */
static void DisHUDQueueFinishedObjective( TArray<FDisObjectivePopupInfo>& _rQueue, UDishonoredObjective* _pObjective, BYTE _Event )
{
	FDisObjectivePopupInfo Popup( EC_EventParm );
	Popup.m_pObjective = _pObjective;
	Popup.m_Event = _Event;
	for( INT Index = _rQueue.Num() - 1; Index >= 0; Index-- )
	{
		if( _rQueue(Index).m_pObjective != _pObjective || _rQueue(Index).m_Event != 1 )
		{
			continue;
		}
		for( INT TaskIdx = 0; TaskIdx < _rQueue(Index).m_Tasks.Num(); TaskIdx++ )
		{
			if( _rQueue(Index).m_Tasks(TaskIdx).m_Event == _Event )
			{
				Popup.AddTask( _rQueue(Index).m_Tasks(TaskIdx) );
			}
		}
		_rQueue.Remove( Index );
	}
	_rQueue.AddItem( Popup );
}

// DISHONORED(port): 2013 rva 0x7b2500 (2012 0x814fd0)
void UDisGFxMoviePlayerHUD::OnObjectiveCompleted( UDishonoredObjective* _pObjective )
{
	if( m_bHandleObjectiveEvents )
	{
		DisHUDQueueFinishedObjective( m_PopupQueue, _pObjective, 2 );
	}
}

// DISHONORED(port): 2013 rva 0x7b2660 (2012 0x815130)
void UDisGFxMoviePlayerHUD::OnObjectiveFailed( UDishonoredObjective* _pObjective )
{
	if( m_bHandleObjectiveEvents )
	{
		DisHUDQueueFinishedObjective( m_PopupQueue, _pObjective, 3 );
	}
}

// DISHONORED(port): 2013 rva 0x7b28e0 (2012 0x8153b0): joins the objective's queued "updated" popup or queues one.
void UDisGFxMoviePlayerHUD::OnTaskUpdated( UDishonoredTask_Base* _pTask )
{
	if( !m_bHandleObjectiveEvents )
	{
		return;
	}
	UDishonoredObjective* Objective = (UDishonoredObjective*)_pTask->GetOuter();
	FDisTaskPopupInfo Task;
	Task.m_pTask = _pTask;
	Task.m_Event = 1;
	for( INT Index = 0; Index < m_PopupQueue.Num(); Index++ )
	{
		if( m_PopupQueue(Index).m_pObjective == Objective && m_PopupQueue(Index).m_Event == 1 )
		{
			m_PopupQueue(Index).AddTask( Task );
			return;
		}
	}
	FDisObjectivePopupInfo Popup( EC_EventParm );
	Popup.m_pObjective = Objective;
	Popup.m_Event = 1;
	Popup.AddTask( Task );
	m_PopupQueue.AddItem( Popup );
}

// DISHONORED(port): 2013 rva 0x7b2a00 (2012 0x8154d0): joins the objective's queued "updated" or "completed" popup or
// queues an "updated" one.
void UDisGFxMoviePlayerHUD::OnTaskCompleted( UDishonoredTask_Base* _pTask )
{
	if( !m_bHandleObjectiveEvents )
	{
		return;
	}
	UDishonoredObjective* Objective = (UDishonoredObjective*)_pTask->GetOuter();
	FDisTaskPopupInfo Task;
	Task.m_pTask = _pTask;
	Task.m_Event = 2;
	for( INT Index = 0; Index < m_PopupQueue.Num(); Index++ )
	{
		FDisObjectivePopupInfo& Queued = m_PopupQueue(Index);
		if( Queued.m_pObjective == Objective && ( Queued.m_Event == 1 || Queued.m_Event == 2 ) )
		{
			Queued.AddTask( Task );
			return;
		}
	}
	FDisObjectivePopupInfo Popup( EC_EventParm );
	Popup.m_pObjective = Objective;
	Popup.m_Event = 1;
	Popup.AddTask( Task );
	m_PopupQueue.AddItem( Popup );
}

// DISHONORED(port): 2013 rva 0x7b2b20 (2012 0x8155f0): as OnTaskCompleted, except that an objective already queued as
// completed swallows the failed task.
void UDisGFxMoviePlayerHUD::OnTaskFailed( UDishonoredTask_Base* _pTask )
{
	if( !m_bHandleObjectiveEvents )
	{
		return;
	}
	UDishonoredObjective* Objective = (UDishonoredObjective*)_pTask->GetOuter();
	FDisTaskPopupInfo Task;
	Task.m_pTask = _pTask;
	Task.m_Event = 3;
	for( INT Index = 0; Index < m_PopupQueue.Num(); Index++ )
	{
		FDisObjectivePopupInfo& Queued = m_PopupQueue(Index);
		if( Queued.m_pObjective != Objective )
		{
			continue;
		}
		if( Queued.m_Event == 1 || Queued.m_Event == 3 )
		{
			Queued.AddTask( Task );
			return;
		}
		if( Queued.m_Event == 2 )
		{
			return;
		}
	}
	FDisObjectivePopupInfo Popup( EC_EventParm );
	Popup.m_pObjective = Objective;
	Popup.m_Event = 1;
	Popup.AddTask( Task );
	m_PopupQueue.AddItem( Popup );
}

// DISHONORED(port): 2013 rva 0x78d770 (2012 0x7f4c10)
FLOAT FDisHUDMarkerVariation::ComputeValue( FLOAT _fDistance ) const
{
	const FLOAT Range = m_EndDistance - m_StartDistance;
	if( Range == 0.f )
	{
		return m_EndValue;
	}
	return m_StartValue + ( m_EndValue - m_StartValue ) * Clamp( ( _fDistance - m_StartDistance ) / Range, 0.f, 1.f );
}

/** -hidiag: one line per task marker every second */
static UBOOL DisHUDMarkerDiagDue()
{
	static DOUBLE LastTime = 0.0;
	if( !UDishonoredObjectivesComponent::IsDiagEnabled() || appSeconds() - LastTime < 1.0 )
	{
		return FALSE;
	}
	LastTime = appSeconds();
	return TRUE;
}

// DISHONORED(port): 2013 rva 0x7b00c0 (2012 0x8122c0). UpdateAwarenessMarkerInstances (2013 0x7ad960) is not ported:
// the awareness markers above NPCs need the AI awareness state this tree does not keep; m_AwarenessMarkers stays empty.
void UDisGFxMoviePlayerHUD::Tick_Markers( FLOAT _fDeltaTime )
{
	UpdateTaskMarkerInstances();
	for( INT Index = 0; Index < m_SortedMarkers.Num(); Index++ )
	{
		((FDisHUDMarker*)m_SortedMarkers(Index))->Tick( _fDeltaTime );
	}
}

// DISHONORED(port): 2013 rva 0x7ad550 (2012 0x80cd40; no PDB name in 2013). One marker per active task target in the
// targets' order, created on first sight (the optional-target settings for an optional target), kept by tracked actor,
// and destroyed once its target is gone. 2013 reads no targets while the objective-marker option is off, and keeps a
// hidden pawn's marker hidden.
void UDisGFxMoviePlayerHUD::UpdateTaskMarkerInstances()
{
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	if( PlayerController == NULL || PlayerController->m_pObjectivesComponent == NULL || Tweaks == NULL )
	{
		return;
	}
	FMemMark Mark( GMainThreadMemStack );
	TArray<const FDisTaskTarget*, TMemStackAllocator<GMainThreadMemStack> > Targets;
	if( m_Settings.m_bShowObjectiveMarkers )
	{
		PlayerController->m_pObjectivesComponent->FindActiveTaskTargets( Targets, TRUE );
	}
	INT Index = 0;
	for( ; Index < Targets.Num(); Index++ )
	{
		const FDisTaskTarget* Target = Targets(Index);
		INT Found = -1;
		for( INT MarkerIdx = Index; MarkerIdx < m_TaskMarkers.Num(); MarkerIdx++ )
		{
			if( ((FDisHUDMarker*)m_TaskMarkers(MarkerIdx))->m_pTrackedActor == Target->m_pActor )
			{
				Found = MarkerIdx;
				break;
			}
		}
		if( Found == -1 )
		{
			const FDisHUDMarkerSettings* Settings = Target->m_bOptional ? &Tweaks->m_OptionalTaskMarkerSettings : &Tweaks->m_TaskMarkerSettings;
			FDisTaskMarker* Marker = new FDisTaskMarker( Target, Settings, GetMovie() );
			m_TaskMarkers.InsertItem( (FPointer)Marker, Index );
			m_SortedMarkers.AddItem( (FPointer)Marker );
			if( UDishonoredObjectivesComponent::IsDiagEnabled() )
			{
				debugf( TEXT("DISHONORED(bringup): hidiag marker add %s '%s' symbol %s depth %d instance %s"), *Target->m_pActor->GetName(), *Marker->m_TargetName,
					*Settings->m_SymbolName, Marker->m_Depth, Marker->m_GFxInstance.IsDisplayObject() ? TEXT("ok") : TEXT("NOT ATTACHED") );
			}
		}
		else if( Found != Index )
		{
			Exchange( m_TaskMarkers(Index), m_TaskMarkers(Found) );
		}
		const APawn* Pawn = Cast<APawn>( Target->m_pActor );
		if( Pawn )
		{
			((FDisHUDMarker*)m_TaskMarkers(Index))->m_bTrackedActorVisible = !Pawn->bHidden;
		}
	}
	while( Index < m_TaskMarkers.Num() )
	{
		FDisHUDMarker* Marker = (FDisHUDMarker*)m_TaskMarkers(Index);
		if( UDishonoredObjectivesComponent::IsDiagEnabled() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag marker remove %s"), Marker->m_pTrackedActor ? *Marker->m_pTrackedActor->GetName() : TEXT("None") );
		}
		m_SortedMarkers.RemoveItem( (FPointer)Marker );
		delete Marker;
		m_TaskMarkers.RemoveSwap( Index );
	}
}

// DISHONORED(port): 2013 rva 0x7a0890 (2012 0x8049d0). Each marker's world point (its GetLocation raised by the settings'
// world offset) is projected into movie space, clamped into the marker area (and, with m_bKeepBBInMarkerArea, kept
// inside it by its bounding box scaled with distance), and the visible focusable marker nearest the screen centre
// within the focus distance is focused. Nothing is placed unless the markers' show flag (0x20) is set.
// DISHONORED(bringup): retail shrinks the spread while zoomed by UDisPostProcessManager's zoom-lens blend (2013 0x7eb020
// on ADishonoredGameInfo::m_pPpManager), whose fields at +412 and the effect state at +457 this tree does not model; the
// zoom is taken as 0 (no spyglass on the first mission's route).
void UDisGFxMoviePlayerHUD::PreRender_Markers( UCanvas* _pCanvas )
{
	const INT NumMarkers = m_SortedMarkers.Num();
	if( NumMarkers == 0 )
	{
		return;
	}
	FMemMark Mark( GMainThreadMemStack );
	FDisHUDMarkerPosInfo* PosInfos = new( GMainThreadMemStack, MEM_Zeroed, NumMarkers ) FDisHUDMarkerPosInfo;
	INT FocusedIndex = -1;
	ADishonoredPlayerController* PlayerController = ADishonoredPlayerController::s_pInstance;
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	if( ( m_ShowFlagsOnLastTick & 0x20 ) && PlayerController && PlayerController->Pawn && Tweaks && _pCanvas && _pCanvas->SceneView )
	{
		const FDisMovieSpaceInfo& Space = m_MovieSpaceInfoOnLastPreRender;
		const FLOAT CentreX = Space.m_MovieSize.X * 0.5f;
		const FLOAT CentreY = Space.m_MovieSize.Y * 0.5f;
		const FLOAT AreaRatio = Tweaks->m_fMarkerAreaRatio_PC;
		FLOAT BestFocus = Tweaks->m_fMarkerFocusDistance * Tweaks->m_fMarkerFocusDistance;
		const FLOAT MarginX = Space.m_MovieSpaceSize.X * ( 1.f - AreaRatio ) * 0.5f;
		const FLOAT MarginY = Space.m_MovieSpaceSize.Y * ( 1.f - AreaRatio ) * 0.5f;
		const FLOAT AreaMinX = MarginX - Space.m_EmptySpace.X;
		const FLOAT AreaMinY = MarginY - Space.m_EmptySpace.Y;
		const FLOAT AreaMaxX = Space.m_MovieSpaceSize.X - Space.m_EmptySpace.X - MarginX;
		const FLOAT AreaMaxY = Space.m_MovieSpaceSize.Y - Space.m_EmptySpace.Y - MarginY;
		const FLOAT Zoom = 0.f;
		for( INT Index = 0; Index < NumMarkers; Index++ )
		{
			FDisHUDMarker* Marker = (FDisHUDMarker*)m_SortedMarkers(Index);
			const FDisHUDMarkerSettings* Settings = Marker->m_pSettings;
			FVector Location = Marker->GetLocation();
			const FLOAT Distance = ( PlayerController->Pawn->Location - Marker->GetLocation() ).Size();
			const FLOAT ScaleFactor = Settings->m_ScaleVariation.ComputeValue( Distance ) * 0.01f;
			Location.Z += Settings->m_fWorldOffsetZ;
			FDisHUDMarkerPosInfo& Pos = PosInfos[Index];
			const UINT ScreenFlags = DisComputeMarkerScreenPosition( Location, _pCanvas, Zoom, AreaRatio, Space.m_MovieSpaceSize.X, Space.m_MovieSpaceSize.Y,
				Pos.m_TrackedPosition, Pos.m_Position, m_bTripleScreenMode );
			Pos.m_Position.X -= Space.m_EmptySpace.X;
			Pos.m_Position.Y -= Space.m_EmptySpace.Y + Settings->m_fScreenOffsetZ * ScaleFactor;
			Pos.m_TrackedPosition.X -= Space.m_EmptySpace.X;
			Pos.m_TrackedPosition.Y -= Space.m_EmptySpace.Y;
			Pos.m_fDistance = Distance;
			Pos.m_Flags = ( Marker->m_bTrackedActorVisible ? DHMF_Visible : 0 ) | ( ScreenFlags << 3 );
			if( Settings->m_bKeepBBInMarkerArea )
			{
				const FLOAT BBMinX = Settings->m_BBMin.X * ScaleFactor;
				const FLOAT BBMinY = Settings->m_BBMin.Y * ScaleFactor;
				const FLOAT BBMaxX = Settings->m_BBMax.X * ScaleFactor;
				const FLOAT BBMaxY = Settings->m_BBMax.Y * ScaleFactor;
				if( AreaMinX > Pos.m_Position.X + BBMinX )
				{
					Pos.m_Position.X = AreaMinX - BBMinX;
					Pos.m_Flags |= DHMF_ClampedToArea;
				}
				else if( Pos.m_Position.X + BBMaxX > AreaMaxX )
				{
					Pos.m_Position.X = AreaMaxX - BBMaxX;
					Pos.m_Flags |= DHMF_ClampedToArea;
				}
				if( AreaMinY > Pos.m_Position.Y - BBMaxY )
				{
					Pos.m_Position.Y = AreaMinY + BBMaxY;
					Pos.m_Flags |= DHMF_ClampedToArea;
				}
				else if( Pos.m_Position.Y - BBMinY > AreaMaxY )
				{
					Pos.m_Position.Y = AreaMaxY + BBMinY;
					Pos.m_Flags |= DHMF_ClampedToArea;
				}
			}
			const FLOAT FromCentre = Square( Pos.m_Position.Y - CentreY ) + Square( Pos.m_Position.X - CentreX );
			if( Marker->m_bTrackedActorVisible && Settings->m_bCanFocus && BestFocus > FromCentre )
			{
				FocusedIndex = Index;
				BestFocus = FromCentre;
			}
		}
	}
	UBOOL bNewMarkers = FALSE;
	UBOOL bOutOfOrder = FALSE;
	for( INT Index = 0; Index < NumMarkers; Index++ )
	{
		FDisHUDMarker* Marker = (FDisHUDMarker*)m_SortedMarkers(Index);
		FDisHUDMarkerPosInfo& Pos = PosInfos[Index];
		if( Index == FocusedIndex )
		{
			Pos.m_Flags |= DHMF_Focused;
		}
		Marker->Update( Pos.m_Position, Pos.m_TrackedPosition, Pos.m_fDistance, Pos.m_Flags );
		bNewMarkers |= Marker->m_bNeedsSorting;
		bOutOfOrder |= Index > 0 && Pos.m_fDistance > PosInfos[Index - 1].m_fDistance;
		if( DisHUDMarkerDiagDue() )
		{
			debugf( TEXT("DISHONORED(bringup): hidiag marker %s at %.1f,%.1f (tracked %.1f,%.1f) dist %.0f flags 0x%X depth %d"),
				Marker->m_pTrackedActor ? *Marker->m_pTrackedActor->GetName() : TEXT("None"), Pos.m_Position.X, Pos.m_Position.Y,
				Pos.m_TrackedPosition.X, Pos.m_TrackedPosition.Y, Pos.m_fDistance, Pos.m_Flags, Marker->m_Depth );
		}
	}
	if( bNewMarkers || bOutOfOrder )
	{
		SortMarkers( PosInfos, bNewMarkers );
	}
}

// DISHONORED(port): 2013 rva 0x799480 (2012 0x7ff2e0). New markers first move up the list past the ones drawn above them;
// then the list is bubble-sorted far to near, swapping the clips' depths with it, so a nearer marker is drawn on top.
void UDisGFxMoviePlayerHUD::SortMarkers( FDisHUDMarkerPosInfo* _pPosInfos, UBOOL _bSortNewMarkers )
{
	INT Count = m_SortedMarkers.Num();
	if( _bSortNewMarkers )
	{
		for( INT Index = Count - 1; Index >= 0; Index-- )
		{
			FDisHUDMarker* Marker = (FDisHUDMarker*)m_SortedMarkers(Index);
			if( !Marker->m_bNeedsSorting )
			{
				continue;
			}
			for( INT Next = Index; Next < Count - 1 && Marker->m_Depth > ((FDisHUDMarker*)m_SortedMarkers(Next + 1))->m_Depth; Next++ )
			{
				Exchange( m_SortedMarkers(Next), m_SortedMarkers(Next + 1) );
			}
			Marker->m_bNeedsSorting = FALSE;
		}
	}
	do
	{
		INT LastSwap = 0;
		for( INT Index = 1; Index < Count; Index++ )
		{
			if( _pPosInfos[Index].m_fDistance > _pPosInfos[Index - 1].m_fDistance )
			{
				((FDisHUDMarker*)m_SortedMarkers(Index))->SwapDepth( (FDisHUDMarker*)m_SortedMarkers(Index - 1) );
				Exchange( m_SortedMarkers(Index), m_SortedMarkers(Index - 1) );
				Exchange( _pPosInfos[Index], _pPosInfos[Index - 1] );
				LastSwap = Index;
			}
		}
		Count = LastSwap;
	}
	while( Count );
}

// DISHONORED(port): 2013 rva 0x7a0f00 (2012 0x804f70). The grenade and heart lists are emptied the same way
// (RemoveGrenadeMarkers / RemoveHeartMarkers); nothing in this tree fills them.
void UDisGFxMoviePlayerHUD::ClearMarkerInstances()
{
	TArray<FPointer>* Lists[] = { &m_TaskMarkers, &m_AwarenessMarkers, &m_GrenadeMarkers, &m_HeartMarkers };
	for( INT ListIdx = 0; ListIdx < ARRAY_COUNT(Lists); ListIdx++ )
	{
		while( Lists[ListIdx]->Num() > 0 )
		{
			delete (FDisHUDMarker*)(*Lists[ListIdx])(0);
			Lists[ListIdx]->RemoveSwap( 0 );
		}
	}
	m_SortedMarkers.Empty();
}

// DISHONORED(port): 2013 rva 0x7afe70 (2012 0x812080; 2013 added the pause test). The next queued popup is shown once the
// start-of-game delay has run out and the clip of the one on screen reports _opened false.
// DISHONORED(bringup): event 4 (a target notification) goes to ShowTargetNotification (2012 0x7fef30), not ported; such a
// popup is dropped.
void UDisGFxMoviePlayerHUD::Tick_ObjectivePopup( FLOAT _fDeltaTime )
{
	GFxValue* Clips = DisHUDClips( this );
	if( Clips[DHMC_ObjectivePopup].IsUndefined() )
	{
		return;
	}
	if( GWorld && !GWorld->IsPaused() )
	{
		m_fPopupTimer -= _fDeltaTime;
	}
	UBOOL bShowNextPopup = m_fPopupTimer < 0.f;
	if( m_ActivePopup.m_pObjective || m_ActivePopup.m_pTargetNotifSeq )
	{
		GFxValue Opened;
		Clips[ m_ActivePopup.m_Event == 4 ? DHMC_SpecialNotification : DHMC_ObjectivePopup ].GetMember( "_opened", &Opened );
		if( Opened.IsBool() && Opened.GetBool() )
		{
			bShowNextPopup = FALSE;
		}
	}
	if( !bShowNextPopup )
	{
		return;
	}
	m_ActivePopup.m_pObjective = NULL;
	m_ActivePopup.m_Event = 0;
	m_ActivePopup.m_Tasks.Reset();
	m_ActivePopup.m_pTargetNotifSeq = NULL;
	if( !m_Settings.m_bShowObjectivePopups )
	{
		m_PopupQueue.Empty();
		return;
	}
	while( m_PopupQueue.Num() > 0 && m_ActivePopup.m_pObjective == NULL && m_ActivePopup.m_pTargetNotifSeq == NULL )
	{
		const FDisObjectivePopupInfo& Next = m_PopupQueue(0);
		m_ActivePopup.m_pObjective = Next.m_pObjective;
		m_ActivePopup.m_Event = Next.m_Event;
		m_ActivePopup.m_Tasks = Next.m_Tasks;
		m_ActivePopup.m_pTargetNotifSeq = Next.m_pTargetNotifSeq;
		m_PopupQueue.Remove( 0 );
	}
	if( m_ActivePopup.m_pObjective != NULL && m_ActivePopup.m_Event != 4 )
	{
		ShowObjectivePopup( m_ActivePopup );
	}
}

// DISHONORED(port): 2013 rva 0x798a40 (2012 0x7fe8b0). objectives_mc.SetObjectives( <objective>, <title>, <event>,
// [{objState, objDescription} per task] ) - the task list is null for a failed objective. A task whose event is not
// "completed" and whose status string is not empty shows ")" alone: retail builds the line with three wcscpy_s calls
// where wcscat_s was meant (0xb98d50, 0xb98d97, 0xb98daa), and that is kept.
void UDisGFxMoviePlayerHUD::ShowObjectivePopup( const FDisObjectivePopupInfo& _rPopup )
{
	UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
	GFxMovieView* View = DisHUDView( this );
	if( Tweaks == NULL || View == NULL || _rPopup.m_pObjective == NULL )
	{
		return;
	}
	const FString* Title = NULL;
	switch( _rPopup.m_Event )
	{
	case 0: Title = &Tweaks->m_NewObjectivePopupTitle;       break;
	case 1: Title = &Tweaks->m_UpdatedObjectivePopupTitle;   break;
	case 2: Title = &Tweaks->m_CompletedObjectivePopupTitle; break;
	case 3: Title = &Tweaks->m_FailedObjectivePopupTitle;    break;
	default: break;
	}
	GFxValue Args[4];
	Args[0].SetStringW( *_rPopup.m_pObjective->m_Description );
	Args[1].SetStringW( Title ? **Title : NULL );
	Args[2].SetNumber( (DOUBLE)_rPopup.m_Event );
	if( _rPopup.m_Event == 3 )
	{
		Args[3].SetNull();
	}
	else
	{
		View->CreateArray( &Args[3] );
		for( INT Index = 0; Index < _rPopup.m_Tasks.Num(); Index++ )
		{
			const FDisTaskPopupInfo& Task = _rPopup.m_Tasks(Index);
			TCHAR Description[1024];
			appStrncpy( Description, *Task.m_pTask->m_Description, ARRAY_COUNT(Description) );
			if( Task.m_Event != 2 && Task.m_pTask->GetStatusString().Len() > 0 )
			{
				appStrcpy( Description, TEXT(")") );
			}
			GFxValue State;
			State.SetNumber( (DOUBLE)Task.m_Event );
			GFxValue Text;
			Text.SetStringW( Description );
			GFxValue Entry;
			View->CreateObject( &Entry, NULL, NULL, 0 );
			Entry.SetMember( "objState", State );
			Entry.SetMember( "objDescription", Text );
			Args[3].PushBack( Entry );
		}
	}
	DisHUDInvoke( this, DHMC_ObjectivePopup, "SetObjectives", Args, 4 );
}

/*-----------------------------------------------------------------------------
	The tutorial line. 2013 keeps where the text lives (loc file, section, key) instead of the 2012 resolved string,
	and resolves it every time it is shown, so the key glyphs follow the device in use.
-----------------------------------------------------------------------------*/

/** the parameter block of event 29, the 2012 PDB's FArkGameEventTutorialTimedOut_Params */
struct FArkGameEventTutorialTimedOut_Params
{
	INT m_TutorialID;
};
enum { DisGameEventType_TutorialTimedOut = 29 };

// DISHONORED(port): 2013 rva 0x7988e0 (2012 0x7fe430)
INT UDisGFxMoviePlayerHUD::FindTutorial( INT _ID ) const
{
	if( _ID == -1 )
	{
		return -1;
	}
	for( INT Index = 0; Index < m_TutorialStack.Num(); Index++ )
	{
		if( m_TutorialStack(Index).m_ID == _ID )
		{
			return Index;
		}
	}
	return -1;
}

// DISHONORED(port): 2013 rva 0x79eef0 (2012 0x803180)
FDisTutorialInfo* UDisGFxMoviePlayerHUD::FindTutorialInfo( INT _ID )
{
	const INT Index = FindTutorial( _ID );
	return Index != -1 ? &m_TutorialStack(Index) : NULL;
}

// DISHONORED(port): 2013 rva 0x7a7390, TArray<FDisTutorialInfo>::Remove with FDisTutorialInfo's destructor (0x794b90), which
// also frees the note parameters
void UDisGFxMoviePlayerHUD::RemoveTutorialAt( INT _Index )
{
	FDisTutorialInfo& Info = m_TutorialStack(_Index);
	if( Info.m_pNoteParams )
	{
		appFree( Info.m_pNoteParams );
		Info.m_pNoteParams = NULL;
	}
	m_TutorialStack.Remove( _Index );
}

// DISHONORED(port): 2013 rva 0x7a6c10 (2012 0x80a000; 2013 added type 6 and moved type 5 to priority 2). The stack is
// kept in priority order; a cycling tutorial goes after the last one of at least its priority, any other before the
// first one of at most its priority, and types 3, 4 and 6 are unique against their neighbour.
INT UDisGFxMoviePlayerHUD::InsertTutorial( const FDisTutorialInfo& _rTutorialInfo )
{
	static const INT TypePriority[7] = { 3, 2, 2, 2, 2, 2, 1 };
	const BYTE Type = _rTutorialInfo.m_Type;
	const UBOOL bUniqueItem = Type == 3 || Type == 6 || Type == 4;
	const INT Priority = TypePriority[Type];
	if( Type == 5 )
	{
		INT Index = m_TutorialStack.Num();
		while( Index > 0 )
		{
			Index--;
			if( Priority <= TypePriority[m_TutorialStack(Index).m_Type] )
			{
				if( bUniqueItem && Type == m_TutorialStack(Index).m_Type )
				{
					return -1;
				}
				const INT Inserted = m_TutorialStack.InsertItem( _rTutorialInfo, Index + 1 );
				return Inserted != -1 ? Inserted : m_TutorialStack.InsertItem( _rTutorialInfo, 0 );
			}
		}
		return m_TutorialStack.InsertItem( _rTutorialInfo, 0 );
	}
	for( INT Index = 0; Index < m_TutorialStack.Num(); Index++ )
	{
		if( Priority >= TypePriority[m_TutorialStack(Index).m_Type] )
		{
			if( bUniqueItem && Type == m_TutorialStack(Index).m_Type )
			{
				return -1;
			}
			const INT Inserted = m_TutorialStack.InsertItem( _rTutorialInfo, Index );
			return Inserted != -1 ? Inserted : m_TutorialStack.AddItem( _rTutorialInfo );
		}
	}
	return m_TutorialStack.AddItem( _rTutorialInfo );
}

// DISHONORED(port): 2013 rva 0x7ac5a0 (2012 0x811810). Refused unless forced, or the player's tutorial setting, the HUD's
// m_bTutorialsEnabled and (from Kismet or m_bSystemicTutorialsEnabled) all allow it. The text is not stored: the
// tutorial keeps where it is localised (2013 0x79ed50 - the object's loc file and section, the property's name as key;
// the object's vtable +220 call in it is an empty body, 0xc715d0) and the ID handed back is the next one.
INT UDisGFxMoviePlayerHUD::ShowTutorial( UObject* _pLocObject, const TCHAR* _pLocProperty, FLOAT _fDuration,
	UBOOL _bShowOnce, UBOOL _bAllowCycling, UBOOL _bFromKismet, UBOOL _bForceDisplay )
{
	if( !_bForceDisplay
		&& ( !m_Settings.m_bShowTutorialNotifications || !m_bTutorialsEnabled || ( !_bFromKismet && !m_bSystemicTutorialsEnabled ) ) )
	{
		if( DisHUDUIDiag() )
		{
			debugf( TEXT("DISHONORED(bringup): puidiag ShowTutorial %s.%s refused: setting %d, enabled %d, systemic %d, kismet %d"),
				_pLocObject ? *_pLocObject->GetName() : TEXT("none"), _pLocProperty, (INT)m_Settings.m_bShowTutorialNotifications,
				(INT)m_bTutorialsEnabled, (INT)m_bSystemicTutorialsEnabled, (INT)_bFromKismet );
		}
		return -1;
	}
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	FDisTutorialInfo Info( EC_EventParm );
	Info.m_ID = m_NextTutorialID;
	Info.m_Type = _bAllowCycling ? 5 : 0;
	Info.m_fTimeLeft = _fDuration;
	Info.m_bShowOnce = _bShowOnce ? 1 : 0;
	Info.m_bGamepad = ( PC && PC->PlayerInput && PC->PlayerInput->bUsingGamepad ) ? 1 : 0;
	const INT Index = InsertTutorial( Info );
	if( Index == -1 )
	{
		return -1;
	}
	FDisTutorialInfo& Inserted = m_TutorialStack(Index);
	FString LocPrefix;
	_pLocObject->GetLocalizationDataLocation( _pLocObject, Inserted.m_MessageLocFile, Inserted.m_MessageLocSection, LocPrefix );
	Inserted.m_MessageLocKey = _pLocProperty;
	if( DisHUDUIDiag() )
	{
		debugf( TEXT("DISHONORED(bringup): puidiag ShowTutorial id %d type %d duration %.2f once %d at %d: [%s] %s in %s"),
			Inserted.m_ID, (INT)Inserted.m_Type, _fDuration, (INT)_bShowOnce, Index, *Inserted.m_MessageLocSection,
			*Inserted.m_MessageLocKey, *Inserted.m_MessageLocFile );
	}
	return m_NextTutorialID++;
}

// DISHONORED(port): 2013 rva 0x7acdc0 (2012 0x80caa0, same 584 bytes). "<Strings:Package.Section.Key>" is replaced by that
// localised string; anything else is left alone. (It is not what writes "[Unbound]": FormatInteractionText is.)
void UDisGFxMoviePlayerHUD::ResolveMarkupString( FString& _rText ) const
{
	if( !_rText.StartsWith( TEXT("<Strings:") ) )
	{
		return;
	}
	FString Inner = _rText.Right( _rText.Len() - 9 );
	Inner = Inner.Left( Inner.Len() - 1 );
	TArray<FString> Parts;
	Inner.ParseIntoArray( &Parts, TEXT("."), TRUE );
	if( Parts.Num() >= 3 )
	{
		_rText = Localize( *Parts(1), *Parts(2), *Parts(0) );
	}
}

// DISHONORED(port): 2013 rva 0x7ad060, new in 2013 (2012's SetTutorialMessage 0x7fe4b0 took the resolved string): the text
// is localised, its markup and key glyphs resolved for the device the tutorial was raised on, and handed to
// tutorialMsg_mc.SetTutorialMessage after _txtAttached is cleared.
void UDisGFxMoviePlayerHUD::SetTutorialMessage( const FDisTutorialInfo& _rTutorialInfo )
{
	if( !m_Settings.m_bShowTutorialNotifications )
	{
		return;
	}
	FString Text = Localize( *_rTutorialInfo.m_MessageLocSection, *_rTutorialInfo.m_MessageLocKey, *_rTutorialInfo.m_MessageLocFile );
	ResolveMarkupString( Text );
	FormatInteractionText( Text, !_rTutorialInfo.m_bGamepad, _rTutorialInfo.m_bGamepad );
	GFxValue* Clips = DisHUDClips( this );
	if( !Clips[DHMC_Tutorials].IsUndefined() )
	{
		GFxValue Attached;
		Attached.SetBoolean( false );
		Clips[DHMC_Tutorials].SetMember( "_txtAttached", Attached );
	}
	GFxValue Arg;
	Arg.SetStringW( *Text );
	DisHUDInvoke( this, DHMC_Tutorials, "SetTutorialMessage", &Arg, 1 );
}

// DISHONORED(port): 2013 rva 0x798960 (2012 0x7fe620): SetTutorialMessage( null )
void UDisGFxMoviePlayerHUD::ClearTutorialMessage()
{
	if( m_pMovieClips == NULL )
	{
		return;
	}
	GFxValue Arg;
	Arg.SetNull();
	DisHUDInvoke( this, DHMC_Tutorials, "SetTutorialMessage", &Arg, 1 );
}

// DISHONORED(port): 2013 rva 0x7ad2f0, new in 2013 (2012's ShowTutorialWindow 0x7fe700 took the resolved string):
// tutorialWindow_mc.SetTutorialWindow( <text>, <type - 1> ).
void UDisGFxMoviePlayerHUD::ShowTutorialWindow( const FDisTutorialInfo& _rTutorialInfo )
{
	if( !m_Settings.m_bShowTutorialNotifications )
	{
		return;
	}
	FString Text = Localize( *_rTutorialInfo.m_MessageLocSection, *_rTutorialInfo.m_MessageLocKey, *_rTutorialInfo.m_MessageLocFile );
	ResolveMarkupString( Text );
	FormatInteractionText( Text, !_rTutorialInfo.m_bGamepad, _rTutorialInfo.m_bGamepad );
	GFxValue Args[2];
	Args[0].SetStringW( *Text );
	Args[1].SetNumber( (DOUBLE)( (INT)_rTutorialInfo.m_Type - 1 ) );
	DisHUDInvoke( this, DHMC_TutorialWindow, "SetTutorialWindow", Args, 2 );
}

// DISHONORED(port): 2013 rva 0x78d6d0 (2012 0x7f6bf0): tutorialWindow_mc.Close()
void UDisGFxMoviePlayerHUD::HideTutorialWindow()
{
	DisHUDInvoke( this, DHMC_TutorialWindow, "Close", NULL, 0 );
}

// DISHONORED(port): 2013 rva 0x7afa60 (2012 0x80c6b0; 2013 has the 64-byte FDisTutorialInfo, type 6, show bits 0x1000/0xE00
// where 2012 had 0x400/0x200, and the m_bBlockInteractionWindow test). The current tutorial's time runs down; the top of
// the stack is the one wanted when its show bit is on (or it is forced), a cycling one rotating every m_fTutorialCycleDelay;
// on a change the line or the window is redrawn and a show-once tutorial that was current is dropped. Event 29 carries the
// current ID - after a change that is the new one, as in retail.
void UDisGFxMoviePlayerHUD::Tick_Tutorials( FLOAT _fDeltaTime, INT _ShowFlags )
{
	GFxValue* Clips = DisHUDClips( this );
	if( Clips[DHMC_Tutorials].IsUndefined() )
	{
		return;
	}
	INT CurrentIndex = FindTutorial( m_CurrentTutorialID );
	if( CurrentIndex != -1 )
	{
		FDisTutorialInfo& Current = m_TutorialStack(CurrentIndex);
		if( Current.m_fTimeLeft > 0.f )
		{
			Current.m_fTimeLeft -= _fDeltaTime;
			if( Current.m_fTimeLeft < 0.0001f )
			{
				RemoveTutorialAt( CurrentIndex );
				CurrentIndex = -1;
				FArkGameEventTutorialTimedOut_Params Params = { m_CurrentTutorialID };
				FArkGameEventDispatcher::GetInstance()->ProcessEvent( FArkGameEvent( DisGameEventType_TutorialTimedOut, &Params, this ) );
			}
		}
	}

	INT DesiredIndex = -1;
	INT DesiredID = -1;
	if( m_TutorialStack.Num() > 0 )
	{
		const FDisTutorialInfo& Top = m_TutorialStack(0);
		if( ( Top.m_bShowInWindow && ( _ShowFlags & 0x1000 ) != 0 && !m_bBlockInteractionWindow )
			|| ( !Top.m_bShowInWindow && ( _ShowFlags & 0xE00 ) != 0 )
			|| Top.m_bForceDisplay )
		{
			if( Top.m_Type == 5 )
			{
				UDisTweaks_GFxMoviePlayerHUD* Tweaks = (UDisTweaks_GFxMoviePlayerHUD*)m_pTweaks;
				const FLOAT CycleDelay = Tweaks ? Tweaks->m_fTutorialCycleDelay : 0.f;
				m_fCurrentCyclingTutorialTime += _fDeltaTime;
				DesiredIndex = Max( CurrentIndex, 0 );
				if( m_fCurrentCyclingTutorialTime > CycleDelay )
				{
					DesiredIndex = ( DesiredIndex + 1 ) % m_TutorialStack.Num();
					m_fCurrentCyclingTutorialTime -= CycleDelay;
				}
				DesiredID = m_TutorialStack(DesiredIndex).m_ID;
			}
			else if( Top.m_Type <= 6 )
			{
				DesiredID = Top.m_ID;
				DesiredIndex = 0;
				m_fCurrentCyclingTutorialTime = 0.f;
			}
		}
	}

	if( DesiredID == m_CurrentTutorialID )
	{
		return;
	}
	if( DesiredID != -1 && !m_TutorialStack(DesiredIndex).m_bShowInWindow )
	{
		SetTutorialMessage( m_TutorialStack(DesiredIndex) );
	}
	else
	{
		ClearTutorialMessage();
	}
	if( m_bTutorialWindowSet )
	{
		HideTutorialWindow();
		m_bTutorialWindowSet = FALSE;
	}
	if( DesiredID != -1 && m_TutorialStack(DesiredIndex).m_bShowInWindow )
	{
		ShowTutorialWindow( m_TutorialStack(DesiredIndex) );
		m_bTutorialWindowSet = TRUE;
	}
	if( DisHUDUIDiag() )
	{
		debugf( TEXT("DISHONORED(bringup): puidiag tutorial %d -> %d (stack %d, show flags 0x%04X)"),
			m_CurrentTutorialID, DesiredID, m_TutorialStack.Num(), _ShowFlags );
	}
	m_CurrentTutorialID = DesiredID;
	m_fCurrentCyclingTutorialTime = 0.f;
	if( CurrentIndex != -1 && m_TutorialStack(CurrentIndex).m_bShowOnce )
	{
		RemoveTutorialAt( CurrentIndex );
		FArkGameEventTutorialTimedOut_Params Params = { m_CurrentTutorialID };
		FArkGameEventDispatcher::GetInstance()->ProcessEvent( FArkGameEvent( DisGameEventType_TutorialTimedOut, &Params, this ) );
	}
}

// DISHONORED(port): 2013 rva 0x7ae4a0 (2012 0x8125a0). Input 0 shows the line unless it is already up, input 1 takes it
// down; the gamepad text is used when the player is on a pad and the action has one, and the action's m_bForceDisplay is
// copied onto the tutorial it raised.
// DISHONORED(bringup): retail dereferences DisGetGFxHUD() unguarded; under -nodishud this tree has no HUD.
void UDisSeqAct_SetTutorialMessage::Activated()
{
	UDisGFxMoviePlayerHUD* HUD = DisGetGFxHUD();
	if( HUD != NULL )
	{
		if( !InputLinks(0).bHasImpulse || HUD->FindTutorialInfo( m_TutorialID ) )
		{
			if( InputLinks(1).bHasImpulse && HUD->FindTutorialInfo( m_TutorialID ) )
			{
				const INT Index = HUD->FindTutorial( m_TutorialID );
				if( Index != -1 )
				{
					HUD->RemoveTutorialAt( Index );
				}
				m_TutorialID = -1;
			}
		}
		else
		{
			ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
			const UBOOL bGamepad = PC && PC->PlayerInput && PC->PlayerInput->bUsingGamepad;
			const TCHAR* Property = ( bGamepad && m_TutorialMessage_Gamepad.Len() > 0 ) ? TEXT("m_TutorialMessage_Gamepad") : TEXT("m_TutorialMessage");
			m_TutorialID = HUD->ShowTutorial( this, Property, m_fDuration, m_bShowOnce, m_bAllowCycling, TRUE, FALSE );
			if( FDisTutorialInfo* Info = HUD->FindTutorialInfo( m_TutorialID ) )
			{
				Info->m_bForceDisplay = m_bForceDisplay;
			}
		}
	}
	USequenceAction::Activated();
}

// DISHONORED(port): agent HJ, 2013 rva 0x7b02f0 (2012 0x8124f0, 168 bytes each). An action cooked without a variable link gets the
// class default's first one - its expected type, description and property name, writeable - and its package is marked dirty.
void UDisSeqAct_SetTutorialMessage::PostLoad()
{
	USequenceOp::PostLoad();
	if( VariableLinks.Num() == 0 )
	{
		const FSeqVarLink& Default = ((UDisSeqAct_SetTutorialMessage*)GetClass()->GetDefaultObject())->VariableLinks(0);
		FSeqVarLink& Link = VariableLinks( VariableLinks.AddZeroed() );
		Link.ExpectedType = Default.ExpectedType;
		Link.LinkDesc = Default.LinkDesc;
		Link.PropertyName = Default.PropertyName;
		Link.bWriteable = TRUE;
		MarkPackageDirty( TRUE );
	}
}

/*-----------------------------------------------------------------------------
	The crosshair's interaction prompt: the focus text and the interaction lines beside the crosshair. Agent HF.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x787da0, new in 2013 (2012's FDisUIInteractionContext::HasGroupChanged 0x7f48e0): a group
// changed when its count or any interaction's index differs
static UBOOL DisHUDInteractionGroupChanged( const FDisUIInteractionContext& _rNew, const FDisUIInteractionContext& _rOld, INT _Group )
{
	const FDisUIInteractionGroup& New = _rNew.m_Groups[_Group];
	const FDisUIInteractionGroup& Old = _rOld.m_Groups[_Group];
	if( New.m_InteractionCount != Old.m_InteractionCount )
	{
		return TRUE;
	}
	for( INT Index = 0; Index < New.m_InteractionCount; Index++ )
	{
		if( New.m_Interactions[Index].m_Index != Old.m_Interactions[Index].m_Index )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(port): 2013 rva 0x7984f0 (2012 0x7fe040): crosshairInfosTxt_mc.SetText( <text>, or null when empty )
void UDisGFxMoviePlayerHUD::UpdateCrosshairFocusText( const FString& _rText )
{
	GFxValue Arg;
	if( _rText.Len() > 0 )
	{
		Arg.SetStringW( *_rText );
	}
	else
	{
		Arg.SetNull();
	}
	DisHUDInvoke( this, DHMC_CrosshairInfo, "SetText", &Arg, 1 );
}

// DISHONORED(port): 2013 rva 0x7a6150 (2012 0x8097d0). interactions_window_mc.SetInteractions( <name>, <lines> ): the
// group's texts joined by newlines and run through FormatInteractionText, or ( null, null ) for an empty group.
void UDisGFxMoviePlayerHUD::UpdateCrosshairInteractions( const FString& _rName, const FDisUIInteractionGroup& _rGroup )
{
	GFxValue Args[2];
	FString InteractionText;
	if( _rGroup.m_InteractionCount <= 0 )
	{
		Args[0].SetNull();
		Args[1].SetNull();
	}
	else
	{
		InteractionText = *(const FString*)_rGroup.m_Interactions[0].m_pText;
		for( INT Index = 1; Index < _rGroup.m_InteractionCount; Index++ )
		{
			InteractionText += TEXT("\n");
			InteractionText += *(const FString*)_rGroup.m_Interactions[Index].m_pText;
		}
		FormatInteractionText( InteractionText, FALSE, FALSE );
		Args[0].SetStringW( *_rName );
		Args[1].SetStringW( *InteractionText );
	}
	DisHUDInvoke( this, DHMC_CrosshairInteractions, "SetInteractions", Args, 2 );
}

// DISHONORED(port): 2013 rva 0x7acb10 (2012 0x80c450). With the interaction show bits (0xE00) on and an interactable under the
// crosshair, its focus text (interface +52) and name (+56), each through its FormatText (+80); otherwise both empty. The
// context's group counts are masked by the player's interaction settings and the show bits, and only what changed since
// the last call is sent.
// DISHONORED(bringup): the context is retail's ADishonoredPlayerPawn::FillUIInteractions (2013 0x6a9130, 2012 0x7033f0) plus the
// probe's (0x78cc60). The pawn's reaches the player state machine's FillUIInteractions (vtable +372/+416) and the
// interactable's interface slots +60/+64, none of which this tree declares, and the probe's five checks are unported; the
// context stays empty, so the crosshair group is the focus text alone and UpdateContextInteractions (0x7a63c0),
// UpdateSpecialInteractions (0x7a6630) and the QTE group (0x7a6920) never see a change and are not ported.
void UDisGFxMoviePlayerHUD::Tick_Interaction( IDisInteractableInterface* _pInteractable )
{
	const UBOOL bShowInteractions = ( m_ShowFlagsOnLastTick & 0xE00 ) != 0;
	FDisUIInteractionContext Context( EC_EventParm );
	FString FocusText;
	FString Name;
	if( _pInteractable && bShowInteractions )
	{
		FocusText = _pInteractable->GetCrosshairFocusText();
		Name = _pInteractable->GetInteractableName();
		_pInteractable->FormatText( FocusText );
		_pInteractable->FormatText( Name );
	}
	if( bShowInteractions )
	{
		const INT ContextCount = Context.m_Groups[0].m_InteractionCount > 0 ? 0 : Context.m_Groups[1].m_InteractionCount;
		Context.m_Groups[0].m_InteractionCount *= m_Settings.m_bShowInteractions ? 1 : 0;
		Context.m_Groups[1].m_InteractionCount = ContextCount * ( m_Settings.m_bShowInteractions & ( m_ShowFlagsOnLastTick >> 10 ) & 1 );
		Context.m_Groups[2].m_InteractionCount *= m_Settings.m_bShowContextualIcons & ( m_ShowFlagsOnLastTick >> 11 ) & 1;
		Context.m_Groups[3].m_InteractionCount *= m_Settings.m_bShowContextualIcons ? 1 : 0;
	}
	if( FocusText != m_CrosshairFocusText )
	{
		UpdateCrosshairFocusText( FocusText );
		m_CrosshairFocusText = FocusText;
	}
	if( Name != m_InteractableName || DisHUDInteractionGroupChanged( Context, m_InteractionContext, 0 ) )
	{
		UpdateCrosshairInteractions( Name, Context.m_Groups[0] );
		m_InteractableName = Name;
		m_InteractionContext.m_Groups[0] = Context.m_Groups[0];
	}
}

// DISHONORED(port): 2013 rva 0x7af540 (2012 0x8117c0): the interaction prompt, then the highlight requests the settings left
void UDisGFxMoviePlayerHUD::OnCrosshairActorSet( IDisInteractableInterface* _pInteractable )
{
	if( m_pMovieClips == NULL )
	{
		return;
	}
	Tick_Interaction( _pInteractable );
	if( m_bShowHighlightNextFrame )
	{
		if( _pInteractable )
		{
			_pInteractable->DoHighlight();
		}
		m_bShowHighlightNextFrame = FALSE;
	}
	if( m_bHideHighlightNextFrame )
	{
		if( _pInteractable )
		{
			_pInteractable->UnDoHighlight();
		}
		m_bHideHighlightNextFrame = FALSE;
	}
}

/*-----------------------------------------------------------------------------
	Who opens the HUD: UDisGlobalUIManager::Init, and nobody else. Agent FP, wave 20.

	Until wave 20 this file carried DisHUDBringUp, a second owner for the HUD movie player that constructed
	UDisGFxMoviePlayerHUD and UDisGFxMoviePlayerHUDFX itself and raced the -gfxuimenu bring-up in
	GFxUI/Src/gfxuiengine.cpp (agent FM: 2 of 3 runs dead in the menu's own open until one was latched behind
	the other). Both are gone. Retail's chain, resolved by hand by agent FM in the 2013 database and
	unconditional end to end: ADishonoredGameInfo::PostBeginPlay (2013 rva 0x6155d0) -> InitGlobalManagers
	(0x5e9bd0, call site 0x5e9c38) -> UDisGlobalUIManager::Init (0x85fdb0, 1428 bytes, 2012 0x8c45d0) ->
	UDisGFxMoviePlayerHUD::StartHUD (0x78cd00, call site 0x86031b). Init is ported in disglobaluimanager.cpp:
	it constructs pHUD under the manager from the config movie set (DefaultUI.ini,
	[DishonoredGame.DisGlobalUIManager]), the effects player comes with it as the HUD class's own pHudFX
	default sub-object, and the cooked class defaults carry the flags the bring-up used to set by hand
	(bAllowFocus, bAllowInput, bCaptureInput, bDisplayWithHudOff; Init logs them).

	The HUD movie is therefore open on every map, the menu map included. What hides it there is
	UDisGlobalUIManager::RefreshGlobalUIState (0x847070), which recomputes pMovie->fVisible as
	   !PersistentWorldInfo.m_bNeverDisplayHUD && AND-over-every-open-player( m_bShowHUDWhileActive )
	on every movie stack change, and FGFxEngine::RenderUI, which tests fVisible before anything else.

	-nodishud is still the regression harness's switch: the three calibrated stages pass it, and Init leaves
	the global movie and the HUD closed when it is on. -dishud is accepted and means nothing.
-----------------------------------------------------------------------------*/

// DISHONORED(port): ADishonoredPlayerController::PreRender, 2013 rva 0x6a0c50. Four instructions in
// retail: the UI manager, its HUD, the movie's open bit, and the HUD's PreRender.
// DISHONORED(bringup): retail reaches it as a native virtual of ADishonoredPlayerController that
// UGameViewportClient::Draw calls where Engine/Src/UnPlayer.cpp now has its
// `PreRender is not a 2013 event` line. Engine cannot call into DishonoredGame, so it arrives through
// the same function-pointer seam GDisEngineTickHook already uses (Engine/Inc/UnWorld.h).
static void DisPlayerControllerPreRender( UCanvas* Canvas )
{
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
