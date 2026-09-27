/*=============================================================================
	TileRendering.h: Simple tile rendering implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_TILERENDERING
#define _INC_TILERENDERING

/** 
* vertex data for a screen quad 
*/
struct FMaterialTileVertex
{
	FVector			Position;
	FPackedNormal	TangentX,
					TangentZ;
	DWORD			Color;
	FLOAT			U,
					V;

	inline void Initialize(FLOAT InX, FLOAT InY, FLOAT InU, FLOAT InV)
	{
		Position.X = InX; Position.Y = InY; Position.Z = 0.0f;
		TangentX = FVector(1, 0, 0); 
		//TangentY = FVector(0, 1, 0); 
		TangentZ = FVector(0, 0, 1);
		// TangentZ.w contains the sign of the tangent basis determinant. Assume +1
		TangentZ.Vector.W = 255;
		Color = FColor(255,255,255,255).DWColor();
		U = InU; V = InV;
	}
};

/** 
* Vertex buffer
*/
class FMaterialTileVertexBuffer : public FVertexBuffer
{
public:
	/** 
	* Initialize the RHI for this rendering resource 
	*/
	void InitRHI()
	{
		// used with a tristrip, so only 4 vertices are needed
		DWORD Size = 4 * sizeof(FMaterialTileVertex);
		// create vertex buffer
		VertexBufferRHI = RHICreateVertexBuffer(Size,NULL,RUF_Static);
		// lock it
		void* Buffer = RHILockVertexBuffer(VertexBufferRHI,0,Size,FALSE);
        	// first vertex element
		FMaterialTileVertex* DestVertex = (FMaterialTileVertex*)Buffer;

		// fill out the verts
		DestVertex[0].Initialize(1, -1, 1, 1);
		DestVertex[1].Initialize(1, 1, 1, 0);
		DestVertex[2].Initialize(-1, -1, 0, 1);
		DestVertex[3].Initialize(-1, 1, 0, 0);

		// Unlock the buffer.
		RHIUnlockVertexBuffer(VertexBufferRHI);        
	}
};

class FTileRenderer
{
public:
	FTileRenderer();

	/**
	 * Draw a full view tile at using the full material (UV = [0..1]
	 */
	void DrawTile(const class FViewInfo& View, const FMaterialRenderProxy* MaterialRenderProxy);

	/**
	 * Draw a tile at the given location and size, using the full material
	 * (UV = [0..1]
	 */
	void DrawTile(const class FViewInfo& View, const FMaterialRenderProxy* MaterialRenderProxy, FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY, UBOOL bIsHitTesting=FALSE, const FHitProxyId HitProxyId=FHitProxyId());

	/**
	 * Draw a tile at the given location and size, using tjhe given UVs
	 * (UV = [0..1]
	 */
	void DrawTile(const class FViewInfo& View, const FMaterialRenderProxy* MaterialRenderProxy, FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY, FLOAT U, FLOAT V, FLOAT SizeU, FLOAT SizeV, UBOOL bIsHitTesting=FALSE, const FHitProxyId HitProxyId=FHitProxyId());

	/**
	 * DISHONORED(port): 2013 rva 0x51e510 (2012 0x55f000) (tilerendering.h:117) - a full-view tile drawn through a policy factory of
	 * its own, with that factory's context. Arkane's post-process material node draws every node of its graph this way
	 * (TPpMaterialDrawingPolicyFactory, arkppnodematerial.cpp), which is why the reference DrawTile above - hard-wired
	 * to the base-pass and translucency factories - gained this template in retail rather than a fourth overload.
	 */
	template<typename FactoryType>
	void DrawTile(const class FViewInfo& View, const FMaterialRenderProxy* MaterialRenderProxy, typename FactoryType::ContextType Context)
	{
		Mesh.UseDynamicData = FALSE;
		Mesh.MaterialRenderProxy = MaterialRenderProxy;
		PrepareShaders<FactoryType>(View, MaterialRenderProxy, Context, FMatrix::Identity, TRUE, FALSE, FHitProxyId());
	}

private:
	/**
	 * DISHONORED(port): 2013 rva 0x51d9c0 (2012 0x55e510) (tilerendering.h:128) - the template PrepareShaders the one above calls: it
	 * takes the same identity view-projection detour as the reference one, sets the view parameters and hands the
	 * shared FMeshBatch to the factory. It neither reads the material's blend mode (retail fetches it and drops the
	 * result) nor sets a blend state: a post-process node pass owns its own.
	 */
	template<typename FactoryType>
	void PrepareShaders(const class FViewInfo& View, const FMaterialRenderProxy* MaterialRenderProxy, typename FactoryType::ContextType Context, const FMatrix& LocalToWorld, UBOOL bUseIdentityViewProjection, UBOOL bIsHitTesting, const FHitProxyId HitProxyId)
	{
		static FMatrix SaveViewProjectionMatrix = FMatrix::Identity;
		static FVector4 SaveViewOrigin(0,0,0,0);
		static FVector SavePreViewTranslation(0,0,0);
		if (bUseIdentityViewProjection)
		{
			SaveViewProjectionMatrix = View.TranslatedViewProjectionMatrix;
			SaveViewOrigin = View.ViewOrigin;
			SavePreViewTranslation = View.PreViewTranslation;
			const_cast<FViewInfo&>(View).TranslatedViewProjectionMatrix = FMatrix::Identity;
			const_cast<FViewInfo&>(View).ViewOrigin.Set( 0, 0, 0, 0 );
			const_cast<FViewInfo&>(View).PreViewTranslation.Set( 0, 0, 0 );
		}

		RHISetViewParameters(View);

		Mesh.Elements(0).LocalToWorld = LocalToWorld;
		FactoryType::DrawDynamicMesh(View, Context, Mesh, FALSE, FALSE, NULL, HitProxyId);

		if (bUseIdentityViewProjection)
		{
			const_cast<FViewInfo&>(View).TranslatedViewProjectionMatrix = SaveViewProjectionMatrix;
			const_cast<FViewInfo&>(View).ViewOrigin = SaveViewOrigin;
			const_cast<FViewInfo&>(View).PreViewTranslation = SavePreViewTranslation;
		}
	}

	static UBOOL bInitialized;
	/** Global vertex factory used by material post process effects */
	static TGlobalResource<FLocalVertexFactory> VertexFactory;
	/** Has the four screen verts for the screen quad */
	static TGlobalResource<FMaterialTileVertexBuffer> VertexBuffer;
	/** Shared mesh element for dynamic mesh draw calls */
	static FMeshBatch Mesh;

	/**
	 * Set up the shader appropriate for this tile to use the given matrix
	 */
	void PrepareShaders(const class FViewInfo& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMatrix& LocalToWorld, UBOOL bUseIdentityViewProjection, UBOOL bIsHitTesting=FALSE, const FHitProxyId HitProxyId=FHitProxyId());
};

#endif
