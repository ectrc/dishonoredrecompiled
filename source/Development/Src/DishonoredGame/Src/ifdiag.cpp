/*=============================================================================
	ifdiag.cpp: agent IF (PHASE23 IF) - the -ifdiag measurements of the player's crouch, sprint, jump and swim (ifdiag.h).
	Nothing here changes behaviour except the IFWATER console command, which exists only under -ifdiag: it moves the
	player the way -distouchprobe does (UWorld::FarMoveActor), to put him in the water the brief asks to walk into -
	the dock route has no walkable way down to the water and this build has no cheat manager (no BugItGo).
=============================================================================*/
#include "DishonoredGame.h"
#include "ifdiag.h"

static INT GDisIFDiag = -1;

UBOOL DisIFDiagEnabled()
{
	if( GDisIFDiag < 0 )
	{
		GDisIFDiag = ParseParam( appCmdLine(), TEXT("ifdiag") ) ? 1 : 0;
	}
	return GDisIFDiag != 0;
}

static const TCHAR* DisIFPhysicsName( BYTE Physics )
{
	static const TCHAR* Names[] = { TEXT("None"), TEXT("Walking"), TEXT("Falling"), TEXT("Swimming"), TEXT("Flying"), TEXT("Rotating"),
		TEXT("Projectile"), TEXT("Interpolating"), TEXT("Spider"), TEXT("Ladder"), TEXT("RigidBody"), TEXT("SoftBody"), TEXT("NavMeshWalking"),
		TEXT("Unused"), TEXT("Custom") };
	return Physics < ARRAY_COUNT(Names) ? Names[Physics] : TEXT("?");
}

static FString DisIFStateName( const UObject* State )
{
	return State ? State->GetClass()->GetName() : FString( TEXT("none") );
}

void DisIFReportPlayer( ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime )
{
	static FLOAT Accum = 0.f;
	static BYTE LastPhysics = 255;
	if( !DisIFDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	if( Pawn->Physics != LastPhysics )
	{
		debugf( TEXT("DISHONORED(bringup): ifdiag water physics %s -> %s at %s, PhysicsVolume %s (water %d), active water volume %s, volumes %d"),
			LastPhysics == 255 ? TEXT("-") : DisIFPhysicsName( LastPhysics ), DisIFPhysicsName( Pawn->Physics ), *Pawn->Location.ToString(),
			Pawn->PhysicsVolume ? *Pawn->PhysicsVolume->GetName() : TEXT("none"), Pawn->PhysicsVolume ? (INT)Pawn->PhysicsVolume->bWaterVolume : 0,
			Pawn->m_pActiveWaterVolume ? *Pawn->m_pActiveWaterVolume->GetName() : TEXT("none"), Pawn->m_CurrentWaterVolumes.Num() );
		LastPhysics = Pawn->Physics;
	}
	Accum += DeltaTime;
	if( Accum < 0.25f )
	{
		return;
	}
	Accum = 0.f;
	UDishonoredNativeStateMachine* Master = Pawn->GetPlayerMasterFSM();
	const FLOAT Height = Pawn->CylinderComponent ? Pawn->CylinderComponent->CollisionHeight : 0.f;
	debugf( TEXT("DISHONORED(bringup): ifdiag player at %s vel %s speed2D %.1f accel %.1f physics %s height %.2f camera above feet %.2f eye %.2f/%.2f crouched %d sneaking %d sprinting %d walking %d master %s GroundSpeed %.1f WaterSpeed %.1f mod %.3f"),
		*Pawn->Location.ToString(), *Pawn->Velocity.ToString(), Pawn->Velocity.Size2D(), Pawn->Acceleration.Size(), DisIFPhysicsName( Pawn->Physics ),
		Height, Pawn->GetCameraPos().Z - ( Pawn->Location.Z - Height ), Pawn->EyeHeight, Pawn->BaseEyeHeight, (INT)Pawn->bIsCrouched,
		(INT)Pawn->IsSneaking(), (INT)Pawn->m_bSprinting, (INT)Pawn->bIsWalking,
		*DisIFStateName( Master ? Master->GetCurrentlyActiveState() : NULL ), Pawn->GroundSpeed, Pawn->WaterSpeed, Pawn->m_fLastSpeedModifier );
}

void DisIFNoteSpeedModifier( ADishonoredPawn* Pawn, FLOAT EngineModifier, FLOAT DerivedModifier, FLOAT Result )
{
	if( !DisIFDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	static TArray<ADishonoredPawn*> Seen;
	static FLOAT LastPlayerResult = -1.f;
	const UBOOL bPlayer = Pawn->IsA( ADishonoredPlayerPawn::StaticClass() );
	if( bPlayer ? Abs( Result - LastPlayerResult ) < 0.001f : Seen.ContainsItem( Pawn ) )
	{
		return;
	}
	if( bPlayer )
	{
		LastPlayerResult = Result;
	}
	else
	{
		Seen.AddItem( Pawn );
	}
	debugf( TEXT("DISHONORED(bringup): ifdiag speed %s: crouched %d sprinting %d sneaking %d strafing %d backwards %d, engine/sprint modifier %.3f x derived %.3f -> %.3f (GroundSpeed %.1f, max speed %.1f, CrouchedPct %.3f WalkingPct %.3f DesiredSpeed %.3f)"),
		*Pawn->GetName(), (INT)Pawn->bIsCrouched, (INT)Pawn->m_bSprinting, (INT)Pawn->IsSneaking(), (INT)Pawn->m_bStrafing, (INT)Pawn->m_bMovingBackwards,
		EngineModifier, DerivedModifier, Result, Pawn->GroundSpeed, Pawn->GroundSpeed * Result, Pawn->CrouchedPct, Pawn->WalkingPct, Pawn->DesiredSpeed );
}

static UBOOL DisIFIsPlayerMaster( UDishonoredNativeStateMachine* StateMachine )
{
	ADishonoredPlayerPawn* Pawn = Cast<ADishonoredPlayerPawn>( StateMachine ? StateMachine->GetManagedObject() : NULL );
	return Pawn != NULL && Pawn->GetPlayerMasterFSM() == StateMachine;
}

void DisIFNoteTransition( UDishonoredNativeStateMachine* StateMachine, UDishonoredNativeState* Last, UDishonoredNativeState* Next )
{
	if( DisIFDiagEnabled() && Next != NULL && DisIFIsPlayerMaster( StateMachine ) )
	{
		APawn* Pawn = Cast<APawn>( StateMachine->GetManagedObject() );
		debugf( TEXT("DISHONORED(bringup): ifdiag fsm %s -> %s (physics %s, Z %.1f)"), *DisIFStateName( Last ), *DisIFStateName( Next ),
			DisIFPhysicsName( Pawn->Physics ), Pawn->Location.Z );
	}
}

void DisIFNoteRejection( UDishonoredNativeStateMachine* StateMachine, const UClass* StateID, UDishonoredNativeState* RejectingState )
{
	if( DisIFDiagEnabled() && DisIFIsPlayerMaster( StateMachine ) )
	{
		debugf( TEXT("DISHONORED(bringup): ifdiag fsm request %s refused by %s"), StateID ? *StateID->GetName() : TEXT("none"),
			*DisIFStateName( RejectingState ) );
	}
}

UBOOL DisIFDiagExec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( !DisIFDiagEnabled() || !ParseCommand( &Cmd, TEXT("IFWATER") ) )
	{
		return FALSE;
	}
	static FVector DryLocation( 0.f, 0.f, 0.f );
	static UBOOL bHaveDry = FALSE;
	ADishonoredPlayerPawn* Pawn = ADishonoredPlayerPawn::s_pInstance;
	if( Pawn == NULL || GWorld == NULL )
	{
		Ar.Logf( TEXT("IFWATER: no player") );
		return TRUE;
	}
	if( ParseCommand( &Cmd, TEXT("EXIT") ) )
	{
		const UBOOL bMoved = bHaveDry && GWorld->FarMoveActor( Pawn, DryLocation, FALSE, FALSE, FALSE );
		debugf( TEXT("DISHONORED(bringup): ifdiag water IFWATER EXIT -> %s: moved %d, pawn at %s, physics %s"), *DryLocation.ToString(), (INT)bMoved,
			*Pawn->Location.ToString(), DisIFPhysicsName( Pawn->Physics ) );
		return TRUE;
	}
	ADishonoredWaterVolume* Best = NULL;
	FLOAT BestDist = BIG_NUMBER;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredWaterVolume* Volume = Cast<ADishonoredWaterVolume>( *It );
		if( Volume && Volume->CollisionComponent )
		{
			const FLOAT Dist = ( Volume->CollisionComponent->Bounds.Origin - Pawn->Location ).Size2D();
			if( Dist < BestDist )
			{
				BestDist = Dist;
				Best = Volume;
			}
		}
	}
	if( Best == NULL )
	{
		Ar.Logf( TEXT("IFWATER: no DishonoredWaterVolume") );
		return TRUE;
	}
	DryLocation = Pawn->Location;
	bHaveDry = TRUE;
	const FBoxSphereBounds& Bounds = Best->CollisionComponent->Bounds;
	const FLOAT SurfaceZ = Best->GetSurfaceZ();
	const FVector Center( Bounds.Origin.X, Bounds.Origin.Y, 0.f );
	FVector Probe( Clamp( Pawn->Location.X, Bounds.Origin.X - Bounds.BoxExtent.X, Bounds.Origin.X + Bounds.BoxExtent.X ),
		Clamp( Pawn->Location.Y, Bounds.Origin.Y - Bounds.BoxExtent.Y, Bounds.Origin.Y + Bounds.BoxExtent.Y ), 0.f );
	const FVector Step = ( Center - Probe ).SafeNormal() * 250.f;
	UBOOL bMoved = FALSE;
	INT Tries = 0;
	for( ; Tries < 40 && !bMoved; Tries++, Probe += Step )
	{
		if( Best->Encompasses( FVector( Probe.X, Probe.Y, SurfaceZ - 50.f ) ) )
		{
			bMoved = GWorld->FarMoveActor( Pawn, FVector( Probe.X, Probe.Y, SurfaceZ + Pawn->CylinderComponent->CollisionHeight + 40.f ), FALSE, FALSE, FALSE );
		}
	}
	debugf( TEXT("DISHONORED(bringup): ifdiag water IFWATER %s (surface Z %.1f, bounds %s +- %s): moved %d after %d tries, pawn at %s, PhysicsVolume %s, physics %s"),
		*Best->GetName(), SurfaceZ, *Bounds.Origin.ToString(), *Bounds.BoxExtent.ToString(), (INT)bMoved, Tries, *Pawn->Location.ToString(),
		Pawn->PhysicsVolume ? *Pawn->PhysicsVolume->GetName() : TEXT("none"), DisIFPhysicsName( Pawn->Physics ) );
	return TRUE;
}
