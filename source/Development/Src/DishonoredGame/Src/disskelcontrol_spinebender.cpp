// DishonoredGame/src/disskelcontrol_spinebender.cpp
// DISHONORED(port): agent FT (PHASE17 FT). The spine bender: a USkelControlSingleBone whose BoneRotation pitch and yaw
// chase two targets the pawn sets in degrees. Live on every NPC at spine_0_jnt with ControlStrength 1.00; until this
// package USkelControlBase's empty CalculateNewBoneTransforms made it a silent no-op. Retail's unit has 9 functions
// (2012 rvas 0x6d0280..0x6f4df0); the three class-registration ones are the generated registrant's.
// DISHONORED(bringup): the pawn side that sets the targets - ADishonoredNPCPawn::Tick_SpineBending (2013 rva 0x77fbd0),
// Tick_SpineBending_Apply (0x77b3d0), IsActionSpineBendingEnabled (0x7772c0), IsSlopeOffsettingEnabled (0x77b300) - is
// reached from ADishonoredNPCPawn::Tick_Body (0x7be180 in 2012), which this tree has not ported, so the targets stay at
// the cooked defaults and the control composes the bone it was given. Inventoried in agentFT.md.

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x68cbe0 (2012 0x6dae30), 256 bytes. 0.0054931641 is 360/65536 (unreal units to degrees),
// 182.04445 is 65536/360. An axis stops interpolating the tick it reaches or passes its target.
void UDisSkelControl_SpineBender::TickSkelControl( FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp )
{
	// agent FT: -flheaddiag names every spine bender once - its owner, strength and the rotation it composes - because
	// the control went from a no-op to live on every anim tree that carries one, the player's included.
	extern UBOOL DisFLHeadDiagEnabled();
	static TArray<UDisSkelControl_SpineBender*> Reported;
	if( DisFLHeadDiagEnabled() && !Reported.ContainsItem( this ) )
	{
		Reported.AddItem( this );
		debugf( TEXT("DISHONORED(bringup): flhead spinebender %s on %s (%s): strength %.2f rot(%i,%i,%i) applyRot %i addRot %i rotspace %i | applyTrans %i addTrans %i trans(%.1f,%.1f,%.1f) transspace %i | targets pitch %.1f yaw %.1f"),
			*GetName(), SkelComp && SkelComp->GetOwner() ? *SkelComp->GetOwner()->GetName() : TEXT("none"),
			SkelComp && SkelComp->SkeletalMesh ? *SkelComp->SkeletalMesh->GetName() : TEXT("none"),
			ControlStrength, BoneRotation.Pitch, BoneRotation.Yaw, BoneRotation.Roll, (INT)bApplyRotation, (INT)bAddRotation, (INT)BoneRotationSpace,
			(INT)bApplyTranslation, (INT)bAddTranslation, BoneTranslation.X, BoneTranslation.Y, BoneTranslation.Z, (INT)BoneTranslationSpace,
			m_fCurBendPitchTarget, m_fCurBendYawTarget );
	}
	if( m_bInterpolatingPitch )
	{
		const FLOAT CurrentPitch = (FLOAT)BoneRotation.Pitch * 0.0054931641f;
		const FLOAT Step = m_fCurBendPitchBlendSpeed * DeltaSeconds;
		FLOAT NewPitch;
		UBOOL bStillGoing;
		if( m_fCurBendPitchTarget <= CurrentPitch )
		{
			NewPitch = CurrentPitch - Step;
			bStillGoing = m_fCurBendPitchTarget <= NewPitch;
		}
		else
		{
			NewPitch = CurrentPitch + Step;
			bStillGoing = NewPitch <= m_fCurBendPitchTarget;
		}
		if( !bStillGoing )
		{
			m_bInterpolatingPitch = FALSE;
			NewPitch = m_fCurBendPitchTarget;
		}
		BoneRotation.Pitch = appTrunc( NewPitch * 182.04445f );
	}
	if( m_bInterpolatingYaw )
	{
		const FLOAT CurrentYaw = (FLOAT)BoneRotation.Yaw * 0.0054931641f;
		const FLOAT Step = m_fBendYawBlendSpeed * DeltaSeconds;
		FLOAT NewYaw;
		UBOOL bStillGoing;
		if( m_fCurBendYawTarget <= CurrentYaw )
		{
			NewYaw = CurrentYaw - Step;
			bStillGoing = m_fCurBendYawTarget <= NewYaw;
		}
		else
		{
			NewYaw = CurrentYaw + Step;
			bStillGoing = NewYaw <= m_fCurBendYawTarget;
		}
		if( !bStillGoing )
		{
			m_bInterpolatingYaw = FALSE;
			NewYaw = m_fCurBendYawTarget;
		}
		BoneRotation.Yaw = appTrunc( NewYaw * 182.04445f );
	}
	Super::TickSkelControl( DeltaSeconds, SkelComp );
}

// DISHONORED(port): 2013 rva 0x68cce0 (2012 0x6daf30), 309 bytes. The stock single-bone transform, then the bone's
// current (pre-controller) space base kept as a matrix, which is what the pawn's spine-bend geometry reads back.
void UDisSkelControl_SpineBender::CalculateNewBoneTransforms( INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FBoneAtom>& OutBoneTransforms )
{
	Super::CalculateNewBoneTransforms( BoneIndex, SkelComp, OutBoneTransforms );
	if( SkelComp && SkelComp->SpaceBases.IsValidIndex( BoneIndex ) )
	{
		m_UnbentBoneTransform = SkelComp->SpaceBases( BoneIndex ).ToMatrix();
	}
}

// DISHONORED(port): 2013 rva 0x67eb90 (2012 0x6d02a0), 70 bytes.
void UDisSkelControl_SpineBender::EnableSpineBending( UBOOL _bEnable, UBOOL _bSetInstantly )
{
	if( _bSetInstantly )
	{
		SetSkelControlStrength( _bEnable ? 1.f : 0.f, 0.f );
	}
	else
	{
		SetSkelControlActive( _bEnable );
	}
}

// DISHONORED(port): 2013 rva 0x67ebe0 (2012 0x6d02f0), 23 bytes.
UBOOL UDisSkelControl_SpineBender::IsSpineBendingEnabled()
{
	return ControlStrength > 0.f;
}

// DISHONORED(port): 2013 rva 0x684370 (2012 0x6d6150), 96 bytes. Without a speed the default blend speed applies.
void UDisSkelControl_SpineBender::SetBendPitchTarget( FLOAT _fBendPitchTarget, UBOOL _bSetInstantly, const FLOAT* const _pfBendPitchBlendSpeed )
{
	m_fCurBendPitchTarget = _fBendPitchTarget;
	if( _bSetInstantly )
	{
		m_bInterpolatingPitch = FALSE;
		BoneRotation.Pitch = appTrunc( _fBendPitchTarget * 182.04445f );
	}
	else
	{
		m_bInterpolatingPitch = TRUE;
		m_fCurBendPitchBlendSpeed = _pfBendPitchBlendSpeed ? *_pfBendPitchBlendSpeed : m_fBendPitchDefaultBlendSpeed;
	}
}

// DISHONORED(port): 2013 rva 0x6843d0 (2012 0x6d61b0), 65 bytes.
void UDisSkelControl_SpineBender::SetBendYawTarget( FLOAT _fBendYawTarget, UBOOL _bSetInstantly )
{
	m_fCurBendYawTarget = _fBendYawTarget;
	if( _bSetInstantly )
	{
		m_bInterpolatingYaw = FALSE;
		BoneRotation.Yaw = appTrunc( _fBendYawTarget * 182.04445f );
	}
	else
	{
		m_bInterpolatingYaw = TRUE;
	}
}
