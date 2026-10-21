/*=============================================================================
	jgdiag.cpp: agent JG (PHASE24 JG) - the -jgdiag measurements of the jump's camera (Inc/jgdiag.h). Measurement only.
=============================================================================*/
#include "DishonoredGame.h"
#include "jgdiag.h"

static INT GDisJGDiag = -1;
static FLOAT GDisJGLastLanded = -100.f;

void DishonoredLinkJG_Diag()
{
}

UBOOL DisJGDiagEnabled()
{
	if( GDisJGDiag < 0 )
	{
		GDisJGDiag = ParseParam( appCmdLine(), TEXT("jgdiag") ) ? 1 : 0;
	}
	return GDisJGDiag != 0;
}

static FLOAT DisJGNow()
{
	return GWorld ? GWorld->GetTimeSeconds() : 0.f;
}

static UBOOL DisJGFinite( const FVector& V )
{
	return appIsFinite( V.X ) && appIsFinite( V.Y ) && appIsFinite( V.Z ) && !appIsNaN( V.X ) && !appIsNaN( V.Y ) && !appIsNaN( V.Z );
}

static FString DisJGPoint( const FDisSpringPoint& Point )
{
	const FVector Rel = Point.m_Pos - Point.m_BoundPos;
	return FString::Printf( TEXT("rel %.2f %.2f %.2f vel %.1f %.1f %.1f"), Rel.X, Rel.Y, Rel.Z, Point.m_Velocity.X, Point.m_Velocity.Y, Point.m_Velocity.Z );
}

void DisJGNotePhysicalReact( UDishonoredCamera_PhysicalReact* React, const TCHAR* What )
{
	if( !DisJGDiagEnabled() || !React )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): jgdiag react %s %s t=%.3f: weight %.2f/%.2f active %d sleeping %d fixed %d step %.4f; pivot %s arm bounce %.2f %.1f..%.1f"),
		*React->GetName(), What, DisJGNow(), React->m_Weight, React->m_TargetWeight, (INT)React->m_bActive, (INT)React->m_bIsSleeping,
		(INT)React->m_bUseFixedTimeStep, React->m_fFixedTimeStep, *React->m_CameraPivotOffset.ToString(), React->m_ArmBounce, React->m_ArmBounce_Start,
		React->m_ArmBounce_End );
	debugf( TEXT("DISHONORED(bringup): jgdiag react %s %s springs: strength bound %s %s k %.1f d %.1f cap %d %.0f; stability bound %s %s k %.1f d %.1f cap %d %.0f"),
		*React->GetName(), What,
		*React->m_StrengthPoint.m_BoundPos.ToString(), *DisJGPoint( React->m_StrengthPoint ), React->m_StrengthConstants.m_Springiness,
		React->m_StrengthConstants.m_Damping, (INT)React->m_StrengthConstants.m_bCapVelocity, React->m_StrengthConstants.m_fMaxVelocity,
		*React->m_StabilityPoint.m_BoundPos.ToString(), *DisJGPoint( React->m_StabilityPoint ), React->m_StabilityConstants.m_Springiness,
		React->m_StabilityConstants.m_Damping, (INT)React->m_StabilityConstants.m_bCapVelocity, React->m_StabilityConstants.m_fMaxVelocity );
}

void DisJGNoteSpringStep( UDishonoredCamera_PhysicalReact* React )
{
	static UBOOL bSaid = FALSE;
	if( bSaid || !React )
	{
		return;
	}
	if( !DisJGFinite( React->m_StrengthPoint.m_Pos ) || !DisJGFinite( React->m_StabilityPoint.m_Pos ) || !DisJGFinite( React->m_StrengthPoint.m_Velocity ) || !DisJGFinite( React->m_StabilityPoint.m_Velocity ) )
	{
		bSaid = TRUE;
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): jgdiag NAN in %s's spring at t=%.3f: strength %s stability %s"), *React->GetName(), DisJGNow(),
			*DisJGPoint( React->m_StrengthPoint ), *DisJGPoint( React->m_StabilityPoint ) );
	}
}

void DisJGNoteReactOutput( UDishonoredCamera_PhysicalReact* React, const FDishonoredViewTarget& OutVT )
{
	if( !DisJGDiagEnabled() || !React )
	{
		return;
	}
	const FDishonoredVTSettings& Settings = OutVT.m_DishonoredSettings;
	debugf( TEXT("DISHONORED(bringup): jgdiag react %s t=%.3f w %.2f sleep %d acc %.4f | strength %s | stability %s | out loc %.2f %.2f %.2f rot %d %d %d arm %.3f"),
		*React->GetName(), DisJGNow(), React->m_Weight, (INT)React->m_bIsSleeping, React->m_fAccumulatedDelta,
		*DisJGPoint( React->m_StrengthPoint ), *DisJGPoint( React->m_StabilityPoint ),
		OutVT.m_VT.POV.Location.X, OutVT.m_VT.POV.Location.Y, OutVT.m_VT.POV.Location.Z,
		OutVT.m_VT.POV.Rotation.Pitch, OutVT.m_VT.POV.Rotation.Yaw, OutVT.m_VT.POV.Rotation.Roll, Settings.m_ArmFollowWeight );
}

void DisJGNoteSmootherWeight( ADishonoredPawn* Pawn, const TCHAR* Where, FLOAT TargetWeight, FLOAT BlendTime )
{
	if( !DisJGDiagEnabled() || !Pawn )
	{
		return;
	}
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	UDishonoredCamera_BumpSmoother* Smoother = Camera ? Camera->GetCamera_BumpSmoother() : NULL;
	debugf( TEXT("DISHONORED(bringup): jgdiag smoother %s t=%.3f: SetWeight(%.2f, %.2f) on %s (weight %.2f target %.2f default %.2f)"), Where, DisJGNow(),
		TargetWeight, BlendTime, Smoother ? *Smoother->GetName() : TEXT("none"), Smoother ? Smoother->m_Weight : -1.f,
		Smoother ? Smoother->m_TargetWeight : -1.f, Smoother ? Smoother->m_fDefaultWeight : -1.f );
}

void DisJGNoteLanded( ADishonoredPlayerPawn* Pawn, const FVector& Velocity, UClass* ContactType, UBOOL bImpact, UBOOL bCamera )
{
	if( !Pawn )
	{
		return;
	}
	GDisJGLastLanded = DisJGNow();
	if( !DisJGDiagEnabled() )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): jgdiag land t=%.3f: Velocity %.1f %.1f %.1f, contact %s (UDishonoredContactSystem::ApplyContact not ported), camera %d impact %d, ClearFOVTarget(1), DisDLC06SeqEvent_Landed fired"),
		GDisJGLastLanded, Velocity.X, Velocity.Y, Velocity.Z, ContactType ? *ContactType->GetName() : TEXT("none"), bCamera, bImpact );
}

void DisJGNoteAttributes( ADishonoredPawn* Pawn )
{
	if( !DisJGDiagEnabled() || !Pawn )
	{
		return;
	}
	const UBOOL bPlayer = Pawn->IsA( ADishonoredPlayerPawn::StaticClass() );
	static TArray<ADishonoredPawn*> Seen;
	static FLOAT LastPlayer[8] = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f };
	const FLOAT Now[8] = { Pawn->GroundSpeed, Pawn->WaterSpeed, Pawn->AccelRate, Pawn->MaxFallSpeed, Pawn->CrouchHeight, Pawn->WalkableFloorZ, Pawn->AirControl, Pawn->ViewPitchMin };
	if( bPlayer )
	{
		if( appMemcmp( Now, LastPlayer, sizeof(Now) ) == 0 )
		{
			return;
		}
		appMemcpy( LastPlayer, Now, sizeof(Now) );
	}
	else
	{
		if( Seen.ContainsItem( Pawn ) || Seen.Num() >= 64 )
		{
			return;
		}
		Seen.AddItem( Pawn );
	}
	debugf( TEXT("DISHONORED(bringup): jgdiag attrib %s t=%.3f: GroundSpeed %.1f WaterSpeed %.1f AccelRate %.1f MaxFallSpeed %.1f CrouchHeight %.1f WalkableFloorZ %.3f AirControl %.3f ViewPitch %.0f..%.0f JumpZ %.1f"),
		*Pawn->GetName(), DisJGNow(), Now[0], Now[1], Now[2], Now[3], Now[4], Now[5], Now[6], Pawn->ViewPitchMin, Pawn->ViewPitchMax, Pawn->JumpZ );
}

void DisJGReportPlayer( ADishonoredPlayerPawn* Pawn )
{
	if( !DisJGDiagEnabled() || !Pawn )
	{
		return;
	}
	const FLOAT Now = DisJGNow();
	if( Pawn->Physics != PHYS_Falling && Now - GDisJGLastLanded > 1.5f )
	{
		return;
	}
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	UDishonoredCamera_BumpSmoother* Smoother = Camera ? Camera->GetCamera_BumpSmoother() : NULL;
	UDishonoredCamera_PhysicalReact* React = Camera ? Camera->GetCamera_PhysicalReact() : NULL;
	UDishonoredNativeStateMachine* Master = Pawn->GetPlayerMasterFSM();
	UObject* State = Master ? Master->GetCurrentlyActiveState() : NULL;
	debugf( TEXT("DISHONORED(bringup): jgdiag player t=%.3f pawnZ %.2f vz %.1f v2d %.1f phys %d master %s | smoother w %.2f target %.2f active %d | react w %.2f active %d sleep %d strength %s"),
		Now, Pawn->Location.Z, Pawn->Velocity.Z, Pawn->Velocity.Size2D(), (INT)Pawn->Physics, State ? *State->GetClass()->GetName() : TEXT("none"),
		Smoother ? Smoother->m_Weight : -1.f, Smoother ? Smoother->m_TargetWeight : -1.f, Smoother ? (INT)Smoother->m_bActive : -1,
		React ? React->m_Weight : -1.f, React ? (INT)React->m_bActive : -1, React ? (INT)React->m_bIsSleeping : -1,
		React ? *DisJGPoint( React->m_StrengthPoint ) : TEXT("-") );
}
