// DishonoredGame/src/dispostprocessmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (21):
//   0x849590  public: static void __cdecl UDisPostProcessManager::InitializePrivateStaticClassUDisPostProcessManager(void)
//   0x8495b0  public: static void __cdecl UDisTweaks_PostProcess::InitializePrivateStaticClassUDisTweaks_PostProcess(void)
//   0x8495d0  public: unsigned int __thiscall UDisPostProcessManager::IsEffectRequired(enum eEffectPp)
//   0x8495f0  public: void __thiscall UDisPostProcessManager::StartEffect(enum eEffectPp, unsigned int)
//   0x849620  public: void __thiscall UDisPostProcessManager::StopEffect(enum eEffectPp)
//   0x849640  public: void __thiscall UDisPostProcessManager::Init(void)
//   0x849660  public: virtual void __thiscall UDisPostProcessManager::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x8496b0  public: void __thiscall UDisPostProcessManager::SetKismetPPParams(struct FArkUberPpParameters const &, float, float, float)
//   0x849700  public: void __thiscall UDisPostProcessManager::SetUIPPParams(struct FArkUberPpParameters const &, float, float, float)
//   0x84c1e0  public: void __thiscall UDisPostProcessManager::TickPossession(float)
//   0x84c400  public: virtual void __thiscall UDisPostProcessManager::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x84c5a0  public: virtual void __thiscall UDisPostProcessManager::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x8502e0  public: void __thiscall UDisPostProcessManager::TickZoomLens(float)
//   0x850970  public: void __thiscall UDisPostProcessManager::TickMaskOn(float)
//   0x850ef0  public: void __thiscall UDisPostProcessManager::ApplyKismetPostProcessSettings(struct FArkPpConfig &, float)
//   0x8511a0  public: void __thiscall UDisPostProcessManager::ApplyUIPostProcessSettings(struct FArkPpConfig &, float)
//   0x8556b0  public: static class UClass * __cdecl UDisPostProcessManager::GetPrivateStaticClassUDisPostProcessManager(wchar_t const *)
//   0x857400  public: static class UClass * __cdecl UDisPostProcessManager::StaticClassNoInline(void)
//   0x857430  public: void __thiscall UDisPostProcessManager::Tick(float)
//   0x85c540  public: static class UClass * __cdecl UDisTweaks_PostProcess::GetPrivateStaticClassUDisTweaks_PostProcess(wchar_t const *)
//   0x85d1c0  public: static class UClass * __cdecl UDisTweaks_PostProcess::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(port): agent EQ, 2013 rva 0x7e7e30 (2012 0x849660). The anti-aliasing option lands in three
// places: the manager's own m_PCAntialiasingType, GSystemSettings.iType_AntiAlias - which is an ini key, so
// this is the one option on the screen that writes itself to DishonoredEngine.ini - and, if the post-process
// graph has been built, the m_Type of the graph's AA node (m_PpBridge.m_PpNodeAA @324 of the object, the node's
// m_Type @104). An unrecognised value leaves the first two alone and still republishes to the node, which is
// retail's own control flow (its default arm jumps past the global assignment only).
void UDisPostProcessManager::ApplyGameSettings( const ArkSettingsParameters* Parameters, EChangeReason Reason )
{
	UBOOL bKnownMode = TRUE;
	switch( Parameters->m_AntiAliasingMode )
	{
	case 0:		m_PCAntialiasingType = 0; break;
	case 1:		m_PCAntialiasingType = 1; break;
	case 2:		m_PCAntialiasingType = 2; break;
	default:	bKnownMode = FALSE; break;
	}
	if( bKnownMode )
	{
		GSystemSettings.iType_AntiAlias = m_PCAntialiasingType;
	}
	if( m_PpBridge.m_PpNodeAA != NULL )
	{
		m_PpBridge.m_PpNodeAA->m_Type = m_PCAntialiasingType;
	}
}
