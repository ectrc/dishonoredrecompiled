/*=============================================================================
	gbdiag.cpp: agent GB (PHASE19 GB) - the ride boat's Move track against the dock, measured.

	  gbdiag track   once per live Move track instance: the group's actor and base, the track's frame (MoveFrame, RotMode,
	                 InitialTM, the reference frame GetMoveRefFrame answers now) and the key count
	  gbdiag key     one line per key of that track: time, relative position, euler, lookup group, and the WORLD position
	                 ComputeWorldSpaceKeyTransform gives it - what the actor is put at when the matinee reaches the key
	  gbdiag move    once a second per moving group: the matinee position against its length, where the track says the
	                 actor should be and where the actor is
	  gbdiag actor   once a second: the dock actors by name - the ride boat, the hidden boat, the moored boat, the mover
	Nothing here changes behaviour.
=============================================================================*/
#include "DishonoredGame.h"
#include "gbdiag.h"
#include "arkcomponentlocomotion.h"

static INT GDisGBDiag = -1;

UBOOL DisGBDiagEnabled()
{
	if( GDisGBDiag < 0 )
	{
		GDisGBDiag = ParseParam( appCmdLine(), TEXT("gbdiag") ) ? 1 : 0;
	}
	return GDisGBDiag != 0;
}

static FString DisGBRot( const FRotator& Rot )
{
	return FString::Printf( TEXT("%d/%d/%d"), Rot.Pitch & 0xffff, Rot.Yaw & 0xffff, Rot.Roll & 0xffff );
}

static FString DisGBActorState( AActor* Actor )
{
	if( Actor == NULL )
	{
		return TEXT("none");
	}
	FString State = FString::Printf( TEXT("%s at %s rot %s hidden %d physics %d collide %d block %d base %s"),
		*Actor->GetName(), *Actor->Location.ToString(), *DisGBRot( Actor->Rotation ), (INT)Actor->bHidden, (INT)Actor->Physics,
		(INT)Actor->bCollideActors, (INT)Actor->bBlockActors, Actor->Base ? *Actor->Base->GetName() : TEXT("none") );
	if( Actor->Base != NULL )
	{
		State += FString::Printf( TEXT(" (base at %s rot %s, relLoc %s relRot %s)"), *Actor->Base->Location.ToString(),
			*DisGBRot( Actor->Base->Rotation ), *Actor->RelativeLocation.ToString(), *DisGBRot( Actor->RelativeRotation ) );
	}
	return State;
}

static void DisGBDumpKeys( USeqAct_Interp* Seq, UInterpGroupInst* GrInst, UInterpTrackMove* Move, UInterpTrackInstMove* MoveInst )
{
	const FMatrix RefTM = Move->GetMoveRefFrame( MoveInst );
	debugf( TEXT("DISHONORED(bringup): gbdiag track %s group '%s' pos %.2f: actor %s; moveFrame %d rotMode %d disableMovement %d lookAt %s lodDist %.0f keys %d; initialTM origin %s rot %s; refTM origin %s rot %s"),
		*Seq->GetPathName(), *GrInst->Group->GroupName.ToString(), Seq->Position, *DisGBActorState( GrInst->GetGroupActor() ),
		(INT)Move->MoveFrame, (INT)Move->RotMode, (INT)Move->bDisableMovement, *Move->LookAtGroupName.ToString(), Move->fLODDistance,
		Move->PosTrack.Points.Num(), *MoveInst->InitialTM.GetOrigin().ToString(), *DisGBRot( MoveInst->InitialTM.Rotator() ),
		*RefTM.GetOrigin().ToString(), *DisGBRot( RefTM.Rotator() ) );
	for( INT KeyIndex = 0; KeyIndex < Move->PosTrack.Points.Num(); KeyIndex++ )
	{
		FLOAT KeyTime = 0.f, RotTime = 0.f;
		FVector KeyPos( 0.f ), KeyEuler( 0.f ), WorldPos( 0.f );
		FRotator WorldRot( 0, 0, 0 );
		Move->GetKeyframePosition( MoveInst, KeyIndex, KeyTime, KeyPos, NULL, NULL );
		Move->GetKeyframeRotation( MoveInst, KeyIndex, RotTime, KeyEuler, NULL, NULL );
		Move->ComputeWorldSpaceKeyTransform( MoveInst, KeyPos, FRotator::MakeFromEuler( KeyEuler ), WorldPos, WorldRot );
		const FName Lookup = ( KeyIndex < Move->LookupTrack.Points.Num() ) ? Move->LookupTrack.Points( KeyIndex ).GroupName : NAME_None;
		debugf( TEXT("DISHONORED(bringup): gbdiag key %s '%s' %d t %.2f rel %s euler %s mode %d lookup %s -> world %s rot %s"),
			*Seq->GetName(), *GrInst->Group->GroupName.ToString(), KeyIndex, KeyTime, *KeyPos.ToString(), *KeyEuler.ToString(),
			(INT)Move->PosTrack.Points( KeyIndex ).InterpMode, *Lookup.ToString(), *WorldPos.ToString(), *DisGBRot( WorldRot ) );
	}
}

static void DisGBDumpVariableLinks( USeqAct_Interp* Seq )
{
	for( INT LinkIndex = 0; LinkIndex < Seq->VariableLinks.Num(); LinkIndex++ )
	{
		const FSeqVarLink& Link = Seq->VariableLinks( LinkIndex );
		FString Vars;
		for( INT VarIndex = 0; VarIndex < Link.LinkedVariables.Num(); VarIndex++ )
		{
			USequenceVariable* Var = Link.LinkedVariables( VarIndex );
			if( Var == NULL )
			{
				Vars += TEXT(" NULL");
				continue;
			}
			Vars += FString::Printf( TEXT(" %s(%s"), *Var->GetClass()->GetName(), *Var->GetName() );
			USeqVar_Named* Named = Cast<USeqVar_Named>( Var );
			if( Named != NULL )
			{
				Vars += FString::Printf( TEXT(" finds '%s'"), *Named->FindVarName.ToString() );
			}
			UObject** ObjRef = Var->GetObjectRef( 0 );
			AActor* Actor = ObjRef ? Cast<AActor>( *ObjRef ) : NULL;
			Vars += FString::Printf( TEXT(" = %s)"), Actor ? *DisGBActorState( Actor ) : ( ObjRef && *ObjRef ) ? *(*ObjRef)->GetName() : TEXT("none") );
		}
		debugf( TEXT("DISHONORED(bringup): gbdiag varlink %s '%s':%s"), *Seq->GetPathName(), *Link.LinkDesc, *Vars );
	}
}

static void DisGBDumpAttachment( USeqAct_Interp* Seq, UInterpGroupInst* GrInst, UInterpTrackAttachment* Attach )
{
	UInterpGroupInst* TargetInst = Seq->FindFirstGroupInstByName( Attach->m_TargetName );
	debugf( TEXT("DISHONORED(bringup): gbdiag attach %s group '%s' -> target group '%s' (%s) becomeDynamic %d detachOnInterrupt %d keys %d disabled %d"),
		*Seq->GetPathName(), *GrInst->Group->GroupName.ToString(), *Attach->m_TargetName.ToString(),
		TargetInst ? *DisGBActorState( TargetInst->GetGroupActor() ) : TEXT("no group inst"),
		( TargetInst && TargetInst->Group ) ? (INT)TargetInst->Group->bBecomeDynamic : -1, (INT)Attach->m_bDetachActorOnInteruption,
		Attach->m_AttachmentKeys.Num(), (INT)Attach->IsDisabled() );
	for( INT KeyIndex = 0; KeyIndex < Attach->m_AttachmentKeys.Num(); KeyIndex++ )
	{
		const FAttachmentControlTrackKey& AttachKey = Attach->m_AttachmentKeys( KeyIndex );
		UInterpTrackAttachmentKeyProperties* Props = AttachKey.Properties;
		if( Props == NULL )
		{
			debugf( TEXT("DISHONORED(bringup): gbdiag attachkey %s '%s' %d t %.2f NO properties"), *Seq->GetName(), *GrInst->Group->GroupName.ToString(), KeyIndex, AttachKey.Time );
			continue;
		}
		debugf( TEXT("DISHONORED(bringup): gbdiag attachkey %s '%s' %d t %.2f type %d offset %s rot %s bone %s hard %d useOffset %d useRot %d"),
			*Seq->GetName(), *GrInst->Group->GroupName.ToString(), KeyIndex, AttachKey.Time, (INT)Props->m_KeyType,
			*Props->m_AttachmentInfo.RelativeOffset.ToString(), *DisGBRot( Props->m_AttachmentInfo.RelativeRotation ),
			*Props->m_AttachmentInfo.BoneName.ToString(), (INT)Props->m_AttachmentInfo.bHardAttach,
			(INT)Props->m_AttachmentInfo.bUseRelativeOffset, (INT)Props->m_AttachmentInfo.bUseRelativeRotation );
	}
}

void DisGBReport()
{
	static FLOAT NextAt = 0.f;
	static TArray<FString> Dumped;
	if( !DisGBDiagEnabled() || GWorld == NULL )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < NextAt )
	{
		return;
	}
	NextAt = Now + 1.f;
	for( TObjectIterator<USeqAct_Interp> It; It; ++It )
	{
		USeqAct_Interp* Seq = *It;
		if( Seq->IsTemplate() || !Seq->bIsPlaying || Seq->GroupInst.Num() == 0 )
		{
			continue;
		}
		const FString SeqKey = Seq->GetPathName();
		if( !Dumped.ContainsItem( SeqKey ) )
		{
			Dumped.AddItem( SeqKey );
			DisGBDumpVariableLinks( Seq );
		}
		for( INT GroupIndex = 0; GroupIndex < Seq->GroupInst.Num(); GroupIndex++ )
		{
			UInterpGroupInst* GrInst = Seq->GroupInst( GroupIndex );
			if( GrInst == NULL || GrInst->Group == NULL )
			{
				continue;
			}
			for( INT TrackIndex = 0; TrackIndex < GrInst->TrackInst.Num() && TrackIndex < GrInst->Group->InterpTracks.Num(); TrackIndex++ )
			{
				const FString Key = Seq->GetPathName() + TEXT("/") + GrInst->Group->GroupName.ToString() + FString::Printf( TEXT("/%d"), TrackIndex );
				UInterpTrackAttachment* Attach = Cast<UInterpTrackAttachment>( GrInst->Group->InterpTracks( TrackIndex ) );
				if( Attach != NULL && !Dumped.ContainsItem( Key ) )
				{
					Dumped.AddItem( Key );
					DisGBDumpAttachment( Seq, GrInst, Attach );
				}
				UInterpTrackInstMove* MoveInst = Cast<UInterpTrackInstMove>( GrInst->TrackInst( TrackIndex ) );
				UInterpTrackMove* Move = Cast<UInterpTrackMove>( GrInst->Group->InterpTracks( TrackIndex ) );
				if( MoveInst == NULL || Move == NULL || Move->PosTrack.Points.Num() == 0 )
				{
					continue;
				}
				if( !Dumped.ContainsItem( Key ) )
				{
					Dumped.AddItem( Key );
					DisGBDumpKeys( Seq, GrInst, Move, MoveInst );
				}
				AActor* Actor = GrInst->GetGroupActor();
				FVector EvalPos = Actor ? Actor->Location : FVector( 0.f );
				FRotator EvalRot = Actor ? Actor->Rotation : FRotator( 0, 0, 0 );
				Move->GetLocationAtTime( MoveInst, Seq->Position, EvalPos, EvalRot );
				UMatineeData* MData = GrInst->Group->GetMatineeData();
				debugf( TEXT("DISHONORED(bringup): gbdiag move t=%.2f %s '%s' pos %.2f of %.2f: track says %s rot %s; actor %s"),
					Now, *Seq->GetName(), *GrInst->Group->GroupName.ToString(), Seq->Position, MData ? MData->m_RunData.InterpLength : -1.f,
					*EvalPos.ToString(), *DisGBRot( EvalRot ), *DisGBActorState( Actor ) );
			}
		}
	}
	// the player's movement path: what the controller gives the pawn and what the locomotion component makes of it
	for( FActorIterator ActorIt; ActorIt; ++ActorIt )
	{
		APawn* Pawn = Cast<APawn>( *ActorIt );
		if( Pawn == NULL || Pawn->Controller == NULL || !Pawn->Controller->IsA( APlayerController::StaticClass() ) )
		{
			continue;
		}
		const FArkComponentLocomotion* Loco = FArkComponentLocomotion::GetLocomotionComponent( Pawn );
		debugf( TEXT("DISHONORED(bringup): gbdiag playermove t=%.2f %s physics %d hardAttach %d collide %d block %d base %s skel %s bone %s accel %s vel %s loco %s steering %s"),
			Now, *Pawn->GetName(), (INT)Pawn->Physics, (INT)Pawn->bHardAttach, (INT)Pawn->bCollideActors, (INT)Pawn->bBlockActors,
			Pawn->Base ? *Pawn->Base->GetName() : TEXT("none"), Pawn->BaseSkelComponent ? *Pawn->BaseSkelComponent->GetName() : TEXT("none"),
			*Pawn->BaseBoneName.ToString(), *Pawn->Acceleration.ToString(), *Pawn->Velocity.ToString(),
			Loco ? ( Loco->IsDisabled() ? TEXT("DISABLED") : TEXT("enabled") ) : TEXT("none"), Loco ? *Loco->GetSteeringForce().ToString() : TEXT("-") );
	}
	static const TCHAR* Names[] = { TEXT("SkeletalMeshActorMAT_1"), TEXT("SkeletalMeshActorMAT_2"), TEXT("SkeletalMeshActor_2"), TEXT("InterpActor_3") };
	for( FActorIterator ActorIt; ActorIt; ++ActorIt )
	{
		AActor* Actor = *ActorIt;
		for( INT NameIndex = 0; NameIndex < ARRAY_COUNT( Names ); NameIndex++ )
		{
			if( Actor->GetName() == Names[NameIndex] )
			{
				debugf( TEXT("DISHONORED(bringup): gbdiag actor t=%.2f %s attached %d"), Now, *DisGBActorState( Actor ), Actor->Attached.Num() );
				ASkeletalMeshActor* SkelActor = Cast<ASkeletalMeshActor>( Actor );
				USkeletalMeshComponent* Mesh = SkelActor ? SkelActor->SkeletalMeshComponent : NULL;
				if( Mesh != NULL && Mesh->SkeletalMesh != NULL )
				{
					static const TCHAR* Bones[] = { TEXT("Player"), TEXT("Captain"), TEXT("Driver"), TEXT("Boat_Socket") };
					FString Line;
					for( INT BoneIndex = 0; BoneIndex < ARRAY_COUNT( Bones ); BoneIndex++ )
					{
						const INT Bone = Mesh->MatchRefBone( FName( Bones[BoneIndex] ) );
						if( Bone == INDEX_NONE )
						{
							// agent GK: the four names are sockets, not bones (agentGB.md section 2) - their world positions are what
							// the attachment track and retail's retargeting put the riders on
							FVector SocketLocation( 0.f, 0.f, 0.f );
							FRotator SocketRotation( 0, 0, 0 );
							if( Mesh->GetSocketWorldLocationAndRotation( FName( Bones[BoneIndex] ), SocketLocation, &SocketRotation ) )
							{
								Line += FString::Printf( TEXT(" %s: socket %s rot %s;"), Bones[BoneIndex], *SocketLocation.ToString(), *DisGBRot( SocketRotation ) );
							}
							else
							{
								Line += FString::Printf( TEXT(" %s: no bone;"), Bones[BoneIndex] );
							}
							continue;
						}
						const FMatrix BoneTM = Mesh->GetBoneMatrix( Bone );
						Line += FString::Printf( TEXT(" %s: %s rot %s;"), Bones[BoneIndex], *BoneTM.GetOrigin().ToString(), *DisGBRot( BoneTM.Rotator() ) );
					}
					const INT Root = ( Mesh->SkeletalMesh->RefSkeleton.Num() > 0 ) ? 0 : INDEX_NONE;
					if( Root != INDEX_NONE )
					{
						Line += FString::Printf( TEXT(" root(%s): %s;"), *Mesh->SkeletalMesh->RefSkeleton( 0 ).Name.ToString(), *Mesh->GetBoneMatrix( 0 ).GetOrigin().ToString() );
					}
					debugf( TEXT("DISHONORED(bringup): gbdiag bones t=%.2f %s bones %d:%s"), Now, *Actor->GetName(), Mesh->SkeletalMesh->RefSkeleton.Num(), *Line );
				}
			}
		}
	}
}
