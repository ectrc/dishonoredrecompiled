// DishonoredGame/src/disinterptrackallowplayerlook.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x8d92d0  public: static void __cdecl UDisInterpTrackAllowPlayerLook::InitializePrivateStaticClassUDisInterpTrackAllowPlayerLook(void)
//   0x8d92f0  public: static void __cdecl UDisPlayerLookAngleKeyProperties::InitializePrivateStaticClassUDisPlayerLookAngleKeyProperties(void)
//   0x8dc350  public: static class UClass * __cdecl UDisPlayerLookAngleKeyProperties::GetPrivateStaticClassUDisPlayerLookAngleKeyProperties(wchar_t const *)
//   0x8dc3e0  public: virtual float __thiscall UInterpTrackVisibility::GetKeyframeTime(int)const
//   0x8dc440  public: virtual class UObject * __thiscall UDisInterpTrackAllowPlayerLook::GetKeyProperties(int)
//   0x8dc4a0  public: virtual void __thiscall UDisInterpTrackAllowPlayerLook::UpdateTrack(float, class UInterpTrackInst *, unsigned int)
//   0x8e00d0  public: static class UClass * __cdecl UDisPlayerLookAngleKeyProperties::StaticClassNoInline(void)
//   0x8e3030  public: virtual int __thiscall UDisInterpTrackAllowPlayerLook::AddKeyframe(float, class UInterpTrackInst *, enum EInterpCurveMode)
//   0x8edd50  public: static class UClass * __cdecl UDisInterpTrackAllowPlayerLook::GetPrivateStaticClassUDisInterpTrackAllowPlayerLook(wchar_t const *)
//   0x8ef080  public: static class UClass * __cdecl UDisInterpTrackAllowPlayerLook::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): UpdateTrack, 2012 0x8dc4a0 / 2013 0x86eaa0 (named in both, same 296 B). Every one of the six
	player groups in l_tower_p carries this track with zero keys (FR8.log), which in retail means: no player look at all
	during the scene - the limits go to 0 and the AnimDriven influence's flag bit 0 is cleared.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_InterpTrackAllowPlayerLook()
{
}

/**
 * In game only, on the player camera (through ADishonoredPlayerPawn::s_pInstance, as retail does): the last key whose
 * start time is at or before the position gives its UDisPlayerLookAngleKeyProperties - yaw min/max, pitch min/max in
 * degrees and m_bMatchControllerRot as flag bit 0 - to the AnimDriven influence; no such key gives zeros.
 */
void UDisInterpTrackAllowPlayerLook::UpdateTrack( FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump )
{
	if( !GIsGame || !ADishonoredPlayerPawn::s_pInstance )
	{
		return;
	}
	ADishonoredPlayerCamera* Camera = ADishonoredPlayerPawn::s_pInstance->GetDishonoredCamera();
	if( !Camera )
	{
		return;
	}
	INT KeyIndex = INDEX_NONE;
	for( INT Index = 0; Index < m_TrackLookAngleKeys.Num(); Index++ )
	{
		if( m_TrackLookAngleKeys(Index).m_fKeyStartTime > NewPosition )
		{
			break;
		}
		KeyIndex = Index;
	}
	FLOAT YawNeg = 0.f, YawPos = 0.f, PitchNeg = 0.f, PitchPos = 0.f;
	INT Flags = 0;
	if( KeyIndex != INDEX_NONE )
	{
		const UDisPlayerLookAngleKeyProperties* Props = m_TrackLookAngleKeys(KeyIndex).m_pProperties;
		if( Props )
		{
			YawNeg = Props->m_fYaw_Min;
			YawPos = Props->m_fYaw_Max;
			PitchNeg = Props->m_fPitch_Min;
			PitchPos = Props->m_fPitch_Max;
			Flags = Props->m_bMatchControllerRot ? 1 : 0;
		}
	}
	UDishonoredCamera_AnimDriven* AnimDriven = Camera->GetCamera_AnimDriven();
	if( AnimDriven )
	{
		AnimDriven->SetPlayerControlledLimit( YawNeg, YawPos, PitchNeg, PitchPos, Flags );
	}
}
