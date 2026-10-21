/*=============================================================================
	jhdiag.cpp: agent JH (PHASE24 JH) - the -jhdiag measurements of the interaction prompt and the hover glow (Inc/jhdiag.h).
	Measurement only.
=============================================================================*/
#include "DishonoredGame.h"
#include "jhdiag.h"

static INT GDisJHDiag = -1;

UBOOL DisJHDiagEnabled()
{
	if( GDisJHDiag < 0 )
	{
		GDisJHDiag = ParseParam( appCmdLine(), TEXT("jhdiag") ) ? 1 : 0;
	}
	return GDisJHDiag != 0;
}

static FLOAT DisJHNow()
{
	return ( GWorld && GWorld->GetWorldInfo() ) ? GWorld->GetWorldInfo()->TimeSeconds : 0.f;
}

void DisJHLogHighlight( IDisInteractableInterface* _pInteractable, const TCHAR* _What, INT _State )
{
	if( !DisJHDiagEnabled() || !_pInteractable )
	{
		return;
	}
	UObject* Owner = _pInteractable->GetUObjectInterfaceDisInteractableInterface();
	INT* Flags = _pInteractable->GetHighlightFlags();
	const UDisTweaks_InteractableInterface& Tweaks = _pInteractable->GetInteractableTweaks();
	debugf( TEXT("DISHONORED(bringup): jhdiag highlight frame %d t=%.3f %s %s (%s): state %d flags %d allowed %d material %s"),
		(INT)GFrameCounter, DisJHNow(), _What, Owner ? *Owner->GetName() : TEXT("?"), Owner ? *Owner->GetClass()->GetName() : TEXT("-"),
		_State, Flags ? *Flags : -1, (INT)Tweaks.m_bAllowedToHighlight, Tweaks.m_HighLightMaterial ? *Tweaks.m_HighLightMaterial->GetPathName() : TEXT("none") );
}

void DisJHLogInvoke( const TCHAR* _Clip, const TCHAR* _Method, const FString& _Args )
{
	if( DisJHDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): jhdiag invoke frame %d t=%.3f %s.%s(%s)"), (INT)GFrameCounter, DisJHNow(), _Clip, _Method, *_Args );
	}
}

void DisJHLogFocus( AActor* _pOld, AActor* _pNew, INT _Status, UBOOL _bCanInteract )
{
	if( DisJHDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): jhdiag focus frame %d t=%.3f %s -> %s (%s) status %d canInteract %d"), (INT)GFrameCounter, DisJHNow(),
			_pOld ? *_pOld->GetName() : TEXT("none"), _pNew ? *_pNew->GetName() : TEXT("none"), _pNew ? *_pNew->GetClass()->GetName() : TEXT("-"),
			_Status, (INT)_bCanInteract );
	}
}

void DisJHLogContext( const FDisUIInteractionContext& _rContext, const FString& _rName, const FString& _rFocusText )
{
	static FLOAT NextAt = -1.f;
	const FLOAT Now = DisJHNow();
	if( !DisJHDiagEnabled() || ( Now < NextAt && Now > NextAt - 2.f ) )
	{
		return;
	}
	NextAt = Now + 1.f;
	FString Groups;
	for( INT Group = 0; Group < DUIG_MAX; Group++ )
	{
		const FDisUIInteractionGroup& Entries = _rContext.m_Groups[Group];
		Groups += FString::Printf( TEXT(" [%d:"), Group );
		for( INT Index = 0; Index < Entries.m_InteractionCount && Index < ARRAY_COUNT(Entries.m_Interactions); Index++ )
		{
			const FString* Text = (const FString*)Entries.m_Interactions[Index].m_pText;
			Groups += FString::Printf( TEXT(" %d '%s'"), (INT)Entries.m_Interactions[Index].m_Index, Text ? **Text : TEXT("") );
		}
		Groups += TEXT("]");
	}
	debugf( TEXT("DISHONORED(bringup): jhdiag fill t=%.3f name '%s' focus '%s'%s"), Now, *_rName, *_rFocusText, *Groups );
}

static void DisJHCensus()
{
	static UWorld* CensusWorld = NULL;
	if( !GWorld || CensusWorld == GWorld )
	{
		return;
	}
	CensusWorld = GWorld;
	INT Count = 0;
	for( FActorIterator It; It; ++It )
	{
		IDisInteractableInterface* Interactable = InterfaceCast<IDisInteractableInterface>( *It );
		if( !Interactable )
		{
			continue;
		}
		Count++;
		const UDisTweaks_InteractableInterface& Tweaks = Interactable->GetInteractableTweaks();
		debugf( TEXT("DISHONORED(bringup): jhdiag census %s (%s) at %s: name '%s' interact '%s' alt '%s' focus '%s' highlight %d %s"),
			*It->GetName(), *It->GetClass()->GetName(), *It->Location.ToString(), *Interactable->GetInteractableName(), *Tweaks.m_InteractText,
			*Tweaks.m_AltInteractText, *Interactable->GetCrosshairFocusText(), (INT)Tweaks.m_bAllowedToHighlight,
			Tweaks.m_HighLightMaterial ? *Tweaks.m_HighLightMaterial->GetName() : TEXT("none") );
	}
	debugf( TEXT("DISHONORED(bringup): jhdiag census %d interactables on %s"), Count, *GWorld->GetOutermost()->GetName() );
}

static AActor* DisJHFindTarget( const FString& _rSpec, const FVector& _rFrom )
{
	const UBOOL bByClass = _rSpec.StartsWith( TEXT("class:") );
	const FString Wanted = bByClass ? _rSpec.Mid( 6 ) : _rSpec;
	AActor* Best = NULL;
	FLOAT BestDistSq = BIG_NUMBER;
	for( FActorIterator It; It; ++It )
	{
		if( !bByClass )
		{
			if( It->GetName() == Wanted )
			{
				return *It;
			}
			continue;
		}
		UBOOL bMatch = FALSE;
		for( UClass* Class = It->GetClass(); Class && !bMatch; Class = Class->GetSuperClass() )
		{
			bMatch = Class->GetName() == Wanted;
		}
		const FLOAT DistSq = ( It->Location - _rFrom ).SizeSquared();
		if( bMatch && !It->bDeleteMe && !It->bHidden && DistSq < BestDistSq )
		{
			Best = *It;
			BestDistSq = DistSq;
		}
	}
	return Best;
}

static void DisJHFace( ADishonoredPlayerController* _pPC, FLOAT _Now )
{
	static TArray<FString> Orders;
	static INT Parsed = -1;
	if( Parsed < 0 )
	{
		Parsed = 0;
		FString Value;
		if( Parse( appCmdLine(), TEXT("jhface="), Value, FALSE ) )
		{
			Value.ParseIntoArray( &Orders, TEXT(","), TRUE );
		}
	}
	APawn* Pawn = _pPC->Pawn;
	for( INT i = 0; i < Orders.Num(); i++ )
	{
		TArray<FString> Parts;
		Orders(i).ParseIntoArray( &Parts, TEXT("@"), TRUE );
		if( Parts.Num() < 2 || appAtof( *Parts(1) ) > _Now )
		{
			continue;
		}
		const FString Spec = Parts(0);
		const FLOAT Dist = Parts.Num() > 2 ? appAtof( *Parts(2) ) : 120.f;
		Orders.Remove( i-- );
		AActor* Target = Pawn ? DisJHFindTarget( Spec, Pawn->Location ) : NULL;
		if( !Target || !GWorld )
		{
			debugf( TEXT("DISHONORED(bringup): jhdiag face t=%.2f: no actor for %s"), _Now, *Spec );
			continue;
		}
		const FVector Centre = Target->CollisionComponent ? Target->CollisionComponent->Bounds.Origin : Target->Location;
		FVector Away = Pawn->Location - Centre;
		Away.Z = 0.f;
		if( Away.SizeSquared() < 1.f )
		{
			Away = Target->Rotation.Vector();
			Away.Z = 0.f;
		}
		Away = Away.SafeNormal();
		FVector Dest = Centre + Away * Dist;
		if( Target->CollisionComponent && Pawn->CylinderComponent )
		{
			Dest.Z = Target->CollisionComponent->Bounds.Origin.Z - Target->CollisionComponent->Bounds.BoxExtent.Z + Pawn->CylinderComponent->CollisionHeight + 2.f;
		}
		const UBOOL bMoved = GWorld->FarMoveActor( Pawn, Dest, FALSE, TRUE, FALSE );
		const FVector Eye = Pawn->Location + FVector( 0.f, 0.f, Pawn->BaseEyeHeight );
		FRotator Facing = ( Centre - Eye ).Rotation();
		Facing.Roll = 0;
		FRotator PawnFacing = Facing;
		PawnFacing.Pitch = 0;
		Pawn->SetRotation( PawnFacing );
		_pPC->SetRotation( Facing );
		debugf( TEXT("DISHONORED(bringup): jhdiag face t=%.2f -> %s (%s) centre %s: pawn moved %d to %s, view yaw %d pitch %d"),
			_Now, *Target->GetName(), *Target->GetClass()->GetName(), *Centre.ToString(), (INT)bMoved, *Pawn->Location.ToString(), Facing.Yaw, Facing.Pitch );
	}
}

void DisJHReport( ADishonoredPlayerController* _pPC )
{
	if( !_pPC || !GWorld || !GWorld->GetWorldInfo() )
	{
		return;
	}
	DisJHFace( _pPC, DisJHNow() );
	if( DisJHDiagEnabled() && _pPC->Pawn )
	{
		DisJHCensus();
	}
}
