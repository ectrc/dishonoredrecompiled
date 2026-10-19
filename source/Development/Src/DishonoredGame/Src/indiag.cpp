/*=============================================================================
	indiag.cpp: agent IN (PHASE23 IN) - the -indiag switch (Inc/indiag.h).
	Nothing here changes behaviour: INPROBE and -inprobeat only read the world and trace.
=============================================================================*/
#include "DishonoredGame.h"
#include "indiag.h"

static INT GDisINDiag = -1;

UBOOL DisINDiagEnabled()
{
	if( GDisINDiag < 0 )
	{
		GDisINDiag = ParseParam( appCmdLine(), TEXT("indiag") ) ? 1 : 0;
	}
	return GDisINDiag != 0;
}

static const DWORD INNpcTraceFlags = 0x100030DF;
static const DWORD INPlayerTraceFlags = 0x200030DF;

static FVector INDefaultPoint()
{
	FVector Point( 10327.f, 23053.f, 2415.f );
	FString Spec;
	if( Parse( appCmdLine(), TEXT("inprobept="), Spec ) )
	{
		TArray<FString> Parts;
		Spec.ParseIntoArray( &Parts, TEXT(","), TRUE );
		if( Parts.Num() == 3 )
		{
			Point = FVector( appAtof( *Parts(0) ), appAtof( *Parts(1) ), appAtof( *Parts(2) ) );
		}
	}
	return Point;
}

static FString INDescribeHit( const FCheckResult& Hit )
{
	if( !Hit.Actor && !Hit.Component )
	{
		return FString( TEXT("nothing") );
	}
	return FString::Printf( TEXT("%s (%s) comp %s %s z %.1f normal %.2f/%.2f/%.2f time %.3f"),
		Hit.Actor ? *Hit.Actor->GetName() : TEXT("none"), Hit.Actor ? *Hit.Actor->GetClass()->GetName() : TEXT("-"),
		Hit.Component ? *Hit.Component->GetClass()->GetName() : TEXT("-"), Hit.Component ? *Hit.Component->GetName() : TEXT("-"),
		Hit.Location.Z, Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z, Hit.Time );
}

static FString INOwnTrace( UPrimitiveComponent* Primitive, const FVector& End, const FVector& Start, const FVector& Extent, DWORD TraceFlags )
{
	FCheckResult Hit( 1.f );
	if( Primitive->LineCheck( Hit, End, Start, Extent, TraceFlags ) )
	{
		return FString( TEXT("miss") );
	}
	return FString::Printf( TEXT("HIT z %.1f n.z %.2f"), Hit.Location.Z, Hit.Normal.Z );
}

static FString INAggGeom( const FKAggregateGeom& Agg )
{
	return FString::Printf( TEXT("box %d convex %d sphere %d sphyl %d"), Agg.BoxElems.Num(), Agg.ConvexElems.Num(), Agg.SphereElems.Num(), Agg.SphylElems.Num() );
}

static FString INMeshCollision( UPrimitiveComponent* Primitive )
{
	if( UStaticMeshComponent* StaticComponent = Cast<UStaticMeshComponent>( Primitive ) )
	{
		UStaticMesh* Mesh = StaticComponent->StaticMesh;
		if( !Mesh )
		{
			return FString( TEXT("static mesh none") );
		}
		INT CollidingElements = 0;
		INT Elements = 0;
		if( Mesh->LODModels.Num() > 0 )
		{
			Elements = Mesh->LODModels(0).Elements.Num();
			for( INT ElementIdx = 0; ElementIdx < Elements; ++ElementIdx )
			{
				CollidingElements += Mesh->LODModels(0).Elements(ElementIdx).EnableCollision ? 1 : 0;
			}
		}
		return FString::Printf( TEXT("static %s body %s kdop nodes %d tris %d simple box %d line %d rb %d elements %d colliding %d det %.2f scale3d %s"),
			*Mesh->GetPathName(), Mesh->BodySetup ? *INAggGeom( Mesh->BodySetup->AggGeom ) : TEXT("NONE"), Mesh->kDOPTree.Nodes.Num(), Mesh->kDOPTree.Triangles.Num(),
			(INT)Mesh->UseSimpleBoxCollision, (INT)Mesh->UseSimpleLineCollision, (INT)Mesh->UseSimpleRigidBodyCollision, Elements, CollidingElements,
			StaticComponent->LocalToWorldDeterminant, *StaticComponent->Scale3D.ToString() );
	}
	if( USkeletalMeshComponent* SkelComponent = Cast<USkeletalMeshComponent>( Primitive ) )
	{
		USkeletalMesh* Mesh = SkelComponent->SkeletalMesh;
		UPhysicsAsset* Asset = SkelComponent->PhysicsAsset;
		FString Bodies;
		if( Asset )
		{
			for( INT BodyIdx = 0; BodyIdx < Asset->BodySetup.Num(); ++BodyIdx )
			{
				URB_BodySetup* Body = Asset->BodySetup(BodyIdx);
				Bodies += FString::Printf( TEXT(" [%s %s]"), Body ? *Body->BoneName.ToString() : TEXT("null"), Body ? *INAggGeom( Body->AggGeom ) : TEXT("-") );
			}
		}
		return FString::Printf( TEXT("skeletal %s simple line %d box %d perpoly bones %d | PA %s%s | spacebases %d"),
			Mesh ? *Mesh->GetPathName() : TEXT("none"), Mesh ? (INT)Mesh->bUseSimpleLineCollision : -1, Mesh ? (INT)Mesh->bUseSimpleBoxCollision : -1,
			Mesh ? Mesh->PerPolyCollisionBones.Num() : -1, Asset ? *Asset->GetPathName() : TEXT("none"), *Bodies, SkelComponent->SpaceBases.Num() );
	}
	if( UBrushComponent* BrushComponent = Cast<UBrushComponent>( Primitive ) )
	{
		return FString::Printf( TEXT("brush %s nodes %d agg %s"), BrushComponent->Brush ? *BrushComponent->Brush->GetName() : TEXT("none"),
			BrushComponent->Brush ? BrushComponent->Brush->Nodes.Num() : -1, *INAggGeom( BrushComponent->BrushAggGeom ) );
	}
	return FString( TEXT("-") );
}

static FString INSource( UPrimitiveComponent* Primitive )
{
	AActor* Owner = Primitive->GetOwner();
	if( !Owner )
	{
		return FString( TEXT("no owner") );
	}
	FString Source;
	if( Owner->Components.ContainsItem( Primitive ) )
	{
		Source += TEXT("Components ");
	}
	if( Owner->AllComponents.ContainsItem( Primitive ) )
	{
		Source += TEXT("AllComponents ");
	}
	if( Owner->IsA( AStaticMeshCollectionActor::StaticClass() ) )
	{
		Source += TEXT("SMCA ");
	}
	return Source.Len() ? Source : FString( TEXT("neither") );
}

static UBOOL INNear( UPrimitiveComponent* Primitive, const FBox& Box )
{
	if( Primitive->IsAttached() )
	{
		return Box.Intersect( Primitive->Bounds.GetBox() );
	}
	UStaticMeshComponent* StaticComponent = Cast<UStaticMeshComponent>( Primitive );
	if( StaticComponent && StaticComponent->StaticMesh )
	{
		const FMatrix ParentToWorld = StaticComponent->GetOwner() && !StaticComponent->GetOwner()->IsA( AStaticMeshCollectionActor::StaticClass() ) ?
			StaticComponent->GetOwner()->LocalToWorld() : StaticComponent->CachedParentToWorld;
		return Box.Intersect( StaticComponent->StaticMesh->Bounds.GetBox().TransformBy( FScaleMatrix( StaticComponent->Scale3D ) * ParentToWorld ) );
	}
	return FALSE;
}

static void INProbe( const FVector& Point, FLOAT Radius, const TCHAR* Label )
{
	if( !GWorld )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	const FVector Start = Point + FVector( 0.f, 0.f, 300.f );
	const FVector End = Point - FVector( 0.f, 0.f, 300.f );
	const FVector NpcExtent( 25.f, 25.f, 25.f );
	const FBox Box( Point - FVector( Radius, Radius, Radius ), Point + FVector( Radius, Radius, Radius ) );
	debugf( TEXT("DISHONORED(bringup): indiag probe %s world %.3f at %s r %.0f"), Label, Now, *Point.ToString(), Radius );

	for( INT LevelIdx = 0; LevelIdx < GWorld->Levels.Num(); ++LevelIdx )
	{
		ULevel* Level = GWorld->Levels(LevelIdx);
		debugf( TEXT("DISHONORED(bringup): indiag level %d %s actors %d bsp nodes %d model components %d"), LevelIdx, *Level->GetOutermost()->GetName(),
			Level->Actors.Num(), Level->Model ? Level->Model->Nodes.Num() : -1, Level->ModelComponents.Num() );
	}
	AWorldInfo* Info = GWorld->GetWorldInfo();
	for( INT StreamIdx = 0; Info && StreamIdx < Info->StreamingLevels.Num(); ++StreamIdx )
	{
		ULevelStreaming* Streaming = Info->StreamingLevels(StreamIdx);
		if( Streaming )
		{
			debugf( TEXT("DISHONORED(bringup): indiag streaming %s %s loaded %d visible %d shouldload %d shouldvisible %d"), *Streaming->GetClass()->GetName(),
				*Streaming->PackageName.ToString(), Streaming->LoadedLevel ? 1 : 0, (INT)Streaming->bIsVisible, (INT)Streaming->bShouldBeLoaded, (INT)Streaming->bShouldBeVisible );
		}
	}

	struct FINTrace { const TCHAR* Name; DWORD Flags; UBOOL bExtent; };
	const FINTrace Traces[] =
	{
		{ TEXT("npc 0x100030DF extent"), INNpcTraceFlags, TRUE },
		{ TEXT("player 0x200030DF extent"), INPlayerTraceFlags, TRUE },
		{ TEXT("allblocking zero"), TRACE_AllBlocking, FALSE },
		{ TEXT("allblocking|complex zero"), TRACE_AllBlocking | TRACE_ComplexCollision, FALSE },
	};
	for( INT TraceIdx = 0; TraceIdx < ARRAY_COUNT( Traces ); ++TraceIdx )
	{
		FCheckResult Hit( 1.f );
		GWorld->SingleLineCheck( Hit, NULL, End, Start, Traces[TraceIdx].Flags, Traces[TraceIdx].bExtent ? NpcExtent : FVector( 0.f, 0.f, 0.f ) );
		debugf( TEXT("DISHONORED(bringup): indiag world %s down 600: %s"), Traces[TraceIdx].Name, *INDescribeHit( Hit ) );
	}

	TArray<UPrimitiveComponent*> InHash;
	GWorld->Hash->GetIntersectingPrimitives( Box, InHash );
	debugf( TEXT("DISHONORED(bringup): indiag hash query: %d primitives in the box"), InHash.Num() );

	INT Rows = 0;
	for( TObjectIterator<UPrimitiveComponent> It; It; ++It )
	{
		UPrimitiveComponent* Primitive = *It;
		if( Primitive->IsTemplate() || Primitive->IsPendingKill() || !INNear( Primitive, Box ) )
		{
			continue;
		}
		AActor* Owner = Primitive->GetOwner();
		const FDisPrimTraceMask& Mask = Primitive->m_CollisionTraceTypes;
		const FString Flags = FString::Printf( TEXT("attached %d hidden %d bounds %s +- %s | collide %d block %d zero %d nonzero %d rb %d | actor collide %d block %d worldgeo %d static %d hidden %d physics %d base %s loc %s rot %s"),
			(INT)Primitive->IsAttached(), (INT)Primitive->HiddenGame, *Primitive->Bounds.Origin.ToString(), *Primitive->Bounds.BoxExtent.ToString(),
			(INT)Primitive->CollideActors, (INT)Primitive->BlockActors, (INT)Primitive->BlockZeroExtent, (INT)Primitive->BlockNonZeroExtent, (INT)Primitive->BlockRigidBody,
			Owner ? (INT)Owner->bCollideActors : -1, Owner ? (INT)Owner->bBlockActors : -1, Owner ? (INT)Owner->bWorldGeometry : -1, Owner ? (INT)Owner->IsStatic() : -1,
			Owner ? (INT)Owner->bHidden : -1, Owner ? (INT)Owner->Physics : -1, Owner && Owner->Base ? *Owner->Base->GetName() : TEXT("none"),
			Owner ? *Owner->Location.ToString() : TEXT("-"), Owner ? *Owner->Rotation.ToString() : TEXT("-") );
		const FString Octree = FString::Printf( TEXT("mask np %d npc %d pl %d | shouldcollide %d octree nodes %d hashquery %d"),
			(INT)Mask.m_bTraceForMove_NonPawn, (INT)Mask.m_bTraceForMove_NonPlayerPawn, (INT)Mask.m_bTraceForMove_Player,
			(INT)Primitive->ShouldCollide(), Primitive->OctreeNodes.Num(), (INT)InHash.ContainsItem( Primitive ) );
		const FString Own = FString::Printf( TEXT("own zero %s, npc extent %s, complex zero %s"),
			*INOwnTrace( Primitive, End, Start, FVector( 0.f, 0.f, 0.f ), TRACE_AllBlocking ), *INOwnTrace( Primitive, End, Start, NpcExtent, TRACE_AllBlocking ),
			*INOwnTrace( Primitive, End, Start, FVector( 0.f, 0.f, 0.f ), TRACE_AllBlocking | TRACE_ComplexCollision ) );
		debugf( TEXT("DISHONORED(bringup): indiag row %d %s %s owner %s (%s) level %s source %s | %s | %s | %s | %s"),
			Rows++, *Primitive->GetClass()->GetName(), *Primitive->GetName(), Owner ? *Owner->GetName() : TEXT("none"), Owner ? *Owner->GetClass()->GetName() : TEXT("-"),
			*Primitive->GetOutermost()->GetName(), *INSource( Primitive ), *Flags, *Octree, *INMeshCollision( Primitive ), *Own );
	}
	debugf( TEXT("DISHONORED(bringup): indiag probe %s end: %d rows"), Label, Rows );
}

static void INProbePlayer( APawn* Pawn )
{
	if( !Pawn || !Pawn->CylinderComponent )
	{
		return;
	}
	const FVector Feet = Pawn->Location - FVector( 0.f, 0.f, Pawn->CylinderComponent->CollisionHeight );
	INProbe( Feet, 150.f, TEXT("player") );
	const FVector Extent( Pawn->CylinderComponent->CollisionRadius, Pawn->CylinderComponent->CollisionRadius, Pawn->CylinderComponent->CollisionHeight );
	for( INT Dir = 0; Dir < 8; ++Dir )
	{
		const FLOAT Angle = Dir * PI / 4.f;
		const FVector Delta( appCos( Angle ) * 80.f, appSin( Angle ) * 80.f, 0.f );
		FCheckResult Hit( 1.f );
		GWorld->SingleLineCheck( Hit, Pawn, Pawn->Location + Delta, Pawn->Location, INPlayerTraceFlags, Extent );
		debugf( TEXT("DISHONORED(bringup): indiag player sweep %d deg from %s: %s"), Dir * 45, *Pawn->Location.ToString(), *INDescribeHit( Hit ) );
	}
}

void DisINReport( APlayerController* Controller )
{
	if( !DisINDiagEnabled() || !GWorld )
	{
		return;
	}
	static TArray<FLOAT> Times;
	static UBOOL bParsed = FALSE;
	if( !bParsed )
	{
		bParsed = TRUE;
		FString Spec;
		if( Parse( appCmdLine(), TEXT("inprobeat="), Spec ) )
		{
			TArray<FString> Parts;
			Spec.ParseIntoArray( &Parts, TEXT(","), TRUE );
			for( INT PartIdx = 0; PartIdx < Parts.Num(); ++PartIdx )
			{
				Times.AddItem( appAtof( *Parts(PartIdx) ) );
			}
		}
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	for( INT TimeIdx = 0; TimeIdx < Times.Num(); ++TimeIdx )
	{
		if( Now >= Times(TimeIdx) )
		{
			Times.Remove( TimeIdx-- );
			INProbe( INDefaultPoint(), 300.f, TEXT("auto") );
			INProbePlayer( Controller ? Controller->Pawn : NULL );
		}
	}
}

UBOOL DisINDiagExec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( !DisINDiagEnabled() || !ParseCommand( &Cmd, TEXT("INPROBE") ) )
	{
		return FALSE;
	}
	FVector Point = INDefaultPoint();
	FLOAT Radius = 300.f;
	const FString Args( Cmd );
	TArray<FString> Parts;
	Args.ParseIntoArray( &Parts, TEXT(" "), TRUE );
	if( Parts.Num() >= 3 )
	{
		Point = FVector( appAtof( *Parts(0) ), appAtof( *Parts(1) ), appAtof( *Parts(2) ) );
	}
	if( Parts.Num() >= 4 )
	{
		Radius = appAtof( *Parts(3) );
	}
	INProbe( Point, Radius, TEXT("console") );
	Ar.Logf( TEXT("INPROBE done") );
	return TRUE;
}
