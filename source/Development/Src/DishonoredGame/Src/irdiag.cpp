/*=============================================================================
	irdiag.cpp: agent IR (PHASE23 IR) - the -irdiag / -irface switches (Inc/irdiag.h).
	Nothing here changes what an NPC does; -irface moves only the player's own pawn.
=============================================================================*/
#include "DishonoredGame.h"
#include "irdiag.h"

static INT GDisIRDiag = -1;
static AActor* GDisIRSpeaker = NULL;
static UWorld* GDisIRSpeakerWorld = NULL;
static FLOAT GDisIRSpeakerAt = -1.f;
static FString GDisIRSpeakerEvent;

UBOOL DisIRDiagEnabled()
{
	if( GDisIRDiag < 0 )
	{
		GDisIRDiag = ParseParam( appCmdLine(), TEXT("irdiag") ) ? 1 : 0;
	}
	return GDisIRDiag != 0;
}

static UBOOL DisIRIsFaceBone( const FString& Name )
{
	static const TCHAR* Parts[] = { TEXT("jaw"), TEXT("lip"), TEXT("mouth"), TEXT("teeth"), TEXT("tooth"), TEXT("tongue"), TEXT("chin"), TEXT("cheek") };
	const FString Lower = Name.ToLower();
	for( INT i = 0; i < ARRAY_COUNT(Parts); i++ )
	{
		if( Lower.InStr( Parts[i] ) != INDEX_NONE )
		{
			return TRUE;
		}
	}
	return FALSE;
}

static FString DisIRTextureInfo( UTexture* Texture )
{
	UTexture2D* Tex2D = Cast<UTexture2D>( Texture );
	if( !Tex2D )
	{
		return Texture ? FString::Printf( TEXT("%s (%s)"), *Texture->GetPathName(), *Texture->GetClass()->GetName() ) : FString( TEXT("none") );
	}
	return FString::Printf( TEXT("%s %dx%d fmt %d mips %d resident %d requested %d streamable %d tail %d srgb %d lodgroup %d"),
		*Tex2D->GetPathName(), Tex2D->SizeX, Tex2D->SizeY, (INT)Tex2D->Format, Tex2D->Mips.Num(), Tex2D->ResidentMips, Tex2D->RequestedMips,
		(INT)Tex2D->bIsStreamable, Tex2D->MipTailBaseIdx, (INT)Tex2D->SRGB, (INT)Tex2D->LODGroup );
}

static void DisIRDumpMaterial( const TCHAR* Who, INT Slot, UMaterialInterface* MeshMaterial, UMaterialInterface* Used )
{
	debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d mesh %s used %s"), Who, Slot,
		MeshMaterial ? *MeshMaterial->GetPathName() : TEXT("none"), Used ? *Used->GetPathName() : TEXT("none") );
	if( !Used )
	{
		return;
	}
	UMaterial* Base = Used->GetMaterial();
	for( UMaterialInterface* Link = Used; Link; )
	{
		UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>( Link );
		if( !MIC )
		{
			break;
		}
		debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d   MIC %s parent %s staticPermutation %d"), Who, Slot, *MIC->GetPathName(),
			MIC->Parent ? *MIC->Parent->GetPathName() : TEXT("none"), (INT)MIC->bHasStaticPermutationResource );
		for( INT i = 0; i < MIC->ScalarParameterValues.Num(); i++ )
		{
			debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d     scalar %s = %.3f"), Who, Slot, *MIC->ScalarParameterValues(i).ParameterName.ToString(), MIC->ScalarParameterValues(i).ParameterValue );
		}
		for( INT i = 0; i < MIC->VectorParameterValues.Num(); i++ )
		{
			const FLinearColor& C = MIC->VectorParameterValues(i).ParameterValue;
			debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d     vector %s = (%.3f %.3f %.3f %.3f)"), Who, Slot, *MIC->VectorParameterValues(i).ParameterName.ToString(), C.R, C.G, C.B, C.A );
		}
		for( INT i = 0; i < MIC->TextureParameterValues.Num(); i++ )
		{
			debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d     texture %s = %s"), Who, Slot, *MIC->TextureParameterValues(i).ParameterName.ToString(),
				*DisIRTextureInfo( MIC->TextureParameterValues(i).ParameterValue ) );
		}
		Link = MIC->Parent;
	}
	if( Base )
	{
		TArray<FName> Names;
		TArray<FGuid> Ids;
		Base->GetAllScalarParameterNames( Names, Ids );
		for( INT i = 0; i < Names.Num(); i++ )
		{
			FLOAT Value = 0.f;
			const UBOOL bFound = Used->GetScalarParameterValue( Names(i), Value );
			debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d   effective scalar %s = %.3f (found %d)"), Who, Slot, *Names(i).ToString(), Value, (INT)bFound );
		}
		Names.Empty(); Ids.Empty();
		Base->GetAllVectorParameterNames( Names, Ids );
		for( INT i = 0; i < Names.Num(); i++ )
		{
			FLinearColor Value( 0.f, 0.f, 0.f, 0.f );
			const UBOOL bFound = Used->GetVectorParameterValue( Names(i), Value );
			debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d   effective vector %s = (%.3f %.3f %.3f %.3f) (found %d)"), Who, Slot, *Names(i).ToString(), Value.R, Value.G, Value.B, Value.A, (INT)bFound );
		}
		Names.Empty(); Ids.Empty();
		Base->GetAllTextureParameterNames( Names, Ids );
		for( INT i = 0; i < Names.Num(); i++ )
		{
			UTexture* Value = NULL;
			const UBOOL bFound = Used->GetTextureParameterValue( Names(i), Value );
			debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d   effective texture %s = %s (found %d)"), Who, Slot, *Names(i).ToString(), *DisIRTextureInfo( Value ), (INT)bFound );
		}
		FMaterialResource* Resource = Used->GetMaterialResource();
		const FMaterialShaderMap* ShaderMap = Resource ? Resource->GetShaderMap() : NULL;
		FVertexFactoryType* SkinType = FindVertexFactoryType( FName( TEXT("FGPUSkinVertexFactory") ) );
		const FMeshMaterialShaderMap* SkinMap = ( ShaderMap && SkinType ) ? ShaderMap->GetMeshShaderMap( SkinType ) : NULL;
		debugf( TEXT("DISHONORED(bringup): irdiag %s slot %d   base %s blend %d lighting %d twoSided %d skeletal %d usageOk %d resource %d shaderMap %d (%d shaders) gpuskin %d shaders, compile errors %d"),
			Who, Slot, *Base->GetPathName(), (INT)Base->BlendMode, (INT)Base->LightingModel, (INT)Base->TwoSided, (INT)Base->bUsedWithSkeletalMesh,
			(INT)Used->CheckMaterialUsage( MATUSAGE_SkeletalMesh ), Resource ? 1 : 0, ShaderMap ? 1 : 0, ShaderMap ? ShaderMap->GetNumShaders() : -1,
			SkinMap ? SkinMap->GetNumShaders() : -1, Resource ? Resource->GetCompileErrors().Num() : -1 );
	}
}

static void DisIRDumpComponent( const TCHAR* Who, USkeletalMeshComponent* Comp )
{
	if( !Comp || !Comp->SkeletalMesh )
	{
		debugf( TEXT("DISHONORED(bringup): irdiag %s: no component or mesh"), Who );
		return;
	}
	USkeletalMesh* Mesh = Comp->SkeletalMesh;
	debugf( TEXT("DISHONORED(bringup): irdiag %s mesh %s: %d materials, %d LODs (predicted %d, forced %d), %d bones, parentAnim %s, hidden %d, faceFX %s, FaceFXActorInstance %d, castShadow %d, attached %d"),
		Who, *Mesh->GetPathName(), Mesh->Materials.Num(), Mesh->LODModels.Num(), Comp->PredictedLODLevel, Comp->ForcedLodModel, Mesh->RefSkeleton.Num(),
		Comp->ParentAnimComponent ? *Comp->ParentAnimComponent->GetName() : TEXT("none"), (INT)Comp->HiddenGame,
		Mesh->FaceFXAsset ? *Mesh->FaceFXAsset->GetPathName() : TEXT("none"), Comp->FaceFXActorInstance ? 1 : 0, (INT)Comp->CastShadow, (INT)Comp->IsAttached() );
	for( INT Lod = 0; Lod < Mesh->LODModels.Num(); Lod++ )
	{
		const FStaticLODModel& Model = Mesh->LODModels(Lod);
		FString Line;
		for( INT s = 0; s < Model.Sections.Num(); s++ )
		{
			const FSkelMeshSection& Section = Model.Sections(s);
			INT UseIndex = Section.MaterialIndex;
			if( Lod > 0 && Lod < Mesh->LODInfo.Num() && Section.MaterialIndex < Mesh->LODInfo(Lod).LODMaterialMap.Num() )
			{
				UseIndex = Mesh->LODInfo(Lod).LODMaterialMap( Section.MaterialIndex );
			}
			const UBOOL bHidden = ( Lod < Comp->LODInfo.Num() && UseIndex < Comp->LODInfo(Lod).HiddenMaterials.Num() ) ? Comp->LODInfo(Lod).HiddenMaterials(UseIndex) : FALSE;
			Line += FString::Printf( TEXT(" [s%d mat %d->%d tris %d chunk %d hidden %d]"), s, Section.MaterialIndex, UseIndex, Section.NumTriangles, Section.ChunkIndex, (INT)bHidden );
		}
		debugf( TEXT("DISHONORED(bringup): irdiag %s LOD %d:%s"), Who, Lod, *Line );
	}
	for( INT i = 0; i < Mesh->Materials.Num(); i++ )
	{
		DisIRDumpMaterial( Who, i, Mesh->Materials(i), Comp->GetMaterial( i ) );
	}
	FString Bones;
	for( INT b = 0; b < Mesh->RefSkeleton.Num(); b++ )
	{
		Bones += FString::Printf( TEXT(" %s"), *Mesh->RefSkeleton(b).Name.ToString() );
	}
	debugf( TEXT("DISHONORED(bringup): irdiag %s bones:%s"), Who, *Bones );
}

void DisIRDumpHead( ADishonoredNPCPawn* Pawn, const TCHAR* Why )
{
	static TArray<FString> Done;
	if( !DisIRDiagEnabled() || !Pawn )
	{
		return;
	}
	const FString Key = FString::Printf( TEXT("%s/%s/%s"), Why, Pawn->m_pHeadMesh && Pawn->m_pHeadMesh->SkeletalMesh ? *Pawn->m_pHeadMesh->SkeletalMesh->GetName() : TEXT("none"),
		Pawn->Mesh && Pawn->Mesh->SkeletalMesh ? *Pawn->Mesh->SkeletalMesh->GetName() : TEXT("none") );
	if( Done.ContainsItem( Key ) )
	{
		return;
	}
	Done.AddItem( Key );
	debugf( TEXT("DISHONORED(bringup): irdiag %s pawn %s"), Why, *Pawn->GetName() );
	DisIRDumpComponent( *FString::Printf( TEXT("%s.head"), *Pawn->GetName() ), Pawn->m_pHeadMesh );
	DisIRDumpComponent( *FString::Printf( TEXT("%s.body"), *Pawn->GetName() ), Pawn->Mesh );
}

void DisIRNoteSpeaker( USkeletalMeshComponent* FaceFXMesh, const FString& EventName )
{
	if( !GWorld || !FaceFXMesh || !FaceFXMesh->GetOwner() )
	{
		return;
	}
	GDisIRSpeaker = FaceFXMesh->GetOwner();
	GDisIRSpeakerWorld = GWorld;
	GDisIRSpeakerAt = GWorld->GetTimeSeconds();
	GDisIRSpeakerEvent = EventName;
	if( DisIRDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): irdiag t=%.2f speaker %s event %s faceFX mesh %s (%s) faceFX asset on it %s"), GDisIRSpeakerAt,
			*FaceFXMesh->GetOwner()->GetName(), *EventName, *FaceFXMesh->GetName(), FaceFXMesh->SkeletalMesh ? *FaceFXMesh->SkeletalMesh->GetName() : TEXT("none"),
			FaceFXMesh->m_pFaceFXAsset ? *FaceFXMesh->m_pFaceFXAsset->GetPathName() : TEXT("none") );
		DisIRDumpHead( Cast<ADishonoredNPCPawn>( FaceFXMesh->GetOwner() ), TEXT("speaker") );
	}
}

static void DisIRLogFaceBones( const TCHAR* Who, USkeletalMeshComponent* Comp, FLOAT Now )
{
	if( !Comp || !Comp->SkeletalMesh )
	{
		return;
	}
	const TArray<FMeshBone>& Ref = Comp->SkeletalMesh->RefSkeleton;
	FString Line;
	INT Count = 0;
	for( INT b = 0; b < Ref.Num(); b++ )
	{
		if( !DisIRIsFaceBone( Ref(b).Name.ToString() ) )
		{
			continue;
		}
		Count++;
		if( b < Comp->LocalAtoms.Num() )
		{
			const FVector Pos = Comp->LocalAtoms(b).GetTranslation();
			const FQuat Rot = Comp->LocalAtoms(b).GetRotation();
			const FVector RefPos = Ref(b).BonePos.Position;
			const FQuat& RefRot = Ref(b).BonePos.Orientation;
			const FLOAT Dot = Abs( Rot.X * RefRot.X + Rot.Y * RefRot.Y + Rot.Z * RefRot.Z + Rot.W * RefRot.W );
			const FLOAT Degrees = 2.f * appAcos( Min( 1.f, Dot ) ) * 180.f / PI;
			Line += FString::Printf( TEXT(" %s d=%.2f r=%.1f"), *Ref(b).Name.ToString(), ( Pos - RefPos ).Size(), Degrees );
		}
		else
		{
			Line += FString::Printf( TEXT(" %s (no local atom)"), *Ref(b).Name.ToString() );
		}
	}
	INT Required = 0;
	for( INT b = 0; b < Ref.Num(); b++ )
	{
		if( DisIRIsFaceBone( Ref(b).Name.ToString() ) && Comp->RequiredBones.ContainsItem( (BYTE)b ) )
		{
			Required++;
		}
	}
	debugf( TEXT("DISHONORED(bringup): irdiag t=%.2f bones %s (%d face bones, %d required, %d local atoms, lod %d, parent lod %d, recently rendered %d, faceFX asset %s):%s"), Now, Who, Count, Required,
		Comp->LocalAtoms.Num(), Comp->PredictedLODLevel, Comp->ParentAnimComponent ? Comp->ParentAnimComponent->PredictedLODLevel : -1, (INT)Comp->bRecentlyRendered,
		Comp->m_pFaceFXAsset ? *Comp->m_pFaceFXAsset->GetName() : TEXT("none"), *Line );
}

void DisIRTick( APlayerController* PC )
{
	if( !GWorld || !PC || !PC->Pawn )
	{
		return;
	}
	static INT FaceDist = -2;
	if( FaceDist == -2 )
	{
		FaceDist = -1;
		if( ParseParam( appCmdLine(), TEXT("irface") ) )
		{
			FaceDist = 45;
		}
		INT Value = 0;
		if( Parse( appCmdLine(), TEXT("irface="), Value ) && Value > 0 )
		{
			FaceDist = Value;
		}
	}
	ADishonoredNPCPawn* Speaker = ( GDisIRSpeakerWorld == GWorld && GDisIRSpeaker && !GDisIRSpeaker->bDeleteMe ) ? Cast<ADishonoredNPCPawn>( GDisIRSpeaker ) : NULL;
	if( !Speaker || !Speaker->Mesh )
	{
		return;
	}
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( DisIRDiagEnabled() )
	{
		static FLOAT NextAt = 0.f;
		if( Now >= NextAt && Now - GDisIRSpeakerAt < 6.f )
		{
			NextAt = Now + 0.1f;
			DisIRLogFaceBones( *FString::Printf( TEXT("%s.body"), *Speaker->GetName() ), Speaker->Mesh, Now );
			DisIRLogFaceBones( *FString::Printf( TEXT("%s.head"), *Speaker->GetName() ), Speaker->m_pHeadMesh, Now );
		}
	}
	if( FaceDist < 0 || Now - GDisIRSpeakerAt > 30.f )
	{
		return;
	}
	const INT HeadBone = Speaker->Mesh->MatchRefBone( FName( TEXT("head_jnt") ) );
	const FVector Head = HeadBone != INDEX_NONE ? Speaker->Mesh->GetBoneLocation( FName( TEXT("head_jnt") ) ) : Speaker->Location + FVector( 0.f, 0.f, 70.f );
	const FVector Facing = FRotator( 0, Speaker->Rotation.Yaw, 0 ).Vector();
	const FVector Target = Head + FVector( 0.f, 0.f, 4.f ) + Facing * (FLOAT)FaceDist;
	FVector ViewLoc;
	FRotator ViewRot;
	PC->GetPlayerViewPoint( ViewLoc, ViewRot );
	APawn* Pawn = PC->Pawn;
	Pawn->Velocity = FVector( 0.f, 0.f, 0.f );
	Pawn->Acceleration = FVector( 0.f, 0.f, 0.f );
	Pawn->bCollideWorld = FALSE;
	if( Pawn->Physics != PHYS_None )
	{
		Pawn->setPhysics( PHYS_None );
	}
	GWorld->FarMoveActor( Pawn, Pawn->Location + ( Target - ViewLoc ), FALSE, TRUE, FALSE );
	const FRotator Look = ( Head + FVector( 0.f, 0.f, 4.f ) - Target ).Rotation();
	PC->SetRotation( Look );
	Pawn->SetRotation( FRotator( 0, Look.Yaw, 0 ) );
	static FLOAT NextLog = 0.f;
	if( DisIRDiagEnabled() && Now >= NextLog )
	{
		NextLog = Now + 1.f;
		debugf( TEXT("DISHONORED(bringup): irdiag t=%.2f face cam on %s head %s view %s target %s"), Now, *Speaker->GetName(), *Head.ToString(), *ViewLoc.ToString(), *Target.ToString() );
	}
}
