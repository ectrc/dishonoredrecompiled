// Engine/src/arkcomponentmeshoffset.cpp
// DISHONORED(port): agent GA (PHASE18 package GA). The mesh-offset component, from the 2012 decompile; every body below
// names its 2012 rva (build/agentGA/decomp2012) and the 2013 one where the function is named there
// (build/agentGA/decomp2013). The two tick entries are read off the disassembly (build/agentGA/disasm2012), because
// Hex-Rays mis-adjusts the two interface subobjects' this pointers in them.
// PDB functions attributed to this file (13):
//   0x527040  public: virtual unsigned long __thiscall FArkComponentMeshOffset::GetMemoryFootprint(void)const
//   0x527050  public: void __thiscall FArkComponentMeshOffset::DisableByFlags(int)
//   0x529d40  public: virtual void __thiscall FArkComponentMeshOffset::Stopping(void)
//   0x529db0  protected: void __thiscall FArkComponentMeshOffset::GetGroundOrientation(class FVector &)
//   0x52a130  public: void __thiscall FArkComponentMeshOffset::ForceMeshOffset(class FVector, unsigned int)
//   0x52f730  private: void __thiscall FArkComponentMeshOffset::Tick(float)
//   0x533620  public: __thiscall FArkComponentMeshOffset::FArkComponentMeshOffset(void)
//   0x535810  public: static class FArkComponentMeshOffset * __cdecl FArkComponentMeshOffset::GetMeshOffsetComponent(class APawn const *, unsigned int)
//   0x535840  public: virtual void __thiscall FArkComponentMeshOffset::Starting(void)
//   0x5358e0  public: virtual void __thiscall FArkComponentMeshOffset::PreAsyncWorkTick(float)
//   0x535990  public: virtual void __thiscall FArkComponentMeshOffset::PreAsyncWorkJustBeforeProceduralAnimTick(float)
//   0x535a40  public: void __thiscall FArkComponentMeshOffset::ForceInstantSnapMesh(void)
//   0x535a90  public: void __thiscall FArkComponentMeshOffset::EnableByFlags(int)

#include "EnginePrivate.h"
#include "EngineArkaneClasses.h"
#include "arkcomponentmeshoffset.h"

checkAtCompileTime( sizeof( FArkComponentMeshOffset ) == 136, FArkComponentMeshOffset_is_136 );
checkAtCompileTime( sizeof( FArkComponentMeshOffset::FArkCpntMeshOffsetFootProps ) == 16, FArkCpntMeshOffsetFootProps_is_16 );

ARKCOMPONENT_IMPLEMENT_TYPE( FArkComponentMeshOffset )

// 2012 0x533620 (191 B). The config members are left at their zero defaults and written by the pawn that creates the
// component (ADishonoredNPCPawn::PreBeginPlay_NativeComponents); m_fGoDownMinZSpeed's default is retail's -0.01.
FArkComponentMeshOffset::FArkComponentMeshOffset()
	: m_pPawn( NULL )
	, m_fLastPawnZ( 0.f )
	, m_fLastBaseZ( 0.f )
	, m_pLastBase( NULL )
	, m_fCurrentOffsetZ( 0.f )
	, m_fCurrentSpeedZ( 0.f )
	, m_fBaseZMove( 0.f )
	, m_CurrentPitch( 0 )
	, m_DisableFlags( 0 )
	, m_LastLineCheckLocation( FLT_MAX, FLT_MAX, FLT_MAX )
	, m_fLastLineCheckHitZ( 0.f )
	, m_ExternalRotationOffset( 0, 0, 0 )
	, m_ExternalTranslationOffset( 0.f, 0.f, 0.f )
	, m_bPitchOnGround( FALSE )
	, m_fPitchSpeed( 0.f )
	, m_fMinHeightSpeed( 0.f )
	, m_fMinHeightSpeedWithFootProps( 0.f )
	, m_fOffsetDown( 0.f )
	, m_fGoDownMinZSpeed( -0.01f )
{
}

// 2012 0x535810 (33 B), 2013 0x4faa90
FArkComponentMeshOffset* FArkComponentMeshOffset::GetMeshOffsetComponent( const APawn* _pPawnOwner, UBOOL _bMustBeStarted )
{
	if( _pPawnOwner && _pPawnOwner->m_ComponentContainer )
	{
		return _pPawnOwner->m_ComponentContainer->GetFirstComponent<FArkComponentMeshOffset>( _bMustBeStarted );
	}
	return NULL;
}

// 2012 0x535840 (146 B). Retail registers both interface subobjects with GWorld->m_pComponentManager's two policies;
// the manager is not ported (DishonoredGame/Inc/arkcomponentlocomotion.h explains), and USkeletalMeshComponent::Tick
// runs the two tick entries on the owning pawn's component in its place.
void FArkComponentMeshOffset::Starting()
{
	m_pPawn = m_pOwner ? m_pOwner->GetAPawn() : NULL;
	m_fLastPawnZ = m_pPawn ? m_pPawn->Location.Z : 0.f;
	m_FeetProps.Empty();
}

// 2012 0x529d40 (112 B), 2013 0x4f17d0: unregister from both policies, forget the pawn
void FArkComponentMeshOffset::Stopping()
{
	m_pPawn = NULL;
}

// The head both tick entries share (read off the disassembly of 0x5358e0 / 0x535990): how far the base moved up or down
// since last frame, so the mesh follows a lift or a boat without lagging behind it.
void FArkComponentMeshOffset::UpdateBaseMove()
{
	AActor* Base = m_pPawn ? m_pPawn->Base : NULL;
	m_fBaseZMove = ( Base != NULL && Base == m_pLastBase ) ? Base->Location.Z - m_fLastBaseZ : 0.f;
	m_pLastBase = Base;
	m_fLastBaseZ = Base ? Base->Location.Z : 0.f;
}

// 2012 0x5358e0 (167 B): the PreAsyncWork pass ticks the offset only while no foot props are pending - with foot props
// the JustBeforeProceduralAnim pass does, after the foot-placement control has filled them in.
void FArkComponentMeshOffset::PreAsyncWorkTick( FLOAT _fTimeStep )
{
	if( !m_pPawn || m_FeetProps.Num() != 0 )
	{
		return;
	}
	UpdateBaseMove();
	if( m_DisableFlags == 0 )
	{
		Tick( _fTimeStep );
	}
	m_FeetProps.Empty();
}

// 2012 0x535990 (167 B)
void FArkComponentMeshOffset::PreAsyncWorkJustBeforeProceduralAnimTick( FLOAT _fTimeStep )
{
	if( !m_pPawn || m_FeetProps.Num() <= 0 )
	{
		return;
	}
	UpdateBaseMove();
	if( m_DisableFlags == 0 )
	{
		Tick( _fTimeStep );
	}
	m_FeetProps.Empty();
}

// 2012 0x52a130 (74 B), 2013 0x4f1bc0: only the instant form does anything - the offset and the mesh are set, and the
// base is forgotten so the next tick does not read a base move into it.
void FArkComponentMeshOffset::ForceMeshOffset( FVector _vOffset, UBOOL _bInstant )
{
	if( !_bInstant || !m_pPawn || !m_pPawn->Mesh )
	{
		return;
	}
	m_fCurrentOffsetZ = _vOffset.Z;
	m_pPawn->Mesh->Translation = _vOffset;
	m_pPawn->Mesh->BeginDeferredUpdateTransform();
	m_pLastBase = NULL;
}

// DISHONORED(port): agent HC. 2013 0x8f1c10 (65 B, unnamed; reached from ADishonoredPawn 2013 0xb498c0, which
// FDisComponentRetargeting::StartRetargeting 2013 0xc88440 calls with 0): the mesh's rotation becomes the external rotation
// offset plus the given pitch, which becomes the current pitch. 2013 addresses the external rotation at +84 (2012 PDB +76).
void FArkComponentMeshOffset::ForcePitch( INT _Pitch )
{
	if( !m_pPawn || !m_pPawn->Mesh )
	{
		return;
	}
	m_pPawn->Mesh->Rotation = FRotator( m_ExternalRotationOffset.Pitch + _Pitch, m_ExternalRotationOffset.Yaw, m_ExternalRotationOffset.Roll );
	m_pPawn->Mesh->BeginDeferredUpdateTransform();
	m_CurrentPitch = _Pitch;
}

// 2012 0x535a40 (67 B): the feet forgotten, one tick with a zero time step - which snaps the mesh to its target
void FArkComponentMeshOffset::ForceInstantSnapMesh()
{
	m_FeetProps.Empty();
	if( m_pPawn )
	{
		Tick( 0.f );
	}
}

// 2012 0x535a90 (86 B): when the last disable flag goes, the pawn's Z is re-read so the first enabled tick does not
// see the whole disabled period as one pawn move, and pending feet are dropped.
void FArkComponentMeshOffset::EnableByFlags( INT _bFlags )
{
	const INT OldFlags = m_DisableFlags;
	m_DisableFlags &= ~_bFlags;
	if( OldFlags != 0 && m_DisableFlags == 0 )
	{
		m_fLastPawnZ = m_pPawn ? m_pPawn->Location.Z : 0.f;
		m_FeetProps.Empty();
	}
}

// 2012 0x529db0 (895 B), 2013 0x4f1840: the ground's slope under the pawn along its facing - two world traces, one
// 0.9 radius behind and one 0.9 radius ahead of the cylinder's top, each four heights down; the normalized vector from
// the first hit to the second is the orientation. With no cylinder, or a trace that misses, the facing itself.
void FArkComponentMeshOffset::GetGroundOrientation( FVector& _rOrientation )
{
	const FVector Forward = m_pPawn->Rotation.Vector();
	_rOrientation = Forward;
	UCylinderComponent* Cylinder = m_pPawn->CylinderComponent;
	if( !Cylinder )
	{
		return;
	}
	const FLOAT Height = Cylinder->CollisionHeight;
	const FLOAT Reach = Cylinder->CollisionRadius * 0.9f;
	const FVector Origin = Cylinder->Bounds.Origin;
	const FVector Top( Origin.X, Origin.Y, Origin.Z + Height );
	const FVector Behind = Top - Forward * Reach;
	const FVector Ahead = Top + Forward * Reach;
	const FVector Drop( 0.f, 0.f, Height * 4.f );

	FCheckResult HitBehind( 1.f );
	if( GWorld->SingleLineCheck( HitBehind, m_pPawn, Behind - Drop, Behind, TRACE_World ) )
	{
		return;
	}
	FCheckResult HitAhead( 1.f );
	if( GWorld->SingleLineCheck( HitAhead, m_pPawn, Ahead - Drop, Ahead, TRACE_World ) )
	{
		return;
	}
	const FVector Slope = HitAhead.Location - HitBehind.Location;
	const FLOAT SizeSquared = Slope.SizeSquared();
	if( SizeSquared == 1.f )
	{
		_rOrientation = Slope;
	}
	else if( SizeSquared < SMALL_NUMBER * SMALL_NUMBER * 100.f )
	{
		_rOrientation = FVector( 0.f, 0.f, 0.f );
	}
	else
	{
		_rOrientation = Slope * appInvSqrt( SizeSquared );
	}
}

// 2012 0x52f730 (2,129 B), 2013 0x4f5e20 (2,140 B). Retail's order:
//   1. nothing while the mesh is fully physics-driven (PhysicsWeight >= 1);
//   2. with m_bPitchOnGround, the ground slope along the facing becomes a target pitch (unreal units) and m_CurrentPitch
//      turns toward it at m_fPitchSpeed, scaled down below 20 degrees of remaining turn;
//   3. the target world height of the mesh's origin: with fewer than two planted feet, a trace from the cylinder's
//      bottom down twice MaxStepHeight (cached by position while the base has not moved) plus the collision height;
//      with feet, the lowest foot's delta, pulled toward the highest foot's by distance while moving, less m_fOffsetDown;
//      a translated cylinder adds the pitch's tangent;
//   4. the offset is kept in world terms across the pawn's own Z change and the base's, clamped to +-2 MaxStepHeight;
//   5. it approaches the target at max(min speed, pawn speed) with a quadratic ease-in, instantly when the pawn was not
//      rendered in the last 0.2 s;
//   6. the mesh gets the external rotation/translation offsets plus the pitch and the Z offset, and is marked for a
//      transform update; the vertical speed is the mesh origin's.
void FArkComponentMeshOffset::Tick( FLOAT _fTimeStep )
{
	USkeletalMeshComponent* Mesh = m_pPawn->Mesh;
	if( !Mesh || Mesh->PhysicsWeight >= 1.f )
	{
		return;
	}

	if( m_bPitchOnGround )
	{
		FVector Ground;
		GetGroundOrientation( Ground );
		const FVector Forward = m_pPawn->Rotation.Vector();
		const FLOAT Cosine = Clamp<FLOAT>( Forward | Ground, -1.f, 1.f );
		const FLOAT Angle = appAcos( Cosine );
		const INT Sign = ( Ground.Z == 0.f ) ? 0 : ( ( Ground.Z > 0.f ) ? 1 : -1 );
		const INT TargetPitch = appTrunc( Angle * 10430.378f ) * Sign;
		const INT Current = m_CurrentPitch & 0xFFFF;
		const INT Target = TargetPitch & 0xFFFF;
		INT Delta = Target - Current;
		if( Current <= Target )
		{
			if( Delta >= 0x8000 )
			{
				Delta -= 0x10000;
			}
		}
		else if( Current - Target >= 0x8000 )
		{
			Delta += 0x10000;
		}
		const INT AbsDelta = Abs( Delta );
		const FLOAT Ratio = Clamp<FLOAT>( (FLOAT)AbsDelta * 0.0002746582f, 0.1f, 1.f );
		const INT Step = appTrunc( m_fPitchSpeed * Ratio * _fTimeStep );
		if( Step < AbsDelta )
		{
			m_CurrentPitch += Step * ( ( Delta > 0 ) ? 1 : ( ( Delta < 0 ) ? -1 : 0 ) );
		}
		else
		{
			m_CurrentPitch = TargetPitch;
		}
	}

	const FLOAT PawnSpeed = m_pPawn->Velocity.Size();
	const FLOAT MaxDelta = m_pPawn->MaxStepHeight * 2.f;
	const INT FeetCount = m_FeetProps.Num();
	UCylinderComponent* Cylinder = m_pPawn->CylinderComponent;
	const FLOAT CollisionHeight = Cylinder ? Cylinder->CollisionHeight : 0.f;
	FLOAT TargetZ = m_pPawn->Location.Z;

	if( FeetCount < 2 )
	{
		if( Cylinder )
		{
			const FVector Bottom( Cylinder->Bounds.Origin.X, Cylinder->Bounds.Origin.Y, Cylinder->Bounds.Origin.Z - CollisionHeight );
			if( Bottom.Equals( m_LastLineCheckLocation, 0.0001f ) && Abs( m_fBaseZMove ) < 0.00000001f )
			{
				TargetZ = CollisionHeight + m_fLastLineCheckHitZ;
			}
			else
			{
				FCheckResult Hit( 1.f );
				const FVector End( Bottom.X, Bottom.Y, Bottom.Z - MaxDelta );
				if( !GWorld->SingleLineCheck( Hit, m_pPawn, End, Bottom, TRACE_AllColliding | TRACE_Blocking | TRACE_ComplexCollision ) )
				{
					m_LastLineCheckLocation = Bottom;
					m_fLastLineCheckHitZ = Hit.Location.Z;
					TargetZ = CollisionHeight + Hit.Location.Z;
				}
			}
		}
	}
	else
	{
		INT Lowest = 0;
		for( INT i = 1; i < FeetCount; i++ )
		{
			if( m_FeetProps(i).m_fTargetedZDelta < m_FeetProps(Lowest).m_fTargetedZDelta )
			{
				Lowest = i;
			}
		}
		INT Highest = INDEX_NONE;
		for( INT i = 0; i < FeetCount; i++ )
		{
			if( i != Lowest && ( Highest == INDEX_NONE || m_FeetProps(i).m_fTargetedZDelta > m_FeetProps(Highest).m_fTargetedZDelta ) )
			{
				Highest = i;
			}
		}
		const FLOAT LowestDelta = m_FeetProps(Lowest).m_fTargetedZDelta;
		const FLOAT DistToLowest = ( m_pPawn->Location - m_FeetProps(Lowest).m_WorldLocation ).Size2D();
		const FLOAT DistToHighest = ( m_pPawn->Location - m_FeetProps(Highest).m_WorldLocation ).Size2D();
		const FLOAT TotalDist = DistToLowest + DistToHighest;
		FLOAT Z = LowestDelta;
		if( TotalDist > 0.f && Abs( PawnSpeed ) >= 0.1f )
		{
			FLOAT Weight = 1.f;
			if( m_fCurrentSpeedZ < -0.1f && m_fCurrentSpeedZ > m_fGoDownMinZSpeed * 2.f )
			{
				Weight = Min<FLOAT>( m_fCurrentSpeedZ - m_fGoDownMinZSpeed, 0.f ) / m_fGoDownMinZSpeed;
			}
			const FLOAT Blended = Weight * ( ( m_FeetProps(Highest).m_fTargetedZDelta - LowestDelta ) / TotalDist * DistToLowest ) + LowestDelta - m_fOffsetDown;
			if( LowestDelta < Blended )
			{
				Z = Blended;
			}
		}
		TargetZ = CollisionHeight + m_fBaseZMove + Z;
	}

	if( Cylinder && !Cylinder->Translation.IsNearlyZero() )
	{
		TargetZ -= appTan( (FLOAT)m_CurrentPitch * 0.000095873802f ) * Cylinder->Translation.X;
	}

	m_fCurrentOffsetZ -= ( m_pPawn->Location.Z - m_fLastPawnZ ) - m_fBaseZMove;
	m_fCurrentOffsetZ = Clamp<FLOAT>( m_fCurrentOffsetZ, -MaxDelta, MaxDelta );
	const FLOAT PreviousMeshZ = m_fCurrentOffsetZ + m_fLastPawnZ;
	m_fLastPawnZ = m_pPawn->Location.Z;
	const FLOAT CurrentMeshZ = m_pPawn->Location.Z + m_fCurrentOffsetZ;
	const FLOAT Remaining = Abs( Clamp<FLOAT>( TargetZ - CurrentMeshZ, -MaxDelta, MaxDelta ) );
	const FLOAT MinSpeed = ( FeetCount > 0 ) ? m_fMinHeightSpeedWithFootProps : m_fMinHeightSpeed;
	FLOAT Step = Remaining;
	if( GWorld->GetTimeSeconds() - m_pPawn->LastRenderTime <= 0.2f )
	{
		const FLOAT Speed = Max<FLOAT>( MinSpeed, PawnSpeed );
		const FLOAT Ease = 1.f - Square( 1.f - Remaining / MaxDelta );
		Step = Min<FLOAT>( Ease * Speed * _fTimeStep, Remaining );
	}
	const FLOAT Direction = ( TargetZ <= CurrentMeshZ ) ? -1.f : 1.f;
	m_fCurrentOffsetZ = Clamp<FLOAT>( m_fCurrentOffsetZ + Direction * Step, -MaxDelta, MaxDelta );

	Mesh->Rotation = m_ExternalRotationOffset;
	Mesh->Translation = m_ExternalTranslationOffset;
	Mesh->Rotation.Pitch += m_CurrentPitch;
	Mesh->Translation.Z += m_fCurrentOffsetZ;
	Mesh->BeginDeferredUpdateTransform();
	if( Abs( _fTimeStep ) >= 0.00000001f )
	{
		m_fCurrentSpeedZ = ( ( m_pPawn->Location.Z + m_fCurrentOffsetZ ) - PreviousMeshZ ) / _fTimeStep;
	}
}
