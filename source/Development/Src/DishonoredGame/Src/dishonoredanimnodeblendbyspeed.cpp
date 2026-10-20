// DishonoredGame/src/dishonoredanimnodeblendbyspeed.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x6cfef0  public: static void __cdecl UDishonoredAnimNodeBlendBySpeed::InitializePrivateStaticClassUDishonoredAnimNodeBlendBySpeed(void)
//   0x6f26d0  public: static class UClass * __cdecl UDishonoredAnimNodeBlendBySpeed::GetPrivateStaticClassUDishonoredAnimNodeBlendBySpeed(wchar_t const *)
//   0x6f4b40  public: static class UClass * __cdecl UDishonoredAnimNodeBlendBySpeed::StaticClassNoInline(void)
//   0x6f7100  public: virtual float __thiscall UDishonoredAnimNodeBlendBySpeed::CalcSpeed(void)

/*-----------------------------------------------------------------------------
	Agent IP (PHASE23 IP): CalcSpeed 2012 0x6f7100 / 2013 0x689900 (324 B, the same code; 2013 shifts the two pawn virtuals
	one slot: PlayerControlled +496 -> +500, MaxSpeedModifier +1136 -> +1152). A pawn nobody controls plays at its top speed
	(MaxSpeedModifier x GroundSpeed); a controlled one measures the raw stick (m_bUseControllerInputSpeed: RawJoyRight,
	RawJoyUp of the controller's input), else its acceleration (bUseAcceleration) or velocity, flattened under m_bUse2DSpeed.
	Named gap: DisGetCarryCorpseState (2012 0x6fa000 region, the carried corpse's pawn replaces the owner) is not ported here;
	nothing is carried on the routes this was measured on.
	DISHONORED(written): -ipnospeed (diagnosis only) answers the engine's UAnimNodeBlendBySpeed::CalcSpeed, as before the port.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "ipdiag.h"

void DishonoredLinkIP_DishonoredAnimNodeBlendBySpeed()
{
}

FLOAT UDishonoredAnimNodeBlendBySpeed::CalcSpeed()
{
	if( DisIPNoSpeed() )
	{
		return Super::CalcSpeed();
	}
	APawn* Pawn = SkelComponent ? Cast<APawn>( SkelComponent->GetOwner() ) : NULL;
	if( !Pawn )
	{
		return Speed;
	}
	if( !Pawn->PlayerControlled() )
	{
		return Pawn->MaxSpeedModifier() * Pawn->GroundSpeed;
	}
	FVector WorkingVel( 0.f, 0.f, 0.f );
	if( m_bUseControllerInputSpeed )
	{
		ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Pawn->Controller );
		if( PC && PC->PlayerInput )
		{
			WorkingVel.X = PC->PlayerInput->RawJoyRight;
			WorkingVel.Y = PC->PlayerInput->RawJoyUp;
		}
	}
	else
	{
		WorkingVel = bUseAcceleration ? Pawn->Acceleration : Pawn->Velocity;
	}
	if( m_bUse2DSpeed )
	{
		WorkingVel.Z = 0.f;
	}
	return WorkingVel.Size();
}
