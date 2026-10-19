// DishonoredGame/src/dishonoredplayercontroller_tracking.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (18):
//   0x6d14c0  public: void __thiscall FDisPlayerViewTarget::SetTarget(class AActor *, class USkeletalMeshComponent *, class FName, class FVector const &, float, int, int, float, float, int, int, float)
//   0x6d15b0  public: void __thiscall FDisPlayerViewTarget::SetTarget(int, float, int, int, float, float, int, int, float)
//   0x6d1670  public: void __thiscall FDisPlayerViewTarget::ClearTarget(int)
//   0x6d1690  public: void __thiscall ADishonoredPlayerController::StopTrackingTarget(int)
//   0x6d16e0  protected: int __thiscall ADishonoredPlayerController::FetchTrackedTargetRot(struct FDisPlayerViewTarget &, unsigned int, class FVector const &)
//   0x6d1a20  public: void __thiscall ADishonoredPlayerController::StopLimitingPlayerPitch(void)
//   0x6d1a30  public: void __thiscall ADishonoredPlayerController::StopLimitingPlayerYaw(void)
//   0x6d7420  public: void __thiscall FDisPlayerViewTarget::ApplyPlayerControlDelta(int)
//   0x6d74d0  public: void __thiscall ADishonoredPlayerController::LimitPlayerControlledPitchFromCurrent(float, float)
//   0x6d7540  public: void __thiscall ADishonoredPlayerController::LimitPlayerControlledYawFromCurrent(float, float)
//   0x6e3bc0  public: float __thiscall ADishonoredPlayerController::GetCurMouseSensitivity(float)
//   0x6e3ce0  public: float __thiscall ADishonoredPlayerController::GetCurJoySensitivity(float)
//   0x6e3e00  public: virtual void __thiscall ADishonoredPlayerController::ProcessViewRotation(float, class FRotator &, class FRotator)
//   0x6f8840  public: void __thiscall FDisPlayerViewTarget::TickTracking(float)
//   0x6f8960  public: int __thiscall ADishonoredPlayerController::StartTrackingActor(class AActor *, class USkeletalMeshComponent *, class FName, class FVector const &, float, int, struct FDisPlayerViewTracking_ExtraParams const *)
//   0x6f8b90  public: int __thiscall ADishonoredPlayerController::StartTrackingFixedAngles(int, int, float, int, struct FDisPlayerViewTracking_ExtraParams const *)
//   0x6f9c60  public: int __thiscall ADishonoredPlayerController::StartTrackingPawn_ByRegion(class ADishonoredPawn *, enum eDisHitRegion, enum eDisRegionFocusType, float, int, struct FDisPlayerViewTracking_ExtraParams const *)
//   0x6f9ce0  protected: void __thiscall ADishonoredPlayerController::TickCameraTracking(float, class FVector const &)

/*-----------------------------------------------------------------------------
	Agent HD (PHASE22 HD): the fixed-angle view tracking the PlayerControl camera influence starts when it takes the controller back
	after a soiree (UDishonoredCamera_PlayerControl::GatherInfluence_Derived 2013 0x6cba40: StartTrackingFixedAngles( target pitch,
	target yaw, m_TransitionSpeed, 0x22 ), so the yaw turns from the pawn's to the remembered target at the influence's transition
	speed instead of jumping). Ported: StartTrackingFixedAngles 2012 0x6f8b90 / 2013 0x6af6f0 (538 B both), the fixed-angle
	FDisPlayerViewTarget::SetTarget 0x6d15b0 / 0x6a0da0, ClearTarget 0x6d1670 / 0x6a0e60, TickTracking 0x6f8840 / 0x6af3a0,
	StopTrackingTarget 0x6d1690 / 0x6a0ec0, FetchTrackedTargetRot 0x6d16e0 / 0x6a0f10 (2013 drops the player delta from the untracked
	answer), TickCameraTracking 0x6f9ce0 / 0x6b4e60 (from ADishonoredPlayerController::OnCameraUpdate pass 0, 2013 0x6ba860).
	Agent IA (PHASE23 IA) added ProcessViewRotation 0x6e3e00 / 0x6afb50 and FDisPlayerViewTarget::ApplyPlayerControlDelta 0x6d7420 /
	0x6a34d0 (deviations at the function: the input dilation is the world's, the ranged dispersion is reported, not applied) and the
	2013 doubled zoom blend speed in TickCameraTracking.
	Not ported: the actor/pawn starts (StartTrackingActor, StartTrackingPawn_ByRegion, the actor SetTarget), the Limit*FromCurrent /
	StopLimiting* setters and the sensitivity getters.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkHD_PlayerControllerTracking()
{
}

static const FLOAT DisDegreesToUnrealUnits = 65536.f / 360.f;

// DISHONORED(port): 2012 0x6d15b0 / 2013 0x6a0da0. With flag 0x20 the blend argument is already a speed (weight per second)
void FDisPlayerViewTarget::SetTarget( INT _TargetFixedAngle, FLOAT _fBlendTime, INT _Flags, INT _StartRot, FLOAT _fMaxRadius, FLOAT _fMinRadius, INT _MaxPlayerControlledDelta, INT _PlayerControlledDelta_SoftenThreshold, FLOAT _fZoomFOVOverride )
{
	m_bHasCachedBonePos = FALSE;
	m_MaxPlayerControlledDelta = _MaxPlayerControlledDelta;
	m_fLimitRadius_Max = _fMaxRadius;
	m_TargetFixedRot = _TargetFixedAngle;
	m_PlayerControlledDelta_SoftenThreshold = _PlayerControlledDelta_SoftenThreshold;
	m_fLimitRadius_Min = _fMinRadius;
	m_Flags = _Flags;
	m_CurPlayerControlledDelta = 0;
	m_CurPlayerControlledDelta_Blended = 0;
	m_SoftenType = 0;
	m_TrackType = 2;
	m_fZoomFOVOverride = _fZoomFOVOverride;
	if( _Flags & 0x20 )
	{
		m_fBlendSpeed = _fBlendTime;
		m_fCurWeight = 0.f;
	}
	else if( _fBlendTime <= 0.0001f )
	{
		m_fCurWeight = 1.f;
		m_fBlendSpeed = 0.f;
	}
	else
	{
		m_fCurWeight = 0.f;
		m_fBlendSpeed = 1.f / _fBlendTime;
	}
	m_StartRot = _StartRot;
}

// DISHONORED(port): 2012 0x6d1670 / 2013 0x6a0e60
void FDisPlayerViewTarget::ClearTarget( INT _TrackID )
{
	if( m_TrackID == _TrackID )
	{
		m_TrackType = 0;
		m_pTarget = NULL;
		m_TrackID = -1;
	}
}

// DISHONORED(port): 2012 0x6f8840 / 2013 0x6af3a0. The weight climbs at the blend speed; a target without flag 4 (hold) is cleared
// when it arrives. The player-controlled delta is softened at the input tweaks' m_fLookSoftenSpeed (smallest step 18 units).
void FDisPlayerViewTarget::TickTracking( FLOAT _fDeltaTime )
{
	if( !IsTracking() )
	{
		return;
	}
	if( m_fBlendSpeed > 0.0001f )
	{
		m_fCurWeight += m_fBlendSpeed * _fDeltaTime;
		if( m_fCurWeight >= 1.f )
		{
			if( !( m_Flags & 4 ) )
			{
				ClearTarget( m_TrackID );
			}
			m_fCurWeight = 1.f;
		}
	}
	if( m_SoftenType )
	{
		const UDisTweaks_PlayerInput* Tweaks = DisGetPlayerInputTweaks();
		m_CurPlayerControlledDelta_Blended = appTrunc( DisInterpolateBySpeedWithDif( (FLOAT)m_CurPlayerControlledDelta_Blended, (FLOAT)m_CurPlayerControlledDelta, Tweaks ? Tweaks->m_fLookSoftenSpeed : 0.f, 18.f, _fDeltaTime ) );
		if( m_CurPlayerControlledDelta_Blended == m_CurPlayerControlledDelta )
		{
			m_SoftenType = 0;
		}
	}
	else
	{
		m_CurPlayerControlledDelta_Blended = m_CurPlayerControlledDelta;
	}
}

/**
 * DISHONORED(port): 2012 0x6f8b90 / 2013 0x6af6f0. Flag 2 tracks the yaw, flag 1 the pitch; a held (flag 4) target already running
 * refuses a new one unless flag 0x80 forces it. Each target starts from the controller's current angle; the player may push it by
 * the params' delta (degrees), softened over the tweaks' percent of it, at most m_fMaxLookSoftenAngle. Returns the new track id,
 * or -1 when refused.
 */
INT ADishonoredPlayerController::StartTrackingFixedAngles( INT _PitchFixed, INT _YawFixed, FLOAT _fBlendTime, INT _Flags, const FDisPlayerViewTracking_ExtraParams* _pParams )
{
	FDisPlayerViewTarget& YawTarget = m_ViewTracking.m_ViewYawTarget;
	FDisPlayerViewTarget& PitchTarget = m_ViewTracking.m_ViewPitchTarget;
	UBOOL bYawFree = FALSE;
	if( _Flags & 2 )
	{
		bYawFree = !YawTarget.IsTracking() || !( YawTarget.m_Flags & 4 ) || ( _Flags & 0x80 );
	}
	if( _Flags & 1 )
	{
		if( PitchTarget.IsTracking() && ( PitchTarget.m_Flags & 4 ) && !( _Flags & 0x80 ) )
		{
			return -1;
		}
	}
	else if( !bYawFree )
	{
		return -1;
	}
	FDisPlayerViewTracking_ExtraParams DefaultParams( EC_EventParm );
	const FDisPlayerViewTracking_ExtraParams* Params = _pParams ? _pParams : &DefaultParams;
	const INT TrackID = ++m_ViewTracking.m_CurTrackID;
	const UDisTweaks_PlayerInput* Tweaks = DisGetPlayerInputTweaks();
	const INT MaxSoftenAngle = appTrunc( ( Tweaks ? Tweaks->m_fMaxLookSoftenAngle : 0.f ) * DisDegreesToUnrealUnits );
	const FLOAT SoftenPercent = Tweaks ? Tweaks->m_fLookSoftenPercent : 0.f;
	if( _Flags & 2 )
	{
		const INT MaxDelta = appTrunc( Params->m_fMaxPlayerControlledDelta_Yaw * DisDegreesToUnrealUnits );
		YawTarget.SetTarget( _YawFixed, _fBlendTime, _Flags, Rotation.Yaw, Params->m_fMaxRadius, Params->m_fMinRadius, MaxDelta,
			Min( MaxSoftenAngle, appTrunc( (FLOAT)MaxDelta * SoftenPercent ) ), Params->m_fZoomFOVOverride );
		YawTarget.m_TrackID = TrackID;
	}
	if( !( _Flags & 1 ) )
	{
		return TrackID;
	}
	const INT MaxDelta = appTrunc( Params->m_fMaxPlayerControlledDelta_Pitch * DisDegreesToUnrealUnits );
	PitchTarget.SetTarget( _PitchFixed, _fBlendTime, _Flags, Rotation.Pitch, Params->m_fMaxRadius, Params->m_fMinRadius, MaxDelta,
		Min( MaxSoftenAngle, appTrunc( (FLOAT)MaxDelta * SoftenPercent ) ), Params->m_fZoomFOVOverride );
	PitchTarget.m_TrackID = TrackID;
	return TrackID;
}

// DISHONORED(port): 2012 0x6d1690 / 2013 0x6a0ec0
void ADishonoredPlayerController::StopTrackingTarget( INT _TrackID )
{
	FDisPlayerViewTarget& YawTarget = m_ViewTracking.m_ViewYawTarget;
	FDisPlayerViewTarget& PitchTarget = m_ViewTracking.m_ViewPitchTarget;
	if( YawTarget.m_TrackID == _TrackID )
	{
		YawTarget.m_TrackType = 0;
		YawTarget.m_pTarget = NULL;
		YawTarget.m_TrackID = -1;
	}
	if( PitchTarget.m_TrackID == _TrackID )
	{
		PitchTarget.m_TrackType = 0;
		PitchTarget.m_pTarget = NULL;
		PitchTarget.m_TrackID = -1;
	}
}

/**
 * DISHONORED(port): 2013 0x6a0f10 (2012 0x6d16e0 also adds the player delta when nothing is tracked). Untracked: the controller's
 * own angle. A fixed target (type 2) gives its angle; an actor target (type 1) the direction from the camera to its look bone (the
 * offset in the bone's frame, cached relative to the controller with flag 0x200) or, without one, to its collision bounds' centre
 * or its origin. The answer is the start angle lerped toward it by the weight (cosine-eased with flag 0x40) plus the blended
 * player delta.
 */
INT ADishonoredPlayerController::FetchTrackedTargetRot( FDisPlayerViewTarget& _rTarget, UBOOL _bIsPitch, const FVector& _rCurCamPos )
{
	if( !_rTarget.IsTracking() )
	{
		return _bIsPitch ? Rotation.Pitch : Rotation.Yaw;
	}
	INT TargetRot = 0;
	if( _rTarget.m_TrackType == 2 )
	{
		TargetRot = _rTarget.m_TargetFixedRot;
	}
	else if( _rTarget.m_TrackType == 1 )
	{
		FVector LookPos;
		INT BoneIndex = INDEX_NONE;
		const UBOOL bLookBone = _rTarget.m_LookBone != NAME_None && _rTarget.m_pMesh;
		if( bLookBone && ( _rTarget.m_Flags & 0x200 ) && _rTarget.m_bHasCachedBonePos )
		{
			LookPos = _rTarget.m_CachedBonePos + Location;
		}
		else if( bLookBone && ( BoneIndex = _rTarget.m_pMesh->MatchRefBone( _rTarget.m_LookBone ) ) != INDEX_NONE )
		{
			LookPos = _rTarget.m_pMesh->GetBoneMatrix( BoneIndex ).TransformFVector( _rTarget.m_LookBoneOffset );
			_rTarget.m_bHasCachedBonePos = TRUE;
			_rTarget.m_CachedBonePos = LookPos - Location;
		}
		else if( _rTarget.m_pTarget->CollisionComponent )
		{
			LookPos = _rTarget.m_pTarget->CollisionComponent->Bounds.Origin;
		}
		else
		{
			LookPos = _rTarget.m_pTarget->LocalToWorld().GetOrigin();
		}
		FVector LookDir = LookPos - _rCurCamPos;
		LookDir.Normalize( 1e-8f );
		const FRotator LookRot = LookDir.Rotation();
		TargetRot = _bIsPitch ? LookRot.Pitch : LookRot.Yaw;
	}
	const FLOAT Weight = ( _rTarget.m_Flags & 0x40 ) ? ( 1.f - appCos( _rTarget.m_fCurWeight * PI ) ) * 0.5f : _rTarget.m_fCurWeight;
	return _rTarget.m_CurPlayerControlledDelta_Blended + DisLerpAngle( _rTarget.m_StartRot, TargetRot, Weight );
}

/**
 * DISHONORED(port): 2012 0x6f9ce0 / 2013 0x6b4e60. Both targets tick; while either was tracking, an actor target with flag 8 keeps
 * the pawn within its max radius of it and one with flag 0x10 outside its min radius (pushed by the weighted overshoot, the
 * tighter target of the two winning), the controller and the pawn take the fetched pitch and yaw, and a target with flag 0x100
 * sets the camera's controller-look FOV (the smaller override and blend speed of the two); otherwise that FOV is cleared.
 */
void ADishonoredPlayerController::TickCameraTracking( FLOAT _fDeltaTime, const FVector& _rCurCamPos )
{
	FDisPlayerViewTarget& YawTarget = m_ViewTracking.m_ViewYawTarget;
	FDisPlayerViewTarget& PitchTarget = m_ViewTracking.m_ViewPitchTarget;
	const UBOOL bWasTracking = YawTarget.IsTracking() || PitchTarget.IsTracking();
	ADishonoredPlayerCamera* Cam = Cast<ADishonoredPlayerCamera>( PlayerCamera );
	YawTarget.TickTracking( _fDeltaTime );
	PitchTarget.TickTracking( _fDeltaTime );
	UBOOL bZoom = FALSE;
	FLOAT ZoomOverride = 0.f;
	FLOAT ZoomBlendSpeed = 0.f;
	if( bWasTracking )
	{
		UBOOL bLimitMax = FALSE;
		UBOOL bLimitMin = FALSE;
		FLOAT LimitMax = 3.4e38f;
		FLOAT LimitMin = 0.f;
		AActor* TargetMax = NULL;
		AActor* TargetMin = NULL;
		FLOAT WeightMax = 0.f;
		FLOAT WeightMin = 0.f;
		if( PitchTarget.IsTracking() && PitchTarget.m_TrackType != 2 )
		{
			if( PitchTarget.m_Flags & 8 )
			{
				LimitMax = PitchTarget.m_fLimitRadius_Max;
				TargetMax = PitchTarget.m_pTarget;
				WeightMax = PitchTarget.m_fCurWeight;
				bLimitMax = TRUE;
			}
			if( PitchTarget.m_Flags & 0x10 )
			{
				LimitMin = PitchTarget.m_fLimitRadius_Min;
				TargetMin = PitchTarget.m_pTarget;
				WeightMin = PitchTarget.m_fCurWeight;
				bLimitMin = TRUE;
			}
		}
		if( YawTarget.IsTracking() && YawTarget.m_TrackType != 2 )
		{
			if( YawTarget.m_Flags & 8 )
			{
				if( LimitMax > YawTarget.m_fLimitRadius_Max )
				{
					LimitMax = YawTarget.m_fLimitRadius_Max;
					TargetMax = YawTarget.m_pTarget;
					WeightMax = YawTarget.m_fCurWeight;
				}
				bLimitMax = TRUE;
			}
			if( YawTarget.m_Flags & 0x10 )
			{
				if( YawTarget.m_fLimitRadius_Min > LimitMin )
				{
					LimitMin = YawTarget.m_fLimitRadius_Min;
					TargetMin = YawTarget.m_pTarget;
					WeightMin = YawTarget.m_fCurWeight;
				}
				bLimitMin = TRUE;
			}
		}
		if( bLimitMax )
		{
			const FVector Delta( TargetMax->LocalToWorld().GetOrigin() - Pawn->LocalToWorld().GetOrigin() );
			const FLOAT Dist = Delta.Size2D();
			if( Dist > LimitMax )
			{
				Pawn->moveSmooth( FVector( Delta.X / Dist, Delta.Y / Dist, 0.f ) * ( ( Dist - LimitMax ) * WeightMax ) );
			}
		}
		if( bLimitMin )
		{
			const FVector Delta( TargetMin->LocalToWorld().GetOrigin() - Pawn->LocalToWorld().GetOrigin() );
			const FLOAT Dist = Delta.Size2D();
			if( LimitMin > Dist )
			{
				Pawn->moveSmooth( FVector( Delta.X / Dist, Delta.Y / Dist, 0.f ) * -( ( LimitMin - Dist ) * WeightMin ) );
			}
		}
		const INT NewPitch = FetchTrackedTargetRot( PitchTarget, TRUE, _rCurCamPos );
		const INT NewYaw = FetchTrackedTargetRot( YawTarget, FALSE, _rCurCamPos );
		FRotator FinalRot( NewPitch, NewYaw, Rotation.Roll );
		FinalRot.MakeShortestRoute();
		SetRotation( FinalRot );
		Pawn->eventUpdatePawnRotation( FinalRot );
		if( PitchTarget.IsTracking() && ( PitchTarget.m_Flags & 0x100 ) )
		{
			ZoomOverride = PitchTarget.m_fZoomFOVOverride;
			ZoomBlendSpeed = PitchTarget.m_fBlendSpeed;
			bZoom = TRUE;
		}
		if( YawTarget.IsTracking() && ( YawTarget.m_Flags & 0x100 ) )
		{
			ZoomOverride = Min( ZoomOverride, YawTarget.m_fZoomFOVOverride );
			ZoomBlendSpeed = Min( ZoomBlendSpeed, YawTarget.m_fBlendSpeed );
			bZoom = TRUE;
		}
	}
	if( Cam )
	{
		// agent IA, against the bytes of 2013 0x6b4e60 (0x6b5228..0x6b52ef): the Min of the two overrides starting from 0 is
		// retail's own (a yaw-only zoom asks for FOV 0, which TickFOV then skips), and 2013 doubles the blend speed (mulss 2.0;
		// 2012 0x6f9ce0 passes it as is)
		if( bZoom )
		{
			Cam->SetFOVTarget( eDisCamFOVPriority_ControllerLook, ZoomOverride, ZoomBlendSpeed * 2.f, FALSE );
		}
		else
		{
			Cam->ClearFOVTarget( eDisCamFOVPriority_ControllerLook );
		}
	}
	if( PitchTarget.m_TrackID == -1 )
	{
		PitchTarget.m_pTarget = NULL;
	}
	if( YawTarget.m_TrackID == -1 )
	{
		YawTarget.m_pTarget = NULL;
	}
}

/**
 * DISHONORED(port): agent IA (PHASE23 IA), 2012 0x6d7420 / 2013 0x6a34d0 (same bytes). While the target is not tracking, or its
 * weight is full, the player's look input moves the controlled delta inside +-m_MaxPlayerControlledDelta; passing the soften
 * threshold toward an edge marks that side (1 positive, 2 negative), coming back past the blended delta clears it.
 */
void FDisPlayerViewTarget::ApplyPlayerControlDelta( INT _Delta )
{
	const INT MaxDelta = m_MaxPlayerControlledDelta;
	if( MaxDelta == 0 )
	{
		return;
	}
	if( IsTracking() && Abs( m_fCurWeight - 1.f ) >= 0.0001f )
	{
		return;
	}
	m_CurPlayerControlledDelta = Clamp( m_CurPlayerControlledDelta + _Delta, -MaxDelta, MaxDelta );
	if( _Delta <= 0 )
	{
		if( m_SoftenType == 1 )
		{
			if( m_CurPlayerControlledDelta < m_CurPlayerControlledDelta_Blended )
			{
				m_SoftenType = 0;
			}
		}
		else if( m_CurPlayerControlledDelta < m_PlayerControlledDelta_SoftenThreshold - MaxDelta )
		{
			m_SoftenType = 2;
		}
	}
	else if( m_SoftenType == 2 )
	{
		if( m_CurPlayerControlledDelta > m_CurPlayerControlledDelta_Blended )
		{
			m_SoftenType = 0;
		}
	}
	else if( m_CurPlayerControlledDelta > MaxDelta - m_PlayerControlledDelta_SoftenThreshold )
	{
		m_SoftenType = 1;
	}
}

/**
 * DISHONORED(port): agent IA (PHASE23 IA), 2013 rva 0x6afb50 (2012 0x6e3e00, 756 B, ratio .986). With a pawn the input delta is
 * scaled by the input time dilation. While either view target tracks, its axis of the delta goes into the target's player-controlled
 * delta (ApplyPlayerControlDelta) and is removed from the view; otherwise the Kismet pitch/yaw limits (m_bLimitPlayerPitch /
 * m_bLimitPlayerYaw, degrees around m_fStartingPlayerPitch / Yaw) cut the delta at the limit. A ranged weapon in the secondary hand
 * gains dispersion from the look speed. Then APlayerController::ProcessViewRotation (camera modifiers, pawn).
 * DISHONORED(bringup), named: ADishonoredGameInfo::GetBendTimeInputDilation (2013 vtable +1076, 2012 0x62fb00) has no body here,
 * so the world's TimeDilation is the scale (the branch retail takes without a game info; both are 1 outside bend time);
 * UDishonoredWeapon_Ranged::IncreaseDispersion (2013 0x81cba0, 2012 0x886c60) is not ported, so the dispersion is reported once.
 */
void ADishonoredPlayerController::ProcessViewRotation( FLOAT DeltaTime, FRotator& out_ViewRotation, FRotator DeltaRot )
{
	static const FLOAT DisUnrealUnitsToDegrees = 360.f / 65536.f;
	if( Pawn )
	{
		DeltaRot *= WorldInfo->TimeDilation;
	}
	FDisPlayerViewTarget& YawTarget = m_ViewTracking.m_ViewYawTarget;
	FDisPlayerViewTarget& PitchTarget = m_ViewTracking.m_ViewPitchTarget;
	if( YawTarget.IsTracking() || PitchTarget.IsTracking() )
	{
		if( PitchTarget.IsTracking() )
		{
			PitchTarget.ApplyPlayerControlDelta( DeltaRot.Pitch );
			DeltaRot.Pitch = 0;
		}
		if( YawTarget.IsTracking() )
		{
			YawTarget.ApplyPlayerControlDelta( DeltaRot.Yaw );
			DeltaRot.Yaw = 0;
		}
	}
	else
	{
		const FLOAT CurPitch = FRotator::NormalizeAxis( appTrunc( ( Rotation.Pitch * DisUnrealUnitsToDegrees - m_fStartingPlayerPitch ) * DisDegreesToUnrealUnits ) ) * DisUnrealUnitsToDegrees;
		const FLOAT CurYaw = FRotator::NormalizeAxis( appTrunc( ( Rotation.Yaw * DisUnrealUnitsToDegrees - m_fStartingPlayerYaw ) * DisDegreesToUnrealUnits ) ) * DisUnrealUnitsToDegrees;
		if( m_bLimitPlayerPitch )
		{
			const FLOAT NewPitch = DeltaRot.Pitch * DisUnrealUnitsToDegrees + CurPitch;
			if( m_fLimitPlayerPitch_Min > NewPitch )
			{
				DeltaRot.Pitch = appTrunc( ( m_fLimitPlayerPitch_Min - CurPitch ) * DisDegreesToUnrealUnits );
			}
			if( NewPitch > m_fLimitPlayerPitch_Max )
			{
				DeltaRot.Pitch = appTrunc( ( m_fLimitPlayerPitch_Max - CurPitch ) * DisDegreesToUnrealUnits );
			}
		}
		if( m_bLimitPlayerYaw )
		{
			const FLOAT NewYaw = DeltaRot.Yaw * DisUnrealUnitsToDegrees + CurYaw;
			if( m_fLimitPlayerYaw_Min > NewYaw )
			{
				DeltaRot.Yaw = appTrunc( ( m_fLimitPlayerYaw_Min - CurYaw ) * DisDegreesToUnrealUnits );
			}
			if( NewYaw > m_fLimitPlayerYaw_Max )
			{
				DeltaRot.Yaw = appTrunc( ( m_fLimitPlayerYaw_Max - CurYaw ) * DisDegreesToUnrealUnits );
			}
		}
	}
	ADishonoredPlayerPawn* PlayerPawn = Cast<ADishonoredPlayerPawn>( Pawn );
	if( PlayerPawn && PlayerPawn->m_pInventory && Cast<UDishonoredWeapon_Ranged>( PlayerPawn->m_pInventory->GetEquippedItem( EDisEquipUsage_Secondary ) ) )
	{
		static UBOOL bSaid = FALSE;
		if( !bSaid )
		{
			bSaid = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): ADishonoredPlayerController::ProcessViewRotation: UDishonoredWeapon_Ranged::IncreaseDispersion (2013 0x81cba0) is not ported; the ranged weapon's look dispersion does not grow") );
		}
	}
	APlayerController::ProcessViewRotation( DeltaTime, out_ViewRotation, DeltaRot );
}

// DISHONORED(port): agent IA, the script entry of the virtual above (DishonoredPlayerController.uc ProcessViewRotation)
void ADishonoredPlayerController::execProcessViewRotation( FFrame& Stack, RESULT_DECL )
{
	P_GET_FLOAT( DeltaTime );
	P_GET_ROTATOR_REF( out_ViewRotation );
	P_GET_ROTATOR( DeltaRot );
	P_FINISH;
	ProcessViewRotation( DeltaTime, out_ViewRotation, DeltaRot );
}
