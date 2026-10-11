/*-----------------------------------------------------------------------------
	Agent FO (PHASE16 package FO): -foupstream=<substring>[,<substring>], the Kismet upstream of an op.

	Walks every USequenceOp in the loaded world and prints, for each op whose path contains the
	substring, the ops whose output links feed it, then theirs, breadth first, to -foupstreamdepth
	(default 8) levels. Crosses the three indirections a cooked sequence uses: a SeqEvent_SequenceActivated
	is fed by whatever links into its owning USequence's matching input, a SeqEvent_RemoteEvent by every
	SeqAct_ActivateRemoteEvent with the same EventName in any level, and a USequence op's own output is
	fed by the SeqAct_FinishSequence inside it with the same name. Each line carries the class, the
	activation count, the link taken, its delay and disabled bit, and the state that decides whether the
	op can pass an impulse on (a Gate's bOpen, an event's bEnabled and trigger count).

	Prints on each -fkdiag pass. Nothing when the switch is absent.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

static INT GDisFOUpstream = -1;
static TArray<FString> GDisFOUpstreamPatterns;
static INT GDisFOUpstreamDepth = 8;

static UBOOL DisFOUpstreamEnabled()
{
	if( GDisFOUpstream < 0 )
	{
		FString Value;
		GDisFOUpstream = Parse( appCmdLine(), TEXT("foupstream="), Value ) ? 1 : 0;
		if( GDisFOUpstream )
		{
			Value.ParseIntoArray( &GDisFOUpstreamPatterns, TEXT(","), TRUE );
			Parse( appCmdLine(), TEXT("foupstreamdepth="), GDisFOUpstreamDepth );
		}
	}
	return GDisFOUpstream != 0;
}

static void DisFOCollectOps( USequence* Seq, INT Depth, TArray<USequenceOp*>& Ops )
{
	if( Seq == NULL || Depth > 12 )
	{
		return;
	}
	for( INT Index = 0; Index < Seq->SequenceObjects.Num(); Index++ )
	{
		USequenceOp* Op = Cast<USequenceOp>( Seq->SequenceObjects( Index ) );
		if( Op == NULL )
		{
			continue;
		}
		Ops.AddItem( Op );
		USequence* Sub = Cast<USequence>( Op );
		if( Sub != NULL )
		{
			DisFOCollectOps( Sub, Depth + 1, Ops );
		}
	}
}

static FString DisFOOpState( USequenceOp* Op )
{
	FString State;
	USequenceEvent* Event = Cast<USequenceEvent>( Op );
	if( Event != NULL )
	{
		State += FString::Printf( TEXT(" event enabled %d triggers %d/%d retrigger %.2f originator %s"),
			(INT)Event->bEnabled, Event->TriggerCount, Event->MaxTriggerCount, Event->ReTriggerDelay,
			Event->Originator != NULL ? *Event->Originator->GetName() : TEXT("none") );
	}
	USeqAct_Gate* Gate = Cast<USeqAct_Gate>( Op );
	if( Gate != NULL )
	{
		State += FString::Printf( TEXT(" gate open %d autoClose %d/%d"), (INT)Gate->bOpen, Gate->CurrentCloseCount, Gate->AutoCloseCount );
	}
	USeqAct_Delay* Delay = Cast<USeqAct_Delay>( Op );
	if( Delay != NULL )
	{
		State += FString::Printf( TEXT(" delay %.2f remaining %.2f"), Delay->Duration, Delay->RemainingTime );
	}
	USeqAct_ActivateRemoteEvent* Remote = Cast<USeqAct_ActivateRemoteEvent>( Op );
	if( Remote != NULL )
	{
		State += FString::Printf( TEXT(" remoteEvent '%s'"), *Remote->EventName.ToString() );
	}
	USeqEvent_RemoteEvent* RemoteEvent = Cast<USeqEvent_RemoteEvent>( Op );
	if( RemoteEvent != NULL )
	{
		State += FString::Printf( TEXT(" remoteEvent '%s'"), *RemoteEvent->EventName.ToString() );
	}
	USeqEvent_SequenceActivated* SeqActivated = Cast<USeqEvent_SequenceActivated>( Op );
	if( SeqActivated != NULL )
	{
		State += FString::Printf( TEXT(" inputLabel '%s'"), *SeqActivated->InputLabel );
	}
	USeqAct_FinishSequence* Finish = Cast<USeqAct_FinishSequence>( Op );
	if( Finish != NULL )
	{
		State += FString::Printf( TEXT(" outputLabel '%s'"), *Finish->OutputLabel );
	}
	USequence* Seq = Cast<USequence>( Op );
	if( Seq != NULL )
	{
		State += FString::Printf( TEXT(" sequence enabled %d"), (INT)Seq->IsEnabled() );
	}
	if( Cast<USequenceCondition>( Op ) != NULL )
	{
		State += TEXT(" CONDITION");
	}
	INT Impulses = 0;
	for( INT Index = 0; Index < Op->InputLinks.Num(); Index++ )
	{
		Impulses += Op->InputLinks( Index ).bHasImpulse ? 1 : 0;
		Impulses += Op->InputLinks( Index ).QueuedActivations;
	}
	State += FString::Printf( TEXT(" active %d pendingInputs %d"), (INT)Op->bActive, Impulses );
	return State;
}

struct FDisFOFeeder
{
	USequenceOp*	Op;
	FString			How;
};

static void DisFOFindFeeders( USequenceOp* Target, const TArray<USequenceOp*>& Ops, TArray<FDisFOFeeder>& Feeders )
{
	for( INT OpIndex = 0; OpIndex < Ops.Num(); OpIndex++ )
	{
		USequenceOp* Op = Ops( OpIndex );
		for( INT OutIndex = 0; OutIndex < Op->OutputLinks.Num(); OutIndex++ )
		{
			const FSeqOpOutputLink& Link = Op->OutputLinks( OutIndex );
			for( INT LinkIndex = 0; LinkIndex < Link.Links.Num(); LinkIndex++ )
			{
				if( Link.Links( LinkIndex ).LinkedOp == Target )
				{
					FDisFOFeeder Feeder;
					Feeder.Op = Op;
					Feeder.How = FString::Printf( TEXT("output %d '%s' -> input %d, delay %.2f, disabled %d, impulse %d"),
						OutIndex, *Link.LinkDesc, Link.Links( LinkIndex ).InputLinkIdx, Link.ActivateDelay, (INT)Link.bDisabled, (INT)Link.bHasImpulse );
					Feeders.AddItem( Feeder );
				}
			}
		}
	}

	USeqEvent_SequenceActivated* SeqActivated = Cast<USeqEvent_SequenceActivated>( Target );
	if( SeqActivated != NULL )
	{
		USequence* Owner = Cast<USequence>( SeqActivated->GetOuter() );
		if( Owner != NULL )
		{
			INT InputIndex = INDEX_NONE;
			for( INT Index = 0; Index < Owner->InputLinks.Num(); Index++ )
			{
				if( Owner->InputLinks( Index ).LinkDesc == SeqActivated->InputLabel )
				{
					InputIndex = Index;
					break;
				}
			}
			FDisFOFeeder Feeder;
			Feeder.Op = Owner;
			Feeder.How = FString::Printf( TEXT("owning sequence input %d '%s'"), InputIndex, *SeqActivated->InputLabel );
			Feeders.AddItem( Feeder );
		}
	}

	USeqEvent_RemoteEvent* RemoteEvent = Cast<USeqEvent_RemoteEvent>( Target );
	if( RemoteEvent != NULL )
	{
		for( INT OpIndex = 0; OpIndex < Ops.Num(); OpIndex++ )
		{
			USeqAct_ActivateRemoteEvent* Remote = Cast<USeqAct_ActivateRemoteEvent>( Ops( OpIndex ) );
			if( Remote != NULL && Remote->EventName == RemoteEvent->EventName )
			{
				FDisFOFeeder Feeder;
				Feeder.Op = Remote;
				Feeder.How = FString::Printf( TEXT("remote event '%s'"), *Remote->EventName.ToString() );
				Feeders.AddItem( Feeder );
			}
		}
	}

	USequence* Seq = Cast<USequence>( Target );
	if( Seq != NULL )
	{
		for( INT Index = 0; Index < Seq->SequenceObjects.Num(); Index++ )
		{
			USeqAct_FinishSequence* Finish = Cast<USeqAct_FinishSequence>( Seq->SequenceObjects( Index ) );
			if( Finish != NULL )
			{
				FDisFOFeeder Feeder;
				Feeder.Op = Finish;
				Feeder.How = FString::Printf( TEXT("finish sequence '%s' inside it"), *Finish->OutputLabel );
				Feeders.AddItem( Feeder );
			}
		}
	}
}

static void DisFODumpLinks( USequenceOp* Target )
{
	debugf( TEXT("DISHONORED(bringup): foupstream   object %p outer %s parent %s comment '%s'"),
		Target, Target->GetOuter() != NULL ? *Target->GetOuter()->GetPathName() : TEXT("none"),
		Target->ParentSequence != NULL ? *Target->ParentSequence->GetPathName() : TEXT("none"), *Target->ObjComment );
	for( INT Index = 0; Index < Target->InputLinks.Num(); Index++ )
	{
		const FSeqOpInputLink& Link = Target->InputLinks( Index );
		debugf( TEXT("DISHONORED(bringup): foupstream   input %d '%s' impulse %d queued %d disabled %d linkedOp %s"),
			Index, *Link.LinkDesc, (INT)Link.bHasImpulse, Link.QueuedActivations, (INT)Link.bDisabled,
			Link.LinkedOp != NULL ? *Link.LinkedOp->GetName() : TEXT("none") );
	}
	for( INT Index = 0; Index < Target->OutputLinks.Num(); Index++ )
	{
		const FSeqOpOutputLink& Link = Target->OutputLinks( Index );
		FString Targets;
		for( INT LinkIndex = 0; LinkIndex < Link.Links.Num(); LinkIndex++ )
		{
			Targets += FString::Printf( TEXT("%s%s:%d"), Targets.Len() ? TEXT(", ") : TEXT(""),
				Link.Links( LinkIndex ).LinkedOp != NULL ? *Link.Links( LinkIndex ).LinkedOp->GetName() : TEXT("none"), Link.Links( LinkIndex ).InputLinkIdx );
		}
		debugf( TEXT("DISHONORED(bringup): foupstream   output %d '%s' -> %d link(s): %s"), Index, *Link.LinkDesc, Link.Links.Num(), *Targets );
	}
	for( INT Index = 0; Index < Target->VariableLinks.Num(); Index++ )
	{
		const FSeqVarLink& Link = Target->VariableLinks( Index );
		FString Vars;
		for( INT VarIndex = 0; VarIndex < Link.LinkedVariables.Num(); VarIndex++ )
		{
			USequenceVariable* Var = Link.LinkedVariables( VarIndex );
			USeqVar_Object* ObjVar = Cast<USeqVar_Object>( Var );
			Vars += FString::Printf( TEXT("%s%s(%s%s%s)"), Vars.Len() ? TEXT(", ") : TEXT(""),
				Var != NULL ? *Var->GetName() : TEXT("none"), Var != NULL ? *Var->GetClass()->GetName() : TEXT(""),
				ObjVar != NULL ? TEXT(" = ") : TEXT(""),
				ObjVar != NULL ? ( ObjVar->ObjValue != NULL ? *ObjVar->ObjValue->GetName() : TEXT("NULL") ) : TEXT("") );
		}
		debugf( TEXT("DISHONORED(bringup): foupstream   var %d '%s' prop %s: %s"), Index, *Link.LinkDesc, *Link.PropertyName.ToString(), Vars.Len() ? *Vars : TEXT("none") );
	}
	for( INT Index = 0; Index < Target->EventLinks.Num(); Index++ )
	{
		debugf( TEXT("DISHONORED(bringup): foupstream   eventlink %d '%s' %d event(s)"), Index, *Target->EventLinks( Index ).LinkDesc, Target->EventLinks( Index ).LinkedEvents.Num() );
	}
}

static void DisFODumpReferencers( UObject* Target )
{
	INT Referencers = 0;
	for( TObjectIterator<UObject> It; It; ++It )
	{
		UObject* Object = *It;
		if( Object == Target || Object->IsA( UField::StaticClass() ) || Object->IsA( UPackage::StaticClass() ) )
		{
			continue;
		}
		FArchiveFindCulprit Ar( Target, Object, FALSE );
		TArray<UProperty*> Properties;
		if( Ar.GetCount( Properties ) == 0 )
		{
			continue;
		}
		Referencers++;
		FString Props;
		for( INT Index = 0; Index < Properties.Num(); Index++ )
		{
			Props += FString::Printf( TEXT("%s%s"), Props.Len() ? TEXT(", ") : TEXT(""), *Properties( Index )->GetName() );
		}
		debugf( TEXT("DISHONORED(bringup): foupstream   referenced by %s (%s) x%d via %s"),
			*Object->GetPathName(), *Object->GetClass()->GetName(), Ar.GetCount(), Props.Len() ? *Props : TEXT("(native)") );
		if( Referencers >= 40 )
		{
			debugf( TEXT("DISHONORED(bringup): foupstream   ... more referencers not listed") );
			break;
		}
	}
	debugf( TEXT("DISHONORED(bringup): foupstream   %d referencer(s) of %s"), Referencers, *Target->GetName() );
}

static void DisFOReportUpstreamOf( USequenceOp* Target, const TArray<USequenceOp*>& Ops )
{
	debugf( TEXT("DISHONORED(bringup): foupstream target %s (%s) activations %d%s"),
		*Target->GetPathName(), *Target->GetClass()->GetName(), Target->ActivateCount, *DisFOOpState( Target ) );
	DisFODumpLinks( Target );
	if( ParseParam( appCmdLine(), TEXT("foupstreamrefs") ) )
	{
		DisFODumpReferencers( Target );
	}

	TArray<USequenceOp*> Visited;
	TArray<USequenceOp*> Frontier;
	Visited.AddItem( Target );
	Frontier.AddItem( Target );
	INT Lines = 0;
	for( INT Depth = 1; Depth <= GDisFOUpstreamDepth && Frontier.Num() > 0 && Lines < 400; Depth++ )
	{
		TArray<USequenceOp*> Next;
		for( INT FrontIndex = 0; FrontIndex < Frontier.Num(); FrontIndex++ )
		{
			USequenceOp* Node = Frontier( FrontIndex );
			TArray<FDisFOFeeder> Feeders;
			DisFOFindFeeders( Node, Ops, Feeders );
			if( Feeders.Num() == 0 )
			{
				debugf( TEXT("DISHONORED(bringup): foupstream   depth %d: %s has NO feeder"), Depth, *Node->GetName() );
				Lines++;
			}
			for( INT FeedIndex = 0; FeedIndex < Feeders.Num(); FeedIndex++ )
			{
				USequenceOp* Feeder = Feeders( FeedIndex ).Op;
				const UBOOL bSeen = Visited.ContainsItem( Feeder );
				debugf( TEXT("DISHONORED(bringup): foupstream   depth %d: %s <- %s (%s) activations %d via %s%s%s"),
					Depth, *Node->GetName(), *Feeder->GetPathName(), *Feeder->GetClass()->GetName(), Feeder->ActivateCount,
					*Feeders( FeedIndex ).How, *DisFOOpState( Feeder ), bSeen ? TEXT(" [seen]") : TEXT("") );
				Lines++;
				if( !bSeen )
				{
					Visited.AddItem( Feeder );
					Next.AddItem( Feeder );
				}
			}
		}
		Frontier = Next;
	}
}

static UBOOL DisFOGuidIsZero( const FGuid& Guid )
{
	return Guid.A == 0 && Guid.B == 0 && Guid.C == 0 && Guid.D == 0;
}

/** -fosoiree: every UDisConv_Soiree node in memory and the USeqAct_Interp whose m_MatineeGUID (retail SDK @472) equals the
    node's m_SoireeGUID, which is how retail's UDisConv_Soiree::FindMatinee (2012 0x900d80, 2013 0x8b5de0) reaches a cutscene
    that has no Kismet input link */
static void DisFOReportSoireeNodes()
{
	if( !ParseParam( appCmdLine(), TEXT("fosoiree") ) || GWorld == NULL )
	{
		return;
	}
	TArray<USeqAct_Interp*> Named;
	INT Nodes = 0, Matched = 0;
	for( TObjectIterator<UDisConv_Soiree> It; It; ++It )
	{
		UDisConv_Soiree* Node = *It;
		if( Node->IsTemplate() )
		{
			continue;
		}
		Nodes++;
		USeqAct_Interp* Found = NULL;
		for( TObjectIterator<USeqAct_Interp> InterpIt; InterpIt; ++InterpIt )
		{
			if( !InterpIt->IsTemplate() && InterpIt->m_MatineeGUID == Node->m_SoireeGUID && !DisFOGuidIsZero( Node->m_SoireeGUID ) )
			{
				Found = *InterpIt;
				break;
			}
		}
		if( Found != NULL )
		{
			Matched++;
			Named.AddUniqueItem( Found );
		}
		debugf( TEXT("DISHONORED(bringup): fosoiree node %s soiree '%s' guid %s preview %d -> matinee %s (activations %d)"),
			*Node->GetPathName(), *Node->m_SoireeName.ToString(), *Node->m_SoireeGUID.String(), (INT)Node->m_UsePreviewBlurbsOnly,
			Found != NULL ? *Found->GetPathName() : TEXT("NONE"), Found != NULL ? Found->ActivateCount : -1 );
	}
	INT Orphans = 0;
	for( TObjectIterator<USeqAct_Interp> InterpIt; InterpIt; ++InterpIt )
	{
		if( !InterpIt->IsTemplate() && !DisFOGuidIsZero( InterpIt->m_MatineeGUID ) && !Named.ContainsItem( *InterpIt ) )
		{
			Orphans++;
			debugf( TEXT("DISHONORED(bringup): fosoiree matinee %s carries matinee guid %s and no loaded soiree node names it"),
				*InterpIt->GetPathName(), *InterpIt->m_MatineeGUID.String() );
		}
	}
	debugf( TEXT("DISHONORED(bringup): fosoiree: %d soiree node(s), %d matched to a matinee, %d matinee(s) with a guid and no node"), Nodes, Matched, Orphans );
}

void DisFOReportUpstream()
{
	DisFOReportSoireeNodes();
	if( !DisFOUpstreamEnabled() || GWorld == NULL )
	{
		return;
	}
	TArray<USequenceOp*> Ops;
	for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels( LevelIndex );
		for( INT SeqIndex = 0; Level != NULL && SeqIndex < Level->GameSequences.Num(); SeqIndex++ )
		{
			DisFOCollectOps( Level->GameSequences( SeqIndex ), 0, Ops );
		}
	}
	INT Targets = 0;
	for( INT OpIndex = 0; OpIndex < Ops.Num(); OpIndex++ )
	{
		const FString Path = Ops( OpIndex )->GetPathName();
		for( INT PatternIndex = 0; PatternIndex < GDisFOUpstreamPatterns.Num(); PatternIndex++ )
		{
			if( Path.InStr( GDisFOUpstreamPatterns( PatternIndex ), FALSE, TRUE ) != INDEX_NONE )
			{
				Targets++;
				DisFOReportUpstreamOf( Ops( OpIndex ), Ops );
				break;
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): foupstream: %d ops in %d levels, %d target(s) matched"), Ops.Num(), GWorld->Levels.Num(), Targets );
}
