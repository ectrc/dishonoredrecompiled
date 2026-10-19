/*=============================================================================
	imdiag.cpp: agent IM (PHASE23 IM) - the -imdiag switch (Inc/imdiag.h).
	Nothing here changes behaviour except the IMSCAN / IMGO console commands, which exist only under -imdiag and move the player the
	way IGLEDGE and IFWATER do (UWorld::FarMoveActor).
=============================================================================*/
#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "imdiag.h"

static INT GDisIMDiag = -1;

UBOOL DisIMDiagEnabled()
{
	if( GDisIMDiag < 0 )
	{
		GDisIMDiag = ParseParam( appCmdLine(), TEXT("imdiag") ) ? 1 : 0;
	}
	return GDisIMDiag != 0;
}

static const TCHAR* DisIMPhysicsName( BYTE Physics )
{
	static const TCHAR* Names[] = { TEXT("None"), TEXT("Walking"), TEXT("Falling"), TEXT("Swimming"), TEXT("Flying"), TEXT("Rotating"),
		TEXT("Projectile"), TEXT("Interpolating"), TEXT("Spider"), TEXT("Ladder"), TEXT("RigidBody"), TEXT("SoftBody"), TEXT("NavMeshWalking"),
		TEXT("Unused"), TEXT("Custom") };
	return Physics < ARRAY_COUNT(Names) ? Names[Physics] : TEXT("?");
}

static const TCHAR* DisIMPickerName( INT StateType )
{
	static const TCHAR* Names[] = { TEXT("FullBody"), TEXT("UpperBody"), TEXT("Special") };
	return StateType >= 0 && StateType < 3 ? Names[StateType] : TEXT("?");
}

static FString DisIMStateName( UDishonoredNativeState* State )
{
	return State ? State->GetClass()->GetName() : FString( TEXT("none") );
}

static FString DisIMMachineStates( UDishonoredNativeStateMachine* FSM )
{
	if( FSM == NULL )
	{
		return FString( TEXT("NO MACHINE") );
	}
	FString States;
	for( INT Index = 0; Index < FSM->m_NativeStates.Num(); Index++ )
	{
		States += FSM->m_NativeStates(Index) ? FSM->m_NativeStates(Index)->GetClass()->GetName() : FString( TEXT("null") );
		States += TEXT(" ");
	}
	return FString::Printf( TEXT("%s: %d states [%s] current %s"), *FSM->GetClass()->GetName(), FSM->m_NativeStates.Num(), *States, *DisIMStateName( FSM->GetCurrentlyActiveState() ) );
}

static FString DisIMMachineName( ADishonoredPlayerPawn* Pawn, UDishonoredNativeStateMachine* FSM )
{
	if( Pawn && FSM == Pawn->m_pPlayerMasterFSM )
	{
		return FString( TEXT("master") );
	}
	if( Pawn && FSM == Pawn->m_pPlayerUpperFSM )
	{
		return FString( TEXT("upper") );
	}
	if( Pawn && FSM == Pawn->m_pPlayerLeftArmFSM )
	{
		return FString( TEXT("leftarm") );
	}
	return FString( TEXT("other") );
}

void DisIMNoteInit( ADishonoredPlayerPawn* Pawn )
{
	if( !DisIMDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): imdiag init master %s"), *DisIMMachineStates( Pawn->m_pPlayerMasterFSM ) );
	debugf( TEXT("DISHONORED(bringup): imdiag init upper %s"), *DisIMMachineStates( Pawn->m_pPlayerUpperFSM ) );
	debugf( TEXT("DISHONORED(bringup): imdiag init leftarm %s"), *DisIMMachineStates( Pawn->m_pPlayerLeftArmFSM ) );
	UDisNativeStateMachine_PlayerAction* Upper = Cast<UDisNativeStateMachine_PlayerAction>( Pawn->m_pPlayerUpperFSM );
	UDisNativeStateMachine_PlayerAction* LeftArm = Cast<UDisNativeStateMachine_PlayerAction>( Pawn->m_pPlayerLeftArmFSM );
	if( Upper && LeftArm )
	{
		debugf( TEXT("DISHONORED(bringup): imdiag init upper picker %d usage %d action usage %d nav %d; leftarm picker %d usage %d action usage %d nav %d"),
			Upper->m_BoundAnimStatePicker, Upper->m_BoundToItemUsage, Upper->m_ActionUsage, (INT)Upper->m_bUseNavigationState,
			LeftArm->m_BoundAnimStatePicker, LeftArm->m_BoundToItemUsage, LeftArm->m_ActionUsage, (INT)LeftArm->m_bUseNavigationState );
	}
	debugf( TEXT("DISHONORED(bringup): imdiag init owner interface: GetInterfaceAddress %p, C++ base %p"), Pawn->GetInterfaceAddress( UDisAnimStateOwnerInterface::StaticClass() ),
		(void*)(IDisAnimStateOwnerInterface*)Pawn );
	UDisAnimStateComponent* Comp = Pawn->m_pAnimStateComp;
	debugf( TEXT("DISHONORED(bringup): imdiag init anim-state component %s, %d pickers, %d picker infos"), Comp ? *Comp->GetName() : TEXT("NONE"),
		Comp ? Comp->m_StatePickers.Num() : 0, Pawn->m_AnimStatePickerInfo.Num() );
	for( INT Index = 0; Comp && Index < Comp->m_StatePickers.Num(); Index++ )
	{
		const FDisAnimState_Picker& Picker = Comp->m_StatePickers(Index);
		const INT NumStates = Index < Pawn->m_AnimStatePickerInfo.Num() ? Pawn->m_AnimStatePickerInfo(Index).m_AnimStateMap.Num() : -1;
		const FDisAnimState* Cur = (const FDisAnimState*)Picker.m_pCurState;
		debugf( TEXT("DISHONORED(bringup): imdiag init picker %s: node %s parent %d filter %s, %d states, current %s"), DisIMPickerName( Index ),
			Picker.m_pStatePicker ? *Picker.m_pStatePicker->NodeName.ToString() : TEXT("NONE"), Picker.m_parentPicker,
			Picker.m_pFilter ? *Picker.m_pFilter->NodeName.ToString() : TEXT("none"), NumStates, Cur ? *Cur->m_StateName.ToString() : TEXT("none") );
	}
}

static FString DisIMCustomSequences( const FDisPawnAnimState* State )
{
	FString Out;
	for( INT Index = 0; State && Index < State->m_CustomSequences.Num(); Index++ )
	{
		Out += FString::Printf( TEXT("%s%s=%s"), Index ? TEXT(", ") : TEXT(""), *State->m_CustomSequences(Index).m_Alias.ToString(), *State->m_CustomSequences(Index).m_Sequence.ToString() );
	}
	return Out;
}

void DisIMNoteSetAnimState( ADishonoredPlayerPawn* Pawn, INT StateType, FName Requested, const FDisPawnAnimState* State, INT Status, UDishonoredNativeState* AttachToState )
{
	if( !DisIMDiagEnabled() )
	{
		return;
	}
	if( Status == eSetAnimStateStatus_Failed_AlreadySet )
	{
		return;
	}
	if( Status != eSetAnimStateStatus_Success )
	{
		static TArray<FName> SaidFailed;
		if( SaidFailed.ContainsItem( Requested ) )
		{
			return;
		}
		SaidFailed.AddItem( Requested );
	}
	static const TCHAR* StatusNames[] = { TEXT("Failed"), TEXT("Failed_AlreadySet"), TEXT("Failed_LockedOut"), TEXT("Success") };
	debugf( TEXT("DISHONORED(bringup): imdiag anim %s %s -> %s (tree %s, blend %.2f, nonlooping %d, root motion %d/%d): %s, attached to %s, t=%.2f; sequences [%s]"),
		DisIMPickerName( StateType ), *Requested.ToString(), State ? *State->m_StateName.ToString() : TEXT("NOT FOUND"), State ? *State->m_AnimStateTree.ToString() : TEXT("-"),
		State ? State->m_fBlendTime : 0.f, State ? (INT)State->m_bNonLooping : -1, State ? State->m_RootMotionMode : -1, State ? State->m_RootMotionRotationMode : -1,
		Status >= 0 && Status < 4 ? StatusNames[Status] : TEXT("no state / no picker"), *DisIMStateName( AttachToState ), GWorld ? GWorld->GetTimeSeconds() : 0.f,
		*DisIMCustomSequences( State ) );
}

void DisIMNoteClearAnimState( ADishonoredPlayerPawn* Pawn, INT StateType, FLOAT BlendTime )
{
	if( !DisIMDiagEnabled() || Pawn == NULL || Pawn->m_pAnimStateComp == NULL || StateType < 0 || StateType >= Pawn->m_pAnimStateComp->m_StatePickers.Num() )
	{
		return;
	}
	const FDisAnimState* Cur = (const FDisAnimState*)Pawn->m_pAnimStateComp->m_StatePickers( StateType ).m_pCurState;
	if( Cur )
	{
		debugf( TEXT("DISHONORED(bringup): imdiag anim %s cleared (was %s), blend %.2f, t=%.2f"), DisIMPickerName( StateType ), *Cur->m_StateName.ToString(), BlendTime,
			GWorld ? GWorld->GetTimeSeconds() : 0.f );
	}
}

void DisIMNoteTransition( UDishonoredNativeStateMachine* StateMachine, UDishonoredNativeState* Last, UDishonoredNativeState* Next )
{
	if( !DisIMDiagEnabled() || StateMachine == NULL || Next == NULL )
	{
		return;
	}
	ADishonoredPlayerPawn* Pawn = Cast<ADishonoredPlayerPawn>( StateMachine->GetManagedObject() );
	if( Pawn == NULL )
	{
		return;
	}
	const FString Machine = DisIMMachineName( Pawn, StateMachine );
	UStateSharedActionBase_PlayerOnly* Action = Cast<UStateSharedActionBase_PlayerOnly>( Next );
	debugf( TEXT("DISHONORED(bringup): imdiag fsm %s %s -> %s%s%s (t=%.2f, Z %.1f, physics %s)"), *Machine, *DisIMStateName( Last ), *DisIMStateName( Next ),
		Action && Action->GetCurAction() ? TEXT(" action ") : TEXT(""), Action && Action->GetCurAction() ? *Action->GetCurAction()->m_ActionName.ToString() : TEXT(""),
		GWorld ? GWorld->GetTimeSeconds() : 0.f, Pawn->Location.Z, DisIMPhysicsName( Pawn->Physics ) );
}

void DisIMNoteRequest( ADishonoredPlayerPawn* Pawn, BYTE ActionUsage, const FPawnAction* Action, UBOOL bGranted, const TCHAR* Why )
{
	static FString LastLine;
	if( !DisIMDiagEnabled() || Action == NULL )
	{
		return;
	}
	FString States;
	for( INT Index = 0; Index < Action->m_AnimStates.Num(); Index++ )
	{
		States += FString::Printf( TEXT("%s%s"), Index ? TEXT(" ") : TEXT(""), *Action->m_AnimStates(Index).ToString() );
	}
	const FString Line = FString::Printf( TEXT("action %s [%s]: %s - %s"), *Action->m_ActionName.ToString(), *States, bGranted ? TEXT("granted") : TEXT("refused"), Why );
	if( Line == LastLine )
	{
		return;
	}
	LastLine = Line;
	debugf( TEXT("DISHONORED(bringup): imdiag %s (t=%.2f)"), *Line, GWorld ? GWorld->GetTimeSeconds() : 0.f );
}

void DisIMNoteExit( const TCHAR* Kind, INT StateType, UDishonoredNativeState* AttachedState, FLOAT TimeToGo )
{
	if( DisIMDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): imdiag %s picker %s attached %s time to go %.3f (t=%.2f)"), Kind, StateType < 0 ? TEXT("-") : DisIMPickerName( StateType ),
			*DisIMStateName( AttachedState ), TimeToGo, GWorld ? GWorld->GetTimeSeconds() : 0.f );
	}
}

// ---- the animated mantle track ----

static FLOAT GDisIMMantleTime = -1.f;
static FLOAT GDisIMMantleStartZ = 0.f;
static FLOAT GDisIMMantleHeight = 0.f;
static FLOAT GDisIMMantleLeftAt = -1.f;
static FLOAT GDisIMMantleEnteredAt = -1.f;

static UBOOL DisIMInMantleState( ADishonoredPlayerPawn* Pawn )
{
	return Pawn->m_pPlayerMasterFSM && Cast<UStatePlayerMasterMantle>( Pawn->m_pPlayerMasterFSM->GetCurrentlyActiveState() ) != NULL;
}

void DisIMNoteMantleRequest( ADishonoredPlayerPawn* Pawn, FLOAT EdgeHeight )
{
	if( !DisIMDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	GDisIMMantleTime = 0.f;
	GDisIMMantleStartZ = Pawn->Location.Z;
	GDisIMMantleHeight = EdgeHeight;
	GDisIMMantleLeftAt = -1.f;
	GDisIMMantleEnteredAt = -1.f;
	debugf( TEXT("DISHONORED(bringup): imdiag mantle requested: edge height %.2f from %s physics %s master %s"), EdgeHeight, *Pawn->Location.ToString(),
		DisIMPhysicsName( Pawn->Physics ), *DisIMStateName( Pawn->m_pPlayerMasterFSM ? Pawn->m_pPlayerMasterFSM->GetCurrentlyActiveState() : NULL ) );
}

static void DisIMTrackMantle( ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime )
{
	if( GDisIMMantleTime < 0.f )
	{
		return;
	}
	GDisIMMantleTime += DeltaTime;
	const UBOOL bInMantle = DisIMInMantleState( Pawn );
	if( bInMantle && GDisIMMantleEnteredAt < 0.f )
	{
		GDisIMMantleEnteredAt = GDisIMMantleTime;
	}
	if( !bInMantle && GDisIMMantleEnteredAt >= 0.f && GDisIMMantleLeftAt < 0.f )
	{
		GDisIMMantleLeftAt = GDisIMMantleTime;
	}
	UDishonoredNativeState* Master = Pawn->m_pPlayerMasterFSM ? Pawn->m_pPlayerMasterFSM->GetCurrentlyActiveState() : NULL;
	debugf( TEXT("DISHONORED(bringup): imdiag mantle t=%.3f Z %.2f (%+.2f) at %s physics %s master %s collideworld %d rmm %d"), GDisIMMantleTime, Pawn->Location.Z,
		Pawn->Location.Z - GDisIMMantleStartZ, *Pawn->Location.ToString(), DisIMPhysicsName( Pawn->Physics ), *DisIMStateName( Master ), (INT)Pawn->bCollideWorld,
		Pawn->Mesh ? Pawn->Mesh->RootMotionMode : -1 );
	const UBOOL bDone = ( GDisIMMantleLeftAt >= 0.f && GDisIMMantleTime - GDisIMMantleLeftAt >= 1.5f ) || ( GDisIMMantleEnteredAt < 0.f && GDisIMMantleTime >= 1.5f ) || GDisIMMantleTime >= 10.f;
	if( bDone )
	{
		debugf( TEXT("DISHONORED(bringup): imdiag mantle summary: edge height %.2f, Z rise %.2f (diff %+.2f), in the Mantle state %.3f s (entered %.3f, left %.3f), end physics %s, master %s, collideworld %d noencroach %d, collision height %.1f"),
			GDisIMMantleHeight, Pawn->Location.Z - GDisIMMantleStartZ, ( Pawn->Location.Z - GDisIMMantleStartZ ) - GDisIMMantleHeight,
			GDisIMMantleLeftAt >= 0.f ? GDisIMMantleLeftAt - GDisIMMantleEnteredAt : -1.f, GDisIMMantleEnteredAt, GDisIMMantleLeftAt, DisIMPhysicsName( Pawn->Physics ),
			*DisIMStateName( Master ), (INT)Pawn->bCollideWorld, (INT)Pawn->bNoEncroachCheck, Pawn->CylinderComponent ? Pawn->CylinderComponent->CollisionHeight : -1.f );
		GDisIMMantleTime = -1.f;
	}
}

// ---- IMSCAN / IMGO: ledges in the finder's range, tested with the real finder ----

struct FDisIMLedge
{
	FVector Stand;
	INT Yaw;
	FLOAT GeometricHeight;
	FLOAT Distance;
	INT Result;
	FLOAT FinderHeight;
};

static TArray<FDisIMLedge> GDisIMLedges;
static INT GDisIMTesting = -1;
static FLOAT GDisIMTestTime = 0.f;
static const DWORD DisIMFinderTraceFlags = 0x800020C6;

static UBOOL DisIMGroundZ( AActor* Source, const FVector& Top, FLOAT Depth, FLOAT& OutZ )
{
	FCheckResult Hit;
	if( GWorld->SingleLineCheck( Hit, Source, Top - FVector( 0.f, 0.f, Depth ), Top, DisIMFinderTraceFlags ) )
	{
		return FALSE;
	}
	OutZ = Hit.Location.Z;
	return TRUE;
}

static FLOAT GDisIMHealTime = 0.f;

static void DisIMPlace( ADishonoredPlayerPawn* Pawn, const FVector& Dest, INT Yaw )
{
	// a teleport between floors registers as a fall with the old physics; the measurement aid keeps the player alive (and so walking:
	// APawn::SetPostLandedPhysics lands a pawn without health in PHYS_None)
	Pawn->Health = Max( Pawn->Health, Pawn->HealthMax );
	GDisIMHealTime = 1.5f;
	GWorld->FarMoveActor( Pawn, Dest, FALSE, FALSE, FALSE );
	const FRotator Facing( 0, Yaw, 0 );
	Pawn->SetRotation( Facing );
	Pawn->Velocity = FVector( 0.f, 0.f, 0.f );
	Pawn->Acceleration = FVector( 0.f, 0.f, 0.f );
	if( Pawn->Controller )
	{
		Pawn->Controller->SetRotation( Facing );
	}
}

static void DisIMScan( ADishonoredPlayerPawn* Pawn, const FVector& Center, FOutputDevice& Ar )
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = Pawn->m_pMantleEdgeFinder ? Pawn->m_pMantleEdgeFinder->GetMantleTweaks() : NULL;
	const FLOAT MinHeight = Tweaks ? Tweaks->m_MantleMinEdgeHeight : 55.f;
	const FLOAT MaxHeight = Tweaks ? Tweaks->m_MantleMaxEdgeHeight : 230.f;
	const FLOAT Radius = Pawn->CylinderComponent ? Pawn->CylinderComponent->CollisionRadius : 34.f;
	const FLOAT HalfHeight = Pawn->CylinderComponent ? Pawn->CylinderComponent->CollisionHeight : 88.f;
	const FLOAT Step = 48.f;
	const INT Cells = appTrunc( 3000.f / Step );
	const INT Side = Cells * 2 + 1;
	TArray<FLOAT> Heights;
	Heights.Add( Side * Side );
	const FLOAT NoGround = -1.e9f;
	const FVector Top( Center.X, Center.Y, Center.Z + 1500.f );
	for( INT Y = 0; Y < Side; Y++ )
	{
		for( INT X = 0; X < Side; X++ )
		{
			FLOAT GroundZ = NoGround;
			const FVector Column( Top.X + ( X - Cells ) * Step, Top.Y + ( Y - Cells ) * Step, Top.Z );
			if( Square( X - Cells ) + Square( Y - Cells ) > Square( Cells ) || !DisIMGroundZ( Pawn, Column, 3000.f, GroundZ ) )
			{
				GroundZ = NoGround;
			}
			Heights( Y * Side + X ) = GroundZ;
		}
	}
	TArray<FDisIMLedge> Found;
	INT NumGround = 0, NumSteps = 0, NumNoFace = 0;
	for( INT Index = 0; Index < Heights.Num(); Index++ )
	{
		NumGround += Heights( Index ) != NoGround ? 1 : 0;
	}
	static const INT DX[4] = { 1, -1, 0, 0 };
	static const INT DY[4] = { 0, 0, 1, -1 };
	for( INT Y = 2; Y < Side - 2; Y++ )
	{
		for( INT X = 2; X < Side - 2; X++ )
		{
			const FLOAT Low = Heights( Y * Side + X );
			if( Low == NoGround )
			{
				continue;
			}
			for( INT Dir = 0; Dir < 4; Dir++ )
			{
				const FLOAT High = Heights( ( Y + DY[Dir] ) * Side + X + DX[Dir] );
				const FLOAT Behind = Heights( ( Y - DY[Dir] ) * Side + X - DX[Dir] );
				const FLOAT Beyond = Heights( ( Y + 2 * DY[Dir] ) * Side + X + 2 * DX[Dir] );
				if( High == NoGround || Behind == NoGround || Beyond == NoGround )
				{
					continue;
				}
				const FLOAT Height = High - Low;
				if( Height < MinHeight || Height > MaxHeight || Abs( Behind - Low ) > 6.f || Abs( Beyond - High ) > 6.f )
				{
					continue;
				}
				NumSteps++;
				const FVector Facing( (FLOAT)DX[Dir], (FLOAT)DY[Dir], 0.f );
				const FVector LowPoint( Top.X + ( X - Cells ) * Step, Top.Y + ( Y - Cells ) * Step, Low );
				FCheckResult FaceHit;
				const FVector FaceStart = LowPoint - Facing * Step + FVector( 0.f, 0.f, 30.f );
				if( GWorld->SingleLineCheck( FaceHit, Pawn, FaceStart + Facing * Step * 3.f, FaceStart, DisIMFinderTraceFlags ) )
				{
					NumNoFace++;
					continue;
				}
				FDisIMLedge Ledge;
				Ledge.Stand = FVector( FaceHit.Location.X, FaceHit.Location.Y, Low + HalfHeight + 4.f ) - Facing * ( Radius + 14.f );
				Ledge.Yaw = Facing.Rotation().Yaw & 65535;
				Ledge.GeometricHeight = Height;
				Ledge.Distance = ( Ledge.Stand - Center ).Size2D();
				Ledge.Result = 0;
				Ledge.FinderHeight = 0.f;
				UBOOL bNear = FALSE;
				for( INT Other = 0; Other < Found.Num() && !bNear; Other++ )
				{
					bNear = ( Found(Other).Stand - Ledge.Stand ).Size() < 200.f;
				}
				if( !bNear )
				{
					Found.AddItem( Ledge );
				}
			}
		}
	}
	GDisIMLedges.Empty();
	while( Found.Num() && GDisIMLedges.Num() < 24 )
	{
		INT Best = 0;
		for( INT Index = 1; Index < Found.Num(); Index++ )
		{
			if( Found(Index).Distance < Found(Best).Distance )
			{
				Best = Index;
			}
		}
		GDisIMLedges.AddItem( Found(Best) );
		Found.Remove( Best );
	}
	debugf( TEXT("DISHONORED(bringup): imdiag ledge scan around %s: %d cells (%d with ground), edge range %.0f..%.0f uu, %d flat steps in range (%d without a face), %d candidates kept (%d more)"),
		*Center.ToString(), Side * Side, NumGround, MinHeight, MaxHeight, NumSteps, NumNoFace, GDisIMLedges.Num(), Found.Num() );
	for( INT Index = 0; Index < GDisIMLedges.Num(); Index++ )
	{
		debugf( TEXT("DISHONORED(bringup): imdiag ledge candidate %d: stand %s yaw %d, geometric step %.1f uu, %.0f uu away"), Index, *GDisIMLedges(Index).Stand.ToString(),
			GDisIMLedges(Index).Yaw, GDisIMLedges(Index).GeometricHeight, GDisIMLedges(Index).Distance );
	}
	Ar.Logf( TEXT("IMSCAN: %d candidates"), GDisIMLedges.Num() );
	GDisIMTesting = GDisIMLedges.Num() ? 0 : -1;
	GDisIMTestTime = 0.f;
	if( GDisIMTesting == 0 )
	{
		DisIMPlace( Pawn, GDisIMLedges(0).Stand, GDisIMLedges(0).Yaw );
	}
}

static void DisIMTickScan( ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime )
{
	if( GDisIMTesting < 0 || GDisIMTesting >= GDisIMLedges.Num() )
	{
		return;
	}
	GDisIMTestTime += DeltaTime;
	if( GDisIMTestTime < 1.f )
	{
		return;
	}
	FDisIMLedge& Ledge = GDisIMLedges( GDisIMTesting );
	UDishonoredMantleEdgeFinderComponent* Finder = Pawn->m_pMantleEdgeFinder;
	const UBOOL bValid = Finder && Finder->HasValidMantleSpot();
	const FDisMantleSpot Spot = Finder ? Finder->GetMantleSpot() : FDisMantleSpot( EC_EventParm );
	Ledge.Result = bValid ? 1 : 2;
	Ledge.FinderHeight = bValid ? Spot.m_fEdgeHeight : 0.f;
	debugf( TEXT("DISHONORED(bringup): imdiag ledge test %d: %s, finder edge height %.2f (geometric %.1f), start %s yaw %d impact %d crouch %d; pawn at %s physics %s"), GDisIMTesting,
		bValid ? TEXT("ACCEPTED") : TEXT("rejected"), Ledge.FinderHeight, Ledge.GeometricHeight, *Spot.m_StartLocation.ToString(), Spot.m_StartYaw,
		(INT)Spot.m_bRequiresImpactAnim, (INT)Spot.m_bForcesCrouch, *Pawn->Location.ToString(), DisIMPhysicsName( Pawn->Physics ) );
	GDisIMTesting++;
	GDisIMTestTime = 0.f;
	if( GDisIMTesting < GDisIMLedges.Num() )
	{
		DisIMPlace( Pawn, GDisIMLedges( GDisIMTesting ).Stand, GDisIMLedges( GDisIMTesting ).Yaw );
		return;
	}
	INT NumAccepted = 0;
	for( INT Index = 0; Index < GDisIMLedges.Num(); Index++ )
	{
		if( GDisIMLedges(Index).Result == 1 )
		{
			NumAccepted++;
			debugf( TEXT("DISHONORED(bringup): imdiag ledge accepted %d: IMGO %d -> stand %s yaw %d, finder edge height %.2f (%s)"), NumAccepted, Index,
				*GDisIMLedges(Index).Stand.ToString(), GDisIMLedges(Index).Yaw, GDisIMLedges(Index).FinderHeight,
				GDisIMLedges(Index).FinderHeight > 120.f ? TEXT("animated mantle") : TEXT("step-up") );
		}
	}
	debugf( TEXT("DISHONORED(bringup): imdiag ledge scan done: %d of %d accepted by the finder"), NumAccepted, GDisIMLedges.Num() );
	GDisIMTesting = -1;
}

void DisIMReport( ADishonoredPlayerPawn* Pawn, FLOAT DeltaTime )
{
	if( !DisIMDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	if( GDisIMHealTime > 0.f )
	{
		GDisIMHealTime -= DeltaTime;
		Pawn->Health = Max( Pawn->Health, Pawn->HealthMax );
	}
	DisIMTrackMantle( Pawn, DeltaTime );
	DisIMTickScan( Pawn, DeltaTime );
}

UBOOL DisIMDiagExec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( !DisIMDiagEnabled() )
	{
		return FALSE;
	}
	ADishonoredPlayerPawn* Pawn = ADishonoredPlayerPawn::s_pInstance;
	const UBOOL bScan = ParseCommand( &Cmd, TEXT("IMSCAN") );
	const UBOOL bGo = !bScan && ParseCommand( &Cmd, TEXT("IMGO") );
	const UBOOL bState = !bScan && !bGo && ParseCommand( &Cmd, TEXT("IMSTATE") );
	if( !bScan && !bGo && !bState )
	{
		return FALSE;
	}
	if( Pawn == NULL || GWorld == NULL )
	{
		Ar.Logf( TEXT("IM: no player") );
		return TRUE;
	}
	if( bState )
	{
		DisIMNoteInit( Pawn );
		return TRUE;
	}
	if( bGo )
	{
		const INT Index = appAtoi( Cmd );
		if( Index >= 0 && Index < GDisIMLedges.Num() )
		{
			DisIMPlace( Pawn, GDisIMLedges(Index).Stand, GDisIMLedges(Index).Yaw );
			debugf( TEXT("DISHONORED(bringup): imdiag ledge IMGO %d: placed at %s yaw %d (finder said %.2f)"), Index, *Pawn->Location.ToString(), GDisIMLedges(Index).Yaw,
				GDisIMLedges(Index).FinderHeight );
		}
		return TRUE;
	}
	FLOAT X, Y, Z;
	FVector Center = DisGetPawnFeet( Pawn );
	if( appSSCANF( Cmd, TEXT("%f %f %f"), &X, &Y, &Z ) == 3 )
	{
		Center = FVector( X, Y, Z );
	}
	DisIMScan( Pawn, Center, Ar );
	return TRUE;
}
