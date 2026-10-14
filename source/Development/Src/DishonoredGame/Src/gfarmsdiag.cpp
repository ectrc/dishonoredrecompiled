/*=============================================================================
	gfarmsdiag.cpp: agent GF (PHASE19 GF) - the -gfdiag switch (see gfarmsdiag.h). Nothing here changes behaviour.
=============================================================================*/
#include "DishonoredGame.h"
#include "gfarmsdiag.h"
#include "EdgeAnim.h"

static INT GDisGFDiag = -1;

UBOOL DisGFDiagEnabled()
{
	if( GDisGFDiag < 0 )
	{
		GDisGFDiag = ( appStrfind( appCmdLine(), TEXT("-gfdiag") ) != NULL ) ? 1 : 0;
	}
	return GDisGFDiag == 1;
}

static UBOOL DisGFOncePerSecond( const UObject* Key )
{
	static TMap<const UObject*, FLOAT> LastReportAt;
	const FLOAT Now = GWorld ? GWorld->GetTimeSeconds() : 0.f;
	FLOAT* Last = LastReportAt.Find( Key );
	if( Last && Now - *Last < 1.f )
	{
		return FALSE;
	}
	LastReportAt.Set( Key, Now );
	return TRUE;
}

// the top-weight sequence nodes of a mesh's tree, and every stack / slot / blend-pose node's child weights
static FString DisGFDescribeAnims( USkeletalMeshComponent* Mesh, UBOOL bStacks )
{
	FString Out;
	if( !Mesh || !Mesh->Animations )
	{
		return TEXT("no tree");
	}
	TArray<UAnimNode*> Nodes;
	Mesh->Animations->GetNodes( Nodes );
	for( INT Index = 0; Index < Nodes.Num(); Index++ )
	{
		UAnimNodeSequence* Seq = Cast<UAnimNodeSequence>( Nodes(Index) );
		if( Seq && Seq->NodeTotalWeight > 0.01f )
		{
			const UAnimSet* Set = Seq->AnimSeq ? Seq->AnimSeq->GetAnimSet() : NULL;
			const EdgeAnimAnimation* Edge = ( Seq->AnimSeq && Seq->AnimSeq->CompressedByteStream.Num() >= (INT)sizeof( EdgeAnimAnimation ) ) ? (const EdgeAnimAnimation*)Seq->AnimSeq->CompressedByteStream.GetData() : NULL;
			const EdgeAnimSkeleton* EdgeSkel = ( Mesh->SkeletalMesh && Mesh->SkeletalMesh->m_EdgeSkeleton.Num() >= (INT)sizeof( EdgeAnimSkeleton ) ) ? (const EdgeAnimSkeleton*)Mesh->SkeletalMesh->m_EdgeSkeleton.GetData() : NULL;
			Out += FString::Printf( TEXT("%s(%s set %s tracks %d transBones %d rotOnly %d fmt t%d r%d k%d frames %d edge joints %d T %d+%d vs skel joints %d bones %d) w %.2f t %.2f%s; "), *Seq->AnimSeqName.ToString(), *Seq->GetName(),
				Set ? *Set->GetName() : TEXT("-"), Set ? Set->TrackBoneNames.Num() : -1, Set ? Set->UseTranslationBoneNames.Num() : -1, Set ? (INT)Set->bAnimRotationOnly : -1,
				Seq->AnimSeq ? (INT)Seq->AnimSeq->TranslationCompressionFormat : -1, Seq->AnimSeq ? (INT)Seq->AnimSeq->RotationCompressionFormat : -1, Seq->AnimSeq ? (INT)Seq->AnimSeq->KeyEncodingFormat : -1,
				Seq->AnimSeq ? Seq->AnimSeq->NumFrames : -1, Edge ? (INT)Edge->numJoints : -1, Edge ? (INT)Edge->numConstTChannels : -1, Edge ? (INT)Edge->numAnimTChannels : -1,
				EdgeSkel ? (INT)EdgeSkel->numJoints : -1, Mesh->SkeletalMesh ? Mesh->SkeletalMesh->RefSkeleton.Num() : -1,
				Seq->NodeTotalWeight, Seq->CurrentTime, Seq->bPlaying ? TEXT("") : TEXT(" stopped") );
		}
		UAnimNodeBlendBase* Blend = Cast<UAnimNodeBlendBase>( Nodes(Index) );
		if( bStacks && Blend && Blend->NodeTotalWeight > 0.01f && ( Blend->IsA( UArkAnimNodeStack::StaticClass() ) || Blend->IsA( UAnimNodeSlot::StaticClass() ) || Blend->IsA( UArkAnimNodeBlendPose::StaticClass() ) ) )
		{
			Out += FString::Printf( TEXT("%s %s w %.2f ["), *Blend->GetClass()->GetName(), *Blend->NodeName.ToString(), Blend->NodeTotalWeight );
			for( INT Child = 0; Child < Blend->Children.Num(); Child++ )
			{
				UAnimNode* ChildNode = Blend->Children(Child).Anim;
				const UAnimNodeSequence* ChildSeq = Cast<UAnimNodeSequence>( ChildNode );
				Out += FString::Printf( TEXT("%d:%.2f %s "), Child, Blend->Children(Child).Weight,
					ChildSeq ? *ChildSeq->AnimSeqName.ToString() : ( ChildNode ? *ChildNode->GetClass()->GetName() : TEXT("none") ) );
			}
			Out += TEXT("]; ");
		}
	}
	return Out;
}

// the posed skeleton against the reference skeleton: component-space height of both, the largest bone scale, the largest ratio of a
// bone's local translation length to its reference one, and which bone that is
static FString DisGFDescribeSkeleton( USkeletalMeshComponent* Mesh )
{
	if( !Mesh || !Mesh->SkeletalMesh || Mesh->SpaceBases.Num() == 0 )
	{
		return TEXT("no pose");
	}
	const TArray<FMeshBone>& RefSkel = Mesh->SkeletalMesh->RefSkeleton;
	const INT NumBones = Min( RefSkel.Num(), Mesh->SpaceBases.Num() );
	TArray<FBoneAtom> RefSpace;
	RefSpace.Add( NumBones );
	FLOAT PoseMinZ = BIG_NUMBER, PoseMaxZ = -BIG_NUMBER, RefMinZ = BIG_NUMBER, RefMaxZ = -BIG_NUMBER, MaxScale = 0.f, MaxRatio = 0.f, PoseReach = 0.f, RefReach = 0.f;
	INT MaxRatioBone = INDEX_NONE;
	for( INT Bone = 0; Bone < NumBones; Bone++ )
	{
		const FBoneAtom RefLocal( RefSkel(Bone).BonePos.Orientation, RefSkel(Bone).BonePos.Position );
		RefSpace(Bone) = ( Bone == 0 ) ? RefLocal : RefLocal * RefSpace( RefSkel(Bone).ParentIndex );
		const FVector PosePos = Mesh->SpaceBases(Bone).GetTranslation();
		const FVector RefPos = RefSpace(Bone).GetTranslation();
		PoseMinZ = Min( PoseMinZ, PosePos.Z ); PoseMaxZ = Max( PoseMaxZ, PosePos.Z );
		RefMinZ = Min( RefMinZ, RefPos.Z ); RefMaxZ = Max( RefMaxZ, RefPos.Z );
		PoseReach = Max( PoseReach, PosePos.Size() );
		RefReach = Max( RefReach, RefPos.Size() );
		MaxScale = Max( MaxScale, Abs( Mesh->SpaceBases(Bone).GetScale() ) );
		if( Bone < Mesh->LocalAtoms.Num() )
		{
			const FLOAT RefLen = RefSkel(Bone).BonePos.Position.Size();
			if( RefLen > 1.f )
			{
				const FLOAT Ratio = Mesh->LocalAtoms(Bone).GetTranslation().Size() / RefLen;
				if( Ratio > MaxRatio )
				{
					MaxRatio = Ratio;
					MaxRatioBone = Bone;
				}
			}
		}
	}
	return FString::Printf( TEXT("bones %d poseH %.1f refH %.1f (x%.2f) reach %.1f/%.1f maxBoneScale %.2f maxLocalRatio %.2f at %s"), NumBones,
		PoseMaxZ - PoseMinZ, RefMaxZ - RefMinZ, ( RefMaxZ - RefMinZ ) > 1.f ? ( PoseMaxZ - PoseMinZ ) / ( RefMaxZ - RefMinZ ) : 0.f, PoseReach, RefReach,
		MaxScale, MaxRatio, MaxRatioBone >= 0 ? *RefSkel(MaxRatioBone).Name.ToString() : TEXT("-") );
}

void DisGFReportPawn( ADishonoredPawn* Pawn )
{
	if( !DisGFDiagEnabled() || !Pawn || !Pawn->Mesh || !DisGFOncePerSecond( Pawn ) )
	{
		return;
	}
	USkeletalMeshComponent* Mesh = Pawn->Mesh;
	const FVector AxisX( Mesh->LocalToWorld.M[0][0], Mesh->LocalToWorld.M[0][1], Mesh->LocalToWorld.M[0][2] );
	const FVector AxisZ( Mesh->LocalToWorld.M[2][0], Mesh->LocalToWorld.M[2][1], Mesh->LocalToWorld.M[2][2] );
	debugf( TEXT("DISHONORED(bringup): gfdiag pawn %s t=%.2f at %.0f/%.0f/%.0f physics %d base %s%s%s hard %d rel %.0f/%.0f/%.0f"),
		*Pawn->GetName(), GWorld->GetTimeSeconds(), Pawn->Location.X, Pawn->Location.Y, Pawn->Location.Z, (INT)Pawn->Physics,
		Pawn->Base ? *Pawn->Base->GetName() : TEXT("none"), Pawn->BaseSkelComponent ? TEXT(" skel") : TEXT(""), Pawn->BaseBoneName != NAME_None ? *FString::Printf( TEXT(" bone %s"), *Pawn->BaseBoneName.ToString() ) : TEXT(""),
		(INT)Pawn->bHardAttach, Pawn->RelativeLocation.X, Pawn->RelativeLocation.Y, Pawn->RelativeLocation.Z );
	debugf( TEXT("DISHONORED(bringup): gfdiag scale %s drawScale %.2f/%.2f/%.2f/%.2f meshScale %.2f/%.2f/%.2f/%.2f meshT %.1f/%.1f/%.1f ltwAxes %.2f/%.2f"), *Pawn->GetName(),
		Pawn->DrawScale, Pawn->DrawScale3D.X, Pawn->DrawScale3D.Y, Pawn->DrawScale3D.Z,
		Mesh->Scale, Mesh->Scale3D.X, Mesh->Scale3D.Y, Mesh->Scale3D.Z, Mesh->Translation.X, Mesh->Translation.Y, Mesh->Translation.Z, AxisX.Size(), AxisZ.Size() );
	debugf( TEXT("DISHONORED(bringup): gfdiag skel %s DPG %d LOD %d/%d hidden %d attach %d bounds %.0f/%.0f/%.0f %s anims %s"), *Pawn->GetName(),
		(INT)Mesh->DepthPriorityGroup, Mesh->ForcedLodModel, Mesh->PredictedLODLevel, (INT)Mesh->HiddenGame, Mesh->Attachments.Num(),
		Mesh->Bounds.BoxExtent.X, Mesh->Bounds.BoxExtent.Y, Mesh->Bounds.BoxExtent.Z,
		*DisGFDescribeSkeleton( Mesh ), *DisGFDescribeAnims( Mesh, Pawn->IsA( ADishonoredPlayerPawn::StaticClass() ) || Pawn->Base != NULL ) );
}

static FString DisGFDescribeControl( const TCHAR* Label, const USkelControlSingleBone* Control )
{
	if( !Control )
	{
		return FString::Printf( TEXT("%s none"), Label );
	}
	return FString::Printf( TEXT("%s %s rot %d/%d/%d w %.2f"), Label, *Control->ControlName.ToString(),
		Control->BoneRotation.Pitch, Control->BoneRotation.Yaw, Control->BoneRotation.Roll, Control->ControlStrength );
}

void DisGFReportPlayer( ADishonoredPlayerPawn* Player )
{
	// keyed by the mesh: the pawn report (DisGFReportPawn, from ADishonoredPawn::Tick) already stamped the pawn this frame
	if( !DisGFDiagEnabled() || !Player || !Player->Mesh || !DisGFOncePerSecond( Player->Mesh ) )
	{
		return;
	}
	UDishonoredPlayerSkeletalComponent* Mesh = Cast<UDishonoredPlayerSkeletalComponent>( Player->Mesh );
	ADishonoredPlayerCamera* Camera = Player->GetDishonoredCamera();
	FString Inventory( TEXT("inventory none") );
	if( Player->m_pInventory )
	{
		const UDishonoredInventoryItem* Primary = Player->m_pInventory->GetEquippedItem( EDisEquipUsage_Primary );
		const UDishonoredInventoryItem* Secondary = Player->m_pInventory->GetEquippedItem( EDisEquipUsage_Secondary );
		Inventory = FString::Printf( TEXT("inventory %s slots %d primary %s secondary %s nothingEquipped %d"), *Player->m_pInventory->GetName(), Player->m_pInventory->m_Slots.Num(),
			Primary ? *Primary->GetClass()->GetName() : TEXT("none"), Secondary ? *Secondary->GetClass()->GetName() : TEXT("none"), (INT)Player->HasNothingEquipped() );
	}
	FString Follow( TEXT("no camera") );
	if( Camera )
	{
		const FDishonoredVTSettings& S = Camera->m_DishonoredVTSettings;
		Follow = FString::Printf( TEXT("armFollow %.2f rotP %.2f rotS %.2f offW %.2f/%.2f meshFOVw %.2f armsFOV %.1f pass %d"), S.m_ArmFollowWeight, S.m_ArmFollowWeight_Rot_Primary,
			S.m_ArmFollowWeight_Rot_Secondary, S.m_ArmFollowOffset_Weight_Primary, S.m_ArmFollowOffset_Weight_Secondary, S.m_MeshSpecificFOVWeight, Camera->m_fCurFOV_Arms, Camera->m_PassCount );
	}
	debugf( TEXT("DISHONORED(bringup): gfdiag arms t=%.2f body %d mesh %s DPG %d LOD %d/%d hidden %d offset %.1f/%.1f/%.1f fov %.1f useFOV %d ctrlPitch %d"),
		GWorld->GetTimeSeconds(), (INT)Player->m_BodyMode, Mesh ? *Mesh->GetName() : TEXT("not a player skeletal component"),
		Player->Mesh ? (INT)Player->Mesh->DepthPriorityGroup : -1, Player->Mesh ? Player->Mesh->ForcedLodModel : -1, Player->Mesh ? Player->Mesh->PredictedLODLevel : -1,
		Player->Mesh ? (INT)Player->Mesh->HiddenGame : -1, Mesh ? Mesh->m_Offset.X : 0.f, Mesh ? Mesh->m_Offset.Y : 0.f, Mesh ? Mesh->m_Offset.Z : 0.f,
		Mesh ? Mesh->m_FOV : 0.f, Mesh ? (INT)Mesh->m_bUseFOV : -1, Player->Controller ? Player->Controller->Rotation.Pitch : 0 );
	debugf( TEXT("DISHONORED(bringup): gfdiag arms controls %s; %s; %s; %s; %s"),
		*DisGFDescribeControl( TEXT("right"), Player->m_pLookAtControl_RightHand ), *DisGFDescribeControl( TEXT("left"), Player->m_pLookAtControl_LeftHand ),
		*DisGFDescribeControl( TEXT("cam"), Player->m_pLookAtControl_Camera ), *Follow, *Inventory );
}
