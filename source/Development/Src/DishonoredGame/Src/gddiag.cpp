// DishonoredGame/src/gddiag.cpp
// DISHONORED(bringup): agent GD (PHASE19 GD). See Inc/gddiag.h. Called from ADishonoredPlayerController::Tick beside the other
// per-agent reports; nothing here runs without its switch.

#include "DishonoredGame.h"
#include "gddiag.h"

static INT GDisGDDiag = -1;

UBOOL DisGDDiagEnabled()
{
	if( GDisGDDiag < 0 )
	{
		GDisGDDiag = ParseParam( appCmdLine(), TEXT("gddiag") ) ? 1 : 0;
	}
	return GDisGDDiag != 0;
}

static AActor* DisGDFindActor( const FString& Name )
{
	for( FActorIterator It; It; ++It )
	{
		if( It->GetName() == Name )
		{
			return *It;
		}
	}
	return NULL;
}

static void DisGDFaceActor( ADishonoredPlayerController* PC, FLOAT Now )
{
	static TArray<FString> Orders;
	static INT Parsed = -1;
	if( Parsed < 0 )
	{
		Parsed = 0;
		FString Value;
		if( Parse( appCmdLine(), TEXT("gdface="), Value, FALSE ) )
		{
			Value.ParseIntoArray( &Orders, TEXT(","), TRUE );
		}
	}
	APawn* Pawn = PC->Pawn;
	for( INT i = 0; i < Orders.Num(); i++ )
	{
		TArray<FString> Parts;
		Orders(i).ParseIntoArray( &Parts, TEXT("@"), TRUE );
		if( Parts.Num() < 2 || appAtof( *Parts(1) ) > Now )
		{
			continue;
		}
		const FString Name = Parts(0);
		const FLOAT Dist = Parts.Num() > 2 ? appAtof( *Parts(2) ) : 150.f;
		Orders.Remove( i-- );
		AActor* Target = DisGDFindActor( Name );
		if( !Target || !Pawn || !GWorld )
		{
			debugf( TEXT("DISHONORED(bringup): gddiag face t=%.2f: no actor named %s (pawn %s)"), Now, *Name, Pawn ? *Pawn->GetName() : TEXT("none") );
			continue;
		}
		FVector Forward = Target->Rotation.Vector();
		Forward.Z = 0.f;
		Forward = Forward.SafeNormal();
		FVector Dest = Target->Location + Forward * Dist;
		Dest.Z = Target->Location.Z;
		if( Pawn->CylinderComponent && Target->CollisionComponent )
		{
			Dest.Z = Target->CollisionComponent->Bounds.Origin.Z - Target->CollisionComponent->Bounds.BoxExtent.Z + Pawn->CylinderComponent->CollisionHeight + 2.f;
		}
		FRotator Facing = ( Target->Location - Dest ).Rotation();
		Facing.Pitch = 0;
		Facing.Roll = 0;
		const UBOOL bMoved = GWorld->FarMoveActor( Pawn, Dest, FALSE, TRUE, FALSE );
		Pawn->SetRotation( Facing );
		PC->SetRotation( Facing );
		debugf( TEXT("DISHONORED(bringup): gddiag face t=%.2f -> %s (%s) at %s: pawn moved %d to %s facing yaw %d (target yaw %d)"),
			Now, *Target->GetName(), *Target->GetClass()->GetName(), *Target->Location.ToString(), (INT)bMoved, *Pawn->Location.ToString(), Facing.Yaw, Target->Rotation.Yaw );
	}
}

static void DisGDUseButton( ADishonoredPlayerController* PC, FLOAT Now )
{
	static FLOAT PressAt = -1.f;
	static FLOAT Hold = 0.5f;
	static INT Parsed = -1;
	static UBOOL bHolding = FALSE;
	if( Parsed < 0 )
	{
		Parsed = 0;
		FString Value;
		if( Parse( appCmdLine(), TEXT("gduse="), Value, FALSE ) )
		{
			TArray<FString> Parts;
			Value.ParseIntoArray( &Parts, TEXT("@"), TRUE );
			if( Parts.Num() > 0 )
			{
				PressAt = appAtof( *Parts(0) );
			}
			if( Parts.Num() > 1 )
			{
				Hold = appAtof( *Parts(1) );
			}
		}
	}
	if( PressAt < 0.f )
	{
		return;
	}
	if( !bHolding && Now >= PressAt )
	{
		bHolding = TRUE;
		PC->m_bUseButton = 1;
		debugf( TEXT("DISHONORED(bringup): gddiag use t=%.2f: m_bUseButton raised for %.2f s (crosshair %s status %d canInteract %d)"), Now, Hold,
			PC->m_pCrosshairActor ? *PC->m_pCrosshairActor->GetName() : TEXT("none"), (INT)PC->m_CrosshairStatus, (INT)PC->m_bCanInteractWithCrosshairActor );
	}
	else if( bHolding && Now >= PressAt + Hold )
	{
		PC->m_bUseButton = 0;
		PressAt = -1.f;
		debugf( TEXT("DISHONORED(bringup): gddiag use t=%.2f: m_bUseButton released"), Now );
	}
}

void DisGDReport( ADishonoredPlayerController* PC )
{
	if( !PC || !GWorld || !GWorld->GetWorldInfo() )
	{
		return;
	}
	const FLOAT Now = GWorld->GetWorldInfo()->TimeSeconds;
	DisGDFaceActor( PC, Now );
	DisGDUseButton( PC, Now );
	if( !DisGDDiagEnabled() )
	{
		return;
	}
	static FLOAT NextAt = 0.f;
	if( Now < NextAt )
	{
		return;
	}
	NextAt = Now + 1.f;
	UDishonoredNativeState* State = PC->m_pUseInteractionFSM ? PC->m_pUseInteractionFSM->GetCurrentlyActiveState() : NULL;
	debugf( TEXT("DISHONORED(bringup): gddiag t=%.2f use button %d useInput %d fsm %s state %s crosshair %s (%s) status %d canInteract %d pawn %s yaw %d"),
		Now, (INT)PC->m_bUseButton, (INT)PC->IsInputEnabled( 0x80 ), PC->m_pUseInteractionFSM ? TEXT("yes") : TEXT("none"), State ? *State->GetClass()->GetName() : TEXT("none"),
		PC->m_pCrosshairActor ? *PC->m_pCrosshairActor->GetName() : TEXT("none"), PC->m_pCrosshairActor ? *PC->m_pCrosshairActor->GetClass()->GetName() : TEXT("-"),
		(INT)PC->m_CrosshairStatus, (INT)PC->m_bCanInteractWithCrosshairActor,
		PC->Pawn ? *PC->Pawn->Location.ToString() : TEXT("none"), PC->Pawn ? PC->Pawn->Rotation.Yaw : 0 );
}
