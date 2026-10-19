// DishonoredGame/src/dismantlefinderstate_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x722410  public: static void __cdecl UDisMantleFinderState_Base::InitializePrivateStaticClassUDisMantleFinderState_Base(void)
//   0x722430  protected: virtual void __thiscall UDisMantleFinderState_Base::OnExitState(class UDishonoredNativeState *)
//   0x7279f0  protected: virtual void __thiscall FDisMantleFinderState_Base_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x727a20  protected: void __thiscall UDisMantleFinderState_Base::InitMantlerInfo(void)
//   0x727be0  protected: void __thiscall UDisMantleFinderState_Base::SetMantleCheckSucceeded(class FVector const &, int, float, unsigned int, unsigned int)
//   0x72af30  public: static class UClass * __cdecl UDisMantleFinderState_Base::GetPrivateStaticClassUDisMantleFinderState_Base(wchar_t const *)
//   0x72dbc0  public: static class UClass * __cdecl UDisMantleFinderState_Base::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the mantle edge finder's states, base. Every state knows its component (m_pOwningDMEFC, the machine's
	managed object) and a snapshot of the pawn (m_OwnerInfo) the first state takes. 2012 / 2013 rvas in
	Inc/CppText/UDisMantleFinderState_Base.h; the parameter carriers are Inc/dismantlefinderparams.h.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dismantlefinderparams.h"
#include "dishonoredutilities.h"
#include "igdiag.h"

void DishonoredLinkIG_DisMantleFinderStateBase()
{
}

// 2012 0x7279f0 / 2013 0x6c6b50 (33 B)
void FDisMantleFinderState_Base_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UDisMantleFinderState_Base* State = (UDisMantleFinderState_Base*)PendingState;
	State->m_pOwningDMEFC = (UDishonoredMantleEdgeFinderComponent*)ManagedObject;
	State->m_OwnerInfo = m_OwnerInfo;
}

// 2012 0x722430 / 2013 0x6c1ea0 (10 B)
void UDisMantleFinderState_Base::OnExitState( UDishonoredNativeState* NextState )
{
	m_pOwningDMEFC = NULL;
}

/**
 * 2012 0x727a20 / 2013 0x6c6b80 (433 B, same bytes). The standing extent is the components' bounding box with the class default
 * cylinder's height, the crouching one the same box with CrouchHeight; the feet, facing and velocity are the pawn's unless the
 * component was given an alternate start (the blink finder).
 */
void UDisMantleFinderState_Base::InitMantlerInfo()
{
	APawn* Owner = (APawn*)m_pOwningDMEFC->GetOwner();
	const FBox Box = Owner->GetComponentsBoundingBox( FALSE );
	FVector CurrentExtent( ( Box.Max.X - Box.Min.X ) * 0.5f, ( Box.Max.Y - Box.Min.Y ) * 0.5f, ( Box.Max.Z - Box.Min.Z ) * 0.5f );
	FLOAT DefaultHeight = 0.f;
	UCylinderComponent* DefaultCylinder = Cast<UCylinderComponent>( Owner->GetClass()->GetDefaultObject<APawn>()->CylinderComponent );
	if( DefaultCylinder )
	{
		DefaultHeight = DefaultCylinder->CollisionHeight;
	}
	const FVector StandExtent( CurrentExtent.X, CurrentExtent.Y, ( DefaultHeight - CurrentExtent.Z ) + CurrentExtent.Z );
	CurrentExtent.Z = Owner->CrouchHeight;
	FVector FootPos = DisGetPawnFeet( Owner );
	FVector Facing = Owner->Rotation.Vector();
	const FVector Velocity = Owner->Velocity;
	if( m_pOwningDMEFC->m_bUseAlternateStartPos )
	{
		FootPos = m_pOwningDMEFC->m_AlternateStartPos_FootPos;
		Facing = m_pOwningDMEFC->m_AlternateStartPos_Facing;
	}
	m_OwnerInfo.m_StandExtent = StandExtent;
	m_OwnerInfo.m_CrouchExtent = CurrentExtent;
	m_OwnerInfo.m_FootPos = FootPos;
	m_OwnerInfo.m_Facing = Facing;
	m_OwnerInfo.m_Velocity = Velocity;
}

// 2012 0x727be0 / 2013 0x6c6d40 (108 B): the spot is kept, the check ends, the machine goes back to its first state
void UDisMantleFinderState_Base::SetMantleCheckSucceeded( const FVector& StartLocation, INT StartYaw, FLOAT EdgeHeight, UBOOL bImpactAnimRequired, UBOOL bForceCrouch )
{
	UDishonoredMantleEdgeFinderComponent* Finder = m_pOwningDMEFC;
	Finder->m_bHasValidMantleSpot = TRUE;
	Finder->m_CurrentMantleSpot.m_StartLocation = StartLocation;
	Finder->m_CurrentMantleSpot.m_StartYaw = StartYaw;
	Finder->m_CurrentMantleSpot.m_fEdgeHeight = EdgeHeight;
	Finder->m_CurrentMantleSpot.m_bRequiresImpactAnim = bImpactAnimRequired ? TRUE : FALSE;
	Finder->m_CurrentMantleSpot.m_bForcesCrouch = bForceCrouch ? TRUE : FALSE;
	DisIGNoteFinderResult( Finder, TRUE, TEXT("spot") );
	Finder->OnMantleCheckEnd();
	RequestStateExit();
}

// 2013 0x6c1eb0 (the 2012 bodies inline it): no spot, the check ends, the machine goes back to its first state
void UDisMantleFinderState_Base::MantleCheckFailed( const TCHAR* Reason )
{
	UDishonoredMantleEdgeFinderComponent* Finder = m_pOwningDMEFC;
	Finder->m_bHasValidMantleSpot = FALSE;
	DisIGNoteFinderResult( Finder, FALSE, Reason );
	Finder->OnMantleCheckEnd();
	RequestStateExit();
}
