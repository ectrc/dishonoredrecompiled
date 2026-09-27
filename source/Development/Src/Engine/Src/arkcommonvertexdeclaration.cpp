// Engine/src/arkcommonvertexdeclaration.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x434740  class TDynamicRHIResourceReference<6> const & __cdecl ArkGetCommonVertexDeclaration(enum ArkCommonVertexDeclaration)
//   0x4378f0  public: virtual __thiscall `anonymous namespace'::FCommonVertexDeclaration::~FCommonVertexDeclaration(void)
//   0x43e3e0  public: virtual void __thiscall `anonymous namespace'::FCommonVertexDeclaration::InitRHI(void)
//   0x442a00  public: virtual void __thiscall `anonymous namespace'::FCommonVertexDeclaration::ReleaseRHI(void)
//   0xb9b0e0  _anonymous_namespace_::_dynamic_initializer_for__GCommonVertexDeclaration__

#include "EnginePrivate.h"
#include "arkcommonvertexdeclaration.h"

namespace
{
	/**
	 * DISHONORED(port): the three vertex declarations Arkane's full-screen passes draw with (2013 rva 0x41a3f0
	 * InitRHI): a float2 position, a float4 position, and a float2 position with a float2 texture coordinate. The
	 * DisFog pass and the FArkPp nodes use the first one with FullScreenTriangleFloat2Vertices, stride 8.
	 */
	class FCommonVertexDeclaration : public FRenderResource
	{
	public:
		FVertexDeclarationRHIRef mRhi[ARK_COMMON_VD_MAX];

		virtual ~FCommonVertexDeclaration() {}

		virtual void InitRHI()
		{
			{
				FVertexDeclarationElementList Elements;
				Elements.AddItem(FVertexElement(0,0,VET_Float2,VEU_Position,0));
				mRhi[ARK_COMMON_VD_FLOAT2] = RHICreateVertexDeclaration(Elements);
			}
			{
				FVertexDeclarationElementList Elements;
				Elements.AddItem(FVertexElement(0,0,VET_Float4,VEU_Position,0));
				mRhi[ARK_COMMON_VD_FLOAT4] = RHICreateVertexDeclaration(Elements);
			}
			{
				FVertexDeclarationElementList Elements;
				Elements.AddItem(FVertexElement(0,0,VET_Float2,VEU_Position,0));
				Elements.AddItem(FVertexElement(0,8,VET_Float2,VEU_TextureCoordinate,0));
				mRhi[ARK_COMMON_VD_FLOAT2_UV] = RHICreateVertexDeclaration(Elements);
			}
		}

		virtual void ReleaseRHI()
		{
			for (INT Index = 0; Index < ARK_COMMON_VD_MAX; Index++)
			{
				mRhi[Index].SafeRelease();
			}
		}
	};

	TGlobalResource<FCommonVertexDeclaration> GCommonVertexDeclaration;
}

/** DISHONORED(port): 2013 rva 0x410900. */
const FVertexDeclarationRHIRef& ArkGetCommonVertexDeclaration(EArkCommonVertexDeclaration iValue)
{
	return GCommonVertexDeclaration.mRhi[iValue];
}

/** DISHONORED(layout): 2012 data 0xe32ddc - the clip-space triangle that covers the screen. */
const FVector2D ArkFullScreenTriangleFloat2Vertices[3] =
{
	FVector2D(-1.0f,-1.0f),
	FVector2D(+3.0f,-1.0f),
	FVector2D(-1.0f,+3.0f),
};
