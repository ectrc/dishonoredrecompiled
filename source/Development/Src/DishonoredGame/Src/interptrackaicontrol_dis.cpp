/*=============================================================================
	interptrackaicontrol_dis.cpp: agent JC (PHASE24 JC). The matinee AIControl track: its keys send body-intention, aim, aim-stop
	and fire requests to the group's NPC. Retail has these in Engine/Src/interptrackaicontrol.cpp; the track and its key properties are
	Engine-package classes this tree declares as DishonoredGame shims (DishonoredGameEngineShims.h), so the bodies live here.
	  UInterpTrackAIControl::UpdateTrack                       2013 0x8fee50 / 2012 0x53e120 (382 B both)
	  UInterpTrackInstAIControl::InitTrackInst                 2013 0x8fc900 (26 B)
	  UInterpTrackAIControlNPCFireKeyProperties::GetTarget     2012 0x53b930 (81 B)
	  UInterpTrackAIControlNPCAimKeyProperties::GetTarget      2012 0x53b990 (81 B)
=============================================================================*/
#include "DishonoredGame.h"
#include "jcdiag.h"

void DishonoredLinkJC_InterpTrackAIControl()
{
}

static AActor* DisAIControlKeyTarget( FName TargetName, UInterpTrackInst* const TrInst )
{
	USeqAct_Interp* Matinee = TrInst->GetMatinee();
	UInterpGroupInst* GroupInst = ( TargetName != NAME_Self ) ? Matinee->FindFirstGroupInstByName( TargetName ) : CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	return ( GroupInst != NULL ) ? GroupInst->GetGroupActor() : NULL;
}

// The key's target group by name, or the track's own group for 'Self'.
AActor* UInterpTrackAIControlNPCFireKeyProperties::GetTarget( UInterpTrackInst* const TrInst ) const
{
	return DisAIControlKeyTarget( m_TargetName, TrInst );
}

AActor* UInterpTrackAIControlNPCAimKeyProperties::GetTarget( UInterpTrackInst* const TrInst ) const
{
	return DisAIControlKeyTarget( m_TargetName, TrInst );
}

void UInterpTrackInstAIControl::InitTrackInst( UInterpTrack* InTrack )
{
	Track = InTrack;
	m_LastUpdatePosition = -1.f;
}

// Played forward and not skipping, every key crossed since the last update (or the key at 0 on a start at 0) sends its request:
// KeyType 0 body intention, 1 aim at the key's target, 2 aim stop, 3 fire at the key's target. Retail tests the group actor's type
// flag (+266 bit 0x20) and calls the requests through the NPC vtable (slots 318-321); here the actor is cast to ADishonoredNPCPawn,
// the only class with requests. -jcnoaicontrol: inert.
void UInterpTrackAIControl::UpdateTrack( FLOAT NewPosition, UInterpTrackInst* TrInst, UBOOL bJump )
{
	ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( TrInst->GetGroupActor() );
	if( Pawn == NULL || DisJCNoAIControl() )
	{
		return;
	}
	UInterpTrackInstAIControl* AIControlInst = CastChecked<UInterpTrackInstAIControl>( TrInst );
	const USeqAct_Interp* Matinee = TrInst->GetMatinee();
	const FLOAT LastUpdatePosition = AIControlInst->m_LastUpdatePosition;
	if( LastUpdatePosition <= NewPosition && !Matinee->m_bSkipping )
	{
		for( INT KeyIndex = 0; KeyIndex < m_AIControlKeys.Num(); KeyIndex++ )
		{
			const FAIControlTrackKey& Key = m_AIControlKeys(KeyIndex);
			if( !( ( Key.Time > LastUpdatePosition && NewPosition >= Key.Time ) || ( Key.Time == 0.f && LastUpdatePosition == 0.f && NewPosition == 0.f ) ) )
			{
				continue;
			}
			AActor* Target = ( Key.Properties != NULL ) ? Key.Properties->GetTarget( TrInst ) : NULL;
			if( DisJCDiagEnabled() )
			{
				debugf( TEXT("DISHONORED(bringup): jcdiag aicontrol %s key %d type %d target %s matpos %.3f world %.2f"), *Pawn->GetName(), KeyIndex,
					(INT)Key.KeyType, ( Target != NULL ) ? *Target->GetName() : TEXT("none"), NewPosition, GWorld->GetTimeSeconds() );
			}
			switch( Key.KeyType )
			{
			case 0:
				Pawn->MAT_SendBodyIntentionRequest( (const UInterpTrackAIControlBodyIntentionKeyProperties*)Key.Properties );
				break;
			case 1:
				Pawn->MAT_SendAimRequest( (const UInterpTrackAIControlNPCAimKeyProperties*)Key.Properties, Target );
				break;
			case 2:
				Pawn->MAT_SendAimStopRequest();
				break;
			case 3:
				Pawn->MAT_SendFireRequest( (const UInterpTrackAIControlNPCFireKeyProperties*)Key.Properties, Target );
				break;
			}
		}
	}
	AIControlInst->m_LastUpdatePosition = NewPosition;
}
