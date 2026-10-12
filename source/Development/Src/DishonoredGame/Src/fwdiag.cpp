/*=============================================================================
	fwdiag.cpp: agent FW (PHASE18 FW) - the player's walk to the Empress, measured.

	  fwdiag player     once a second: where the player pawn is and which way the controller looks, so a game_input.py walk
	                    (W held, yaw by mouse deltas) can be read as a track and corrected
	  fwdialogmap       once per map: the scene layer as data. For every DisDialogOneShot actor, its tree's hook nodes and
	                    the graph behind each hook output - conversations (blocking / player-must-be-near / fire-once),
	                    soiree nodes with the matinee their GUID names, Kismet remote events the conversation raises -
	                    which is the map from "the Kismet input 'X' on one-shot Y" (or "the player within R of Y") to the
	                    cutscene that plays. FO's -fosoiree gives node -> matinee; this gives hook -> node.
	Nothing here changes behaviour.
=============================================================================*/
#include "DishonoredGame.h"
#include "fwdiag.h"

static INT GDisFWDiag = -1;

UBOOL DisFWDiagEnabled()
{
	if( GDisFWDiag < 0 )
	{
		GDisFWDiag = ParseParam( appCmdLine(), TEXT("fwdiag") ) ? 1 : 0;
	}
	return GDisFWDiag != 0;
}

void DisFWReport( APlayerController* PC )
{
	static FLOAT NextAt = 0.f;
	if( !DisFWDiagEnabled() || GWorld == NULL || PC == NULL )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < NextAt )
	{
		return;
	}
	NextAt = Now + 1.f;
	APawn* Pawn = PC->Pawn;
	if( Pawn == NULL )
	{
		debugf( TEXT("DISHONORED(bringup): fwdiag player t=%.2f no pawn; controller at %s yaw %d"),
			Now, *PC->Location.ToString(), PC->Rotation.Yaw & 0xffff );
		return;
	}
	debugf( TEXT("DISHONORED(bringup): fwdiag player t=%.2f %s at %s pawnYaw %d viewYaw %d viewPitch %d speed %.0f physics %d base %s volume %s touching %d"),
		Now, *Pawn->GetName(), *Pawn->Location.ToString(), Pawn->Rotation.Yaw & 0xffff, PC->Rotation.Yaw & 0xffff, PC->Rotation.Pitch & 0xffff,
		Pawn->Velocity.Size(), (INT)Pawn->Physics, Pawn->Base ? *Pawn->Base->GetName() : TEXT("none"),
		Pawn->PhysicsVolume ? *Pawn->PhysicsVolume->GetName() : TEXT("none"), Pawn->Touching.Num() );

	// what the pawn's own cylinder would hit 200 units ahead along the view yaw: the thing that stops a walk, by name
	FRotator Heading( 0, PC->Rotation.Yaw, 0 );
	const FVector Start = Pawn->Location;
	const FVector End = Start + Heading.Vector() * 200.f;
	const FVector Extent = Pawn->CylinderComponent ? FVector( Pawn->CylinderComponent->CollisionRadius, Pawn->CylinderComponent->CollisionRadius, Pawn->CylinderComponent->CollisionHeight ) : FVector( 34.f, 34.f, 72.f );
	FCheckResult Hit( 1.f );
	GWorld->SingleLineCheck( Hit, Pawn, End, Start, TRACE_AllBlocking, Extent );
	if( Hit.Actor != NULL )
	{
		debugf( TEXT("DISHONORED(bringup): fwdiag ahead t=%.2f blocked at %.0f of 200 by %s (%s) component %s (%s) hit %s normal %s hidden %d collideActors %d blockActors %d collisionType %d"),
			Now, Hit.Time * 200.f, *Hit.Actor->GetName(), *Hit.Actor->GetClass()->GetName(),
			Hit.Component ? *Hit.Component->GetName() : TEXT("none"), Hit.Component ? *Hit.Component->GetClass()->GetName() : TEXT(""),
			*Hit.Location.ToString(), *Hit.Normal.ToString(), (INT)Hit.Actor->bHidden, (INT)Hit.Actor->bCollideActors, (INT)Hit.Actor->bBlockActors, (INT)Hit.Actor->CollisionType );
	}

	// -fwteleport=<actor>@<seconds>[,...]: at that world time the pawn is placed on the named actor's collision bounds
	// (UWorld::FarMoveActor, the same path a walk-in takes through FindTouchingActors - EM's -distouchprobe, which arms once
	// per process and so is spent on the menu world of a new-game route). The measurement is what the Kismet behind a
	// trigger does once the player is in it, separated from how the player got there.
	static TArray<FString> Teleports;
	static INT TeleportsParsed = -1;
	if( TeleportsParsed < 0 )
	{
		TeleportsParsed = 0;
		FString Value;
		if( Parse( appCmdLine(), TEXT("fwteleport="), Value, FALSE ) )
		{
			Value.ParseIntoArray( &Teleports, TEXT(","), TRUE );
		}
	}
	for( INT i = 0; i < Teleports.Num(); i++ )
	{
		const INT At = Teleports(i).InStr( TEXT("@") );
		if( At == INDEX_NONE || appAtof( *Teleports(i).Mid( At + 1 ) ) > Now )
		{
			continue;
		}
		const FString Name = Teleports(i).Left( At );
		Teleports.Remove( i-- );
		AActor* Target = NULL;
		for( FActorIterator It; It; ++It )
		{
			if( It->GetName() == Name )
			{
				Target = *It;
				break;
			}
		}
		if( Target == NULL )
		{
			debugf( TEXT("DISHONORED(bringup): fwdiag teleport t=%.2f: no actor named %s"), Now, *Name );
			continue;
		}
		const FVector Dest = Target->CollisionComponent ? Target->CollisionComponent->Bounds.Origin : Target->Location;
		const UBOOL bMoved = GWorld->FarMoveActor( Pawn, Dest, FALSE, TRUE, FALSE );
		debugf( TEXT("DISHONORED(bringup): fwdiag teleport t=%.2f -> %s (%s) at %s: moved %d, pawn now at %s, touching %d, touching it %d"),
			Now, *Target->GetName(), *Target->GetClass()->GetName(), *Dest.ToString(), (INT)bMoved, *Pawn->Location.ToString(), Pawn->Touching.Num(), (INT)Pawn->Touching.ContainsItem( Target ) );
	}

	// -fwnear: every five seconds, the colliding actors within 600 units, so a stop can be read against what is there
	static FLOAT NearAt = 0.f;
	if( ParseParam( appCmdLine(), TEXT("fwnear") ) && Now >= NearAt )
	{
		NearAt = Now + 5.f;
		INT Listed = 0;
		for( FActorIterator It; It && Listed < 30; ++It )
		{
			AActor* Actor = *It;
			if( Actor == Pawn || !Actor->bCollideActors || Actor->IsA( ADefaultPhysicsVolume::StaticClass() ) )
			{
				continue;
			}
			const FLOAT Dist = ( Actor->Location - Pawn->Location ).Size();
			if( Dist > 600.f )
			{
				continue;
			}
			Listed++;
			debugf( TEXT("DISHONORED(bringup): fwdiag near t=%.2f %s (%s) at %s dist %.0f hidden %d blockActors %d collisionType %d collision %s"),
				Now, *Actor->GetName(), *Actor->GetClass()->GetName(), *Actor->Location.ToString(), Dist, (INT)Actor->bHidden, (INT)Actor->bBlockActors,
				(INT)Actor->CollisionType, Actor->CollisionComponent ? *Actor->CollisionComponent->GetClass()->GetName() : TEXT("none") );
		}
	}
}

static USeqAct_Interp* DisFWMatineeOf( const UDisConv_Soiree* Node )
{
	const FGuid& Guid = Node->m_SoireeGUID;
	if( Guid.A == 0 && Guid.B == 0 && Guid.C == 0 && Guid.D == 0 )
	{
		return NULL;
	}
	for( TObjectIterator<USeqAct_Interp> It; It; ++It )
	{
		if( !It->IsTemplate() && It->m_MatineeGUID == Guid )
		{
			return *It;
		}
	}
	return NULL;
}

static FString DisFWShortPath( UObject* Object )
{
	FString Path = Object->GetPathName();
	const INT Dot = Path.InStr( TEXT("PersistentLevel."), FALSE, TRUE );
	return Dot != INDEX_NONE ? Path.Mid( Dot + 16 ) : Path;
}

static FString DisFWDescribeNode( UDisConv_Node* Node )
{
	FString Desc = FString::Printf( TEXT("%s %s"), *Node->GetClass()->GetName().Mid( 8 ), *Node->GetName() );
	if( Node->m_Label != NAME_None )
	{
		Desc += FString::Printf( TEXT(" label '%s'"), *Node->m_Label.ToString() );
	}
	if( UDisConversation* Conv = Cast<UDisConversation>( Node ) )
	{
		Desc += FString::Printf( TEXT(" [conv blocking %d playerNear %d r %.0f once %d priority %d speakers %d nodes %d]"),
			(INT)Conv->m_bIsBlocking, (INT)Conv->m_bPlayerMustBeNear, Conv->m_fPlayerNearRadius, (INT)Conv->m_bFireConvOnlyOnce,
			(INT)Conv->m_Priority, Conv->m_Speakers.Num(), Conv->m_Nodes.Num() );
	}
	if( UDisConv_Soiree* Soiree = Cast<UDisConv_Soiree>( Node ) )
	{
		USeqAct_Interp* Matinee = DisFWMatineeOf( Soiree );
		Desc += FString::Printf( TEXT(" ==> MATINEE %s (activations %d)"), Matinee ? *DisFWShortPath( Matinee ) : TEXT("NONE"), Matinee ? Matinee->ActivateCount : -1 );
	}
	if( UDisConv_KismetActivateRemoteEvent* Remote = Cast<UDisConv_KismetActivateRemoteEvent>( Node ) )
	{
		Desc += FString::Printf( TEXT(" ==> KISMET DialogOutputs '%s'"), *Remote->m_EventName.ToString() );
	}
	return Desc;
}

static void DisFWWalk( UDisConv_Node* Node, INT Depth, const FString& Via, TArray<UDisConv_Node*>& Visited, INT& Lines )
{
	if( Node == NULL || Depth > 40 || Lines > 400 )
	{
		return;
	}
	const UBOOL bSeen = Visited.ContainsItem( Node );
	FString Indent;
	for( INT i = 0; i < Depth; i++ )
	{
		Indent += TEXT("  ");
	}
	debugf( TEXT("DISHONORED(bringup): fwdialogmap %s%s-> %s%s"), *Indent, *Via, *DisFWDescribeNode( Node ), bSeen ? TEXT(" [seen]") : TEXT("") );
	Lines++;
	if( bSeen )
	{
		return;
	}
	Visited.AddItem( Node );
	for( INT i = 0; i < Node->m_OutputLinks.Num(); i++ )
	{
		const FDisConvNodeLink& Link = Node->m_OutputLinks(i);
		if( Link.m_pLink == NULL || Cast<UDisDialogTree>( Link.m_pLink ) != NULL )
		{
			continue;
		}
		DisFWWalk( Link.m_pLink, Depth + 1, FString::Printf( TEXT("out %d '%s' "), i, *Link.m_Description ), Visited, Lines );
	}
}

static FString DisFWDescribeHook( UDisConv_DialogHook* Hook )
{
	FString Desc = FString::Printf( TEXT("%s %s hookEnum %d index %d priority %d outputs %d"), *Hook->GetClass()->GetName(), *Hook->GetName(),
		(INT)Hook->m_HookEnum, Hook->m_iHookIndex, (INT)Hook->m_Priority, Hook->m_OutputLinks.Num() );
	if( UDisConv_Hook_Distance* Distance = Cast<UDisConv_Hook_Distance>( Hook ) )
	{
		Desc += FString::Printf( TEXT(" close %.0f medium %.0f far %.0f"), Distance->m_fCloseDistance, Distance->m_fMediumDistance, Distance->m_fFarDistance );
	}
	if( UDisConv_Hook_PlayerLookAt* LookAt = Cast<UDisConv_Hook_PlayerLookAt>( Hook ) )
	{
		Desc += FString::Printf( TEXT(" lookAtRadius %.0f targets %d"), LookAt->m_fLookAtRadius, LookAt->m_LookAtTargets.Num() );
	}
	if( UDisConv_Hook_PlayerLoiter* Loiter = Cast<UDisConv_Hook_PlayerLoiter>( Hook ) )
	{
		Desc += FString::Printf( TEXT(" loiterRadius %.0f time %.1f..%.1f"), Loiter->m_fLoiterRadius, Loiter->m_fLoiterTime_Min, Loiter->m_fLoiterTime_Max );
	}
	return Desc;
}

void DisFWReportDialogMap()
{
	static FString LastMap;
	if( !ParseParam( appCmdLine(), TEXT("fwdialogmap") ) || GWorld == NULL )
	{
		return;
	}
	INT OneShots = 0;
	for( TObjectIterator<ADisDialogOneShot> It; It; ++It )
	{
		OneShots += It->IsTemplate() ? 0 : 1;
	}
	const FString Map = GWorld->GetMapName();
	if( Map == LastMap || OneShots == 0 )
	{
		return;
	}
	LastMap = Map;
	INT Trees = 0, Hooks = 0, Lines = 0;
	for( TObjectIterator<ADisDialogOneShot> It; It; ++It )
	{
		ADisDialogOneShot* OneShot = *It;
		UDisDialogTree* Tree = OneShot->IsTemplate() ? NULL : OneShot->m_pDialogTree;
		if( Tree == NULL )
		{
			continue;
		}
		Trees++;
		debugf( TEXT("DISHONORED(bringup): fwdialogmap oneshot %s at %s tree %s: %d hook(s), %d conversation(s), bound %s"),
			*OneShot->GetName(), *OneShot->Location.ToString(), *Tree->GetName(), Tree->m_DialogHooks.Num(), Tree->m_Conversations.Num(),
			Tree->m_pBoundToOneShot ? *Tree->m_pBoundToOneShot->GetName() : TEXT("none") );
		TArray<UDisConv_Node*> Visited;
		for( INT HookIndex = 0; HookIndex < Tree->m_DialogHooks.Num(); HookIndex++ )
		{
			UDisConv_DialogHook* Hook = Tree->m_DialogHooks( HookIndex );
			if( Hook == NULL )
			{
				continue;
			}
			Hooks++;
			debugf( TEXT("DISHONORED(bringup): fwdialogmap  hook %d: %s"), HookIndex, *DisFWDescribeHook( Hook ) );
			UDisConv_Hook_KismetActivated* Kismet = Cast<UDisConv_Hook_KismetActivated>( Hook );
			for( INT i = 0; i < Hook->m_OutputLinks.Num(); i++ )
			{
				const FDisConvNodeLink& Link = Hook->m_OutputLinks(i);
				FString Via = Kismet != NULL && i < Kismet->m_KismetDialogHooks.Num()
					? FString::Printf( TEXT("kismet input '%s' "), *Kismet->m_KismetDialogHooks(i).ToString() )
					: FString::Printf( TEXT("out %d '%s' "), i, *Link.m_Description );
				if( Link.m_pLink == NULL )
				{
					debugf( TEXT("DISHONORED(bringup): fwdialogmap   %s-> (unlinked)"), *Via );
					continue;
				}
				DisFWWalk( Link.m_pLink, 2, Via, Visited, Lines );
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): fwdialogmap: map %s, %d one-shot tree(s), %d hook(s), %d node line(s)"), *Map, Trees, Hooks, Lines );
}
