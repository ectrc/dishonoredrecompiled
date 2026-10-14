// DishonoredGame/src/disaisubprocessheadtrackpause.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x78d3e0  public: static void __cdecl UDisTweaks_AISubProcess_HeadTrackPause::InitializePrivateStaticClassUDisTweaks_AISubProcess_HeadTrackPause(void)
//   0x79aca0  public: static class UClass * __cdecl UDisAISubProcessHeadTrackPause::GetPrivateStaticClassUDisAISubProcessHeadTrackPause(wchar_t const *)
//   0x79be70  public: static void __cdecl UDisAISubProcessHeadTrackPause::InitializePrivateStaticClassUDisAISubProcessHeadTrackPause(void)
//   0x79be90  private: virtual void __thiscall UDisAISubProcessHeadTrackPause::BeginSubProcess_Derived(void)
//   0x79e1e0  public: static class UClass * __cdecl UDisAISubProcessHeadTrackPause::StaticClassNoInline(void)
//   0x7a3590  public: static class UClass * __cdecl UDisTweaks_AISubProcess_HeadTrackPause::GetPrivateStaticClassUDisTweaks_AISubProcess_HeadTrackPause(wchar_t const *)
//   0x7a3e50  public: static class UClass * __cdecl UDisTweaks_AISubProcess_HeadTrackPause::StaticClassNoInline(void)
//   0x7a4eb0  private: virtual void __thiscall UDisAISubProcessHeadTrackPause::TickSubProcess_Derived(float)

// ---- agent GE ports (PHASE19 GE): the pause on a head track ----
#include "DishonoredGame.h"

// DISHONORED(bringup): every function of this unit is a virtual, so nothing references the object and MSVC would drop it out
// of the static library. Named from DishonoredGameRegistrants.cpp (the generator collects it).
void DishonoredLinkUDisAISubProcessHeadTrackPause()
{
}

// the shortest signed yaw difference in Unreal units (retail's DisAngleDiff, inlined everywhere it is used)
static INT DisGEYawDiff( INT _A, INT _B )
{
	return ( ( _A - _B + 32768 ) & 65535 ) - 32768;
}

// DISHONORED(port): 2012 rva 0x79be90 / 2013 0x73e710 (vtable slot 97)
void UDisAISubProcessHeadTrackPause::BeginSubProcess_Derived()
{
	UDisAISubProcessHeadTrackBase::BeginSubProcess_Derived();
	m_bIsPausing = FALSE;
}

// DISHONORED(port): 2012 rva 0x7a4eb0 (638 B) / 2013 0x73adb0 (vtable slot 99). The two tweak angles are read at their
// retail offsets (m_fRearHalfAngleWidthToPause_Deg @140, m_fFrontHalfAngleWidthToResume_Deg @144): while the target
// stays within the front half-angle of the direction the NPC was walking in, nothing is desired; once it is outside
// that and within the rear half-angle of straight behind, the NPC desires to stand where he is and to face past his
// back on the target's side (+-90 degrees from straight behind), remembering the walking direction once per pause.
// Retail reads the brain's GetAttentionProxyInfo first and treats a target with status 0 as gone.
void UDisAISubProcessHeadTrackPause::TickSubProcess_Derived( const FLOAT _fDeltaTime )
{
	const UDisTweaks_AISubProcess_HeadTrackPause* Tweaks = Cast<UDisTweaks_AISubProcess_HeadTrackPause>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_AISubProcess_HeadTrackPause*)UDisTweaks_AISubProcess_HeadTrackPause::StaticClass()->GetDefaultObject();
	}
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	const FDisAttentionProxy HeadTrackProxy = GetHeadTrackProxy();
	UBOOL bTargetKnown = Pawn != NULL && HeadTrackProxy.m_pTarget.GetObject() != NULL && HeadTrackProxy.m_pTarget.GetInterface() != NULL;
	if( bTargetKnown && HeadTrackProxy.m_pBrain )
	{
		FDisAttentionProxyInfo Info( EC_EventParm );
		HeadTrackProxy.m_pBrain->GetAttentionProxyInfo( HeadTrackProxy.GetProxyAttnTarget(), Info );
		bTargetKnown = Info.m_Status != 0;
	}
	if( bTargetKnown )
	{
		const FVector ToTarget = HeadTrackProxy.GetProxyLocation() - Pawn->Location;
		const INT TargetYaw = ToTarget.Rotation().Yaw;
		const FLOAT DegreesFromWalk = Abs( (FLOAT)DisGEYawDiff( TargetYaw, m_OriginalPauseYaw ) * ( 360.f / 65536.f ) );
		if( DegreesFromWalk >= Tweaks->m_fFrontHalfAngleWidthToResume_Deg )
		{
			const INT OwnerInvYaw = DisGEYawDiff( Pawn->Rotation.Yaw, 32768 );
			const INT FromBehind = DisGEYawDiff( TargetYaw, OwnerInvYaw );
			const FLOAT DegreesFromBehind = Abs( (FLOAT)FromBehind * ( 360.f / 65536.f ) );
			if( DegreesFromBehind < Tweaks->m_fRearHalfAngleWidthToPause_Deg )
			{
				if( !m_bIsPausing )
				{
					const FVector OriginalDest = m_pOwningBrain->GetCurrentDesiredPosition();
					m_OriginalPauseYaw = ( OriginalDest - Pawn->Location ).Rotation().Yaw;
				}
				const INT Side = FromBehind > 0 ? 1 : ( FromBehind < 0 ? -1 : 0 );
				SetLocoLocationDesire( Pawn->Location, ETransitSpeed_Idle, -1.f, 1.f, FALSE, FALSE );
				SetFaceToYawDesire( OwnerInvYaw + ( Side << 14 ), -100.f, FALSE );
				m_bIsPausing = TRUE;
			}
			return;
		}
	}
	ClearLocoDesire();
	ClearFaceToDesire();
	m_bIsPausing = FALSE;
}
