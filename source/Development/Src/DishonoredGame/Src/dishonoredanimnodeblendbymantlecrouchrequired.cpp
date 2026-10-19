// DishonoredGame/src/dishonoredanimnodeblendbymantlecrouchrequired.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6cfeb0  public: static void __cdecl UDishonoredAnimNodeBlendByMantleCrouchRequired::InitializePrivateStaticClassUDishonoredAnimNodeBlendByMantleCrouchRequired(void)
//   0x6d8d00  public: virtual void __thiscall UDishonoredAnimNodeBlendByMantleCrouchRequired::TickAnim(float)
//   0x6f25b0  public: static class UClass * __cdecl UDishonoredAnimNodeBlendByMantleCrouchRequired::GetPrivateStaticClassUDishonoredAnimNodeBlendByMantleCrouchRequired(wchar_t const *)
//   0x6f4ae0  public: static class UClass * __cdecl UDishonoredAnimNodeBlendByMantleCrouchRequired::StaticClassNoInline(void)

// agent IM (PHASE23 IM): 2012 0x6d8d00 (104 B): inside the mantle state, child 1 when the mantle needs a crouch, else child 0

#include "DishonoredGame.h"

void DishonoredLinkIM_DishonoredAnimNodeBlendByMantleCrouchRequired()
{
}

void UDishonoredAnimNodeBlendByMantleCrouchRequired::TickAnim( FLOAT DeltaSeconds )
{
	ADishonoredPlayerPawn* Player = SkelComponent ? Cast<ADishonoredPlayerPawn>( SkelComponent->GetOwner() ) : NULL;
	UStatePlayerMasterMantle* Mantle = ( Player && Player->m_pPlayerMasterFSM ) ? Cast<UStatePlayerMasterMantle>( Player->m_pPlayerMasterFSM->GetLogicalState() ) : NULL;
	if( Mantle )
	{
		SetActiveChild( Mantle->GetCrouchRequired() ? 1 : 0, 0.f );
	}
	Super::TickAnim( DeltaSeconds );
}
