/*=============================================================================
	igdiag.cpp: agent IG (PHASE23 IG) - the -igdiag switch (Inc/igdiag.h).
	Nothing here changes behaviour except the IGLEDGE console command, which exists only under -igdiag: it moves the player
	the way -distouchprobe and IFWATER do (UWorld::FarMoveActor) in front of a ledge the step-up mantle can take.
=============================================================================*/
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "igdiag.h"

static INT GDisIGDiag = -1;

UBOOL DisIGDiagEnabled()
{
	if( GDisIGDiag < 0 )
	{
		GDisIGDiag = ParseParam( appCmdLine(), TEXT("igdiag") ) ? 1 : 0;
	}
	return GDisIGDiag != 0;
}

static const TCHAR* DisIGPhysicsName( BYTE Physics )
{
	static const TCHAR* Names[] = { TEXT("None"), TEXT("Walking"), TEXT("Falling"), TEXT("Swimming"), TEXT("Flying"), TEXT("Rotating"),
		TEXT("Projectile"), TEXT("Interpolating"), TEXT("Spider"), TEXT("Ladder"), TEXT("RigidBody"), TEXT("SoftBody"), TEXT("NavMeshWalking"),
		TEXT("Unused"), TEXT("Custom") };
	return Physics < ARRAY_COUNT(Names) ? Names[Physics] : TEXT("?");
}

static FString DisIGMasterState( ADishonoredPlayerPawn* Pawn )
{
	UDishonoredNativeStateMachine* Master = Pawn ? Pawn->GetPlayerMasterFSM() : NULL;
	UDishonoredNativeState* State = Master ? Master->GetCurrentlyActiveState() : NULL;
	return State ? State->GetClass()->GetName() : FString( TEXT("none") );
}

static FString DisIGSpot( UDishonoredMantleEdgeFinderComponent* Finder )
{
	if( Finder == NULL )
	{
		return FString( TEXT("no finder") );
	}
	const FDisMantleSpot& Spot = Finder->m_CurrentMantleSpot;
	return FString::Printf( TEXT("valid %d start %s yaw %d edge height %.2f impact %d crouch %d"), (INT)Finder->m_bHasValidMantleSpot,
		*Spot.m_StartLocation.ToString(), Spot.m_StartYaw, Spot.m_fEdgeHeight, (INT)Spot.m_bRequiresImpactAnim, (INT)Spot.m_bForcesCrouch );
}

void DisIGNoteFinderAttach( UDishonoredMantleEdgeFinderComponent* Finder )
{
	if( !DisIGDiagEnabled() || Finder == NULL )
	{
		return;
	}
	FString States;
	UDishonoredNativeStateMachine* FSM = Finder->m_pMantleFinderFSM;
	for( INT Index = 0; FSM && Index < FSM->m_NativeStates.Num(); Index++ )
	{
		States += FSM->m_NativeStates(Index) ? FSM->m_NativeStates(Index)->GetClass()->GetName() : FString( TEXT("null") );
		States += TEXT(" ");
	}
	UDisTweaks_PlayerPawn_Mantle* Tweaks = Finder->GetMantleTweaks();
	debugf( TEXT("DISHONORED(bringup): igdiag finder %s attached (owner %s): FSM %d states [%s] current %s; tweaks %s (%s): step %.1f min %.1f max %.1f low max %.1f step-up %d water auto %d forward %.1f search %.1f vert %.1f horiz %.1f slope %.1f grid %d budget %.3f ms blend %.2f no-fall %.2f node %s"),
		*Finder->GetName(), Finder->GetOwner() ? *Finder->GetOwner()->GetName() : TEXT("none"), FSM ? FSM->m_NativeStates.Num() : -1, *States,
		FSM && FSM->GetCurrentlyActiveState() ? *FSM->GetCurrentlyActiveState()->GetClass()->GetName() : TEXT("none"),
		*Tweaks->GetPathName(), Finder->m_pTweaks ? TEXT("m_pTweaks") : TEXT("m_pTweaks None: class default"),
		Tweaks->m_LineCheckStepSize, Tweaks->m_MantleMinEdgeHeight, Tweaks->m_MantleMaxEdgeHeight, Tweaks->m_LowMantleMaxEdgeHeight,
		(INT)Tweaks->m_bLowMantleUseStepUp, (INT)Tweaks->m_bAutomaticallyMantleInWater, Tweaks->m_fMantleForwardMoveAmount, Tweaks->m_MantleEdgeSearchDist,
		Tweaks->m_MaxVerticalAngleForEdgeFace, Tweaks->m_MaxHorizontalAngleForEdgeFace, Tweaks->m_MaxSlopeAngleForEdgeTop, Tweaks->m_FinalVerifyLocationGridSize,
		Tweaks->m_fMaxMantleCheckTimePerTick_ms, Tweaks->m_fLowMantleStepUpBlendTime, Tweaks->m_fLowMantleStepUpDisallowFallTime,
		*Tweaks->m_LowMantleStepUpAnimNodeName.ToString() );
}

struct FDisIGFinderRecord
{
	UDishonoredMantleEdgeFinderComponent* Finder;
	FString LastResult;
	INT Ticks;
	INT Steps;
	DOUBLE Milliseconds;
	FLOAT WorstMilliseconds;
	INT WorstSteps;
	INT Checks;
	DOUBLE NextReport;
};

static FDisIGFinderRecord& DisIGRecord( UDishonoredMantleEdgeFinderComponent* Finder )
{
	static TArray<FDisIGFinderRecord> Records;
	for( INT Index = 0; Index < Records.Num(); Index++ )
	{
		if( Records(Index).Finder == Finder )
		{
			return Records(Index);
		}
	}
	FDisIGFinderRecord* Record = new( Records ) FDisIGFinderRecord;
	Record->Finder = Finder;
	Record->Ticks = Record->Steps = Record->WorstSteps = Record->Checks = 0;
	Record->Milliseconds = 0.0;
	Record->WorstMilliseconds = 0.f;
	Record->NextReport = appSeconds() + 5.0;
	return *Record;
}

void DisIGNoteFinderResult( UDishonoredMantleEdgeFinderComponent* Finder, UBOOL bValid, const TCHAR* Reason )
{
	if( !DisIGDiagEnabled() || Finder == NULL )
	{
		return;
	}
	FDisIGFinderRecord& Record = DisIGRecord( Finder );
	Record.Checks++;
	const FDisMantleSpot& Spot = Finder->m_CurrentMantleSpot;
	const FString Result = bValid
		? FString::Printf( TEXT("spot start %s yaw %d edge height %.0f impact %d crouch %d"), *Spot.m_StartLocation.ToString(), Spot.m_StartYaw,
			Spot.m_fEdgeHeight, (INT)Spot.m_bRequiresImpactAnim, (INT)Spot.m_bForcesCrouch )
		: FString::Printf( TEXT("no spot: %s"), Reason );
	if( Result == Record.LastResult )
	{
		return;
	}
	Record.LastResult = Result;
	AActor* Owner = Finder->GetOwner();
	debugf( TEXT("DISHONORED(bringup): igdiag finder %s check %d: %s (edge height %.2f; owner at %s, physics %s)"), *Finder->GetName(), Record.Checks, *Result,
		bValid ? Spot.m_fEdgeHeight : 0.f, Owner ? *Owner->Location.ToString() : TEXT("-"), Owner ? DisIGPhysicsName( Owner->Physics ) : TEXT("-") );
}

void DisIGNoteFinderTick( UDishonoredMantleEdgeFinderComponent* Finder, INT NumSteps, FLOAT Milliseconds )
{
	if( !DisIGDiagEnabled() || Finder == NULL )
	{
		return;
	}
	FDisIGFinderRecord& Record = DisIGRecord( Finder );
	Record.Ticks++;
	Record.Steps += NumSteps;
	Record.Milliseconds += Milliseconds;
	Record.WorstMilliseconds = Max( Record.WorstMilliseconds, Milliseconds );
	Record.WorstSteps = Max( Record.WorstSteps, NumSteps );
	if( appSeconds() < Record.NextReport )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): igdiag finder %s cost over %d ticks: %.2f steps/tick (worst %d), %.4f ms/tick (worst %.4f ms), %d checks, disabled %d, average check wait %.4f s"),
		*Finder->GetName(), Record.Ticks, Record.Ticks ? (FLOAT)Record.Steps / Record.Ticks : 0.f, Record.WorstSteps,
		Record.Ticks ? (FLOAT)( Record.Milliseconds / Record.Ticks ) : 0.f, Record.WorstMilliseconds, Record.Checks, (INT)Finder->m_bMantleCheckDisabled,
		Finder->m_Debug_AverageCheckWait.m_fCurrentTotal / Clamp<INT>( Finder->m_Debug_AverageCheckWait.m_HistoryCount, 1, 10 ) );
	Record.Ticks = Record.Steps = Record.WorstSteps = Record.Checks = 0;
	Record.Milliseconds = 0.0;
	Record.WorstMilliseconds = 0.f;
	Record.NextReport = appSeconds() + 5.0;
}

void DisIGNotePerformMantle( ADishonoredPlayerPawn* Pawn, UDishonoredMantleEdgeFinderComponent* Finder, const TCHAR* Outcome )
{
	static FString LastLine;
	static DOUBLE LastTime = 0.0;
	if( !DisIGDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Pawn->Controller );
	const FString Line = FString::Printf( TEXT("%s; %s; pawn at %s physics %s master %s jump %d"), Outcome, *DisIGSpot( Finder ), *Pawn->Location.ToString(),
		DisIGPhysicsName( Pawn->Physics ), *DisIGMasterState( Pawn ), PC ? (INT)PC->m_bJumpButton : -1 );
	const DOUBLE Now = appSeconds();
	if( Line == LastLine && Now - LastTime < 0.5 )
	{
		return;
	}
	LastLine = Line;
	LastTime = Now;
	debugf( TEXT("DISHONORED(bringup): igdiag perform %s"), *Line );
}

static FLOAT GDisIGTrackTime = -1.f;
static FLOAT GDisIGTrackStartZ = 0.f;
static FLOAT GDisIGTrackMaxZ = 0.f;
static FLOAT GDisIGTrackHeight = 0.f;

void DisIGNoteStepUp( ADishonoredPlayerPawn* Pawn, const FVector& From, const FVector& Start, FLOAT Height, UBOOL bCameraBlend )
{
	if( !DisIGDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): igdiag stepup from %s to start %s, edge height %.2f, swept to %s (Z %+.2f), camera blend %d, physics %s, master %s, no-fall %.2f"),
		*From.ToString(), *Start.ToString(), Height, *Pawn->Location.ToString(), Pawn->Location.Z - From.Z, (INT)bCameraBlend, DisIGPhysicsName( Pawn->Physics ),
		*DisIGMasterState( Pawn ), Pawn->m_fMantleDisallowFallTimer );
	GDisIGTrackTime = 0.f;
	GDisIGTrackStartZ = From.Z;
	GDisIGTrackMaxZ = Pawn->Location.Z;
	GDisIGTrackHeight = Height;
}

void DisIGNoteMantleState( ADishonoredPlayerPawn* Pawn, UBOOL bEnter, FLOAT Height )
{
	if( DisIGDiagEnabled() && Pawn )
	{
		debugf( TEXT("DISHONORED(bringup): igdiag mantle state %s, edge height %.2f, pawn at %s physics %s"), bEnter ? TEXT("entered") : TEXT("left"), Height,
			*Pawn->Location.ToString(), DisIGPhysicsName( Pawn->Physics ) );
	}
}

void DisIGReportMantle( ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime )
{
	if( !DisIGDiagEnabled() || Pawn == NULL || GDisIGTrackTime < 0.f )
	{
		return;
	}
	GDisIGTrackTime += DeltaTime;
	GDisIGTrackMaxZ = Max( GDisIGTrackMaxZ, Pawn->Location.Z );
	debugf( TEXT("DISHONORED(bringup): igdiag stepup t=%.3f Z %.2f (%+.2f) physics %s master %s"), GDisIGTrackTime, Pawn->Location.Z,
		Pawn->Location.Z - GDisIGTrackStartZ, DisIGPhysicsName( Pawn->Physics ), *DisIGMasterState( Pawn ) );
	if( GDisIGTrackTime >= 1.5f )
	{
		debugf( TEXT("DISHONORED(bringup): igdiag stepup summary: edge height %.2f, Z rise %.2f at 1.5 s (peak %.2f), diff to edge height %+.2f, physics %s, master %s"),
			GDisIGTrackHeight, Pawn->Location.Z - GDisIGTrackStartZ, GDisIGTrackMaxZ - GDisIGTrackStartZ, ( Pawn->Location.Z - GDisIGTrackStartZ ) - GDisIGTrackHeight,
			DisIGPhysicsName( Pawn->Physics ), *DisIGMasterState( Pawn ) );
		GDisIGTrackTime = -1.f;
	}
}

static UBOOL DisIGPlacePlayer( ADishonoredPlayerPawn* Pawn, const FVector& Dest, INT Yaw, FOutputDevice& Ar )
{
	const UBOOL bMoved = GWorld->FarMoveActor( Pawn, Dest, FALSE, FALSE, FALSE );
	const FRotator Facing( 0, Yaw, 0 );
	Pawn->SetRotation( Facing );
	if( Pawn->Controller )
	{
		Pawn->Controller->SetRotation( Facing );
	}
	debugf( TEXT("DISHONORED(bringup): igdiag ledge IGLEDGE placed the player at %s yaw %d: moved %d, now at %s physics %s"), *Dest.ToString(), Yaw, (INT)bMoved,
		*Pawn->Location.ToString(), DisIGPhysicsName( Pawn->Physics ) );
	Ar.Logf( TEXT("IGLEDGE: moved %d"), (INT)bMoved );
	return bMoved;
}

// the ground under a point: the first world hit of a downward trace from 200 above to 3000 below; FALSE when there is none
static UBOOL DisIGGroundZ( ADishonoredPlayerPawn* Pawn, const FVector& Point, FLOAT& OutZ )
{
	FCheckResult Hit;
	if( GWorld->SingleLineCheck( Hit, Pawn, Point - FVector( 0.f, 0.f, 3000.f ), Point + FVector( 0.f, 0.f, 200.f ), TRACE_World ) )
	{
		return FALSE;
	}
	OutZ = Hit.Location.Z;
	return TRUE;
}

UBOOL DisIGDiagExec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( !DisIGDiagEnabled() || !ParseCommand( &Cmd, TEXT("IGLEDGE") ) )
	{
		return FALSE;
	}
	ADishonoredPlayerPawn* Pawn = ADishonoredPlayerPawn::s_pInstance;
	if( Pawn == NULL || GWorld == NULL )
	{
		Ar.Logf( TEXT("IGLEDGE: no player") );
		return TRUE;
	}
	FLOAT X, Y, Z, Yaw;
	const FString Args( Cmd );
	if( appSSCANF( *Args, TEXT("%f %f %f %f"), &X, &Y, &Z, &Yaw ) == 4 )
	{
		DisIGPlacePlayer( Pawn, FVector( X, Y, Z ), appTrunc( Yaw ), Ar );
		return TRUE;
	}
	const FVector Feet = DisGetPawnFeet( Pawn );
	const FLOAT HalfHeight = Pawn->CylinderComponent ? Pawn->CylinderComponent->CollisionHeight : 88.f;
	FLOAT BestScore = BIG_NUMBER;
	FVector BestStand( 0.f, 0.f, 0.f );
	INT BestYaw = 0;
	FLOAT BestHeight = 0.f;
	INT Found = 0;
	for( INT Step = 0; Step < 24; Step++ )
	{
		const INT CandidateYaw = Step * 65536 / 24;
		const FVector Dir = FRotator( 0, CandidateYaw, 0 ).Vector();
		UBOOL bPrevValid = FALSE, bPrev2Valid = FALSE;
		FLOAT PrevZ = 0.f, Prev2Z = 0.f;
		for( FLOAT Dist = 32.f; Dist <= 2000.f; Dist += 16.f )
		{
			FLOAT GroundZ = 0.f;
			const UBOOL bValid = DisIGGroundZ( Pawn, Feet + Dir * Dist, GroundZ );
			FLOAT BeyondZ = 0.f, StandZ = 0.f;
			const FLOAT Height = GroundZ - PrevZ;
			if( bValid && bPrevValid && bPrev2Valid && Abs( PrevZ - Prev2Z ) < 4.f && Height >= 60.f && Height <= 115.f && Dist >= 96.f
				&& DisIGGroundZ( Pawn, Feet + Dir * ( Dist + 48.f ), BeyondZ ) && Abs( BeyondZ - GroundZ ) < 8.f
				&& DisIGGroundZ( Pawn, Feet + Dir * ( Dist - 72.f ), StandZ ) && Abs( StandZ - PrevZ ) < 4.f )
			{
				const FVector Stand = Feet + Dir * ( Dist - 72.f );
				const FLOAT Score = Abs( PrevZ - Feet.Z ) * 4.f + Dist;
				if( Found < 12 )
				{
					debugf( TEXT("DISHONORED(bringup): igdiag ledge candidate yaw %d at %.0f uu: a %.1f uu step from ground %.1f to %.1f (stand at %s)"), CandidateYaw, Dist, Height,
						PrevZ, GroundZ, *FVector( Stand.X, Stand.Y, StandZ ).ToString() );
				}
				Found++;
				if( Score < BestScore )
				{
					BestScore = Score;
					BestStand = FVector( Stand.X, Stand.Y, StandZ + HalfHeight + 2.f );
					BestYaw = CandidateYaw;
					BestHeight = Height;
				}
				break;
			}
			bPrev2Valid = bPrevValid;
			Prev2Z = PrevZ;
			bPrevValid = bValid;
			PrevZ = GroundZ;
		}
	}
	if( Found == 0 )
	{
		debugf( TEXT("DISHONORED(bringup): igdiag ledge IGLEDGE found no 60..115 uu step within 2000 uu of %s"), *Feet.ToString() );
		Ar.Logf( TEXT("IGLEDGE: none") );
		return TRUE;
	}
	debugf( TEXT("DISHONORED(bringup): igdiag ledge IGLEDGE chose yaw %d, a %.1f uu step (%d candidates)"), BestYaw, BestHeight, Found );
	DisIGPlacePlayer( Pawn, BestStand, BestYaw, Ar );
	return TRUE;
}
