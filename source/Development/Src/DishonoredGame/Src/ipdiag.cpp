/*=============================================================================
	ipdiag.cpp: agent IP (PHASE23 IP) - the -ipdiag switch (Inc/ipdiag.h). Measurement only; the two A/B switches restore the
	pre-port behaviour of the bump smoother and the head-bob node for a same-binary comparison.
=============================================================================*/
#include "DishonoredGame.h"
#include "ipdiag.h"

static INT GDisIPDiag = -1;
static INT GDisIPNoBump = -1;
static INT GDisIPNoHeadBob = -1;
static INT GDisIPNoSpeed = -1;
static INT GDisIPNoLand = -1;
static INT GDisIPNoBreath = -1;
static FLOAT GDisIPFrom = 0.f;
static FString GDisIPInfluences;

UBOOL DisIPDiagEnabled()
{
	if( GDisIPDiag < 0 )
	{
		GDisIPDiag = ParseParam( appCmdLine(), TEXT("ipdiag") ) ? 1 : 0;
		Parse( appCmdLine(), TEXT("ipdiagfrom="), GDisIPFrom );
	}
	return GDisIPDiag != 0;
}

UBOOL DisIPNoBump()
{
	if( GDisIPNoBump < 0 )
	{
		GDisIPNoBump = ParseParam( appCmdLine(), TEXT("ipnobump") ) ? 1 : 0;
	}
	return GDisIPNoBump != 0;
}

UBOOL DisIPNoHeadBob()
{
	if( GDisIPNoHeadBob < 0 )
	{
		GDisIPNoHeadBob = ParseParam( appCmdLine(), TEXT("ipnoheadbob") ) ? 1 : 0;
	}
	return GDisIPNoHeadBob != 0;
}

UBOOL DisIPNoSpeed()
{
	if( GDisIPNoSpeed < 0 )
	{
		GDisIPNoSpeed = ParseParam( appCmdLine(), TEXT("ipnospeed") ) ? 1 : 0;
	}
	return GDisIPNoSpeed != 0;
}

UBOOL DisIPNoLand()
{
	if( GDisIPNoLand < 0 )
	{
		GDisIPNoLand = ParseParam( appCmdLine(), TEXT("ipnoland") ) ? 1 : 0;
	}
	return GDisIPNoLand != 0;
}

UBOOL DisIPNoBreath()
{
	if( GDisIPNoBreath < 0 )
	{
		GDisIPNoBreath = ParseParam( appCmdLine(), TEXT("ipnobreath") ) ? 1 : 0;
	}
	return GDisIPNoBreath != 0;
}

static FVector DisIPPawnFrame( const APawn* Pawn, const FVector& WorldOffset )
{
	const FRotationMatrix YawFrame( FRotator( 0, Pawn->Rotation.Yaw, 0 ) );
	return FVector( WorldOffset | YawFrame.GetAxis( 0 ), WorldOffset | YawFrame.GetAxis( 1 ), WorldOffset.Z );
}

static FLOAT DisIPDeg( INT Unreal )
{
	return (FLOAT)( (SWORD)( Unreal & 0xFFFF ) ) * ( 360.f / 65536.f );
}

void DisIPNoteInfluence( UDishonoredCameraInfluenceGroup* Group, UDishonoredCameraInfluence* Influence, const FDishonoredViewTarget& CurVT, FLOAT CurWeight, APawn* Pawn )
{
	if( !DisIPDiagEnabled() || !Group || !Influence )
	{
		return;
	}
	const FTPOV& POV = CurVT.m_VT.POV;
	const FVector Loc = Group->m_GroupType == INF_GROUP_ADDITIVE || !Pawn ? POV.Location : DisIPPawnFrame( Pawn, POV.Location - Pawn->Location );
	GDisIPInfluences += FString::Printf( TEXT("%s.%s w%.2f loc %.2f %.2f %.2f rot %.3f %.3f %.3f; "), *Group->GetName(), *Influence->GetClass()->GetName(), CurWeight,
		Loc.X, Loc.Y, Loc.Z, DisIPDeg( POV.Rotation.Pitch ), DisIPDeg( POV.Rotation.Yaw ), DisIPDeg( POV.Rotation.Roll ) );
}

static void DisIPReportAnim( APawn* Pawn )
{
	USkeletalMeshComponent* Mesh = Pawn->Mesh;
	UAnimTree* Tree = Mesh ? Cast<UAnimTree>( Mesh->Animations ) : NULL;
	if( !Tree )
	{
		return;
	}
	FString Seqs;
	FString Nodes;
	for( INT Index = 0; Index < Tree->AnimTickArray.Num(); Index++ )
	{
		UAnimNode* Node = Tree->AnimTickArray(Index);
		if( !Node || !Node->bRelevant )
		{
			continue;
		}
		UAnimNodeSequence* Seq = Cast<UAnimNodeSequence>( Node );
		if( Seq )
		{
			if( Seq->NodeTotalWeight > 0.01f )
			{
				Seqs += FString::Printf( TEXT("%s w%.2f r%.2f t%.2f%s; "), *Seq->AnimSeqName.ToString(), Seq->NodeTotalWeight, Seq->Rate, Seq->CurrentTime, Seq->bPlaying ? TEXT("") : TEXT(" stopped") );
			}
			continue;
		}
		if( Node->IsA( UDisAnimNodeBlendByHeadBob::StaticClass() ) || Node->IsA( UAnimNodeBlendBySpeed::StaticClass() ) )
		{
			UAnimNodeBlendBase* Blend = (UAnimNodeBlendBase*)Node;
			Nodes += FString::Printf( TEXT("%s"), *Node->GetName() );
			if( Node->IsA( UAnimNodeBlendBySpeed::StaticClass() ) )
			{
				UAnimNodeBlendBySpeed* BySpeed = (UAnimNodeBlendBySpeed*)Node;
				UDishonoredAnimNodeBlendBySpeed* DisBySpeed = Cast<UDishonoredAnimNodeBlendBySpeed>( Node );
				Nodes += FString::Printf( TEXT(" speed %.2f flags input %d 2d %d accel %d constraints"), BySpeed->Speed, DisBySpeed ? (INT)DisBySpeed->m_bUseControllerInputSpeed : -1,
					DisBySpeed ? (INT)DisBySpeed->m_bUse2DSpeed : -1, (INT)BySpeed->bUseAcceleration );
				for( INT Constraint = 0; Constraint < BySpeed->Constraints.Num(); Constraint++ )
				{
					Nodes += FString::Printf( TEXT(" %.2f"), BySpeed->Constraints(Constraint) );
				}
			}
			Nodes += TEXT(" [");
			for( INT Child = 0; Child < Blend->Children.Num(); Child++ )
			{
				Nodes += FString::Printf( TEXT("%s%.2f"), Child ? TEXT(" ") : TEXT(""), Blend->Children(Child).Weight );
			}
			Nodes += FString::Printf( TEXT("] tw%.2f; "), Node->NodeTotalWeight );
		}
	}
	debugf( TEXT("DISHONORED(bringup): ipdiag anim t=%.3f seq %s| nodes %s"), GWorld->GetTimeSeconds(), *Seqs, *Nodes );
}

static void DisIPReportMisc( ADishonoredPlayerCamera* Camera )
{
	FString Modifiers;
	for( INT Index = 0; Index < Camera->ModifierList.Num(); Index++ )
	{
		UCameraModifier* Modifier = Camera->ModifierList(Index);
		Modifiers += FString::Printf( TEXT("%s%s; "), Modifier ? *Modifier->GetClass()->GetName() : TEXT("NULL"), Modifier && Modifier->IsDisabled() ? TEXT(" (disabled)") : TEXT("") );
	}
	FString PostProcess;
	for( INT Target = 0; Target < Camera->m_PostProcessTargets.Num(); Target++ )
	{
		for( INT Entry = 0; Entry < ARRAY_COUNT( Camera->m_PostProcessTargets(Target).m_Entries ); Entry++ )
		{
			const FLOAT Weight = Camera->m_PostProcessTargets(Target).m_Entries[Entry].m_fCurWeight;
			if( Weight > 0.0001f )
			{
				PostProcess += FString::Printf( TEXT("%d.%d w%.2f; "), Target, Entry, Weight );
			}
		}
	}
	FString Groups;
	TArrayNoInit<UDishonoredCameraInfluenceGroup*>& InfluenceGroups = Camera->InfluenceGroups();
	for( INT Index = 0; Index < InfluenceGroups.Num(); Index++ )
	{
		UDishonoredCameraInfluenceGroup* Group = InfluenceGroups(Index);
		Groups += Group ? FString::Printf( TEXT("%d %s type %d w%.2f; "), Index, *Group->GetName(), (INT)Group->m_GroupType, Group->m_Weight ) : FString( TEXT("NULL; ") );
	}
	ADishonoredGameInfo* GameInfo = Cast<ADishonoredGameInfo>( GWorld->GetWorldInfo()->Game );
	debugf( TEXT("DISHONORED(bringup): ipdiag misc t=%.3f gameinfo m_bNewNPCPhysWalking %d bob %.2f roll %.2f reaction %.2f groups [%s] modifiers [%s] post-process [%s]"),
		GWorld->GetTimeSeconds(), GameInfo ? (INT)GameInfo->m_bNewNPCPhysWalking : -1, Camera->m_BobAmount, Camera->m_RollAmount, Camera->m_fReactionWeight, *Groups, *Modifiers, *PostProcess );
}

void DisIPReportCamera( ADishonoredPlayerCamera* Camera, const FTViewTarget& OutVT, APawn* Pawn, FLOAT DeltaTime )
{
	if( !DisIPDiagEnabled() || !Camera || !Pawn || !GWorld )
	{
		GDisIPInfluences.Empty();
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( Now < GDisIPFrom )
	{
		GDisIPInfluences.Empty();
		return;
	}
	static FVector LastLoc( 0.f, 0.f, 0.f );
	static FRotator LastRot( 0, 0, 0 );
	static FVector LastBone( 0.f, 0.f, 0.f );
	static FLOAT LastAnim = -1.f;
	static FLOAT LastMisc = -1.f;
	const ADishonoredPawn* DisPawn = ConstCast<ADishonoredPawn>( Pawn );
	const FVector Bone = DisIPPawnFrame( Pawn, ( DisPawn ? DisPawn->GetCameraPos() : Pawn->Location ) - Pawn->Location );
	const FVector DLoc = OutVT.POV.Location - LastLoc;
	const FRotator DRot = OutVT.POV.Rotation - LastRot;
	const FVector DBone = Bone - LastBone;
	const FString View = FString::Printf( TEXT("t=%.3f pass %d dt=%.4f pov %.2f %.2f %.2f rot %.3f %.3f %.3f d %.3f %.3f %.3f dr %.4f %.4f %.4f"),
		Now, Camera->m_PassCount, DeltaTime, OutVT.POV.Location.X, OutVT.POV.Location.Y, OutVT.POV.Location.Z,
		DisIPDeg( OutVT.POV.Rotation.Pitch ), DisIPDeg( OutVT.POV.Rotation.Yaw ), DisIPDeg( OutVT.POV.Rotation.Roll ),
		DLoc.X, DLoc.Y, DLoc.Z, DisIPDeg( DRot.Pitch ), DisIPDeg( DRot.Yaw ), DisIPDeg( DRot.Roll ) );
	const FString Body = FString::Printf( TEXT("pawn %.2f %.2f %.2f v2d %.1f vz %.1f phys %d | bone %.3f %.3f %.3f dbone %.3f %.3f %.3f | hp %d/%d"),
		Pawn->Location.X, Pawn->Location.Y, Pawn->Location.Z, Pawn->Velocity.Size2D(), Pawn->Velocity.Z, (INT)Pawn->Physics,
		Bone.X, Bone.Y, Bone.Z, DBone.X, DBone.Y, DBone.Z, Pawn->Health, Pawn->HealthMax );
	const ADishonoredPlayerPawn* Player = ConstCast<ADishonoredPlayerPawn>( Pawn );
	const FString Breath = Player ? FString::Printf( TEXT("breath %.3f aim %.3f sprint %d"), Player->m_fBreathlessness, Player->GetAimStability(), (INT)Player->m_bSprinting ) : FString();
	debugf( TEXT("DISHONORED(bringup): ipdiag cam %s | %s | %s | %s"), *View, *Body, *Breath, *GDisIPInfluences );
	GDisIPInfluences.Empty();
	LastLoc = OutVT.POV.Location;
	LastRot = OutVT.POV.Rotation;
	LastBone = Bone;
	if( LastAnim < 0.f || Now - LastAnim >= 0.25f || Now < LastAnim )
	{
		LastAnim = Now;
		DisIPReportAnim( Pawn );
	}
	if( LastMisc < 0.f || Now - LastMisc >= 1.f || Now < LastMisc )
	{
		LastMisc = Now;
		DisIPReportMisc( Camera );
	}
}
