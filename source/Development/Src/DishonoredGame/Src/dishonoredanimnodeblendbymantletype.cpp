// DishonoredGame/src/dishonoredanimnodeblendbymantletype.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6cfed0  public: static void __cdecl UDishonoredAnimNodeBlendByMantleType::InitializePrivateStaticClassUDishonoredAnimNodeBlendByMantleType(void)
//   0x6d8d70  public: virtual void __thiscall UDishonoredAnimNodeBlendByMantleType::TickAnim(float)
//   0x6f2640  public: static class UClass * __cdecl UDishonoredAnimNodeBlendByMantleType::GetPrivateStaticClassUDishonoredAnimNodeBlendByMantleType(wchar_t const *)
//   0x6f4b10  public: static class UClass * __cdecl UDishonoredAnimNodeBlendByMantleType::StaticClassNoInline(void)

// agent IM (PHASE23 IM): 2012 0x6d8d70 / 2013 0x689810 (225 B). Inside the mantle state: child 0 up to m_LowMantleMaxEdgeHeight, 1 up to
// m_MediumMantleMaxEdgeHeight, 2 above, from the player's mantle tweaks (the default player tweaks' when it has none), snapped.
// DISHONORED(written): ADishonoredGameInfo::LoadDefaultPlayerTweaks is the class default of UDisTweaks_PlayerPawn here.

#include "DishonoredGame.h"

void DishonoredLinkIM_DishonoredAnimNodeBlendByMantleType()
{
}

void UDishonoredAnimNodeBlendByMantleType::TickAnim( FLOAT DeltaSeconds )
{
	ADishonoredPlayerPawn* Player = SkelComponent ? Cast<ADishonoredPlayerPawn>( SkelComponent->GetOwner() ) : NULL;
	UStatePlayerMasterMantle* Mantle = ( Player && Player->m_pPlayerMasterFSM ) ? Cast<UStatePlayerMasterMantle>( Player->m_pPlayerMasterFSM->GetLogicalState() ) : NULL;
	if( Mantle )
	{
		const FLOAT Height = Mantle->GetCurrentMantleHeight();
		UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( Player->GetTweaks_Derived() );
		UDisTweaks_PlayerPawn_Mantle* MantleTweaks = Tweaks ? Tweaks->m_pMantleTweaks : NULL;
		if( MantleTweaks == NULL )
		{
			MantleTweaks = UDisTweaks_PlayerPawn::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn>()->m_pMantleTweaks;
		}
		if( MantleTweaks )
		{
			const INT Child = MantleTweaks->m_LowMantleMaxEdgeHeight < Height ? ( MantleTweaks->m_MediumMantleMaxEdgeHeight < Height ? 2 : 1 ) : 0;
			SetActiveChild( Child, 0.f );
		}
	}
	Super::TickAnim( DeltaSeconds );
}
