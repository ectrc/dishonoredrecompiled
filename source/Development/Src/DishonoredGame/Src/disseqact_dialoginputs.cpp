// DishonoredGame/src/disseqact_dialoginputs.cpp
// DISHONORED(port): agent FR (PHASE17 FR). Kismet's way into a dialog tree. Activated 2013 rva 0x79bf00 (2012 0x7d75c0,
// 578 B, the same body); GetDialogHookInitiator 0x790ac0 (0x7d3720, 254 B). For every Target speaker (an actor carrying
// IDisConvSpeakerInterface, or a controller's pawn), every impulsed input link's name is stamped as the speaker's
// FDisHookParameters::m_KismetDialogHook and DDH_KISMET_ACTIVATED is fired on it with the DialogOneShot variable's actor
// (or its controller's pawn) as the initiator - the speaker itself when that is nothing.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

static IDisConvSpeakerInterface* DisSpeakerFromObject( UObject* Object )
{
	if( !Object )
	{
		return NULL;
	}
	IDisConvSpeakerInterface* Speaker = (IDisConvSpeakerInterface*)Object->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() );
	if( !Speaker )
	{
		AController* Controller = Cast<AController>( Object );
		if( Controller && Controller->Pawn )
		{
			Speaker = (IDisConvSpeakerInterface*)Controller->Pawn->GetInterfaceAddress( UDisConvSpeakerInterface::StaticClass() );
		}
	}
	return Speaker;
}

IDisConvSpeakerInterface* UDisSeqAct_DialogInputs::GetDialogHookInitiator()
{
	if( VariableLinks.Num() <= 1 || VariableLinks(1).LinkedVariables.Num() <= 0 )
	{
		return NULL;
	}
	USeqVar_Object* Var = Cast<USeqVar_Object>( VariableLinks(1).LinkedVariables(0) );
	UObject** Ref = Var ? Var->GetObjectRef( 0 ) : NULL;
	return Ref ? DisSpeakerFromObject( *Ref ) : NULL;
}

void UDisSeqAct_DialogInputs::Activated()
{
	IDisConvSpeakerInterface* Initiator = GetDialogHookInitiator();
	if( DisFRDiagEnabled() )
	{
		FString Impulsed;
		for( INT i = 0; i < InputLinks.Num(); i++ )
		{
			if( InputLinks(i).bHasImpulse )
			{
				Impulsed += FString::Printf( TEXT("'%s' "), *InputLinks(i).LinkDesc );
			}
		}
		FRDIAG( "inputs %s: %d target(s), initiator %s, impulsed %s", *GetPathName(), Targets.Num(),
			Initiator && Initiator->GetUObjectInterfaceDisConvSpeakerInterface() ? *Initiator->GetUObjectInterfaceDisConvSpeakerInterface()->GetName() : TEXT("<none>"), *Impulsed );
		for( INT i = 0; i < VariableLinks.Num(); i++ )
		{
			for( INT j = 0; j < VariableLinks(i).LinkedVariables.Num(); j++ )
			{
				USeqVar_Object* Var = Cast<USeqVar_Object>( VariableLinks(i).LinkedVariables(j) );
				FRDIAG( "inputs    var %d '%s' %s = %s", i, *VariableLinks(i).LinkDesc, VariableLinks(i).LinkedVariables(j) ? *VariableLinks(i).LinkedVariables(j)->GetName() : TEXT("<null>"),
					Var && Var->ObjValue ? *Var->ObjValue->GetPathName() : TEXT("NULL") );
				// Who else links this variable: the op that should have written it in retail.
				if( Var && !Var->ObjValue )
				{
					for( TObjectIterator<USequenceOp> It; It; ++It )
					{
						USequenceOp* Op = *It;
						if( Op == this || Op->IsTemplate() )
						{
							continue;
						}
						for( INT k = 0; k < Op->VariableLinks.Num(); k++ )
						{
							if( Op->VariableLinks(k).LinkedVariables.ContainsItem( Var ) )
							{
								FRDIAG( "inputs       %s is also linked by %s (%s) as '%s' (%s), activations %d", *Var->GetName(), *Op->GetPathName(), *Op->GetClass()->GetName(),
									*Op->VariableLinks(k).LinkDesc, Op->VariableLinks(k).bWriteable ? TEXT("writes it") : TEXT("reads it"), Op->ActivateCount );
							}
						}
					}
				}
			}
		}
	}
	for( INT i = 0; i < Targets.Num(); i++ )
	{
		IDisConvSpeakerInterface* Speaker = DisSpeakerFromObject( Targets(i) );
		if( !Speaker )
		{
			FRDIAG( "inputs %s: target %s is not a speaker", *GetName(), Targets(i) ? *Targets(i)->GetName() : TEXT("<null>") );
			continue;
		}
		FDisHookParameters* Params = Speaker->GetHookParameters();
		if( !Params )
		{
			FRDIAG( "inputs %s: target %s has no hook parameters (no attached conversation component)", *GetName(), *Targets(i)->GetName() );
			continue;
		}
		for( INT j = 0; j < InputLinks.Num(); j++ )
		{
			if( InputLinks(j).bHasImpulse )
			{
				Params->m_KismetDialogHook = FName( *InputLinks(j).LinkDesc, FNAME_Add );
				Speaker->FireDialogHook( DDH_KISMET_ACTIVATED, Initiator ? Initiator : Speaker );
			}
		}
	}
	Super::Activated();
}
