// DishonoredGame/src/disanimnodeblendbyheadbob.cpp
// agent IP (PHASE23 IP): UDisAnimNodeBlendByHeadBob, a class 2013 added with the Head Bob Amount option (profile setting 108); no 2012
// counterpart. Registration 2013 VA 0x1309e68 (sizeof 224, InternalConstructor 0xa9e000, vtable 0x110bad8); the one override is
// TickAnim 2013 0x688c30 (209 B): child 0 ("Head Bob On", the walk with its additive camera bob) takes the camera's m_BobAmount,
// child 1 ("Head Bob Off", the bare sequence) the rest, then UAnimNodeBlendBase::TickAnim. Without it both weights stay at the
// package's 0 and the first relevant node fails UAnimNodeBlendBase::GetBoneAtoms' LastChildNonAddIndex check (sprint, IP0s.log).
// DISHONORED(written): -ipnoheadbob (diagnosis only) leaves the weights as loaded.

#include "DishonoredGame.h"
#include "ipdiag.h"

void DishonoredLinkIP_DisAnimNodeBlendByHeadBob()
{
}

void UDisAnimNodeBlendByHeadBob::TickAnim( FLOAT DeltaSeconds )
{
	if( !DisIPNoHeadBob() )
	{
		ADishonoredPawn* Pawn = SkelComponent ? Cast<ADishonoredPawn>( SkelComponent->GetOwner() ) : NULL;
		FLOAT BobAmount = 1.f;
		if( Pawn )
		{
			ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
			if( Camera )
			{
				BobAmount = Camera->m_BobAmount;
			}
		}
		Children(0).Weight = BobAmount;
		Children(1).Weight = 1.f - BobAmount;
	}
	Super::TickAnim( DeltaSeconds );
}
