// DishonoredGame/src/disanimnodeblendbyphysicalcondition.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6b1e80  public: static void __cdecl UDisAnimNodeBlendByPhysicalCondition::InitializePrivateStaticClassUDisAnimNodeBlendByPhysicalCondition(void)
//   0x6bded0  public: virtual void __thiscall UDisAnimNodeBlendByPhysicalCondition::TickAnim(float)
//   0x6c8fd0  public: static class UClass * __cdecl UDisAnimNodeBlendByPhysicalCondition::GetPrivateStaticClassUDisAnimNodeBlendByPhysicalCondition(wchar_t const *)
//   0x6ca450  public: static class UClass * __cdecl UDisAnimNodeBlendByPhysicalCondition::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	Agent IP (PHASE23 IP): the player tree's out-of-breath layer (Ply_Player_at: DisAnimNodeBlendByPhysicalCondition_0 adds
	ADD_Head_OutOfbreath on top of the body). TickAnim 2012 0x6bded0 / 2013 0x688d80: the additive's target weight is the player's
	GetAimStability clamped to 0..1, blended over 0.1 s, then UAnimNodeAdditiveBlending::TickAnim. Unported, the node kept the
	package's weight 1: the head breathed hard all the time - in the soiree (where GetAimStability is 0) the AnimDriven camera,
	which takes the camera bone's rotation, swung +-2.2 deg in pitch at ~0.8 Hz (IP1k.log, world 140-149).
	DISHONORED(written): -ipnobreath (diagnosis only) leaves the weight as loaded.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "ipdiag.h"

void DishonoredLinkIP_DisAnimNodeBlendByPhysicalCondition()
{
}

void UDisAnimNodeBlendByPhysicalCondition::TickAnim( FLOAT DeltaSeconds )
{
	ADishonoredPlayerPawn* Player = SkelComponent ? Cast<ADishonoredPlayerPawn>( SkelComponent->GetOwner() ) : NULL;
	if( Player && !DisIPNoBreath() )
	{
		SetBlendTarget( Clamp( Player->GetAimStability(), 0.f, 1.f ), 0.1f );
	}
	Super::TickAnim( DeltaSeconds );
}
