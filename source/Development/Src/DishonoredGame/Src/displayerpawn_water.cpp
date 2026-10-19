// DishonoredGame/src/displayerpawn_water.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x6ffdc0  private: void __thiscall ADishonoredPlayerPawn::OnCameraEnterWaterVolume(void)
//   0x708ef0  public: virtual void __thiscall ADishonoredPlayerPawn::Tick_Water(float, enum ELevelTick)
//   0x709050  public: float __thiscall ADishonoredPlayerPawn::GetApneaDuration(void)const
//   0x7090c0  private: void __thiscall ADishonoredPlayerPawn::OnCameraExitWaterVolume(void)
//   0x70dea0  public: virtual void __thiscall ADishonoredPlayerPawn::OnChangeWaterVolume(class ADishonoredWaterVolume *, class ADishonoredWaterVolume *)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): the player's water - the camera under water, the breath. 2012 / 2013 rvas: OnChangeWaterVolume
	0x70dea0 / 0x6d4120, Tick_Water 0x708ef0 / 0x6d7be0 (2013 refreshes the apnea duration every under-water tick, 0x6d4240),
	GetApneaDuration 0x709050 / 0x6cf570, OnCameraEnterWaterVolume 0x6ffdc0 / 0x6c46a0, OnCameraExitWaterVolume 0x7090c0 /
	0x6cf5e0. The 2013 bodies are ported.
	DISHONORED(bringup): reached only once a water volume's Touch has put the volume in m_CurrentWaterVolumes, which this engine
	cannot do yet (dishonoredwatervolume.cpp). Named and not called inside: the breath gauge on the HUD (2013 0x78ce70 show,
	0x78cf30 hide, 0x794eb0 update - the HUD's breath clip is not ported) and the exit lens effect's activation
	(ADisEmitterCameraLensEffect, disemittercameralenseffect.cpp, is a skeleton: the effect is spawned, not activated).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "ifdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_PlayerPawnWater()
{
}

static UDisTweaks_PlayerPawn* DisIFWaterTweaks( ADishonoredPlayerPawn* Pawn )
{
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( Pawn->GetTweaks_Derived() );
	return Tweaks ? Tweaks : UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>();
}

/**
 * 2013 0x6d4120. The under-water ambience follows the volume the camera is in (the old one's stop event, the new one's play
 * event); leaving the water clears m_bUnderWater and runs the camera exit, entering it (from no active volume) raises it and
 * runs the camera entry.
 */
void ADishonoredPlayerPawn::OnChangeWaterVolume( ADishonoredWaterVolume* OldVolume, ADishonoredWaterVolume* NewVolume )
{
	Super::OnChangeWaterVolume( OldVolume, NewVolume );
	IFDIAG( TEXT("water camera volume %s -> %s"), OldVolume ? *OldVolume->GetName() : TEXT("none"), NewVolume ? *NewVolume->GetName() : TEXT("none") );
	UAkAudioDevice* AudioDevice = UAkAudioDevice::Get();
	if( AudioDevice )
	{
		UDishonoredWaterVolumeInfo* OldInfo = OldVolume ? OldVolume->m_pWaterVolumeInfo : NULL;
		UDishonoredWaterVolumeInfo* NewInfo = NewVolume ? NewVolume->m_pWaterVolumeInfo : NULL;
		UAkEvent* OldPlay = OldInfo ? OldInfo->m_pUnderwaterPlaySoundEvent : NULL;
		UAkEvent* OldStop = OldInfo ? OldInfo->m_pUnderwaterStopSoundEvent : NULL;
		UAkEvent* NewPlay = NewInfo ? NewInfo->m_pUnderwaterPlaySoundEvent : NULL;
		if( NewPlay != OldPlay )
		{
			if( OldPlay && OldStop )
			{
				AudioDevice->PostEvent( OldStop, NULL, NAME_None, 0, NULL, NULL, FALSE );
			}
			if( NewPlay )
			{
				AudioDevice->PostEvent( NewPlay, NULL, NAME_None, 0, NULL, NULL, FALSE );
			}
		}
	}
	if( OldVolume && !NewVolume )
	{
		m_bUnderWater = FALSE;
		OnCameraExitWaterVolume();
		return;
	}
	if( !NewVolume )
	{
		return;
	}
	if( !m_pActiveWaterVolume )
	{
		m_bUnderWater = TRUE;
		OnCameraEnterWaterVolume();
	}
}

/**
 * 2013 0x6d7be0. Out of the water (or dead, or possessing) the air comes back over m_fApneaRecoveryDuration; under it the
 * apnea duration is refreshed (an upgrade may have changed it: a full or over-full tank follows the new maximum) and the air
 * runs out at one unit per second.
 */
void ADishonoredPlayerPawn::Tick_Water( FLOAT DeltaTime, ELevelTick TickType )
{
	Super::Tick_Water( DeltaTime, TickType );
	if( !m_bUnderWater || IsDead() || DisIsPossessing() )
	{
		const FLOAT Regain = ( m_fMaxAirCapacity / DisIFWaterTweaks( this )->m_fApneaRecoveryDuration ) * DeltaTime;
		m_fAirCapacity = Min( m_fAirCapacity + Regain, m_fMaxAirCapacity );
		m_bConsumingAir = FALSE;
		return;
	}
	if( !m_bConsumingAir )
	{
		IFDIAG_ONCE( TEXT("ADishonoredPlayerPawn::Tick_Water: the HUD breath gauge (2013 0x78ce70 / 0x78cf30 / 0x794eb0) is not ported") );
	}
	const FLOAT OldMax = m_fMaxAirCapacity;
	const FLOAT NewMax = GetApneaDuration();
	m_fMaxAirCapacity = NewMax;
	if( NewMax != OldMax && ( Abs( m_fAirCapacity - OldMax ) < 0.00000001f || m_fAirCapacity > NewMax ) )
	{
		m_fAirCapacity = NewMax;
	}
	m_fAirCapacity = Max( m_fAirCapacity - DeltaTime, 0.f );
	m_bConsumingAir = TRUE;
}

// 2013 0x6cf570: m_fApneaMaxDuration raised by the breath-capacity upgrade attribute (a fraction)
FLOAT ADishonoredPlayerPawn::GetApneaDuration() const
{
	static const FName NAME_BreathCapacityIncrease( TEXT("Attribute_BreathCapacityIncrease") );
	const FLOAT Duration = DisIFWaterTweaks( const_cast<ADishonoredPlayerPawn*>( this ) )->m_fApneaMaxDuration;
	return GetAttributeValue( NAME_BreathCapacityIncrease ) * Duration + Duration;
}

// 2013 0x6c46a0: the exit lens effect, if one is still up, is removed from the camera and destroyed
void ADishonoredPlayerPawn::OnCameraEnterWaterVolume()
{
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	if( Camera && m_pWaterExitEffect && !m_pWaterExitEffect->bDeleteMe && !m_pWaterExitEffect->IsPendingKill() )
	{
		Camera->RemoveCameraLensEffect( m_pWaterExitEffect );
		GWorld->DestroyActor( m_pWaterExitEffect, FALSE, TRUE );
		m_pWaterExitEffect = NULL;
	}
}

/**
 * 2013 0x6cf5e0. With the tweaks' exit effect configured: the previous one is removed, a new ADisEmitterCameraLensEffect is added
 * to the camera with the configured tweaks.
 * DISHONORED(bringup): its tweak application and ActivateLensEffect belong to ADisEmitterCameraLensEffect, a skeleton here; named.
 */
void ADishonoredPlayerPawn::OnCameraExitWaterVolume()
{
	UDisTweaks_PlayerPawn* Tweaks = DisIFWaterTweaks( this );
	ADishonoredPlayerCamera* Camera = GetDishonoredCamera();
	if( Camera == NULL || Tweaks->m_pWaterExitEffectTweaks == NULL )
	{
		return;
	}
	OnCameraEnterWaterVolume();
	m_pWaterExitEffect = (ADisEmitterCameraLensEffect*)Camera->AddCameraLensEffect( ADisEmitterCameraLensEffect::StaticClass() );
	IFDIAG_ONCE( TEXT("OnCameraExitWaterVolume: ADisEmitterCameraLensEffect (tweaks, ActivateLensEffect) is a skeleton; the effect is spawned, not activated") );
}
