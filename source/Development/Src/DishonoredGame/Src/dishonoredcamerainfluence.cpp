// DishonoredGame/src/dishonoredcamerainfluence.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (10):
//   0x6fbd20  public: static void __cdecl UDishonoredCameraInfluence::InitializePrivateStaticClassUDishonoredCameraInfluence(void)
//   0x6fbd40  protected: float __thiscall UDishonoredCameraInfluence::GetFixedInterpScale(void)const
//   0x6fbd80  public: virtual void __thiscall FGFxPixelShaderInterface::SetParameterFilterSize4(class TDynamicRHIResource<8> *, float, float, float, float)const
//   0x6fbd90  public: void __thiscall UDishonoredCameraInfluence::OnActivate(class APawn *)
//   0x6fbdb0  public: void __thiscall UDishonoredCameraInfluence::PutToSleep(void)
//   0x6fbdc0  public: void __thiscall UDishonoredCameraInfluence::Wake(void)
//   0x6fff80  public: void __thiscall UDishonoredCameraInfluence::SetWeight(float, float)
//   0x700010  public: unsigned int __thiscall UDishonoredCameraInfluence::GatherInfluence(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)
//   0x709680  public: static class UClass * __cdecl UDishonoredCameraInfluence::GetPrivateStaticClassUDishonoredCameraInfluence(wchar_t const *)
//   0x70e0e0  public: static class UClass * __cdecl UDishonoredCameraInfluence::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): the base of the player camera's influences. SetWeight 2012 0x6fff80 / 2013 0x6c49a0 (callee 0.870;
	the 2013 body is the one ported - it takes bWaitToBlendOneFrame and writes m_bWaitToBlendOneFrame, the bit 2013 inserted
	into the bitfield), GatherInfluence 0x700010 / 2013 0x6c4a40 (neighbours 0.958; the fixed-time-step bit is mask 8 in 2013,
	mask 4 in 2012 - the header carries the 2013 order and the 2013 body is what is ported), OnActivate 0x6fbd90,
	GetFixedInterpScale 0x6fbd40 (both unnamed in 2013).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_CameraInfluence()
{
}

/**
 * A blend time of 0 snaps the weight (transition speed 1e6), a negative one takes the class default's m_TransitionSpeed,
 * a positive one is 1 / time. A target the influence already has keeps the faster of the two speeds.
 */
void UDishonoredCameraInfluence::SetWeight( FLOAT TargetWeight, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame )
{
	FLOAT Speed;
	if( BlendTime == 0.f )
	{
		Speed = 1000000.f;
		m_Weight = TargetWeight;
	}
	else if( BlendTime < 0.f )
	{
		Speed = ((UDishonoredCameraInfluence*)GetClass()->GetDefaultObject())->m_TransitionSpeed;
	}
	else
	{
		Speed = 1.f / BlendTime;
	}
	if( Abs( m_TargetWeight - TargetWeight ) <= 0.0001f )
	{
		Speed = Max( Speed, m_TransitionSpeed );
	}
	else
	{
		m_TargetWeight = TargetWeight;
	}
	m_TransitionSpeed = Speed;
	m_bWaitToBlendOneFrame = bWaitToBlendOneFrame ? TRUE : FALSE;
}

/**
 * GatherInfluence_Start, then either the plain GatherInfluence_Derived or, for a fixed-time-step influence with time to
 * spend, as many GatherInfluence_FixedTimeStep steps as the accumulated delta holds (the first and last flagged), and the
 * derived pass with the remainder. A fixed step that returns FALSE ends the gather with no contribution.
 */
UBOOL UDishonoredCameraInfluence::GatherInfluence( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime )
{
	GatherInfluence_Start( OutVT, CurVT, Pawn, TimeDelta, UncappedDeltaTime );
	if( !m_bUseFixedTimeStep || UncappedDeltaTime <= 1e-8f )
	{
		return GatherInfluence_Derived( OutVT, CurVT, Pawn, TimeDelta, UncappedDeltaTime );
	}
	const FLOAT Step = Min( UncappedDeltaTime, m_fFixedTimeStep );
	m_fCurFixedTimeStep = Step;
	m_fAccumulatedDelta += Step;
	if( m_fAccumulatedDelta <= Step )
	{
		return GatherInfluence_Derived( OutVT, CurVT, Pawn, TimeDelta, UncappedDeltaTime );
	}
	INT StepIndex = 0;
	while( GatherInfluence_FixedTimeStep( OutVT, CurVT, Pawn, StepIndex == 0, m_fCurFixedTimeStep >= m_fAccumulatedDelta - m_fCurFixedTimeStep ) )
	{
		m_fAccumulatedDelta -= m_fCurFixedTimeStep;
		StepIndex++;
		if( m_fAccumulatedDelta <= m_fCurFixedTimeStep )
		{
			return GatherInfluence_Derived( OutVT, CurVT, Pawn, TimeDelta, UncappedDeltaTime );
		}
	}
	return FALSE;
}

// DISHONORED(port): 2012 0x6fbd90
void UDishonoredCameraInfluence::OnActivate( APawn* Pawn )
{
	m_fAccumulatedDelta = 0.f;
	OnActivate_Derived( Pawn );
}

// DISHONORED(port): 2012 0x6fbd40
FLOAT UDishonoredCameraInfluence::GetFixedInterpScale() const
{
	if( m_fCurFixedTimeStep > 1e-8f )
	{
		return m_fAccumulatedDelta / m_fCurFixedTimeStep;
	}
	return 1.f;
}
