/*=============================================================================
	gediag.cpp: agent GE (PHASE19 GE) - see gediag.h. Diagnostics only, every line under a switch.
=============================================================================*/
#include "DishonoredGame.h"
#include "gediag.h"

extern INT DisEnginePhysXStaticShapes();	// Engine/Src/UnPhysLevel.cpp (agent GE): the primary scene's static shape count, -1 without a scene

static INT GDisGEDiag = -1;
static INT GDisGEDeathDiag = -1;
static INT GDisGELinks = -1;
static TArray<FString> GDisGELinkPatterns;

UBOOL DisGEDiagEnabled()
{
	if( GDisGEDiag < 0 )
	{
		GDisGEDiag = ParseParam( appCmdLine(), TEXT("gediag") ) ? 1 : 0;
	}
	return GDisGEDiag != 0;
}

UBOOL DisGEDeathDiagEnabled()
{
	if( GDisGEDeathDiag < 0 )
	{
		GDisGEDeathDiag = ParseParam( appCmdLine(), TEXT("gedeathdiag") ) ? 1 : 0;
	}
	return GDisGEDeathDiag != 0;
}

static UBOOL DisGELinksEnabled()
{
	if( GDisGELinks < 0 )
	{
		FString Value;
		GDisGELinks = Parse( appCmdLine(), TEXT("gelinks="), Value, FALSE ) ? 1 : 0;
		if( GDisGELinks )
		{
			Value.ParseIntoArray( &GDisGELinkPatterns, TEXT(","), TRUE );
		}
	}
	return GDisGELinks != 0;
}

void DisGEReport( APlayerController* PC )
{
	static FLOAT NextAt = 0.f;
	if( !DisGEDiagEnabled() || GWorld == NULL || PC == NULL )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < NextAt )
	{
		return;
	}
	NextAt = Now + 1.f;
	ACamera* Camera = PC->PlayerCamera;
	if( Camera == NULL )
	{
		debugf( TEXT("DISHONORED(bringup): gediag fade t=%.2f no PlayerCamera"), Now );
	}
	else
	{
		debugf( TEXT("DISHONORED(bringup): gediag fade t=%.2f enable %d amount %.3f alpha %.2f..%.2f time %.2f remaining %.2f color %d/%d/%d/%d audio %d camera %s"),
			Now, Camera->bEnableFading ? 1 : 0, Camera->FadeAmount, Camera->FadeAlpha.X, Camera->FadeAlpha.Y, Camera->FadeTime, Camera->FadeTimeRemaining,
			(INT)Camera->FadeColor.R, (INT)Camera->FadeColor.G, (INT)Camera->FadeColor.B, (INT)Camera->FadeColor.A, Camera->bFadeAudio ? 1 : 0, *Camera->GetName() );
	}
	for( TObjectIterator<USeqAct_CameraFade> It; It; ++It )
	{
		USeqAct_CameraFade* Fade = *It;
		if( Fade->IsTemplate() || !Fade->bActive )
		{
			continue;
		}
		debugf( TEXT("DISHONORED(bringup): gediag fade t=%.2f active %s opacity %.2f time %.2f remaining %.2f persist %d audio %d color %d/%d/%d/%d deprecatedAlpha %.2f..%.2f objVersion %d cachedPCs %d"),
			Now, *Fade->GetPathName(), Fade->FadeOpacity, Fade->FadeTime, Fade->FadeTimeRemaining, Fade->bPersistFade ? 1 : 0, Fade->bFadeAudio ? 1 : 0,
			(INT)Fade->FadeColor.R, (INT)Fade->FadeColor.G, (INT)Fade->FadeColor.B, (INT)Fade->FadeColor.A, Fade->FadeAlpha_DEPRECATED.X, Fade->FadeAlpha_DEPRECATED.Y,
			Fade->ObjInstanceVersion, Fade->CachedPCs.Num() );
	}
}

static void DisGEDescribeOp( USequenceOp* Op )
{
	debugf( TEXT("DISHONORED(bringup): gelinks op %s (%s) activations %d active %d inputs %d outputs %d vars %d"), *Op->GetPathName(), *Op->GetClass()->GetName(),
		Op->ActivateCount, Op->bActive ? 1 : 0, Op->InputLinks.Num(), Op->OutputLinks.Num(), Op->VariableLinks.Num() );
	for( INT i = 0; i < Op->OutputLinks.Num(); i++ )
	{
		const FSeqOpOutputLink& Link = Op->OutputLinks(i);
		FString Targets;
		for( INT j = 0; j < Link.Links.Num(); j++ )
		{
			const FSeqOpOutputInputLink& To = Link.Links(j);
			Targets += FString::Printf( TEXT("%s%s:%d"), Targets.Len() ? TEXT(", ") : TEXT(""), To.LinkedOp ? *To.LinkedOp->GetName() : TEXT("<null>"), To.InputLinkIdx );
		}
		debugf( TEXT("DISHONORED(bringup): gelinks   output %d '%s' impulse %d disabled %d delay %.2f -> %d link(s): %s"), i, *Link.LinkDesc, Link.bHasImpulse ? 1 : 0, Link.bDisabled ? 1 : 0,
			Link.ActivateDelay, Link.Links.Num(), *Targets );
	}
	for( INT i = 0; i < Op->VariableLinks.Num(); i++ )
	{
		const FSeqVarLink& Var = Op->VariableLinks(i);
		FString Values;
		for( INT j = 0; j < Var.LinkedVariables.Num(); j++ )
		{
			USeqVar_Object* ObjVar = Cast<USeqVar_Object>( Var.LinkedVariables(j) );
			Values += FString::Printf( TEXT("%s%s=%s"), Values.Len() ? TEXT(", ") : TEXT(""), Var.LinkedVariables(j) ? *Var.LinkedVariables(j)->GetName() : TEXT("<null>"),
				ObjVar ? ( ObjVar->ObjValue ? *ObjVar->ObjValue->GetPathName() : TEXT("NULL") ) : TEXT("?") );
		}
		debugf( TEXT("DISHONORED(bringup): gelinks   var %d '%s' %s"), i, *Var.LinkDesc, *Values );
	}
	UObject* Default = Op->GetClass()->GetDefaultObject();
	INT Printed = 0;
	for( TFieldIterator<UProperty> It( Op->GetClass() ); It && Printed < 40; ++It )
	{
		UProperty* Property = *It;
		if( Property->GetOwnerClass() == USequenceOp::StaticClass() || Property->GetOwnerClass() == USequenceObject::StaticClass() || ( Property->PropertyFlags & CPF_Transient ) )
		{
			continue;
		}
		for( INT Element = 0; Element < Property->ArrayDim; Element++ )
		{
			FString Value;
			if( Property->ExportText( Element, Value, (BYTE*)Op, (BYTE*)Default, Op, PPF_Localized ) )
			{
				debugf( TEXT("DISHONORED(bringup): gelinks   prop %s%s = %s"), *Property->GetName(), Property->ArrayDim > 1 ? *FString::Printf( TEXT("[%d]"), Element ) : TEXT(""), *Value );
				Printed++;
			}
		}
	}
}

void DisGEReportLinks()
{
	static FString DoneForMap;
	if( !DisGELinksEnabled() || GWorld == NULL )
	{
		return;
	}
	const FString Map = GWorld->GetOutermost()->GetName();
	if( Map == DoneForMap )
	{
		return;
	}
	DoneForMap = Map;
	INT Matched = 0;
	for( TObjectIterator<USequenceOp> It; It; ++It )
	{
		USequenceOp* Op = *It;
		if( Op->IsTemplate() )
		{
			continue;
		}
		const FString Path = Op->GetPathName();
		for( INT p = 0; p < GDisGELinkPatterns.Num(); p++ )
		{
			if( Path.InStr( GDisGELinkPatterns(p) ) != INDEX_NONE )
			{
				DisGEDescribeOp( Op );
				Matched++;
				break;
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): gelinks: map %s, %d op(s) matched %d pattern(s)"), *Map, Matched, GDisGELinkPatterns.Num() );
}

void DisGEReportDamage( APawn* Pawn, INT Damage, AController* InstigatedBy, const FVector& HitLocation, const FVector& Momentum, UClass* DamageType, AActor* DamageCauser, INT HealthBefore )
{
	if( !DisGEDeathDiagEnabled() || Pawn == NULL )
	{
		return;
	}
	debugf( TEXT("DISHONORED(bringup): gedeathdiag t=%.2f TakeDamage %s (%s): %d of type %s from instigator %s causer %s at %s momentum %s, health %d -> %d, location %s velocity %s physics %d base %s volume %s touching %d, PhysX static shapes %d"),
		GWorld ? GWorld->GetTimeSeconds() : 0.f, *Pawn->GetName(), *Pawn->GetClass()->GetName(), Damage, DamageType ? *DamageType->GetName() : TEXT("none"),
		InstigatedBy ? *InstigatedBy->GetName() : TEXT("none"), DamageCauser ? *DamageCauser->GetPathName() : TEXT("none"), *HitLocation.ToString(), *Momentum.ToString(),
		HealthBefore, Pawn->Health, *Pawn->Location.ToString(), *Pawn->Velocity.ToString(), (INT)Pawn->Physics, Pawn->Base ? *Pawn->Base->GetName() : TEXT("none"),
		Pawn->PhysicsVolume ? *Pawn->PhysicsVolume->GetName() : TEXT("none"), Pawn->Touching.Num(), DisEnginePhysXStaticShapes() );
}
