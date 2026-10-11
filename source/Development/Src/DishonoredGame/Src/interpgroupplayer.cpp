/*-----------------------------------------------------------------------------
	Agent FK (PHASE15 package FK): UInterpGroupPlayer and UInterpGroupInstPlayer.

	Engine/Src/interpgroupplayer.cpp is a comment-only skeleton and it has to stay one: both classes are
	declared in DishonoredGame/Inc/DishonoredGameEngineShims.h and IMPLEMENT_CLASS'd in
	DishonoredGameRegistrants.cpp, so an Engine unit cannot see them. The bodies therefore live in this
	module, which is also where the C++ vtable that has to carry them is emitted.

	Retail function set (2013 rva, VA = rva + 0x400000; the Engine skeleton's comments carry the 2012 rvas):

	  UInterpGroupPlayer::NeedWaitToStart         0x4fc450  (2012 0x53b610)   66 B
	  UInterpGroupInstPlayer::GetGroupActor       0x4fc4a0  (2012 0x53b660)    5 B  tail-jump to the base
	  UInterpGroupInstPlayer::GetGroupPawn        0x4fc4b0  (2012 0x53b670)    5 B  tail-jump to the base
	  UInterpGroupInstPlayer::HasActor            0x4fc4c0  (2012 0x53b680)   50 B  identical to the base
	  UInterpGroupInstPlayer::GetStageMarkPosition 0x4fc500 (2012 0x53b6c0)  581 B
	  UInterpGroupInstPlayer::BlendPlayerToMark   0x4fd880  (2012 0x53c5c0)  607 B
	  UInterpGroupInstPlayer::UpdateGroupInst     0x4fe8c0  (2012 0x53dd00)   66 B
	  UInterpGroupInstPlayer::TermGroupInst       0x4fe910  (2012 0x53dd50)  653 B
	  UInterpGroupInstPlayer::InitGroupInst       0x5027f0  (2012 0x541960)  309 B

	The three 5/50-byte ones are not ported and do not need to be: GetGroupActor and GetGroupPawn are
	single `jmp` instructions to UInterpGroupInst's, and HasActor's body is byte for byte the
	`return GetGroupActor() == InActor;` the base already has inline in EngineInterpolationClasses.h.

	The MAT_* calls retail makes from these bodies are NOT reachable in this tree and each one logs once
	instead. ADishonoredPlayerPawn::MAT_BeginPlayerGroup (2012 0x70fcc0) requests a state change into
	UStatePlayerMasterSoiree, and that class, UStatePlayerMasterInDialog::StartSoireeControl,
	ADishonoredPlayerPawn::m_pMatineeBlender and UInterpGroup::HasMoveTrack are all absent or excluded
	here. resources/docs/agents/agentFK.md specifies the whole contract.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "fkmatineediag.h"

/** UInterpGroupPlayer's bitfield word is one DWORD; these are the three bits these bodies read. */
static void DisFKWarnOnce( const TCHAR* What )
{
	static TArray<FString> Said;
	const FString Key( What );
	if( Said.FindItemIndex( Key ) != INDEX_NONE )
	{
		return;
	}
	Said.AddItem( Key );
	debugf( NAME_Warning, TEXT("the player group reached %s and it is not ported in this tree; the matinee continues without it"), What );
}

/**
 * TRUE the first time it is asked about a given instance. Kept out of the class on purpose:
 * UInterpGroupInstPlayer's layout is asserted at 152 bytes (DishonoredGameLayouts.h) and a diagnostic
 * is not worth a member.
 */
static UBOOL DisFKFirstMoveFor( const UInterpGroupInstPlayer* Inst )
{
	static TArray<const UInterpGroupInstPlayer*> Seen;
	if( Seen.FindItemIndex( Inst ) != INDEX_NONE )
	{
		return FALSE;
	}
	Seen.AddItem( Inst );
	return TRUE;
}

/**
 * DISHONORED(port): agent FK, 2013 rva 0x4fc450. Retail reads the pawn through Dishonored's
 * m_ActorTypeFlags & 0x20 fast path, which is what Cast<APawn> compiles to there.
 */
UBOOL UInterpGroupPlayer::NeedWaitToStart( UInterpGroupInst* GrInst ) const
{
	APawn* Pawn = ( GrInst != NULL ) ? Cast<APawn>( GrInst->GetGroupActor() ) : NULL;
	if( Pawn == NULL )
	{
		return FALSE;
	}
	// retail: !Pawn->MAT_IsUnequipped() (ADishonoredPlayerPawn 2012 0x6fb030, vtable slot 305)
	DisFKWarnOnce( TEXT("APawn::MAT_IsUnequipped") );
	return FALSE;
}

/**
 * DISHONORED(port): agent FK. Retail's body is UInterpGroupInstPlayer::UpdateGroupInst (2013 rva
 * 0x4fe8c0): Super, then BlendPlayerToMark( pos ) when the matinee is really playing in game. This
 * tree's update seam is UInterpGroup::UpdateGroup, so the test and the call are the same and only the
 * object carrying the override differs. See Inc/CppText/UInterpGroupPlayer.h.
 */
void UInterpGroupPlayer::UpdateGroup( FLOAT NewPosition, UInterpGroupInst* GrInst, UBOOL bPreview, UBOOL bJump )
{
	Super::UpdateGroup( NewPosition, GrInst, bPreview, bJump );

	UInterpGroupInstPlayer* PlayerInst = Cast<UInterpGroupInstPlayer>( GrInst );
	if( GIsGame && PlayerInst != NULL && !bPreview && !bJump )
	{
		PlayerInst->BlendPlayerToMark( NewPosition );
	}
}

/** DISHONORED(port): agent FK, 2013 rva 0x5027f0 (2012 0x541960, ratio 0.968 by vtable). */
void UInterpGroupInstPlayer::InitGroupInst( UInterpGroup* InGroup, AActor* InGroupActor )
{
	Group = InGroup;
	GroupActor = InGroupActor;
	// retail stores the argument straight into m_pPlayerGroup with no cast: InitInterp only builds this
	// class for a group that already is a UInterpGroupPlayer
	m_pPlayerGroup = Cast<UInterpGroupPlayer>( InGroup );

	AActor* Notify = NULL;
	if( GIsGame && InGroupActor != NULL )
	{
		Notify = InGroupActor;
		m_StartPos = InGroupActor->Location;
		// retail reads APawn::Controller (+620) off the group actor unchecked: for a player group the
		// group actor is the player's pawn and the start rotation it wants is the *view* rotation
		APawn* Pawn = Cast<APawn>( InGroupActor );
		if( Pawn != NULL && Pawn->Controller != NULL )
		{
			m_StartRot = Pawn->Controller->Rotation;
		}
		else
		{
			m_StartRot = InGroupActor->Rotation;
		}
	}
	m_bFinishedBlendIn = FALSE;

	Super::InitGroupInst( InGroup, InGroupActor );

	if( Notify != NULL )
	{
		// retail: Pawn->MAT_BeginPlayerGroup( m_pPlayerGroup, this ) (vtable slot 295)
		DisFKWarnOnce( TEXT("ADishonoredPlayerPawn::MAT_BeginPlayerGroup") );
	}

	if( GIsGame )
	{
		AActor* Actor = GetGroupActor();
		USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GetOuter() );
		if( Actor != NULL && Seq != NULL )
		{
			// the matinee is a latent action on the player for as long as the group lives, both ways round
			Seq->LatentActors.AddUniqueItem( Actor );
			Actor->LatentActions.AddUniqueItem( Seq );

			if( GroupActor != NULL )
			{
				// retail: GroupActor->MAT_SetForceInventory( Seq->m_bForcesUndrawBeforePlaying ) (slot 306)
				DisFKWarnOnce( TEXT("ADishonoredPlayerPawn::MAT_SetForceInventory") );
			}
		}
	}
}

/** DISHONORED(port): agent FK, 2013 rva 0x4fe910 (2012 0x53dd50). */
void UInterpGroupInstPlayer::TermGroupInst( UBOOL bDeleteTrackInst )
{
	AActor* Actor = NULL;
	if( GIsGame )
	{
		Actor = GroupActor;
		if( Actor != NULL && !( Actor->m_bLegalToTerminate && Actor->m_bHasTerminated ) )
		{
			// retail: Actor->MAT_BlendOut( this ) (ADishonoredPlayerPawn 2012 0x705120, slot 301)
			DisFKWarnOnce( TEXT("ADishonoredPlayerPawn::MAT_BlendOut") );
		}
	}

	Super::TermGroupInst( bDeleteTrackInst );

	if( Actor == NULL || m_pPlayerGroup == NULL )
	{
		return;
	}

	// retail: Actor->MAT_FinishPlayerGroup( m_pPlayerGroup, this ) (2012 0x7051f0, slot 296)
	DisFKWarnOnce( TEXT("ADishonoredPlayerPawn::MAT_FinishPlayerGroup") );

	if( GIsGame && Actor->m_bLegalToTerminate && Actor->m_bHasTerminated )
	{
		return;
	}
	if( !m_pPlayerGroup->m_bSnapToRootBoneLocationWhenFinished && !m_pPlayerGroup->m_bSnapToRootBoneRotationWhenFinished )
	{
		return;
	}

	APawn* Pawn = Cast<APawn>( Actor );
	USkeletalMeshComponent* Mesh = ( Pawn != NULL ) ? Pawn->Mesh : NULL;
	if( Mesh == NULL )
	{
		return;
	}
	const FMatrix RootTM = Mesh->GetBoneMatrix( 0 );

	if( m_pPlayerGroup->m_bSnapToRootBoneLocationWhenFinished )
	{
		FVector NewLoc = RootTM.GetOrigin();
		if( Pawn->CylinderComponent != NULL )
		{
			// the root bone is on the floor and the pawn's own location is its centre
			NewLoc.Z += Pawn->CylinderComponent->CollisionHeight;
		}
		Actor->SetLocation( NewLoc );
	}

	if( m_pPlayerGroup->m_bSnapToRootBoneRotationWhenFinished )
	{
		// retail's literal is FRotator( -16384, 0, 0 ): the root bone points up, the pawn points forward
		const FMatrix OffsetMat = FRotationTranslationMatrix( FRotator( -16384, 0, 0 ), FVector::ZeroVector );
		FRotator NewRot = ( OffsetMat * RootTM ).Rotator();
		NewRot.Pitch = 0;
		NewRot.Roll = 0;
		if( Pawn->Controller != NULL )
		{
			Pawn->Controller->SetRotation( NewRot );
		}
		Actor->SetRotation( NewRot );
	}
}

/** DISHONORED(port): agent FK, 2013 rva 0x4fc500 (2012 0x53b6c0). */
UBOOL UInterpGroupInstPlayer::GetStageMarkPosition( FVector& OutPos, FRotator* OutRot )
{
	USeqAct_Interp* Seq = Cast<USeqAct_Interp>( GetOuter() );
	if( Seq == NULL )
	{
		return FALSE;
	}

	AActor* MarkActor = NULL;
	if( GIsGame )
	{
		if( m_pPlayerGroup == NULL )
		{
			return FALSE;
		}
		UInterpGroupInst* MarkInst = Seq->FindFirstGroupInstByName( m_pPlayerGroup->m_StageMarkGroup );
		if( MarkInst == NULL )
		{
			return FALSE;
		}
		MarkActor = MarkInst->GetGroupActor();
	}
	else if( GIsEditor )
	{
		MarkActor = m_StageMarkActor;
	}
	if( MarkActor == NULL )
	{
		return FALSE;
	}

	if( OutRot != NULL )
	{
		OutRot->Pitch = MarkActor->Rotation.Pitch;
		OutRot->Yaw = MarkActor->Rotation.Yaw;
		// retail drops the mark's roll in game and keeps it in the editor; the editor branch is not reachable here
		OutRot->Roll = GIsGame ? 0 : MarkActor->Rotation.Roll;
	}

	FCheckResult Hit;
	const FVector Start = MarkActor->Location;
	const FVector End( Start.X, Start.Y, Start.Z - 100.f );
	if( ( m_pPlayerGroup != NULL && m_pPlayerGroup->m_bDisablePlayerCollision )
		|| GWorld->SingleLineCheck( Hit, MarkActor, End, Start, TRACE_World, FVector( 0.f, 0.f, 0.f ), NULL ) )
	{
		OutPos = Start;
	}
	else
	{
		OutPos = Hit.Location;
	}
	return TRUE;
}

/** DISHONORED(port): agent FK, 2013 rva 0x4fd880 (2012 0x53c5c0). */
void UInterpGroupInstPlayer::BlendPlayerToMark( FLOAT TimeSliderPos )
{
	AActor* Player = GIsGame ? GroupActor : NULL;
	if( m_bFinishedBlendIn || m_pPlayerGroup == NULL )
	{
		return;
	}

	FVector TargetPos( 0.f, 0.f, 0.f );
	FRotator MarkRot( 0, 0, 0 );
	if( !GetStageMarkPosition( TargetPos, &MarkRot ) )
	{
		m_bFinishedBlendIn = TRUE;
		return;
	}

	APawn* Pawn = GIsGame ? Cast<APawn>( GroupActor ) : NULL;
	if( Pawn != NULL && Pawn->CylinderComponent != NULL )
	{
		// the mark is on the floor; the player's location is his centre
		TargetPos.Z += Pawn->CylinderComponent->CollisionHeight;
	}

	FVector NewPos = TargetPos;
	FRotator NewRot = MarkRot;
	const FLOAT BlendTime = m_pPlayerGroup->m_fTimeToBlendToStageMark;
	if( BlendTime <= SMALL_NUMBER )
	{
		m_bFinishedBlendIn = TRUE;
	}
	else
	{
		const FLOAT Weight = Clamp( TimeSliderPos / BlendTime, 0.f, 1.f );
		NewPos = m_StartPos + ( TargetPos - m_StartPos ) * Weight;
		const FQuat Blended = SlerpQuat( m_StartRot.Quaternion(), MarkRot.Quaternion(), Weight );
		NewRot = FQuatRotationTranslationMatrix( Blended, FVector( 0.f, 0.f, 0.f ) ).Rotator();
		if( TimeSliderPos > BlendTime )
		{
			m_bFinishedBlendIn = TRUE;
		}
	}

	if( Player == NULL )
	{
		return;
	}
	const FVector WasAt = Player->Location;
	GWorld->FarMoveActor( Player, NewPos, FALSE, m_pPlayerGroup->m_bDisablePlayerCollision ? TRUE : FALSE, TRUE );

	// the proof that the blend moved the player, and the one number that distinguishes a player group
	// that is instanced from one that is not: logged on the first move of each instance only
	if( DisFKDiagEnabled() && DisFKFirstMoveFor( this ) )
	{
		debugf( TEXT("DISHONORED(bringup): fkdiag blend %s: %s moved %.1f units, %.0f/%.0f/%.0f -> %.0f/%.0f/%.0f, mark %.0f/%.0f/%.0f, blendTime %.2f t %.2f, rot %d/%d/%d"),
			*GetOuter()->GetName(), *Player->GetName(), ( Player->Location - WasAt ).Size(),
			WasAt.X, WasAt.Y, WasAt.Z, Player->Location.X, Player->Location.Y, Player->Location.Z,
			TargetPos.X, TargetPos.Y, TargetPos.Z, BlendTime, TimeSliderPos,
			NewRot.Pitch, NewRot.Yaw, NewRot.Roll );
	}

	// the controller takes the whole rotation (it is the view), the pawn takes it without the pitch
	if( Pawn != NULL && Pawn->Controller != NULL )
	{
		Pawn->Controller->SetRotation( NewRot );
	}
	FRotator PawnRot = NewRot;
	PawnRot.Pitch = 0;
	Player->SetRotation( PawnRot );
}

/*-----------------------------------------------------------------------------
	The InitInterp seam.

	Retail's USeqAct_Interp::InitInterp (2012 0x25ad70, returns UBOOL) has four branches -
	Director, Player, AI, plain - and this tree's has Director, AI, Camera, plain. The Player branch
	cannot be written in Engine/Src/UnInterpolation.cpp because UInterpGroupInstPlayer is declared in
	this module, so it goes behind the same function-pointer seam shape GDisEngineTickHook and
	GDisAudioFrameHook already use. UnInterpolation.cpp calls the hook first and falls through to its
	own branches when it returns FALSE, which is exactly retail's order.
-----------------------------------------------------------------------------*/

static UBOOL DisFKInitPlayerGroup( USeqAct_Interp* Seq, UInterpGroup* Group )
{
	UInterpGroupPlayer* PlayerGroup = Cast<UInterpGroupPlayer>( Group );
	if( Seq == NULL || PlayerGroup == NULL )
	{
		return FALSE;
	}

	// retail: GWorld->GetWorldInfo()->GetALocalPlayerController()->Pawn, and NULL out of game
	APawn* PlayerPawn = NULL;
	if( GIsGame && GWorld != NULL && GWorld->GetWorldInfo() != NULL )
	{
		APlayerController* PC = GWorld->GetWorldInfo()->GetALocalPlayerController();
		if( PC != NULL )
		{
			PlayerPawn = PC->Pawn;
		}
	}

	UInterpGroupInstPlayer* NewInst = ConstructObject<UInterpGroupInstPlayer>(
		UInterpGroupInstPlayer::StaticClass(), Seq, NAME_None, RF_Transactional );
	Seq->GroupInst.AddItem( NewInst );
	NewInst->InitGroupInst( Group, PlayerPawn );

	debugf( TEXT("DISHONORED(bringup): matinee %s: player group '%s' instanced on %s"),
		*Seq->GetName(), *Group->GroupName.ToString(),
		( PlayerPawn != NULL ) ? *PlayerPawn->GetName() : TEXT("no player pawn") );
	return TRUE;
}

void DisFKInstallInterpHooks()
{
	// -fknoplayergroup is the control for this package's own before/after: it leaves the hook NULL, so
	// one binary gives both readings and the comparison cannot be a different build of something else.
	if( ParseParam( appCmdLine(), TEXT("fknoplayergroup") ) )
	{
		debugf( TEXT("DISHONORED(bringup): -fknoplayergroup: the InitInterp player-group branch is off; a player group falls through to a plain UInterpGroupInst") );
		return;
	}
	GDisInterpPlayerGroupHook = &DisFKInitPlayerGroup;
}
