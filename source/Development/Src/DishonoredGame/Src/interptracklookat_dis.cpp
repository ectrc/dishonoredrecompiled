/*=============================================================================
	interptracklookat_dis.cpp: agent HB (PHASE22 HB). The matinee LookAt track's per-frame half: what turns the AI groups'
	LookAt keys into requests on the pawn's FArkComponentLookat.

	Retail has these in Engine/Src/interptracklookat.cpp; FArkComponentLookat is declared in DishonoredGame (agent FT's reason,
	Inc/arkcomponentlookat.h), so the two functions that talk to it are defined here and Engine reaches them through
	GDisLookAtTrackUpdateHook / GDisLookAtTrackTermHook (UnWorld.h), installed below:
	  UInterpTrackLookAt::StaticUpdateTracks  2012 0x547010 (3,756 B) / 2013 0x9082c0 (3,386 B, the only call is
	                                          UInterpGroupInstAI::UpdateGroupInst 2013 0x6305c0 / 2012 0x25bfd0)
	  UInterpTrackLookAt::GetLookAtInfos      2012 0x542c10 / 2013 0x904390 (637 B both)
	  UInterpTrackInstLookAt::TermTrackInst   2012 0x53bcb0 (81 B): its component half
=============================================================================*/
#include "DishonoredGame.h"
#include "arkcomponentlookat.h"
#include "fkmatineediag.h"

void DishonoredLinkHB_InterpTrackLookAt()
{
}

// DISHONORED(port): 2012 rva 0x542c10 / 2013 0x904390. The last key whose [StartTime, StartTime + KeyLength) holds the position;
// the influences from its m_LookAtInfluence (>= 1 head, >= 2 torso), the weight from the float curve, the speed from the key.
// The direction runs from the pawn's eyes to: the local player's pawn (target type 0), the pawn's neutral forward (1, returned
// as a direction), or the key's target group's actor at m_BoneOrSocket (2; weight 0 when the group has no actor).
UBOOL UInterpTrackLookAt::GetLookAtInfos( FLOAT NewPosition, const UInterpTrackInstLookAt* TrInst, FVector& OutWorldDirection, FLOAT& OutWeight, FLOAT& OutHeadInfluence, FLOAT& OutTorsoInfluence, FLOAT& OutSpeedFactor ) const
{
	UInterpGroupInst* GrInst = CastChecked<UInterpGroupInst>( TrInst->GetOuter() );
	APawn* Pawn = Cast<APawn>( GrInst->GetGroupActor() );
	FArkComponentLookat* Lookat = FArkComponentLookat::GetLookatComponent( Pawn );
	USeqAct_Interp* Matinee = TrInst->GetMatinee();
	const FVector EyesPosition = Lookat->GetLookEyesPosition( NULL, NULL, NULL );

	INT KeyIndex = LookAtKeys.Num() - 1;
	for( ; KeyIndex >= 0; KeyIndex-- )
	{
		const FLookAtControlTrackKey& Key = LookAtKeys(KeyIndex);
		if( NewPosition >= Key.StartTime && NewPosition < Key.StartTime + Key.KeyLength )
		{
			break;
		}
	}
	if( KeyIndex < 0 )
	{
		return FALSE;
	}
	const UInterpTrackLookAtKeyProperties* Properties = LookAtKeys(KeyIndex).Properties;
	OutHeadInfluence = ( Properties->m_LookAtInfluence >= 1 ) ? 1.f : 0.f;
	OutTorsoInfluence = ( Properties->m_LookAtInfluence >= 2 ) ? 1.f : 0.f;
	OutWeight = FloatTrack.Eval( NewPosition, 0.f );
	OutSpeedFactor = Properties->m_SpeedFactor;
	if( Properties->m_TargetType == 0 )
	{
		APlayerController* PC = Pawn->GetALocalPlayerController();
		OutWorldDirection = FArkComponentLookat::GetLookAtPosition( PC ? PC->Pawn : NULL, NAME_None ) - EyesPosition;
		return TRUE;
	}
	if( Properties->m_TargetType == 1 )
	{
		OutWorldDirection = Lookat->GetLookAtNeutralDirection();
		return TRUE;
	}
	UInterpGroupInst* TargetInst = ( Properties->m_TargetType == 2 && Matinee != NULL ) ? Matinee->FindFirstGroupInst( Properties->m_Target ) : NULL;
	AActor* TargetActor = ( TargetInst != NULL ) ? TargetInst->GetGroupActor() : NULL;
	if( TargetActor == NULL )
	{
		OutWeight = 0.f;
		return TRUE;
	}
	OutWorldDirection = FArkComponentLookat::GetLookAtPosition( TargetActor, Properties->m_BoneOrSocket ) - EyesPosition;
	return TRUE;
}

struct FTrackLookAtInfos
{
	UInterpTrackInstLookAt* TrInst;
	FLOAT Weight;
	FLOAT HeadInfluence;
	FLOAT TorsoInfluence;
	FLOAT SpeedFactor;
	FLOAT Distance;
	FLOAT Yaw;
	FLOAT Pitch;
};

// DISHONORED(port): 2012 rva 0x547010 / 2013 0x9082c0. Every LookAt instance of the group (m_bIsAUInterpTrackInstLookAt) is
// collected; on a jump they only forget their goal. Otherwise each live key becomes a yaw (round the pawn's up axis, measured
// so forward is 0) and a pitch toward its target in the pawn's frame; the keys are weighted together (distance floored at
// LOOKAT_TARGET_DISTANCE, 200 - an immediate in the 2013 body), the group's previous requests are dropped, and one request to
// look at the point that far along the blended direction from the eyes is made, asker the group instance.
void UInterpTrackLookAt::StaticUpdateTracks( FLOAT NewPosition, UInterpGroupInstAI* GrInstAI, UBOOL bJump, UBOOL bPreview )
{
	// DISHONORED(bringup): agent HB. -hbnolooktrack leaves the track unstepped (no look-at request from a LookAt key): an A/B switch.
	static INT Disabled = -1;
	if( Disabled < 0 )
	{
		Disabled = ( appStrfind( appCmdLine(), TEXT("-hbnolooktrack") ) != NULL ) ? 1 : 0;
	}
	if( Disabled )
	{
		return;
	}
	TArray<FTrackLookAtInfos> LookAtInfos;
	for( INT TrackIndex = 0; TrackIndex < GrInstAI->TrackInst.Num(); TrackIndex++ )
	{
		UInterpTrackInst* TrInst = GrInstAI->TrackInst(TrackIndex);
		// DISHONORED(port): the 2012 body's test; 2013 trusts the bit alone
		if( TrInst != NULL && TrInst->m_bIsAUInterpTrackInstLookAt && TrInst->IsA( UInterpTrackInstLookAt::StaticClass() ) )
		{
			FTrackLookAtInfos& Info = LookAtInfos( LookAtInfos.AddZeroed() );
			Info.TrInst = (UInterpTrackInstLookAt*)TrInst;
		}
	}
	if( bJump )
	{
		for( INT InfoIndex = 0; InfoIndex < LookAtInfos.Num(); InfoIndex++ )
		{
			LookAtInfos(InfoIndex).TrInst->LastReachedGoalIndex = INDEX_NONE;
			LookAtInfos(InfoIndex).TrInst->LastUpdate = NewPosition;
		}
		return;
	}
	if( LookAtInfos.Num() == 0 )
	{
		return;
	}
	APawn* Pawn = Cast<APawn>( GrInstAI->GetGroupActor() );
	FArkComponentLookat* Lookat = FArkComponentLookat::GetLookatComponent( Pawn );
	if( Lookat == NULL || Lookat->GetConfig() == NULL )
	{
		return;
	}
	const FQuat PawnQuat = Pawn->Rotation.Quaternion();
	const FVector NpcAt = PawnQuat.RotateVector( FVector( 1.f, 0.f, 0.f ) );
	const FVector NpcRight = PawnQuat.RotateVector( FVector( 0.f, 1.f, 0.f ) );
	const FVector NpcUp = PawnQuat.RotateVector( FVector( 0.f, 0.f, 1.f ) );

	FLOAT TotalWeight = 0.f;
	for( INT InfoIndex = LookAtInfos.Num() - 1; InfoIndex >= 0; InfoIndex-- )
	{
		FTrackLookAtInfos& Info = LookAtInfos(InfoIndex);
		UInterpTrackLookAt* Track = (UInterpTrackLookAt*)Info.TrInst->Track;
		Info.TrInst->LastUpdate = NewPosition;
		FVector Direction( 0.f, 0.f, 0.f );
		FLOAT Weight = 0.f;
		FLOAT Distance = 0.f;
		if( Track->IsDisabled()
			|| !Track->GetLookAtInfos( NewPosition, Info.TrInst, Direction, Weight, Info.HeadInfluence, Info.TorsoInfluence, Info.SpeedFactor )
			|| Weight <= 0.0001f
			|| ( Distance = Direction.Size() ) <= 0.0001f )
		{
			LookAtInfos.RemoveSwap( InfoIndex );
			continue;
		}
		const FVector ToTarget = Direction / Distance;
		Info.Distance = Distance;
		const FLOAT UpDot = ToTarget | NpcUp;
		const FVector Flat = ( ToTarget - NpcUp * UpDot ).SafeNormal();
		const FLOAT RightAngle = appAcos( Flat | NpcRight );
		if( ( ToTarget | NpcAt ) <= 0.f )
		{
			Info.Yaw = ( RightAngle <= HALF_PI ) ? RightAngle + HALF_PI : RightAngle - 3.f * HALF_PI;
		}
		else
		{
			Info.Yaw = HALF_PI - RightAngle;
		}
		Info.Pitch = HALF_PI - appAcos( UpDot );
		Info.Weight = Min( Weight, 1.f );
		TotalWeight += Info.Weight;
	}
	Lookat->StopAllLookAtFromAsker( GrInstAI );
	if( LookAtInfos.Num() == 0 )
	{
		return;
	}

	const FLOAT InvTotal = 1.f / TotalWeight;
	FLOAT HeadInfluence = 0.f;
	FLOAT TorsoInfluence = 0.f;
	FLOAT SpeedFactor = 0.f;
	FLOAT Distance = 0.f;
	FLOAT Yaw = 0.f;
	FLOAT Pitch = 0.f;
	for( INT InfoIndex = 0; InfoIndex < LookAtInfos.Num(); InfoIndex++ )
	{
		const FTrackLookAtInfos& Info = LookAtInfos(InfoIndex);
		const FLOAT Share = Info.Weight * InvTotal;
		HeadInfluence += Info.HeadInfluence * Share;
		TorsoInfluence += Info.TorsoInfluence * Share;
		SpeedFactor += Info.SpeedFactor * Share;
		Distance += Max( Info.Distance, 200.f ) * Share;
		Yaw += Info.Yaw * Share;
		Pitch += Info.Pitch * Share;
	}
	if( TotalWeight < 1.f )
	{
		Yaw *= TotalWeight;
		Pitch *= TotalWeight;
	}
	const FVector Flat = NpcAt * appCos( Yaw ) + NpcRight * appSin( Yaw );
	const FVector Axis = ( Flat ^ NpcUp ) * appSin( Pitch * 0.5f );
	const FQuat PitchQuat( Axis.X, Axis.Y, Axis.Z, appCos( Pitch * 0.5f ) );
	const FVector LookDirection = PitchQuat.RotateVector( Flat );

	FDisLookAtInfluence Influence;
	Influence.m_fHeadInfluence = Clamp( HeadInfluence, 0.f, 1.f );
	Influence.m_fTorsoInfluence = Clamp( TorsoInfluence, 0.f, 1.f );
	Influence.m_bTorsoInfIsMoveSpeedDependant = 1;
	const FVector Target = Lookat->GetLookEyesPosition( NULL, NULL, NULL ) + LookDirection * Distance;
	const UInterpGroupAI* AIGroup = Cast<UInterpGroupAI>( GrInstAI->Group );
	const INT Priority = GetGameLookatPriority( AIGroup != NULL ? AIGroup->LookatRequestsPriority : 1 );
	const INT RequestID = Lookat->StartLookAtLocation( GrInstAI, GrInstAI->GetFName(), Priority, Target, Influence, SpeedFactor, FALSE, -1.f );

	if( DisFKDiagEnabled() )
	{
		static TMap<const UObject*, INT> LastKey;
		static TMap<const UObject*, FLOAT> LastReportAt;
		const FLOAT Now = GWorld ? GWorld->GetTimeSeconds() : 0.f;
		for( INT InfoIndex = 0; InfoIndex < LookAtInfos.Num(); InfoIndex++ )
		{
			const UInterpTrackLookAt* Track = (UInterpTrackLookAt*)LookAtInfos(InfoIndex).TrInst->Track;
			INT KeyIndex = Track->LookAtKeys.Num() - 1;
			while( KeyIndex >= 0 && !( NewPosition >= Track->LookAtKeys(KeyIndex).StartTime && NewPosition < Track->LookAtKeys(KeyIndex).StartTime + Track->LookAtKeys(KeyIndex).KeyLength ) )
			{
				KeyIndex--;
			}
			const INT* Last = LastKey.Find( Track );
			if( Last != NULL && *Last == KeyIndex )
			{
				continue;
			}
			LastKey.Set( Track, KeyIndex );
			const UInterpTrackLookAtKeyProperties* Properties = Track->LookAtKeys.IsValidIndex( KeyIndex ) ? Track->LookAtKeys(KeyIndex).Properties : NULL;
			debugf( TEXT("DISHONORED(bringup): fkdiag lookattrack %s key %d -> %s (type %d infl %d bone %s) w %.2f prio %d req %d pawn %s pos %.2f world %.2f"),
				GrInstAI->Group ? *GrInstAI->Group->GroupName.ToString() : TEXT("?"), KeyIndex,
				( Properties && Properties->m_Target ) ? *Properties->m_Target->GroupName.ToString() : ( Properties ? *Properties->m_TargetName.ToString() : TEXT("-") ),
				Properties ? (INT)Properties->m_TargetType : -1, Properties ? (INT)Properties->m_LookAtInfluence : -1,
				Properties ? *Properties->m_BoneOrSocket.ToString() : TEXT("-"), LookAtInfos(InfoIndex).Weight, Priority, RequestID,
				*Pawn->GetName(), NewPosition, Now );
		}
		FLOAT* LastAt = LastReportAt.Find( GrInstAI );
		if( LastAt == NULL || Now - *LastAt >= 1.f )
		{
			LastReportAt.Set( GrInstAI, Now );
			const FVector2D& Cursor = Lookat->GetCurrentCursorPosition();
			debugf( TEXT("DISHONORED(bringup): fkdiag lookattrack %s %s request yaw %.1f pitch %.1f dist %.0f head %.2f torso %.2f speed %.2f | cursor %.1f/%.1f active prio %d requests %d world %.2f"),
				GrInstAI->Group ? *GrInstAI->Group->GroupName.ToString() : TEXT("?"), *Pawn->GetName(), Yaw * 180.f / PI, Pitch * 180.f / PI, Distance,
				Influence.m_fHeadInfluence, Influence.m_fTorsoInfluence, SpeedFactor, Cursor.X * 180.f / PI, Cursor.Y * 180.f / PI,
				Lookat->GetActiveRequestPriority(), Lookat->GetRequestsCount(), Now );
		}
	}
}

// DISHONORED(port): 2012 rva 0x53bcb0, the component half of UInterpTrackInstLookAt::TermTrackInst: the requests the track made
// for the group are dropped with it (asker the matinee's first instance of the owning group).
static void DisLookAtTrackTerm( APawn* Pawn, const void* Asker )
{
	FArkComponentLookat* Lookat = FArkComponentLookat::GetLookatComponent( Pawn );
	if( Lookat != NULL && Lookat->GetConfig() != NULL )
	{
		Lookat->StopAllLookAtFromAsker( Asker );
	}
}

struct FDisLookAtTrackHookInstaller
{
	FDisLookAtTrackHookInstaller()
	{
		GDisLookAtTrackUpdateHook = &UInterpTrackLookAt::StaticUpdateTracks;
		GDisLookAtTrackTermHook = &DisLookAtTrackTerm;
	}
};
static FDisLookAtTrackHookInstaller GDisLookAtTrackHookInstaller;
