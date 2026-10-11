/*-----------------------------------------------------------------------------
	Agent FK (PHASE15 package FK): -fkdiag, the level's matinee data as the package loaded it.

	-fediag already reports every matinee that is *running*, off its GroupInst array. That array only
	exists after USeqAct_Interp::InitInterp has run, so a matinee that never started reports nothing at
	all and a matinee whose groups could not be instanced reports "director NONE" whether the data has a
	director group or not. Both readings are what this package has to tell apart, so -fkdiag walks the
	UInterpData the package shipped instead of the instances the runtime built:

	  fkdiag interp   - one line per USeqAct_Interp in the loaded world, playing or not
	  fkdiag group    - one line per group in its UInterpData, with the group's CLASS
	  fkdiag track    - one line per track in that group, with the track's class and key count
	  fkdiag cond     - the census of USequenceCondition classes in the loaded sequences, with the
	                    activation count of each, because an unported Activated kills its chain silently
	  fkdiag census   - the totals

	It prints once per map (the data cannot change) plus a condition census every two seconds, and costs
	nothing when the switch is absent.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

static INT GDisFKDiag = -1;

UBOOL DisFKDiagEnabled()
{
	if( GDisFKDiag < 0 )
	{
		GDisFKDiag = ( appStrfind( appCmdLine(), TEXT("-fkdiag") ) != NULL ) ? 1 : 0;
	}
	return GDisFKDiag != 0;
}

static void DisFKWalkSequenceConditions( USequence* Seq, INT Depth, TArray<FString>& OutNames, TArray<INT>& OutCount, TArray<INT>& OutActivations )
{
	if( Seq == NULL || Depth > 10 )
	{
		return;
	}
	for( INT Index = 0; Index < Seq->SequenceObjects.Num(); Index++ )
	{
		USequenceObject* Obj = Seq->SequenceObjects( Index );
		if( Obj == NULL )
		{
			continue;
		}
		USequenceCondition* Cond = Cast<USequenceCondition>( Obj );
		if( Cond != NULL )
		{
			const FString ClassName = Cond->GetClass()->GetName();
			INT Slot = OutNames.FindItemIndex( ClassName );
			if( Slot == INDEX_NONE )
			{
				Slot = OutNames.AddItem( ClassName );
				OutCount.AddItem( 0 );
				OutActivations.AddItem( 0 );
			}
			OutCount( Slot )++;
			OutActivations( Slot ) += Cond->ActivateCount;
		}
		USequence* Sub = Cast<USequence>( Obj );
		if( Sub != NULL )
		{
			DisFKWalkSequenceConditions( Sub, Depth + 1, OutNames, OutCount, OutActivations );
		}
	}
}

void DisFOReportUpstream();

/** the one-shot half: the matinee data of every USeqAct_Interp this world holds, instanced or not */
static void DisFKReportMatineeData()
{
	DisFOReportUpstream();
	INT Interps = 0, GroupsSeen = 0, Tracks = 0, Directors = 0, Players = 0, AIGroups = 0;
	for( TObjectIterator<USeqAct_Interp> It; It; ++It )
	{
		USeqAct_Interp* Interp = *It;
		if( Interp->IsTemplate() )
		{
			continue;
		}
		Interps++;
		UInterpData* Data = Interp->InterpData;
		if( Data == NULL )
		{
			Data = Interp->FindInterpDataFromVariable();
		}
		const INT NumGroups = ( Data != NULL ) ? Data->GetNbInterpGroups() : -1;
		debugf( TEXT("DISHONORED(bringup): fkdiag interp: %s playing %d pos %.2f activations %d data %s groups %d instances %d"),
			*Interp->GetPathName(), (INT)Interp->bIsPlaying, Interp->Position, Interp->ActivateCount,
			( Data != NULL ) ? *Data->GetName() : TEXT("NONE"), NumGroups, Interp->GroupInst.Num() );
		for( INT GroupIndex = 0; GroupIndex < NumGroups; GroupIndex++ )
		{
			UInterpGroup* Group = Data->GetInterpGroup( GroupIndex );
			if( Group == NULL )
			{
				continue;
			}
			GroupsSeen++;
			const FString GroupClass = Group->GetClass()->GetName();
			if( GroupClass == TEXT("InterpGroupDirector") )
			{
				Directors++;
			}
			else if( GroupClass == TEXT("InterpGroupPlayer") )
			{
				Players++;
			}
			else if( GroupClass == TEXT("InterpGroupAI") )
			{
				AIGroups++;
			}
			UInterpGroupInst* Inst = NULL;
			for( INT InstIndex = 0; InstIndex < Interp->GroupInst.Num(); InstIndex++ )
			{
				if( Interp->GroupInst( InstIndex ) != NULL && Interp->GroupInst( InstIndex )->Group == Group )
				{
					Inst = Interp->GroupInst( InstIndex );
					break;
				}
			}
			debugf( TEXT("DISHONORED(bringup): fkdiag group: %s group %d '%s' class %s folder %d tracks %d inst %s actor %s"),
				*Interp->GetName(), GroupIndex, *Group->GroupName.ToString(), *GroupClass,
				(INT)Group->bIsFolder, Group->InterpTracks.Num(),
				( Inst != NULL ) ? *Inst->GetClass()->GetName() : TEXT("NONE"),
				( Inst != NULL && Inst->GetGroupActor() != NULL ) ? *Inst->GetGroupActor()->GetName() : TEXT("none") );
			for( INT TrackIndex = 0; TrackIndex < Group->InterpTracks.Num(); TrackIndex++ )
			{
				UInterpTrack* Track = Group->InterpTracks( TrackIndex );
				if( Track == NULL )
				{
					continue;
				}
				Tracks++;
				debugf( TEXT("DISHONORED(bringup): fkdiag track: %s group '%s' track %d class %s keys %d disabled %d inst %s"),
					*Interp->GetName(), *Group->GroupName.ToString(), TrackIndex,
					*Track->GetClass()->GetName(), Track->GetNumKeyframes(), (INT)Track->IsDisabled(),
					( Inst != NULL && TrackIndex < Inst->TrackInst.Num() && Inst->TrackInst( TrackIndex ) != NULL )
						? *Inst->TrackInst( TrackIndex )->GetClass()->GetName() : TEXT("NONE") );
			}
		}
	}
	debugf( TEXT("DISHONORED(bringup): fkdiag census: map %s, %d SeqAct_Interp, %d groups, %d tracks, ")
		TEXT("%d director group(s), %d player group(s), %d AI group(s)"),
		( GWorld != NULL ) ? *GWorld->GetMapName() : TEXT("none"),
		Interps, GroupsSeen, Tracks, Directors, Players, AIGroups );
}

void DisFKReport( AActor* Owner )
{
	static FLOAT NextAt = 0.f;
	static FString LastMap;
	if( GWorld == NULL )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	const FString Map = GWorld->GetMapName();

	INT Levels = 0;
	TArray<FString> Names;
	TArray<INT> Count;
	TArray<INT> Activations;
	for( INT LevelIndex = 0; LevelIndex < GWorld->Levels.Num(); LevelIndex++ )
	{
		ULevel* Level = GWorld->Levels( LevelIndex );
		if( Level == NULL )
		{
			continue;
		}
		Levels++;
		for( INT SeqIndex = 0; SeqIndex < Level->GameSequences.Num(); SeqIndex++ )
		{
			DisFKWalkSequenceConditions( Level->GameSequences( SeqIndex ), 0, Names, Count, Activations );
		}
	}

	// Twice per map: once on arrival, because that is when the package data is all there, and once
	// 45 s later, because InitInterp has not run for anything at arrival and "inst NONE" on the first
	// pass only means "not started yet". -fkdiagat=<seconds> moves the second pass.
	static FLOAT SecondPassAt = -1.f;
	if( Map != LastMap && GWorld->Levels.Num() > 0 )
	{
		LastMap = Map;
		FLOAT Delay = 45.f;
		Parse( appCmdLine(), TEXT("fkdiagat="), Delay );
		SecondPassAt = Now + Delay;
		DisFKReportMatineeData();
	}
	else if( SecondPassAt > 0.f && Now >= SecondPassAt )
	{
		SecondPassAt = -1.f;
		debugf( TEXT("DISHONORED(bringup): fkdiag second pass at t %.2f"), Now );
		DisFKReportMatineeData();
	}

	if( Now < NextAt )
	{
		return;
	}
	NextAt = Now + 2.f;

	INT Fired = 0;
	for( INT Index = 0; Index < Names.Num(); Index++ )
	{
		if( Activations( Index ) > 0 )
		{
			Fired++;
		}
		debugf( TEXT("DISHONORED(bringup): fkdiag cond: %s x%d, %d activation(s)"),
			*Names( Index ), Count( Index ), Activations( Index ) );
	}
	debugf( TEXT("DISHONORED(bringup): fkdiag conds: t %.2f map %s, %d level(s), %d condition class(es), %d of them fired"),
		Now, *Map, Levels, Names.Num(), Fired );
}
