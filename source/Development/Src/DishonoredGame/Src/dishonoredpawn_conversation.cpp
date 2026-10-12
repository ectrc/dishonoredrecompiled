// DishonoredGame/src/dishonoredpawn_conversation.cpp
// DISHONORED(port): agent FR (PHASE17 FR). ADishonoredPawn::GetSpeakerLocInfo 2012 rva 0x7b9880 (73 B): one entry at
// the pawn's location with its mesh as the FaceFX component and the pawn as the Ak event owner. The audio propagation
// cache pointer retail stores is the voice package's.
#include "DishonoredGame.h"

void ADishonoredPawn::GetSpeakerLocInfo( TArray<FDisConvSpeakerLocInfo>& _rLocInfoOut ) const
{
	FDisConvSpeakerLocInfo& Info = _rLocInfoOut( _rLocInfoOut.AddZeroed() );
	Info.m_Location = Location;
	Info.m_pSkelMeshCompForFaceFX = Mesh;
	Info.m_pAKEventOwner = const_cast<ADishonoredPawn*>( this );
}
