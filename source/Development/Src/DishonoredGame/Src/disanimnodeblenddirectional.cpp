// DishonoredGame/src/disanimnodeblenddirectional.cpp
// agent IS (PHASE23 IS): UDisAnimNodeBlendDirectional::GetDirectionVector, 2012 0x6d87e0 / 2013 VA 0xa89210 (same bytes, ratio
// 1.000): the engine's direction (Velocity or Acceleration), replaced by the player's stick (PlayerInput RawJoyRight @384,
// RawJoyUp @380, measured against +Y) when m_bUseControllerInputDir is set and the pawn's controller is a player controller
// with a PlayerInput. Retail casts the owner to APawn and reads its Controller without a test; so does this.

#include "DishonoredGame.h"

void DishonoredLinkIS_DisAnimNodeBlendDirectional()
{
}

FVector UDisAnimNodeBlendDirectional::GetDirectionVector( AActor* Actor, UBOOL& bUseActorRotation ) const
{
	FVector Direction = Super::GetDirectionVector( Actor, bUseActorRotation );
	if( m_bUseControllerInputDir )
	{
		APlayerController* PC = Cast<APlayerController>( Cast<APawn>( Actor )->Controller );
		if( PC && PC->PlayerInput )
		{
			Direction.X = PC->PlayerInput->RawJoyRight;
			Direction.Y = PC->PlayerInput->RawJoyUp;
			bUseActorRotation = FALSE;
		}
	}
	return Direction;
}
