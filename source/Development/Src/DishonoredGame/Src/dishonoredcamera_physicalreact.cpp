// DishonoredGame/src/dishonoredcamera_physicalreact.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (25):
//   0x721b70  public: static void __cdecl UDishonoredCamera_PhysicalReact::InitializePrivateStaticClassUDishonoredCamera_PhysicalReact(void)
//   0x721b90  public: void __thiscall FDisSpringPoint::StorePreviousPhysResult(void)
//   0x721bc0  public: virtual void __thiscall UDishonoredCamera_PhysicalReact::OnTeleport(class USeqAct_Teleport *)
//   0x725ce0  public: unsigned int __thiscall FDisSpringPoint::IsSpringing(void)const
//   0x725d40  public: void __thiscall FDisSpringPoint::UpdateSpringPos(struct FDisSpringConstant const &, float, class UObject *, void (__cdecl *)(class UObject *, class FVector &))
//   0x725f90  public: unsigned int __thiscall FDisSpringPoint::ApplyMaxConstraint(float, class FVector const *, int)
//   0x7260e0  public: class FVector const __thiscall FDisSpringPoint::GetPos_Interpolated(float)const
//   0x726140  protected: virtual void __thiscall UDishonoredCamera_PhysicalReact::PostBeginPlay_Derived(void)
//   0x7261e0  protected: virtual void __thiscall UDishonoredCamera_PhysicalReact::OnActivate_Derived(class APawn *)
//   0x726310  protected: virtual void __thiscall UDishonoredCamera_PhysicalReact::OnCameraCollided_Derived(class FVector const &)
//   0x726430  protected: virtual unsigned int __thiscall UDishonoredCamera_PhysicalReact::GatherInfluence_FixedTimeStep(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, unsigned int, unsigned int)
//   0x7265e0  protected: virtual unsigned int __thiscall UDishonoredCamera_PhysicalReact::GatherInfluence_Derived(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)
//   0x726a20  public: void __thiscall UDishonoredCamera_PhysicalReact::OnImpact(class FVector const &)
//   0x72c970  public: static class UClass * __cdecl UDishonoredCamera_PhysicalReact::GetPrivateStaticClassUDishonoredCamera_PhysicalReact(wchar_t const *)
//   0x72ca00  public: static class UClass * __cdecl UDishonoredCamera_HitReact::GetPrivateStaticClassUDishonoredCamera_HitReact(wchar_t const *)
//   0x72ca90  public: static class UClass * __cdecl UDishonoredCamera_Shake::GetPrivateStaticClassUDishonoredCamera_Shake(wchar_t const *)
//   0x72cb20  public: static class UClass * __cdecl UDishonoredCamera_Recoil::GetPrivateStaticClassUDishonoredCamera_Recoil(wchar_t const *)
//   0x72f130  public: static class UClass * __cdecl UDishonoredCamera_PhysicalReact::StaticClassNoInline(void)
//   0x72f160  public: static void __cdecl UDishonoredCamera_HitReact::InitializePrivateStaticClassUDishonoredCamera_HitReact(void)
//   0x72f180  public: static void __cdecl UDishonoredCamera_Shake::InitializePrivateStaticClassUDishonoredCamera_Shake(void)
//   0x72f1a0  public: static void __cdecl UDishonoredCamera_Recoil::InitializePrivateStaticClassUDishonoredCamera_Recoil(void)
//   0x7310d0  public: static class UClass * __cdecl UDishonoredCamera_HitReact::StaticClassNoInline(void)
//   0x731100  public: static class UClass * __cdecl UDishonoredCamera_Shake::StaticClassNoInline(void)
//   0x731130  public: static class UClass * __cdecl UDishonoredCamera_Recoil::StaticClassNoInline(void)
//   0x734b20  protected: virtual void __thiscall UDishonoredCamera_PhysicalReact::GatherInfluence_Start(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)

/*-----------------------------------------------------------------------------
	Agent JG (PHASE24 JG): the reaction group's spring (DefaultCamera.ini: stability springiness 85 / strength 75, damping 8, velocity
	capped at 700, arm bounce 0.2 from 20 over 300 uu, a fixed 60 Hz step). An impact (OnImpact: the landing velocity from
	ADishonoredPlayerPawn::Landed_Native) is added to the velocity of both points and wakes the influence; every fixed step each point
	is pulled back to its bound position and the stability point is kept 100 uu from the strength point; the view is offset by the
	strength point less the pivot offset turned by the tilt of the line between the two (yaw taken from the view), the arms follow
	less by the arm bounce, and the influence sleeps again once neither point moves. HitReact, Shake and Recoil derive from it.
	The 2013 bodies are ported (rvas in Inc/CppText/UDishonoredCamera_PhysicalReact.h and FDisSpringPoint.h). Named, as retail has
	it: OnCameraCollided_Derived tests the strength point's dot product for both points; it is reached only through
	ADishonoredPlayerCamera::HandleCollision (2012 0x709e60), the collision branch of UpdateViewTarget that is not ported.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "jgdiag.h"

void DishonoredLinkJG_DishonoredCameraPhysicalReact()
{
}

void FDisSpringPoint::StorePreviousPhysResult()
{
	m_Pos_Previous = m_Pos;
	m_Velocity_Previous = m_Velocity;
}

UBOOL FDisSpringPoint::IsSpringing() const
{
	return ( m_Pos - m_BoundPos ).Size() > 0.01f || m_Velocity.Size() > 0.01f;
}

/**
 * A point away from its bound position or moving accelerates by the spring toward it less the damping (one at rest with a leftover
 * velocity under 1 uu/s loses it in one second), the velocity is capped when the constant says so, an owner's adjust function may
 * change the velocity used for the move, and the point moves.
 */
void FDisSpringPoint::UpdateSpringPos( const FDisSpringConstant& SpringConst, FLOAT DeltaTime, UObject* FuncOwner, void (*VelAdjustFunc)( UObject*, FVector& ) )
{
	const FVector Dif = m_Pos - m_BoundPos;
	FVector Accel;
	if( Dif.SizeSquared() > 0.0001f || m_Velocity.SizeSquared() > 1.f )
	{
		Accel = Dif * -SpringConst.m_Springiness - m_Velocity * SpringConst.m_Damping;
	}
	else
	{
		Accel = -m_Velocity;
	}
	m_Velocity += Accel * DeltaTime;
	if( SpringConst.m_bCapVelocity )
	{
		const FLOAT VelSize = m_Velocity.Size();
		if( VelSize > SpringConst.m_fMaxVelocity )
		{
			m_Velocity *= SpringConst.m_fMaxVelocity / VelSize;
		}
	}
	FVector MoveVelocity = m_Velocity;
	if( FuncOwner && VelAdjustFunc )
	{
		VelAdjustFunc( FuncOwner, MoveVelocity );
	}
	m_Pos += MoveVelocity * DeltaTime;
}

// 2012 0x725f90: pull the point back to MaxDist from FromPos (or its bound position); flag 2 ignores height, flag 1 always pulls
UBOOL FDisSpringPoint::ApplyMaxConstraint( FLOAT MaxDist, const FVector* FromPos, INT Flags )
{
	FVector Dif = ( FromPos ? *FromPos : m_BoundPos ) - m_Pos;
	if( Flags & 2 )
	{
		Dif.Z = 0.f;
	}
	FVector Dir = Dif;
	if( !Dir.Normalize( 1e-8f ) )
	{
		return FALSE;
	}
	const FLOAT Dist = Dir | Dif;
	if( Dist <= MaxDist && !( Flags & 1 ) )
	{
		return FALSE;
	}
	m_Pos += Dir * ( Dist - MaxDist );
	return TRUE;
}

const FVector FDisSpringPoint::GetPos_Interpolated( FLOAT InterpScale ) const
{
	return m_Pos_Previous + ( m_Pos - m_Pos_Previous ) * InterpScale;
}

// 2013 0x6c0900 (shared with the bump smoother; 2012 0x721bc0): a teleport restarts the influence
void UDishonoredCamera_PhysicalReact::OnTeleport( USeqAct_Teleport* Action )
{
	OnActivate( NULL );
}

// 2013 0x6c0c50: the pivot offset moves both bound positions
void UDishonoredCamera_PhysicalReact::PostBeginPlay_Derived()
{
	m_StrengthPoint.m_BoundPos += m_CameraPivotOffset;
	m_StabilityPoint.m_BoundPos += m_CameraPivotOffset;
	DisJGNotePhysicalReact( this, TEXT("PostBeginPlay") );
}

// 2013 0x6c0cf0: both points at rest on their bound positions
void UDishonoredCamera_PhysicalReact::OnActivate_Derived( APawn* Pawn )
{
	m_StrengthPoint.m_Velocity = FVector( 0.f, 0.f, 0.f );
	m_StrengthPoint.m_Pos = m_StrengthPoint.m_BoundPos;
	m_StrengthPoint.StorePreviousPhysResult();
	m_StabilityPoint.m_Velocity = FVector( 0.f, 0.f, 0.f );
	m_StabilityPoint.m_Pos = m_StabilityPoint.m_BoundPos;
	m_StabilityPoint.StorePreviousPhysResult();
}

// 2013 0x6c0e10 (byte-identical to 2012 0x726310): an awake influence loses the velocity going into the hit; retail's second test
// reads the strength point's dot product for the stability point too
void UDishonoredCamera_PhysicalReact::OnCameraCollided_Derived( const FVector& HitDirection )
{
	if( m_bIsSleeping )
	{
		return;
	}
	const FLOAT StrengthDot = m_StrengthPoint.m_Velocity | HitDirection;
	if( StrengthDot < 0.f )
	{
		m_StrengthPoint.m_Velocity -= HitDirection * StrengthDot;
	}
	const FLOAT StabilityDot = m_StabilityPoint.m_Velocity | HitDirection;
	if( StrengthDot < 0.f )
	{
		m_StabilityPoint.m_Velocity -= HitDirection * StabilityDot;
	}
}

// 2013 0x6d49b0: the spring constants follow the class default (the config can change at run time)
void UDishonoredCamera_PhysicalReact::GatherInfluence_Start( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime )
{
	const UDishonoredCamera_PhysicalReact* Default = Cast<UDishonoredCamera_PhysicalReact>( GetClass()->GetDefaultObject() );
	if( Default )
	{
		m_StrengthConstants = Default->m_StrengthConstants;
		m_StabilityConstants = Default->m_StabilityConstants;
	}
}

// 2013 0x6c0f30: one fixed step of both springs, then the stability point put back 100 uu from the strength point on their line
UBOOL UDishonoredCamera_PhysicalReact::GatherInfluence_FixedTimeStep( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, UBOOL bFirstStep, UBOOL bLastStep )
{
	const FLOAT Step = m_fCurFixedTimeStep;
	m_StrengthPoint.StorePreviousPhysResult();
	m_StabilityPoint.StorePreviousPhysResult();
	m_StrengthPoint.UpdateSpringPos( m_StrengthConstants, Step, NULL, NULL );
	m_StabilityPoint.UpdateSpringPos( m_StabilityConstants, Step, NULL, NULL );
	const FVector Dif = m_StrengthPoint.m_Pos - m_StabilityPoint.m_Pos;
	FVector Dir = Dif;
	if( Dir.Normalize( 1e-8f ) )
	{
		m_StabilityPoint.m_Pos += Dir * ( ( Dir | Dif ) - 100.f );
	}
	DisJGNoteSpringStep( this );
	return TRUE;
}

/**
 * 2013 0x6c5be0. Both points at the fixed step's interpolation; the line from the strength point to the stability point is the
 * view's up axis (forward from the view's yaw) and its pitch and roll are the rotation; the location is the strength point (not
 * interpolated, as retail) less the pivot offset along those axes; past m_ArmBounce_Start the arms follow less, by up to m_ArmBounce
 * over m_ArmBounce_End; with neither point moving the influence goes to sleep.
 */
UBOOL UDishonoredCamera_PhysicalReact::GatherInfluence_Derived( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime )
{
	const FLOAT InterpScale = GetFixedInterpScale();
	const FVector Stability = m_StabilityPoint.GetPos_Interpolated( InterpScale );
	const FVector Strength = m_StrengthPoint.GetPos_Interpolated( InterpScale );
	FVector DirVector = Stability - Strength;
	DirVector.Normalize( 1e-8f );
	const FVector Forward = FRotator( 0, CurVT.m_VT.POV.Rotation.Yaw, 0 ).Vector();
	FVector RightVector = DirVector ^ Forward;
	FVector UpVector = RightVector ^ DirVector;
	FVector Origin( 0.f, 0.f, 0.f );
	FMatrix RotMat = FMatrix::Identity;
	RotMat.SetAxes( &UpVector, &RightVector, &DirVector, &Origin );
	FRotator WorkingRot = RotMat.Rotator();
	WorkingRot.Yaw = 0;
	WorkingRot.MakeShortestRoute();
	OutVT.m_VT.POV.Rotation = WorkingRot;
	OutVT.m_VT.POV.Location = m_StrengthPoint.m_Pos;
	OutVT.m_VT.POV.Location -= RotMat.GetAxis( 0 ) * m_CameraPivotOffset.X;
	OutVT.m_VT.POV.Location -= RotMat.GetAxis( 1 ) * m_CameraPivotOffset.Y;
	OutVT.m_VT.POV.Location -= RotMat.GetAxis( 2 ) * m_CameraPivotOffset.Z;
	const FLOAT StrengthDist = ( Strength - m_StrengthPoint.m_BoundPos ).Size();
	if( StrengthDist > m_ArmBounce_Start )
	{
		const FLOAT ArmWeight = -( m_ArmBounce * Clamp( ( StrengthDist - m_ArmBounce_Start ) / m_ArmBounce_End, 0.f, 1.f ) );
		OutVT.m_DishonoredSettings.m_ArmFollowWeight = ArmWeight;
		OutVT.m_DishonoredSettings.m_ArmFollowWeight_Rot_Primary = ArmWeight;
		OutVT.m_DishonoredSettings.m_ArmFollowWeight_Rot_Secondary = ArmWeight;
	}
	if( !m_StrengthPoint.IsSpringing() && !m_StabilityPoint.IsSpringing() )
	{
		PutToSleep();
	}
	DisJGNoteReactOutput( this, OutVT );
	return TRUE;
}

// 2013 0x6c10f0: the impact velocity on both points, and the influence woken
void UDishonoredCamera_PhysicalReact::OnImpact( const FVector& Velocity )
{
	if( Velocity.Size() > 0.0001f )
	{
		m_StrengthPoint.m_Velocity += Velocity;
		m_StabilityPoint.m_Velocity += Velocity;
		Wake();
	}
	DisJGNotePhysicalReact( this, TEXT("OnImpact") );
}
