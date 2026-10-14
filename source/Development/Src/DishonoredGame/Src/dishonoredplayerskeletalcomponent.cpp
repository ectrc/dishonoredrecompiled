// DishonoredGame/src/dishonoredplayerskeletalcomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (19):
//   0x6fb8f0  public: static void __cdecl UDishonoredPlayerSkeletalComponent::InitializePrivateStaticClassUDishonoredPlayerSkeletalComponent(void)
//   0x6fb910  _UDishonoredPlayerSkeletalComponent::SetFOV_::_8_::UpdateFOVCommand::Execute
//   0x6fb930  _UDishonoredPlayerSkeletalComponent::SetFOV_::_8_::UpdateFOVCommand::DescribeCommand
//   0x6fb940  _UDishonoredPlayerSkeletalComponent::DisableFOV_::_8_::DisableFOVCommand::Execute
//   0x6fb960  _UDishonoredPlayerSkeletalComponent::DisableFOV_::_8_::DisableFOVCommand::DescribeCommand
//   0x6fb970  _UDishonoredPlayerSkeletalComponent::SetOffset_::_5_::UpdateOffsetCommand::Execute
//   0x6fb9a0  _UDishonoredPlayerSkeletalComponent::SetOffset_::_5_::UpdateOffsetCommand::DescribeCommand
//   0x707580  protected: virtual void __thiscall UDishonoredPlayerSkeletalComponent::UpdateChildComponents(void)
//   0x71a7d0  public: static class UClass * __cdecl UDishonoredPlayerSkeletalComponent::GetPrivateStaticClassUDishonoredPlayerSkeletalComponent(wchar_t const *)
//   0x71a860  public: __thiscall FDishonoredPlayerSkeletalSceneProxy::FDishonoredPlayerSkeletalSceneProxy(class USkeletalMeshComponent const *, unsigned int, float, class FVector const &)
//   0x71a8d0  public: virtual void __thiscall FDishonoredPlayerSkeletalSceneProxy::GetWorldMatrices(class FSceneView const *, class FMatrix &, class FMatrix &)
//   0x71ae60  public: virtual unsigned long __thiscall FDishonoredPlayerSkeletalSceneProxy::GetMemoryFootprint(void)const
//   0x71aeb0  public: virtual class FPrimitiveSceneProxy * __thiscall UDishonoredPlayerSkeletalComponent::CreateSceneProxy(void)
//   0x71c450  public: static class UClass * __cdecl UDishonoredPlayerSkeletalComponent::StaticClassNoInline(void)
//   0x71d880  public: void __thiscall UDishonoredPlayerSkeletalComponent::SetFOV(float)
//   0x71dab0  public: void __thiscall UDishonoredPlayerSkeletalComponent::DisableFOV(void)
//   0x71dc90  public: void __thiscall UDishonoredPlayerSkeletalComponent::SetOffset(class FVector const &)
//   0x720360  protected: virtual void __thiscall UDishonoredPlayerSkeletalComponent::UpdateChildForTickEnd_Derived(void)
//   0x7203d0  public: virtual void __thiscall UDishonoredPlayerSkeletalComponent::UpdateBounds(void)

/*-----------------------------------------------------------------------------
	Agent GF (PHASE19 GF): the player's first-person mesh component, ported whole from the 2012 binary with the 2013 bodies
	checked (every function below is named in both databases at the same size; 2013 rva = VA - 0x400000):

	  FDishonoredPlayerSkeletalSceneProxy ctor               2012 0x71a860  2013 0x6debd0  (104 B)
	  FDishonoredPlayerSkeletalSceneProxy::GetWorldMatrices  2012 0x71a8d0  2013 0x6dec40  (1414 B, byte-identical)
	  FDishonoredPlayerSkeletalSceneProxy::GetMemoryFootprint 2012 0x71ae60 2013 0x6df1d0
	  CreateSceneProxy 0x71aeb0 / 0x6df220, SetOffset 0x71dc90 / 0x6cdec0, SetFOV 0x71d880 / 0x6cdab0, DisableFOV 0x71dab0 /
	  0x6cdce0, UpdateBounds 0x7203d0 / 0x6c8880, UpdateChildComponents 0x707580 / 0x6c8360, UpdateChildForTickEnd_Derived
	  0x720360 / 0x6ce100; the three render commands 0x6fb910..0x6fb9a0 / 0x6bfa80..0x6bfb10.

	What it is: the one player mesh (ARMS_ONLY or FULL_BODY, ADishonoredPlayerPawn::SetBodyMode) is drawn through a proxy that
	shifts the mesh by m_Offset (the weighted difference between the final view position and the non-additive camera position,
	so the arms follow the camera bob and shake) and, when the arms' FOV differs from the view's, re-projects it: the local-to-world
	the renderer uses becomes LocalToWorld * View * P(arms FOV) * P(view)^-1 * View^-1 (GetWorldMatrices), which is how retail
	draws the arms at their own field of view inside the world's. Attachments that are player skeletal components (the item
	meshes, UDishonoredItemSkeletalComponent) get the same offset and FOV; attachments that are neither skeletal nor static
	meshes (particle systems, lights) are placed with the offset added (UpdateChildComponents). UpdateChildForTickEnd_Derived is
	the second camera pass of the frame (m_PassCount 1) after the pose is final.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkGF_PlayerSkeletalComponent()
{
}

class FDishonoredPlayerSkeletalSceneProxy : public FSkeletalMeshSceneProxy
{
public:
	FDishonoredPlayerSkeletalSceneProxy( const USkeletalMeshComponent* Component, UBOOL bInUseFOV, FLOAT InFOV, const FVector& InOffset )
		: FSkeletalMeshSceneProxy( Component, FColor( 230, 230, 255 ) )
		, m_bUseFOV( bInUseFOV )
		, m_FOV( InFOV )
		, m_Offset( InOffset )
	{
	}

	virtual void GetWorldMatrices( const FSceneView* View, FMatrix& OutLocalToWorld, FMatrix& OutWorldToLocal );
	virtual DWORD GetMemoryFootprint() const { return sizeof( *this ) + FSkeletalMeshSceneProxy::GetAllocatedSize(); }

	UBOOL m_bUseFOV;
	FLOAT m_FOV;
	FVector m_Offset;
};

// 2012 0x71a8d0: the offset is added to the translation; with a mesh FOV the matrix is re-projected through a perspective matrix of
// that FOV (the view's aspect, the view's near plane pulled in by a thousandth) and back through the inverse of the view's projection
void FDishonoredPlayerSkeletalSceneProxy::GetWorldMatrices( const FSceneView* View, FMatrix& OutLocalToWorld, FMatrix& OutWorldToLocal )
{
	FMatrix Shifted = LocalToWorld;
	Shifted.M[3][0] += m_Offset.X;
	Shifted.M[3][1] += m_Offset.Y;
	Shifted.M[3][2] += m_Offset.Z;
	if( !m_bUseFOV )
	{
		OutLocalToWorld = Shifted;
	}
	else
	{
		const FLOAT InvTanHalfFOV = 1.f / appTan( m_FOV * ( PI / 360.f ) );
		const FLOAT Aspect = View->SizeX / View->SizeY;
		const FLOAT NearScale = 0.999f;
		FMatrix MeshProjection( FMatrix::Identity );
		MeshProjection.M[0][0] = InvTanHalfFOV;
		MeshProjection.M[1][1] = Aspect * InvTanHalfFOV;
		MeshProjection.M[2][2] = NearScale;
		MeshProjection.M[2][3] = 1.f;
		MeshProjection.M[3][2] = -NearScale * View->NearClippingDistance;
		MeshProjection.M[3][3] = 0.f;
		OutLocalToWorld = Shifted * View->ViewMatrix * MeshProjection * View->ProjectionMatrix.Inverse() * View->ViewMatrix.Inverse();
	}
	OutWorldToLocal = OutLocalToWorld.Inverse();
}

// 2012 0x71aeb0: the conditions of USkeletalMeshComponent::CreateSceneProxy, the Dishonored proxy with the current offset and FOV
FPrimitiveSceneProxy* UDishonoredPlayerSkeletalComponent::CreateSceneProxy()
{
	if( SkeletalMesh && SkeletalMesh->LODModels.IsValidIndex( PredictedLODLevel ) && !bHideSkin && MeshObject )
	{
		return ::new FDishonoredPlayerSkeletalSceneProxy( this, m_bUseFOV, m_FOV, m_Offset );
	}
	return NULL;
}

// 2012 0x71dc90: the offset is kept, sent to the proxy on the render thread, and passed to every attached player skeletal component
void UDishonoredPlayerSkeletalComponent::SetOffset( const FVector& Offset )
{
	m_Offset = Offset;
	if( SceneInfo )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateOffsetCommand,
			FDishonoredPlayerSkeletalSceneProxy*, Proxy, (FDishonoredPlayerSkeletalSceneProxy*)Scene_GetProxyFromInfo( SceneInfo ),
			FVector, NewOffset, m_Offset,
		{
			if( Proxy )
			{
				Proxy->m_Offset = NewOffset;
			}
		});
	}
	for( INT Index = 0; Index < Attachments.Num(); Index++ )
	{
		UDishonoredPlayerSkeletalComponent* Child = Cast<UDishonoredPlayerSkeletalComponent>( Attachments(Index).Component );
		if( Child )
		{
			Child->SetOffset( m_Offset );
		}
	}
}

// 2012 0x71d880: the FOV flag is raised; a new value (or a flag that was down) goes to the proxy and the attached player skeletal components
void UDishonoredPlayerSkeletalComponent::SetFOV( FLOAT NewFOV )
{
	const UBOOL bWasUsingFOV = m_bUseFOV;
	m_bUseFOV = TRUE;
	if( m_FOV == NewFOV && bWasUsingFOV )
	{
		return;
	}
	m_FOV = NewFOV;
	if( SceneInfo )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateFOVCommand,
			FDishonoredPlayerSkeletalSceneProxy*, Proxy, (FDishonoredPlayerSkeletalSceneProxy*)Scene_GetProxyFromInfo( SceneInfo ),
			FLOAT, FOV, m_FOV,
		{
			if( Proxy )
			{
				Proxy->m_bUseFOV = TRUE;
				Proxy->m_FOV = FOV;
			}
		});
	}
	for( INT Index = 0; Index < Attachments.Num(); Index++ )
	{
		UDishonoredPlayerSkeletalComponent* Child = Cast<UDishonoredPlayerSkeletalComponent>( Attachments(Index).Component );
		if( Child )
		{
			Child->SetFOV( m_FOV );
		}
	}
}

// 2012 0x71dab0: only a raised flag is lowered, on the proxy and down the attached player skeletal components
void UDishonoredPlayerSkeletalComponent::DisableFOV()
{
	if( !m_bUseFOV )
	{
		return;
	}
	m_bUseFOV = FALSE;
	if( SceneInfo )
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			DisableFOVCommand,
			FDishonoredPlayerSkeletalSceneProxy*, Proxy, (FDishonoredPlayerSkeletalSceneProxy*)Scene_GetProxyFromInfo( SceneInfo ),
		{
			if( Proxy )
			{
				Proxy->m_bUseFOV = FALSE;
			}
		});
	}
	for( INT Index = 0; Index < Attachments.Num(); Index++ )
	{
		UDishonoredPlayerSkeletalComponent* Child = Cast<UDishonoredPlayerSkeletalComponent>( Attachments(Index).Component );
		if( Child )
		{
			Child->DisableFOV();
		}
	}
}

// 2012 0x7203d0: the bounds follow the offset, and on the player pawn they are re-centred from the root bone onto the camera bone
void UDishonoredPlayerSkeletalComponent::UpdateBounds()
{
	Super::UpdateBounds();
	Bounds.Origin += m_Offset;
	const ADishonoredPlayerPawn* Player = ConstCast<ADishonoredPlayerPawn>( Owner );
	if( Player && Player->Mesh )
	{
		const FVector RootPos = Player->Mesh->GetBoneMatrix( 0 ).GetOrigin();
		FVector CameraPos( 0.f, 0.f, 0.f );
		Player->GetCameraBone( &CameraPos, NULL );
		Bounds.Origin.X += CameraPos.X - RootPos.X;
		Bounds.Origin.Y += CameraPos.Y - RootPos.Y;
	}
}

// 2012 0x707580: USkeletalMeshComponent::UpdateChildComponents with the offset added to the bone position of every attachment that is
// neither a skeletal nor a static mesh component (a skeletal one carries the offset itself, a static one gets none)
void UDishonoredPlayerSkeletalComponent::UpdateChildComponents()
{
	for( INT AttachmentIndex = 0; AttachmentIndex < Attachments.Num(); AttachmentIndex++ )
	{
		FAttachment& Attachment = Attachments(AttachmentIndex);
		const INT BoneIndex = MatchRefBone( Attachment.BoneName );
		if( !Attachment.Component || BoneIndex == INDEX_NONE || BoneIndex >= SpaceBases.Num() )
		{
			continue;
		}
		FVector Offset( 0.f, 0.f, 0.f );
		if( !Attachment.Component->IsA( USkeletalMeshComponent::StaticClass() ) && !Attachment.Component->IsA( UStaticMeshComponent::StaticClass() ) )
		{
			Offset = m_Offset;
		}
		const FVector RelativeScale = ( Attachment.RelativeScale == FVector( 0.f ) ) ? FVector( 1.f ) : Attachment.RelativeScale;
		FMatrix BoneToWorld = SpaceBases(BoneIndex).ToMatrix() * LocalToWorld;
		BoneToWorld.M[3][0] += Offset.X;
		BoneToWorld.M[3][1] += Offset.Y;
		BoneToWorld.M[3][2] += Offset.Z;
		const FMatrix AttachmentToWorld = FScaleRotationTranslationMatrix( RelativeScale, Attachment.RelativeRotation, Attachment.RelativeLocation ) * BoneToWorld;
		SetAttachmentOwnerVisibility( Attachment.Component );
		Attachment.Component->UpdateComponent( Scene, GetOwner(), AttachmentToWorld );
	}
}

// 2012 0x720360: the second camera pass of the frame, with the pose final - the Dishonored camera is updated again with no time
// passing and m_PassCount 1, which is the pass that places the items (ADishonoredPawn::OnCameraUpdate) and the crosshair
void UDishonoredPlayerSkeletalComponent::UpdateChildForTickEnd_Derived()
{
	const ADishonoredPlayerPawn* Player = ConstCast<ADishonoredPlayerPawn>( Owner );
	APlayerController* PC = Player ? Cast<APlayerController>( Player->Controller ) : NULL;
	ADishonoredPlayerCamera* Camera = PC ? Cast<ADishonoredPlayerCamera>( PC->PlayerCamera ) : NULL;
	if( Camera )
	{
		Camera->m_PassCount = 1;
		Camera->UpdateCamera( 0.f );
		Camera->m_PassCount = 0;
	}
}
