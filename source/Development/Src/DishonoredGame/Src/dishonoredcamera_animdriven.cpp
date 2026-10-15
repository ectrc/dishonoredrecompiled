// DishonoredGame/src/dishonoredcamera_animdriven.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x721d20  public: static void __cdecl UDishonoredCamera_AnimDriven::InitializePrivateStaticClassUDishonoredCamera_AnimDriven(void)
//   0x721d40  protected: virtual void __thiscall UDishonoredCamera_AnimDriven::OnActivate_Derived(class APawn *)
//   0x721d90  protected: virtual void __thiscall UDishonoredCamera_AnimDriven::OnDeactivate_Derived(class APawn *)
//   0x726e40  public: void __thiscall UDishonoredCamera_AnimDriven::GetAnimDrivenTransform(class ADishonoredPlayerPawn const *, class FVector &, class FRotator &)const
//   0x7298d0  public: static class UClass * __cdecl UDishonoredCamera_AnimDriven::GetPrivateStaticClassUDishonoredCamera_AnimDriven(wchar_t const *)
//   0x729960  public: void __thiscall UDishonoredCamera_AnimDriven::SetPlayerControlledLimit(float, float, float, float, int)
//   0x729aa0  int __cdecl CalcBlendedPlayerDelta(int, int, int, int, int, int, unsigned int, float, enum eDisLookSoftenState &)
//   0x729bb0  public: void __thiscall UDishonoredCamera_AnimDriven::UpdatePlayerControlLimit(class APawn *, float)
//   0x72cf40  public: static class UClass * __cdecl UDishonoredCamera_AnimDriven::StaticClassNoInline(void)
//   0x72cf70  protected: virtual unsigned int __thiscall UDishonoredCamera_AnimDriven::GatherInfluence_Derived(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): the soiree's camera. The view is the player's camera bone - the bone of the full-body mesh the
	matinee's StretchAnimControl track animates - with a player-controlled yaw/pitch offset on top, clamped to the limits
	UDisInterpTrackAllowPlayerLook sets (and to nothing when it sets none). No FOV is written here: the influence leaves the
	camera's FOV at what TickFOV gives it, and the arm-follow weights at 0 so the arms stop following the camera.

	Both databases: GetAnimDrivenTransform 2012 0x726e40 / 2013 0x6c1440 (same 465 B), SetPlayerControlledLimit 0x729960 /
	2013 0x6cb300 (same 306 B), CalcBlendedPlayerDelta 0x729aa0 / 2013 0x6cb440 (same 263 B), UpdatePlayerControlLimit
	0x729bb0 / 2013 0x6cb550 (825 -> 844 B: 2013 skips the whole update when the delta time is nil), GatherInfluence_Derived
	0x72cf70 / 2013 0x6d08a0 (same 315 B), OnActivate_Derived 0x721d40 and OnDeactivate_Derived 0x721d90 (unnamed in 2013).
	The 2013 bodies are what is ported.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_CameraAnimDriven()
{
}

static const FLOAT DisDegreesToUnrealUnits = 65536.f / 360.f;

const UDisTweaks_PlayerInput* DisGetPlayerInputTweaks()
{
	const UDisTweaks_PlayerInput* Tweaks = NULL;
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	UDishonoredPlayerInput* Input = PC ? Cast<UDishonoredPlayerInput>( PC->PlayerInput ) : NULL;
	if( Input )
	{
		Tweaks = Input->m_pTweaks;
	}
	if( !Tweaks )
	{
		Tweaks = (UDisTweaks_PlayerInput*)UDisTweaks_PlayerInput::StaticClass()->GetDefaultObject();
	}
	return Tweaks;
}

// activation snaps the current limits to their targets
void UDishonoredCamera_AnimDriven::OnActivate_Derived( APawn* Pawn )
{
	m_CurMaxPlayerControlledYaw_Neg = m_CurMaxPlayerControlledYaw_Neg_Target;
	m_CurMaxPlayerControlledYaw_Pos = m_CurMaxPlayerControlledYaw_Pos_Target;
	m_CurMaxPlayerControlledPitch_Pos = m_CurMaxPlayerControlledPitch_Pos_Target;
	m_CurMaxPlayerControlledPitch_Neg = m_CurMaxPlayerControlledPitch_Neg_Target;
}

// deactivation clears every limit, offset and soften state
void UDishonoredCamera_AnimDriven::OnDeactivate_Derived( APawn* Pawn )
{
	m_CurMaxPlayerControlledYaw_Neg = 0;
	m_CurMaxPlayerControlledPitch_Neg = 0;
	m_CurMaxPlayerControlledYaw_Pos = 0;
	m_CurMaxPlayerControlledPitch_Pos = 0;
	m_CurMaxPlayerControlledYaw_Neg_Target = 0;
	m_CurMaxPlayerControlledPitch_Neg_Target = 0;
	m_CurMaxPlayerControlledYaw_Pos_Target = 0;
	m_CurMaxPlayerControlledPitch_Pos_Target = 0;
	m_CurPlayerControlledYaw = 0;
	m_CurPlayerControlledPitch = 0;
	m_CurPlayerControlledYaw_Blended = 0;
	m_CurPlayerControlledPitch_Blended = 0;
	m_SoftenState_Pitch = eDisLookSoftenState_NoSoften;
	m_SoftenState_Yaw = eDisLookSoftenState_NoSoften;
	m_PlayerControlledLimitFlags = 0;
}

// the camera bone, rotated by the blended player-controlled yaw/pitch about itself
void UDishonoredCamera_AnimDriven::GetAnimDrivenTransform( const ADishonoredPlayerPawn* Player, FVector& OutLocation, FRotator& OutRotation ) const
{
	Player->GetCameraBone( &OutLocation, &OutRotation );
	if( m_CurPlayerControlledYaw_Blended != 0 || m_CurPlayerControlledPitch_Blended != 0 )
	{
		const FRotationTranslationMatrix CamMat( OutRotation, OutLocation );
		const FRotationTranslationMatrix PlayerControlMat( FRotator( m_CurPlayerControlledPitch_Blended, m_CurPlayerControlledYaw_Blended, 0 ), FVector::ZeroVector );
		const FMatrix Result = PlayerControlMat * CamMat;
		OutLocation = Result.GetOrigin();
		OutRotation = Result.Rotator();
	}
}

/**
 * The limits in degrees become the target limits in Unreal units; the soften thresholds are the tweaks' percent of each
 * range, capped at m_fMaxLookSoftenAngle; four zero limits clear flag bit 0 (no look at all).
 */
void UDishonoredCamera_AnimDriven::SetPlayerControlledLimit( FLOAT MaxDegreesYaw_Neg, FLOAT MaxDegreesYaw_Pos, FLOAT MaxDegreesPitch_Neg, FLOAT MaxDegreesPitch_Pos, INT Flags )
{
	const UDisTweaks_PlayerInput* Tweaks = DisGetPlayerInputTweaks();
	const INT MaxSoftenAngle = appTrunc( ( Tweaks ? Tweaks->m_fMaxLookSoftenAngle : 0.f ) * DisDegreesToUnrealUnits );
	const FLOAT SoftenPercent = Tweaks ? Tweaks->m_fLookSoftenPercent : 0.f;
	m_CurMaxPlayerControlledYaw_Neg_Target = appTrunc( MaxDegreesYaw_Neg * DisDegreesToUnrealUnits );
	m_CurMaxPlayerControlledYaw_Pos_Target = appTrunc( MaxDegreesYaw_Pos * DisDegreesToUnrealUnits );
	m_CurMaxPlayerControlledPitch_Neg_Target = appTrunc( MaxDegreesPitch_Neg * DisDegreesToUnrealUnits );
	m_CurMaxPlayerControlledPitch_Pos_Target = appTrunc( MaxDegreesPitch_Pos * DisDegreesToUnrealUnits );
	m_PlayerControlledLimitFlags = Flags;
	const INT YawRange = Abs( m_CurMaxPlayerControlledYaw_Pos_Target - m_CurMaxPlayerControlledYaw_Neg_Target );
	const INT PitchRange = Abs( m_CurMaxPlayerControlledPitch_Pos_Target - m_CurMaxPlayerControlledPitch_Neg_Target );
	m_CurPlayerControlledYaw_SoftenThreshold = Min( MaxSoftenAngle, appTrunc( (FLOAT)YawRange * SoftenPercent ) );
	m_CurPlayerControlledPitch_SoftenThreshold = Min( MaxSoftenAngle, appTrunc( (FLOAT)PitchRange * SoftenPercent ) );
	if( m_CurMaxPlayerControlledYaw_Neg_Target == 0 && m_CurMaxPlayerControlledYaw_Pos_Target == 0 && m_CurMaxPlayerControlledPitch_Neg_Target == 0 && m_CurMaxPlayerControlledPitch_Pos_Target == 0 )
	{
		m_PlayerControlledLimitFlags = Flags & ~1;
	}
}

/**
 * DISHONORED(port): 2012 0x729aa0, a free function. Near a limit the player's look is softened: once the unblended angle
 * is within the threshold of the limit in the direction of the turn, the blended angle follows it at the tweaks'
 * m_fLookSoftenSpeed (DisInterpolateBySpeedWithDif, smallest step 18 units) until it catches up.
 */
static INT CalcBlendedPlayerDelta( INT CurTurnDelta, INT UnblendedAngle, INT BlendedAngle, INT TargetAngle_Pos, INT TargetAngle_Neg, INT SoftenThreshold, UBOOL bShouldSoften, FLOAT DeltaTime, BYTE& SoftenStateInOut )
{
	if( !bShouldSoften )
	{
		return UnblendedAngle;
	}
	if( CurTurnDelta != 0 )
	{
		if( CurTurnDelta <= 0 )
		{
			if( SoftenStateInOut == eDisLookSoftenState_Soften_Positive )
			{
				if( UnblendedAngle < BlendedAngle )
				{
					SoftenStateInOut = eDisLookSoftenState_NoSoften;
					return UnblendedAngle;
				}
			}
			else if( UnblendedAngle < SoftenThreshold + TargetAngle_Neg )
			{
				SoftenStateInOut = eDisLookSoftenState_Soften_Negative;
			}
		}
		else if( SoftenStateInOut == eDisLookSoftenState_Soften_Negative )
		{
			if( UnblendedAngle > BlendedAngle )
			{
				SoftenStateInOut = eDisLookSoftenState_NoSoften;
				return UnblendedAngle;
			}
		}
		else if( UnblendedAngle > TargetAngle_Pos - SoftenThreshold )
		{
			SoftenStateInOut = eDisLookSoftenState_Soften_Positive;
		}
	}
	if( SoftenStateInOut == eDisLookSoftenState_NoSoften )
	{
		return UnblendedAngle;
	}
	const UDisTweaks_PlayerInput* Tweaks = DisGetPlayerInputTweaks();
	const INT Result = appTrunc( DisInterpolateBySpeedWithDif( (FLOAT)BlendedAngle, (FLOAT)UnblendedAngle, Tweaks ? Tweaks->m_fLookSoftenSpeed : 0.f, 18.f, DeltaTime ) );
	if( Result == UnblendedAngle )
	{
		SoftenStateInOut = eDisLookSoftenState_NoSoften;
	}
	return Result;
}

/**
 * The limits ease toward their targets (5 per second, snapping within 5 units); the frame's turn and look-up input
 * (UPlayerInput::aTurn_BeforeClear / aLookUp_BeforeClear, scaled by the game info's bend-time dilation or the world's
 * time dilation) is added to the player-controlled yaw and pitch; a limit pair spanning the full circle wraps the angle,
 * any other clamps it and marks the axis for softening; then the blended angles through CalcBlendedPlayerDelta.
 * DISHONORED(bringup): ADishonoredGameInfo::GetBendTimeInputDilation (2012 0x62fb00) has no body here, so the world's
 * TimeDilation branch is the one taken.
 */
void UDishonoredCamera_AnimDriven::UpdatePlayerControlLimit( APawn* Pawn, FLOAT DeltaTime )
{
	if( DeltaTime <= 1e-8f )
	{
		return;
	}
	const FLOAT Alpha = Clamp( DeltaTime * 5.f, 0.f, 1.f );
	const INT YawNegTarget = m_CurMaxPlayerControlledYaw_Neg_Target;
	const INT YawPosTarget = m_CurMaxPlayerControlledYaw_Pos_Target;
	const INT NewYawNeg = appTrunc( (FLOAT)m_CurMaxPlayerControlledYaw_Neg + ( (FLOAT)YawNegTarget - (FLOAT)m_CurMaxPlayerControlledYaw_Neg ) * Alpha );
	const INT NewPitchNeg = appTrunc( (FLOAT)m_CurMaxPlayerControlledPitch_Neg + ( (FLOAT)m_CurMaxPlayerControlledPitch_Neg_Target - (FLOAT)m_CurMaxPlayerControlledPitch_Neg ) * Alpha );
	const INT NewYawPos = appTrunc( (FLOAT)m_CurMaxPlayerControlledYaw_Pos + ( (FLOAT)YawPosTarget - (FLOAT)m_CurMaxPlayerControlledYaw_Pos ) * Alpha );
	const INT NewPitchPos = appTrunc( (FLOAT)m_CurMaxPlayerControlledPitch_Pos + ( (FLOAT)m_CurMaxPlayerControlledPitch_Pos_Target - (FLOAT)m_CurMaxPlayerControlledPitch_Pos ) * Alpha );
	m_CurMaxPlayerControlledPitch_Neg = NewPitchNeg;
	m_CurMaxPlayerControlledPitch_Pos = NewPitchPos;
	m_CurMaxPlayerControlledYaw_Neg = NewYawNeg;
	m_CurMaxPlayerControlledYaw_Pos = NewYawPos;
	if( YawNegTarget - NewYawNeg < 5 )
	{
		m_CurMaxPlayerControlledYaw_Neg = YawNegTarget;
	}
	if( YawPosTarget - NewYawPos < 5 )
	{
		m_CurMaxPlayerControlledYaw_Pos = YawPosTarget;
	}
	if( m_CurMaxPlayerControlledPitch_Neg_Target - NewPitchNeg < 5 )
	{
		m_CurMaxPlayerControlledPitch_Neg = m_CurMaxPlayerControlledPitch_Neg_Target;
	}
	if( m_CurMaxPlayerControlledPitch_Pos_Target - NewPitchPos < 5 )
	{
		m_CurMaxPlayerControlledPitch_Pos = m_CurMaxPlayerControlledPitch_Pos_Target;
	}

	ADishonoredPlayerController* PC = Pawn ? Cast<ADishonoredPlayerController>( Pawn->Controller ) : NULL;
	if( !PC || !PC->PlayerInput )
	{
		return;
	}
	const FLOAT Dilation = ( GWorld && GWorld->GetWorldInfo() ) ? GWorld->GetWorldInfo()->TimeDilation : 1.f;
	const FLOAT CurTurn = PC->PlayerInput->aTurn_BeforeClear * Dilation;
	const FLOAT CurLookUp = PC->PlayerInput->aLookUp_BeforeClear * Dilation;
	INT Yaw = appTrunc( (FLOAT)m_CurPlayerControlledYaw + CurTurn );
	INT Pitch = appTrunc( (FLOAT)m_CurPlayerControlledPitch + CurLookUp );
	UBOOL bSoftenPitch = FALSE;
	UBOOL bSoftenYaw = FALSE;
	if( (INT)(WORD)( m_CurMaxPlayerControlledPitch_Neg + 32766 ) - 0x7FFF == m_CurMaxPlayerControlledPitch_Pos - 1 )
	{
		Pitch = (INT)(WORD)( Pitch + 0x7FFF ) - 0x7FFF;
	}
	else
	{
		bSoftenPitch = TRUE;
	}
	if( (INT)(WORD)( m_CurMaxPlayerControlledYaw_Neg + 32766 ) - 0x7FFF == m_CurMaxPlayerControlledYaw_Pos - 1 )
	{
		Yaw = (INT)(WORD)( Yaw + 0x7FFF ) - 0x7FFF;
	}
	else
	{
		bSoftenYaw = TRUE;
	}
	Yaw = Clamp( Yaw, m_CurMaxPlayerControlledYaw_Neg, m_CurMaxPlayerControlledYaw_Pos );
	Pitch = Clamp( Pitch, m_CurMaxPlayerControlledPitch_Neg, m_CurMaxPlayerControlledPitch_Pos );
	m_CurPlayerControlledYaw = Yaw;
	m_CurPlayerControlledPitch = Pitch;
	m_CurPlayerControlledYaw_Blended = CalcBlendedPlayerDelta( appTrunc( CurTurn ), Yaw, m_CurPlayerControlledYaw_Blended, m_CurMaxPlayerControlledYaw_Pos, m_CurMaxPlayerControlledYaw_Neg, m_CurPlayerControlledYaw_SoftenThreshold, bSoftenYaw, DeltaTime, m_SoftenState_Yaw );
	m_CurPlayerControlledPitch_Blended = CalcBlendedPlayerDelta( appTrunc( CurLookUp ), Pitch, m_CurPlayerControlledPitch_Blended, m_CurMaxPlayerControlledPitch_Pos, m_CurMaxPlayerControlledPitch_Neg, m_CurPlayerControlledPitch_SoftenThreshold, bSoftenPitch, DeltaTime, m_SoftenState_Pitch );
}

/**
 * The view from the camera bone plus the blended player look; the arm-follow settings zeroed (the arms are where the
 * animation puts them, not where the camera is); the limits updated. At full weight: with flag bit 0
 * (m_bMatchControllerRot) the PlayerControl influence is told to reset the controller to this view's pitch and yaw when
 * it takes over again; without it a pending controller-reset target on an inactive PlayerControl is dropped.
 */
UBOOL UDishonoredCamera_AnimDriven::GatherInfluence_Derived( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime )
{
	const ADishonoredPlayerPawn* Player = ConstCast<ADishonoredPlayerPawn>( Pawn );
	if( !Player )
	{
		return FALSE;
	}
	GetAnimDrivenTransform( Player, OutVT.m_VT.POV.Location, OutVT.m_VT.POV.Rotation );
	FDishonoredVTSettings& Settings = OutVT.m_DishonoredSettings;
	Settings.m_ArmFollowOffset_Rot_Primary = FRotator( 0, 0, 0 );
	Settings.m_ArmFollowOffset_Rot_Secondary = FRotator( 0, 0, 0 );
	Settings.m_ArmFollowWeight = 0.f;
	Settings.m_ArmFollowWeight_Rot_Primary = 0.f;
	Settings.m_ArmFollowWeight_Rot_Secondary = 0.f;
	Settings.m_ArmFollowOffset_Weight_Primary = 0.f;
	Settings.m_ArmFollowOffset_Weight_Secondary = 0.f;
	Settings.m_MeshSpecificFOVWeight = 0.f;
	m_Debug_POV_Rotator = OutVT.m_VT.POV.Rotation;
	m_Debug_POV_Location = OutVT.m_VT.POV.Location;
	UpdatePlayerControlLimit( Pawn, TimeDelta );
	if( Pawn->Controller && Abs( m_Weight - 1.f ) < 0.0001f && m_pOwningCam )
	{
		UDishonoredCamera_PlayerControl* PlayerControl = m_pOwningCam->GetCamera_PlayerControl();
		if( m_PlayerControlledLimitFlags & 1 )
		{
			FRotator ControllerRot( OutVT.m_VT.POV.Rotation.Pitch, OutVT.m_VT.POV.Rotation.Yaw, 0 );
			ControllerRot.MakeShortestRoute();
			if( PlayerControl )
			{
				PlayerControl->GoToTargetOnNextControllerReset( ControllerRot );
			}
			return TRUE;
		}
		if( PlayerControl && !PlayerControl->IsActive() )
		{
			PlayerControl->m_bGoToTargetOnControllerReset = FALSE;
		}
	}
	return TRUE;
}
