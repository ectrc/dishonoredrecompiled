// DishonoredGame/src/dishonoredcamera_bumpsmoother.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x6fc540  protected: virtual void __thiscall UDishonoredCamera_BumpSmoother::OnActivate_Derived(class APawn *)
//   0x6fc740  public: virtual void __thiscall UDishonoredCamera_Lean::OnTeleport(class USeqAct_Teleport *)
//   0x700540  public: unsigned int __thiscall FDisBumpDetectInfo::DetectBump(class APawn const *, class FVector const &, float, float)const
//   0x7006b0  protected: virtual unsigned int __thiscall UDishonoredCamera_BumpSmoother::GatherInfluence_Derived(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)
//   0x70e780  public: static class UClass * __cdecl UDishonoredCamera_BumpSmoother::GetPrivateStaticClassUDishonoredCamera_BumpSmoother(wchar_t const *)
//   0x70e810  public: static void __cdecl UDishonoredCamera_BumpSmoother::InitializePrivateStaticClassUDishonoredCamera_BumpSmoother(void)
//   0x710b80  public: static class UClass * __cdecl UDishonoredCamera_BumpSmoother::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	Agent IP (PHASE23 IP): the bump smoother, the reaction group's first influence (ReactionInfluence.BumpSmoothInfluence,
	m_fSmoothDistance 400, m_fCompensationSpeed 8). The sample is the pawn's location plus its mesh translation; when its height
	changes faster than m_fSmoothDistance per second against the direction of travel (or with no velocity), the height before the
	change is held and handed back at m_fCompensationSpeed x the remaining difference per second (at least 1 uu/s), and the view
	is offset by the held height minus the new one. Ported from the 2013 bodies (DetectBump 0x6c5030 and OnActivate_Derived
	0x6c07a0 are the 2012 bytes; GatherInfluence_Derived 0x6c51a0 adds the arm-follow weight 1 so the arms take the offset).
	DetectBump's game-info test is ADishonoredGameInfo +980 bit 0x10 (2012 +976), the fifth bit of the first bitfield in the
	cooked mask order: m_bNewNPCPhysWalking here.
	DISHONORED(written): -ipnobump (diagnosis only) answers FALSE before any work, as the unported influence did.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "ipdiag.h"

void DishonoredLinkIP_DishonoredCameraBumpSmoother()
{
}

UBOOL FDisBumpDetectInfo::DetectBump( const APawn* Pawn, const FVector& PawnPos, FLOAT SmoothDist, FLOAT DeltaTime ) const
{
	if( DeltaTime < 0.0001f )
	{
		return FALSE;
	}
	FLOAT LastHeight = m_fLastSampleHeight;
	FVector LastSample = m_LastSample_Full;
	if( m_bFirstHeight )
	{
		LastHeight = PawnPos.Z;
		LastSample = PawnPos;
	}
	if( Abs( PawnPos.Z - LastHeight ) / DeltaTime < SmoothDist )
	{
		return FALSE;
	}
	ADishonoredGameInfo* GameInfo = DisGetGameInfo();
	if( GameInfo && GameInfo->m_bNewNPCPhysWalking )
	{
		return TRUE;
	}
	FVector VelDir = Pawn->Velocity;
	FVector FullDif = PawnPos - LastSample;
	return !VelDir.Normalize( 1e-8f ) || !FullDif.Normalize( 1e-8f ) || ( FullDif | VelDir ) <= 0.8f;
}

void UDishonoredCamera_BumpSmoother::OnActivate_Derived( APawn* Pawn )
{
	m_BumpDetectInfo.m_bFirstHeight = TRUE;
	m_bIsCompensating = FALSE;
}

UBOOL UDishonoredCamera_BumpSmoother::GatherInfluence_Derived( FDishonoredViewTarget& OutVT, const FDishonoredViewTarget& CurVT, APawn* Pawn, FLOAT TimeDelta, FLOAT UncappedDeltaTime )
{
	if( DisIPNoBump() )
	{
		return FALSE;
	}
	const FVector Sample = Pawn->Location + ( Pawn->Mesh ? Pawn->Mesh->Translation : FVector( 0.f, 0.f, 0.f ) );
	if( m_BumpDetectInfo.DetectBump( Pawn, Sample, m_fSmoothDistance, UncappedDeltaTime ) && !m_bIsCompensating )
	{
		m_fLastInterpolatedHeight = m_BumpDetectInfo.m_fLastSampleHeight;
		m_bIsCompensating = TRUE;
	}
	m_BumpDetectInfo.m_bFirstHeight = FALSE;
	m_BumpDetectInfo.m_LastSample_Full = Sample;
	m_BumpDetectInfo.m_fLastSampleHeight = Sample.Z;
	if( !m_bIsCompensating )
	{
		return FALSE;
	}
	const FLOAT Height = Sample.Z;
	const FLOAT Last = m_fLastInterpolatedHeight;
	FLOAT Dif = Height - Last;
	if( Dif >= 0.f )
	{
		Dif = Max( Dif, 1.f );
	}
	else
	{
		Dif = Min( Dif, -1.f );
	}
	const FLOAT Next = m_fCompensationSpeed * Dif * UncappedDeltaTime + Last;
	if( ( Height > Last ) == ( Height > Next ) )
	{
		m_fLastInterpolatedHeight = Next;
	}
	else
	{
		m_fLastInterpolatedHeight = Height;
		m_bIsCompensating = FALSE;
	}
	const FLOAT Offset = m_fLastInterpolatedHeight - Height;
	OutVT.m_VT.POV.Location.Z = Offset;
	m_fDebug_LastOffset = Offset;
	OutVT.m_DishonoredSettings.m_ArmFollowWeight = 1.f;
	return TRUE;
}
