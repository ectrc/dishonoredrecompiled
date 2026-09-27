// DishonoredGame/src/dishonoredplayerpawn_body.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x6fb0c0  public: virtual unsigned int __thiscall ADishonoredPlayerPawn::OnTeleport_Native(class USeqAct_Teleport *)
//   0x6fb110  public: unsigned int __thiscall ADishonoredPlayerPawn::IsLeftHandDisabled(void)const
//   0x6fb120  public: void __thiscall ADishonoredPlayerPawn::EndTatooGlow(float, float)
//   0x6fe720  public: virtual void __thiscall ADishonoredPlayerPawn::ModifyMovementExtents(class FVector &, class FVector &, class FVector &, class FVector &, unsigned int &)const
//   0x6fe800  public: virtual class UClass * __thiscall ADishonoredPlayerPawn::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x6fe820  protected: void __thiscall ADishonoredPlayerPawn::GameLoad_Body(class FArchive &, enum ESaveLoadLocation)
//   0x6fe8b0  public: virtual int __thiscall ADishonoredPlayerPawn::TakeFallingDamage_Native(class FVector, class AActor *)
//   0x6fe910  public: void __thiscall ADishonoredPlayerPawn::CreatePlayerMatMIC(void)
//   0x705310  protected: virtual class UClass * __thiscall ADishonoredPlayerPawn::ChooseFootfallContactType(void)const
//   0x705410  protected: virtual unsigned int __thiscall ADishonoredPlayerPawn::IsAutoFootfallEnabled(void)const
//   0x7054a0  public: void __thiscall ADishonoredPlayerPawn::StartTatooGlow(void)
//   0x705510  public: enum ADishonoredPlayerPawn::ESwimmingState __thiscall ADishonoredPlayerPawn::GetSwimmingState(void)const
//   0x70bdf0  public: virtual class FVector __thiscall ADishonoredPlayerPawn::GetCameraPos(void)const

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF): the player's fall-damage override ----

// DISHONORED(written): 2013 rva 0x6a4fa0 (2012 0x6fe8b0): landing on a pawn costs the player nothing; anything else
// falls through to ADishonoredPawn::TakeFallingDamage_Native. Retail's test is
// FloorActor->m_ActorTypeFlags (BYTE @266) & 0x20, the bit both pawn kinds carry (an NPC is 34, the player 36).
// DISHONORED(bringup): m_ActorTypeFlags is never written in this tree (agent AU follow-up 6), so the Cast<> that means
// the same thing is used instead - as agent AU's ports do.
INT ADishonoredPlayerPawn::TakeFallingDamage_Native( FVector HitNormal, AActor* FloorActor )
{
	if( Cast<ADishonoredPawn>( FloorActor ) )
	{
		return 0;
	}
	return ADishonoredPawn::TakeFallingDamage_Native( HitNormal, FloorActor );
}

// DISHONORED(written): the generated exec wrapper; ICF folded it onto ADishonoredPawn::execTakeFallingDamage_Native
// (2013 rva 0x5ec5f0) because the code is identical - it dispatches through the virtual.
void ADishonoredPlayerPawn::execTakeFallingDamage_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FVector, HitNormal);
	P_GET_ACTOR(FloorActor);
	P_FINISH;
	*(INT*)Result = TakeFallingDamage_Native( HitNormal, FloorActor );
}
