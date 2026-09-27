#pragma once
// GFxUI/inc/gfxuirendererimpl.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x5bc560  public: virtual void __thiscall FGFxFilterPixelShader<4>::SetParameterColorMatrix(class TDynamicRHIResource<8> *, float const *)const
//   0x5bc5d0  public: virtual void __thiscall FGFxFilterPixelShader<20>::SetParameterTexScale(class TDynamicRHIResource<8> *, int, float, float)const
//   0x5bc620  public: virtual void __thiscall FGFxFilterPixelShader<7>::SetParametersCxformAc(class TDynamicRHIResource<8> *, class GRenderer::Cxform const &)const
//   0x5bc6e0  public: virtual void __thiscall FGFxFilterPixelShader<15>::SetParameterFilterSize4(class TDynamicRHIResource<8> *, float, float, float, float)const
//   0x5bc730  public: virtual class FShader * __thiscall FGFxFilterPixelShader<5>::GetNativeShader(void)
//   0x5bc740  public: virtual void __thiscall FGFxFilterPixelShader<29>::SetParameterTextureRHI(class TDynamicRHIResource<8> *, class TDynamicRHIResource<1> *, class TDynamicRHIResource<13> *, int)const
//   0x5bc7a0  public: virtual void __thiscall FGFxFilterPixelShader<12>::SetParameterShadowColor(class TDynamicRHIResource<8> *, int, class GColor)const
//   0x5bc820  public: virtual void __thiscall FGFxFilterPixelShader<24>::SetParameterInverseGamma(class TDynamicRHIResource<8> *, float)const
//   0x5bc840  public: virtual void __thiscall FGFxFilterPixelShader<19>::SetParameterShadowOffset(class TDynamicRHIResource<8> *, float, float)const
//   0x5bc890  public: __thiscall FGFxFilterPixelShader<1>::FGFxFilterPixelShader<1>(void)
//   0x5bc920  public: virtual unsigned int __thiscall FGFxFilterPixelShader<15>::Serialize(class FArchive &)
//   0x5bc9d0  public: virtual void __thiscall FGFxPixelShader<42>::SetDistanceFieldParams(class TDynamicRHIResource<8> *, class UTexture2D const *, struct GRenderer::DistanceFieldParams const &)const
//   0x5bcbe0  public: __thiscall FGFxPixelShader<50>::FGFxPixelShader<50>(void)
//   0x5bcc80  public: virtual unsigned int __thiscall FGFxPixelShader<50>::Serialize(class FArchive &)
//   0x5bcd70  public: virtual class FGFxPixelShaderInterface * __thiscall FGFxPixelShader<38>::GetShaderInterface(void)
//   0x5bcd80  public: virtual void __thiscall FGFxFilterPixelShader<11>::SetParametersColorScaleAndColorBias(class TDynamicRHIResource<8> *, class GRenderer::Cxform const &)const
//   0x5bce30  public: virtual void __thiscall FGFxPixelShader<31>::SetParameterConstantColor(class TDynamicRHIResource<8> *, struct FLinearColor const &)const
//   0x5bce50  public: virtual void __thiscall FGFxPixelShader<30>::SetParameterInverseGamma(class TDynamicRHIResource<8> *, float)const
//   0x5bce70  public: virtual void __thiscall FGFxVertexShader<5>::SetParameterTransform(class TDynamicRHIResource<7> *, class FMatrix const &)const
//   0x5bce90  public: virtual void __thiscall FGFxVertexShader<6>::SetParameterTextureMatrix(class TDynamicRHIResource<7> *, class FMatrix const &, int)const
//   0x5bcec0  public: virtual void __thiscall FGFxVertexShader<6>::SetParameters(class TDynamicRHIResource<7> *, class FMatrix const &, class FMatrix const &, class FMatrix const &)const
//   0x5bcf00  public: virtual unsigned int __thiscall FGFxVertexShader<2>::Serialize(class FArchive &)
//   0x5c5250  public: static class FShader * __cdecl FGFxFilterPixelShader<4>::ConstructSerializedInstance(void)
//   0x5c52c0  public: static class FShader * __cdecl FGFxPixelShader<50>::ConstructSerializedInstance(void)
//   0x5c5330  public: static class FShader * __cdecl FGFxVertexShader<7>::ConstructSerializedInstance(void)
//   0x5d5b90  public: virtual unsigned int __thiscall UGFxMappableTexture::UpdateStreamingStatus(unsigned int)
//   0x5d5bb0  public: __thiscall FGFxUpdatableTexture::FGFxUpdatableTexture(unsigned int, unsigned int, unsigned int, enum EPixelFormat, unsigned int)
//   0x5d5cb0  public: virtual class FTextureResource * __thiscall UGFxMappableTexture::CreateResource(void)
//   0x5d5cf0  public: virtual void __thiscall FGFxUpdatableTexture::InitRHI(void)
//   0x5d5e60  public: virtual void __thiscall FGFxUpdatableTexture::ReleaseRHI(void)
//   0x5d5f50  public: virtual class FTextureResource * __thiscall UGFxUpdatableTexture::CreateResource(void)
// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the render-thread half of the renderer seam. Three things live here, and the
// PDB puts all three in this header: the GFx shader families and the two abstract interfaces the
// renderer drives them through, the element stores GRenderer's SetVertexData / SetIndexData /
// DrawBitmaps fill, and the updatable and mappable textures.
//
// FGFxRendererImpl is a NAMESPACE in retail, not a class: every one of its PDB entries demangles
// as `void __cdecl FGFxRendererImpl::f(...)`, never as `public: static void __cdecl`.
//
// The element-store layouts are the PDB's:
//   FGFxRendererImpl::FGFxRenderElementStoreBase  sizeof 32, base GRendererNode, members
//     @8 GAtomicInt<unsigned long> RefCount, @12 GRenderer::CachedData* CachedData,
//     @16 ULONG ElementSize, @20 ULONG NumElements, @24 void* Elements, @28 bool AllocatedElements
//   FGFxRendererImpl::FGFxVertexStore / FGFxIndexStore / FGFxBitmapDescStore  sizeof 36 each
// and so are the shader ones: FGFxVertexShader<N> 132 (Transform @112, TextureMatrix[2] @118),
// FGFxPixelShader<N> 208 (4 textures @112, then 12 constants @136..@202),
// FGFxPixelShaderInterface / FGFxVertexShaderInterface 4 (pure interfaces, 13 and 5 slots),
// FGFxUpdatableTexture 92 (base FTextureResource, Width @68 .. Texture2DRHI @88).
//
// 2012 rvas of the bodies, all decompiled headlessly into build/agentCC/dec, dec2:
//   FGFxRendererImpl::ConvertFromUI(FillTexture,FFillTextureInfo)  0x5b7710 -> 2013 0x572c30
//   FGFxRendererImpl::ConvertFromUI(GColor,FLinearColor&)          0x5badc0
//   FGFxRendererImpl::ApplyUIColor_RenderThread                    0x5bae50
//   FGFxRendererImpl::ApplyUIBlendMode_RenderThread                0x5cc020
//   FGFxRendererImpl::GetUIVertexDecl_RenderThread                 0x5babe0
//   FGFxRendererImpl::GetUIBoundShaderState_RenderThread           0x5daef0
//   FGFxRendererImpl::DrawUIBackgroundColor_RenderThread           0x5db0d0
//   FGFxRendererImpl::SetUIRenderElementStore<FGFxVertexStore>     0x5d8a10
//   FGFxRendererImpl::LoadTexture                                  0x5b8060
//   FGFxRendererImpl::SoftwareResample                             0x5b7f40
//   GetUIPixelShaderInterface_RenderThread                         0x5d77f0
//   GetUIVertexShaderInterface_RenderThread                        0x5d7b00
#include "gfxuirenderer.h"
#include "EnginePrivate.h"
#include "ShaderManager.h"
#include "GlobalShader.h"

// ---------------------------------------------------------------------------------------------
// DISHONORED(layout): the three shader enumerations, exactly as the 2012 PDB has them
// (resources/tools/pdb/dia_types.py --enum-re "^EGFx", build/agentCC/egfx_enums.json). The filter
// kinds and the pixel-shader kinds are ONE enum in retail - FS2_* 0..29 and GFx_PS_* 30..50 - which
// is why FGFxFilterPixelShader and FGFxPixelShader are both templated on EGFxPixelShaderType.
// ---------------------------------------------------------------------------------------------
enum EGFxPixelShaderType
{
    FS2_None                                    = 0,
    FS2_start_shadows                           = 1,
    FS2_FBox2InnerShadow                        = 1,
    FS2_FBox2InnerShadowHighlight               = 2,
    FS2_FBox2InnerShadowMul                     = 3,
    FS2_FBox2InnerShadowMulHighlight            = 4,
    FS2_FBox2InnerShadowKnockout                = 5,
    FS2_FBox2InnerShadowHighlightKnockout       = 6,
    FS2_FBox2InnerShadowMulKnockout             = 7,
    FS2_FBox2InnerShadowMulHighlightKnockout    = 8,
    FS2_FBox2Shadow                             = 9,
    FS2_FBox2ShadowHighlight                    = 10,
    FS2_FBox2ShadowMul                          = 11,
    FS2_FBox2ShadowMulHighlight                 = 12,
    FS2_FBox2ShadowKnockout                     = 13,
    FS2_FBox2ShadowHighlightKnockout            = 14,
    FS2_FBox2ShadowMulKnockout                  = 15,
    FS2_FBox2ShadowMulHighlightKnockout         = 16,
    FS2_FBox2Shadowonly                         = 17,
    FS2_FBox2ShadowonlyHighlight                = 18,
    FS2_FBox2ShadowonlyMul                      = 19,
    FS2_FBox2ShadowonlyMulHighlight             = 20,
    FS2_end_shadows                             = 20,
    FS2_start_blurs                             = 21,
    FS2_FBox2Blur                               = 22,
    FS2_FBox2BlurMul                            = 24,
    FS2_FBox1Blur                               = 25,
    FS2_FBox1BlurMul                            = 27,
    FS2_end_blurs                               = 27,
    FS2_start_cmatrix                           = 28,
    FS2_FCMatrix                                = 28,
    FS2_FCMatrixMul                             = 29,
    FS2_end_cmatrix                             = 29,
    GFx_PS_SolidColor                           = 30,
    GFx_PS_CxformTexture                        = 31,
    GFx_PS_CxformTextureMultiply                = 32,
    GFx_PS_TextTexture                          = 33,
    GFx_PS_TextTextureColor                     = 34,
    GFx_PS_TextTextureColorMultiply             = 35,
    GFx_PS_TextTextureSRGB                      = 36,
    GFx_PS_TextTextureSRGBMultiply              = 37,
    GFx_PS_CxformGouraud                        = 38,
    GFx_PS_CxformGouraudNoAddAlpha              = 39,
    GFx_PS_CxformGouraudTexture                 = 40,
    GFx_PS_Cxform2Texture                       = 41,
    GFx_PS_CxformGouraudMultiply                = 42,
    GFx_PS_CxformGouraudMultiplyNoAddAlpha      = 43,
    GFx_PS_CxformGouraudMultiplyTexture         = 44,
    GFx_PS_CxformMultiply2Texture               = 45,
    GFx_PS_TextTextureYUV                       = 46,
    GFx_PS_TextTextureYUVMultiply               = 47,
    GFx_PS_TextTextureYUVA                      = 48,
    GFx_PS_TextTextureYUVAMultiply              = 49,
    GFx_PS_TextTextureDFA                       = 50,
    GFx_PS_Count                                = 51
};

enum EGFxVertexShaderType
{
    GFx_VS_None                     = 0,
    GFx_VS_Strip                    = 1,
    GFx_VS_Glyph                    = 2,
    GFx_VS_XY16iC32                 = 3,
    GFx_VS_XY16iCF32                = 4,
    GFx_VS_XY16iCF32_NoTex          = 5,
    GFx_VS_XY16iCF32_NoTexNoAlpha   = 6,
    GFx_VS_XY16iCF32_T2             = 7,
    GFx_VS_Count                    = 8
};

enum EGFxVertexDeclarationType
{
    GFx_VD_None     = 0,
    GFx_VD_Strip    = 1,
    GFx_VD_Glyph    = 2,
    GFx_VD_XY16iC32 = 3,
    GFx_VD_XY16iCF32= 4,
    GFx_VD_Count    = 5
};

// DISHONORED(layout): the two vertex kinds the renderer builds itself. PDB sizeof 4 and 20.
struct FGFxVertex_XY16i
{
    SWORD X; // @0
    SWORD Y; // @2
};

struct FGFxVertex_Glyph
{
    FLOAT  X;     // @0
    FLOAT  Y;     // @4
    FLOAT  U;     // @8
    FLOAT  V;     // @12
    GColor Color; // @16
};

// The colour transform helpers the shader setters share. GFx 3.3's Cxform::M_ is float[4][2] -
// channel-major, multiply in column 0, add in column 1 - and GFx3Gen.h spells the same 32 bytes
// [2][4], so the element is addressed here by index rather than by M_[c][k] (agentCC.md; the
// generated header is agent BB's and stays as the generator writes it).
FORCEINLINE FLOAT FGFxCxformMul(const GRenderer::Cxform& Cx, INT Channel)
{
    return reinterpret_cast<const FLOAT*>(&Cx.M_[0][0])[Channel * 2 + 0];
}
FORCEINLINE FLOAT FGFxCxformAdd(const GRenderer::Cxform& Cx, INT Channel)
{
    return reinterpret_cast<const FLOAT*>(&Cx.M_[0][0])[Channel * 2 + 1];
}
FORCEINLINE void FGFxCxformToScaleAndBias(const GRenderer::Cxform& Cx, FVector4& OutScale,
                                          FVector4& OutBias)
{
    OutScale = FVector4(FGFxCxformMul(Cx,0),FGFxCxformMul(Cx,1),FGFxCxformMul(Cx,2),FGFxCxformMul(Cx,3));
    OutBias = FVector4(FGFxCxformAdd(Cx,0) / 255.f,FGFxCxformAdd(Cx,1) / 255.f,
                       FGFxCxformAdd(Cx,2) / 255.f,FGFxCxformAdd(Cx,3) / 255.f);
}
FORCEINLINE void FGFxCxformSetIdentity(GRenderer::Cxform& Cx)
{
    FLOAT* M = reinterpret_cast<FLOAT*>(&Cx.M_[0][0]);
    for (INT Channel = 0; Channel < 4; Channel++)
    {
        M[Channel * 2 + 0] = 1.f;
        M[Channel * 2 + 1] = 0.f;
    }
}
FORCEINLINE GColor FGFxCxformTransform(const GRenderer::Cxform& Cx, GColor In)
{
    // GRenderer::Cxform::Transform: each channel is clamp(channel * multiply + add).
    const FLOAT R = Clamp<FLOAT>(In.Channels.Red   * FGFxCxformMul(Cx,0) + FGFxCxformAdd(Cx,0),0.f,255.f);
    const FLOAT G = Clamp<FLOAT>(In.Channels.Green * FGFxCxformMul(Cx,1) + FGFxCxformAdd(Cx,1),0.f,255.f);
    const FLOAT B = Clamp<FLOAT>(In.Channels.Blue  * FGFxCxformMul(Cx,2) + FGFxCxformAdd(Cx,2),0.f,255.f);
    const FLOAT A = Clamp<FLOAT>(In.Channels.Alpha * FGFxCxformMul(Cx,3) + FGFxCxformAdd(Cx,3),0.f,255.f);
    return GColor((GUByte)R,(GUByte)G,(GUByte)B,(GUByte)A);
}
FORCEINLINE FVector4 FGFxColorToVector4(GColor Color)
{
    return FVector4(Color.Channels.Red / 255.f,Color.Channels.Green / 255.f,
                    Color.Channels.Blue / 255.f,Color.Channels.Alpha / 255.f);
}

// The 2D affine transform, written into an FMatrix the way retail writes it (2012 0x5bb480).
FORCEINLINE FMatrix FGFxMatrix2DToNative(const GMatrix2D& In)
{
    FMatrix Out(FMatrix::Identity);
    Out.M[0][0] = In.M_[0][0];
    Out.M[0][1] = In.M_[1][0];
    Out.M[1][0] = In.M_[0][1];
    Out.M[1][1] = In.M_[1][1];
    Out.M[2][0] = 0.f;
    Out.M[2][1] = 0.f;
    Out.M[3][0] = In.M_[0][2];
    Out.M[3][1] = In.M_[1][2];
    return Out;
}

// GMatrix2D::Append - this = Other * this: the other transform is applied after this one, which is
// what SetUIViewport does with the user matrix. Prepend below is the other order.
FORCEINLINE void FGFxMatrix2DAppend(GMatrix2D& M, const GMatrix2D& Other)
{
    GMatrix2D R;
    R.M_[0][0] = Other.M_[0][0] * M.M_[0][0] + Other.M_[0][1] * M.M_[1][0];
    R.M_[0][1] = Other.M_[0][0] * M.M_[0][1] + Other.M_[0][1] * M.M_[1][1];
    R.M_[0][2] = Other.M_[0][0] * M.M_[0][2] + Other.M_[0][1] * M.M_[1][2] + Other.M_[0][2];
    R.M_[1][0] = Other.M_[1][0] * M.M_[0][0] + Other.M_[1][1] * M.M_[1][0];
    R.M_[1][1] = Other.M_[1][0] * M.M_[0][1] + Other.M_[1][1] * M.M_[1][1];
    R.M_[1][2] = Other.M_[1][0] * M.M_[0][2] + Other.M_[1][1] * M.M_[1][2] + Other.M_[1][2];
    M = R;
}

// GMatrix2D::Prepend - this = this * Other, in GFx's row-major 2x3 convention. The runtime's own
// body is in libgfx; the seam needs it for ApplyUITransform_RenderThread.
FORCEINLINE void FGFxMatrix2DPrepend(GMatrix2D& M, const GMatrix2D& Other)
{
    GMatrix2D R;
    R.M_[0][0] = M.M_[0][0] * Other.M_[0][0] + M.M_[0][1] * Other.M_[1][0];
    R.M_[0][1] = M.M_[0][0] * Other.M_[0][1] + M.M_[0][1] * Other.M_[1][1];
    R.M_[0][2] = M.M_[0][0] * Other.M_[0][2] + M.M_[0][1] * Other.M_[1][2] + M.M_[0][2];
    R.M_[1][0] = M.M_[1][0] * Other.M_[0][0] + M.M_[1][1] * Other.M_[1][0];
    R.M_[1][1] = M.M_[1][0] * Other.M_[0][1] + M.M_[1][1] * Other.M_[1][1];
    R.M_[1][2] = M.M_[1][0] * Other.M_[0][2] + M.M_[1][1] * Other.M_[1][2] + M.M_[1][2];
    M = R;
}


// ---------------------------------------------------------------------------------------------
// The two interfaces the renderer drives the shader families through. PDB sizeof 4 each; the slots
// are in PDB order and there is no same-name run, so declaration order is slot order.
// ---------------------------------------------------------------------------------------------
class FGFxPixelShaderInterface
{
public:
    virtual ~FGFxPixelShaderInterface() {}

    virtual void SetParameterTextureRHI(FPixelShaderRHIParamRef PixelShader,
                                        FSamplerStateRHIParamRef SamplerState,
                                        FTextureRHIParamRef Texture, INT i) const = 0;      // slot 0
    virtual void SetParameterConstantColor(FPixelShaderRHIParamRef PixelShader,
                                           const FLinearColor& Color) const = 0;            // slot 1
    virtual void SetParametersColorScaleAndColorBias(FPixelShaderRHIParamRef PixelShader,
                                                     const GRenderer::Cxform& Cx) const = 0; // slot 2
    virtual void SetParametersCxformAc(FPixelShaderRHIParamRef PixelShader,
                                       const GRenderer::Cxform& Cx) const {}                // slot 3
    virtual void SetParameterColorMatrix(FPixelShaderRHIParamRef PixelShader,
                                         const FLOAT* Matrix) const {}                      // slot 4
    virtual void SetParameterTexScale(FPixelShaderRHIParamRef PixelShader, INT i, FLOAT X,
                                      FLOAT Y) const {}                                     // slot 5
    virtual void SetParameterFilterSize4(FPixelShaderRHIParamRef PixelShader, FLOAT X, FLOAT Y,
                                         FLOAT Z, FLOAT W) const {}                         // slot 6
    virtual void SetParameterShadowColor(FPixelShaderRHIParamRef PixelShader, INT i,
                                         GColor Color) const {}                             // slot 7
    virtual void SetParameterShadowOffset(FPixelShaderRHIParamRef PixelShader, FLOAT X,
                                          FLOAT Y) const {}                                 // slot 8
    virtual void SetParameterInverseGamma(FPixelShaderRHIParamRef PixelShader,
                                          const FLOAT InverseGamma) const = 0;              // slot 9
    virtual void SetDistanceFieldParams(FPixelShaderRHIParamRef PixelShader,
                                        const UTexture2D* Texture,
                                        const GRenderer::DistanceFieldParams& Params) const = 0; // slot 10
    virtual FShader* GetNativeShader() = 0;                                                 // slot 11
    virtual FGFxPixelShaderInterface* GetShaderInterface() = 0;                              // slot 12
};

class FGFxVertexShaderInterface
{
public:
    virtual ~FGFxVertexShaderInterface() {}

    virtual void SetParameters(FVertexShaderRHIParamRef VertexShader, const FMatrix& Transform,
                               const FMatrix& TextureMatrix,
                               const FMatrix& TextureMatrix2) const = 0;                    // slot 0
    virtual void SetParameterTransform(FVertexShaderRHIParamRef VertexShader,
                                       const FMatrix& Transform) const = 0;                 // slot 1
    virtual void SetParameterTextureMatrix(FVertexShaderRHIParamRef VertexShader,
                                           const FMatrix& TextureMatrix, INT i) const = 0;  // slot 2
    virtual FShader* GetNativeShader() = 0;                                                 // slot 3
    virtual FGFxVertexShaderInterface* GetShaderInterface() = 0;                             // slot 4
};

// What a style asks for (PDB FGFxEnumeratedBoundShaderState, sizeof 12) and what the cache answers
// with (FGFxBoundShaderState, sizeof 12), plus the context a style decides from
// (FGFxRenderStyleContext, sizeof 8).
struct FGFxEnumeratedBoundShaderState
{
    EGFxPixelShaderType       PixelShaderType;       // @0
    EGFxVertexShaderType      VertexShaderType;      // @4
    EGFxVertexDeclarationType VertexDeclarationType; // @8
};

struct FGFxBoundShaderState
{
    FGFxPixelShaderInterface*  PixelShaderInterface;   // @0
    FGFxVertexShaderInterface* VertexShaderInterface;  // @4
    FBoundShaderStateRHIRef    NativeBoundShaderState; // @8
};

struct FGFxRenderStyleContext
{
    GRenderer::VertexFormat VertexFmt; // @0
    GRenderer::BlendType    BlendFmt;  // @4
};

// ---------------------------------------------------------------------------------------------
// DISHONORED(layout): the GFx vertex shader family. Three parameters - the 2D transform and two
// texture matrices (2012 Serialize 0x5bcf00, SetParameterTransform 0x5bce70). The 2D transform is
// written into an FMatrix as the row-vector affine retail writes
// (FGFxFillStyle::StaticApplyTextureMatrix_RenderThread, 2012 0x5bb480):
//   M[0][0]=m00 M[0][1]=m10  M[1][0]=m01 M[1][1]=m11  M[3][0]=m02 M[3][1]=m12
// so the shader's mul() is (x,y,0,1) * M.
// ---------------------------------------------------------------------------------------------
template<EGFxVertexShaderType ShaderType>
class FGFxVertexShader : public FGlobalShader, public FGFxVertexShaderInterface
{
    DECLARE_SHADER_TYPE(FGFxVertexShader,Global);
public:

    static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

    static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

    FGFxVertexShader() {}

    FGFxVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
        FGlobalShader(Initializer)
    {
        TransformParameter.Bind(Initializer.ParameterMap,TEXT("Transform"),TRUE);
        TextureMatrixParams[0].Bind(Initializer.ParameterMap,TEXT("TextureMatrix"),TRUE);
        TextureMatrixParams[1].Bind(Initializer.ParameterMap,TEXT("TextureMatrix2"),TRUE);
    }

    virtual UBOOL Serialize(FArchive& Ar)
    {
        UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
        Ar << TransformParameter;
        Ar << TextureMatrixParams[0];
        Ar << TextureMatrixParams[1];
        return bShaderHasOutdatedParameters;
    }

    // FGFxVertexShaderInterface. 2012 0x5bcec0 / 0x5bce70 / 0x5bce90.
    virtual void SetParameters(FVertexShaderRHIParamRef VertexShader, const FMatrix& Transform,
                               const FMatrix& TextureMatrix, const FMatrix& TextureMatrix2) const
    {
        // The retail decompile passes TextureMatrix for index 1 as well; the second matrix is what
        // the parameter is bound to, and nothing in the draw paths calls this overload - they call
        // the two setters below directly.
        SetParameterTransform(VertexShader,Transform);
        SetParameterTextureMatrix(VertexShader,TextureMatrix,0);
        SetParameterTextureMatrix(VertexShader,TextureMatrix2,1);
    }
    virtual void SetParameterTransform(FVertexShaderRHIParamRef VertexShader,
                                       const FMatrix& Transform) const
    {
        SetVertexShaderValue(VertexShader,TransformParameter,Transform);
    }
    virtual void SetParameterTextureMatrix(FVertexShaderRHIParamRef VertexShader,
                                           const FMatrix& TextureMatrix, INT i) const
    {
        SetVertexShaderValue(VertexShader,TextureMatrixParams[i],TextureMatrix);
    }
    virtual FShader* GetNativeShader() { return this; }
    virtual FGFxVertexShaderInterface* GetShaderInterface() { return this; }

private:
    FShaderParameter TransformParameter;
    FShaderParameter TextureMatrixParams[2];
};

// ---------------------------------------------------------------------------------------------
// DISHONORED(layout): the GFx pixel shader family. Four textures and twelve constants (2012
// Serialize 0x5bcc80): the constant colour, the colour transform as a scale and a bias, the inverse
// gamma, then the eight distance-field text constants.
//
// The colour transform is GFx 3.3's, which is TRANSPOSED relative to GFx 4's (agentBE.md trap 2,
// proved from UGFxObject::execGetColorTransform 2012 0x5b91a0, and again by
// SetParametersColorScaleAndColorBias 2012 0x5bcd80): Cxform::M_ is [4][2], channel-major, with the
// multiply in column 0 and the add in column 1, and the add is in 0..255 so it is scaled by 1/255.
// ---------------------------------------------------------------------------------------------
template<EGFxPixelShaderType ShaderType>
class FGFxPixelShader : public FGlobalShader, public FGFxPixelShaderInterface
{
    DECLARE_SHADER_TYPE(FGFxPixelShader,Global);
public:

    static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

    static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

    FGFxPixelShader() {}

    FGFxPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
        FGlobalShader(Initializer)
    {
        for (INT TextureIndex = 0; TextureIndex < 4; TextureIndex++)
        {
            TextureParams[TextureIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("Texture%u"),TextureIndex),TRUE);
        }
        ConstantColorParameter.Bind(Initializer.ParameterMap,TEXT("ConstantColor"),TRUE);
        ColorScaleParameter.Bind(Initializer.ParameterMap,TEXT("ColorScale"),TRUE);
        ColorBiasParameter.Bind(Initializer.ParameterMap,TEXT("ColorBias"),TRUE);
        InverseGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseGamma"),TRUE);
        DFWidthParameter.Bind(Initializer.ParameterMap,TEXT("DFWidth"),TRUE);
        DFShadowWidthParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowWidth"),TRUE);
        DFShadowOffsetParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowOffset"),TRUE);
        DFShadowEnableParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowEnable"),TRUE);
        DFShadowColorParameter.Bind(Initializer.ParameterMap,TEXT("DFShadowColor"),TRUE);
        DFGlowSizeParameter.Bind(Initializer.ParameterMap,TEXT("DFGlowSize"),TRUE);
        DFGlowEnableParameter.Bind(Initializer.ParameterMap,TEXT("DFGlowEnable"),TRUE);
        DFGlowColorParameter.Bind(Initializer.ParameterMap,TEXT("DFGlowColor"),TRUE);
    }

    virtual UBOOL Serialize(FArchive& Ar)
    {
        UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
        for (INT TextureIndex = 0; TextureIndex < 4; TextureIndex++)
        {
            Ar << TextureParams[TextureIndex];
        }
        Ar << ConstantColorParameter;
        Ar << ColorScaleParameter;
        Ar << ColorBiasParameter;
        Ar << InverseGammaParameter;
        Ar << DFWidthParameter;
        Ar << DFShadowWidthParameter;
        Ar << DFShadowOffsetParameter;
        Ar << DFShadowEnableParameter;
        Ar << DFShadowColorParameter;
        Ar << DFGlowSizeParameter;
        Ar << DFGlowEnableParameter;
        Ar << DFGlowColorParameter;
        return bShaderHasOutdatedParameters;
    }

    // FGFxPixelShaderInterface. 2012 0x5bc740 / 0x5bce30 / 0x5bcd80 / 0x5bce50 / 0x5bc9d0.
    virtual void SetParameterTextureRHI(FPixelShaderRHIParamRef PixelShader,
                                        FSamplerStateRHIParamRef SamplerState,
                                        FTextureRHIParamRef Texture, INT i) const
    {
        if (TextureParams[i].IsBound())
        {
            RHISetSamplerState(PixelShader,TextureParams[i].GetBaseIndex(),
                               TextureParams[i].GetSamplerIndex(),SamplerState,Texture,
                               0.f,-1.f,-1.f,FALSE);
        }
    }
    virtual void SetParameterConstantColor(FPixelShaderRHIParamRef PixelShader,
                                           const FLinearColor& Color) const
    {
        SetPixelShaderValue(PixelShader,ConstantColorParameter,Color);
    }
    virtual void SetParametersColorScaleAndColorBias(FPixelShaderRHIParamRef PixelShader,
                                                     const GRenderer::Cxform& Cx) const
    {
        FVector4 ColorScale, ColorBias;
        FGFxCxformToScaleAndBias(Cx,ColorScale,ColorBias);
        SetPixelShaderValue(PixelShader,ColorScaleParameter,ColorScale);
        SetPixelShaderValue(PixelShader,ColorBiasParameter,ColorBias);
    }
    virtual void SetParameterInverseGamma(FPixelShaderRHIParamRef PixelShader,
                                          const FLOAT InverseGamma) const
    {
        SetPixelShaderValue(PixelShader,InverseGammaParameter,InverseGamma);
    }
    virtual void SetDistanceFieldParams(FPixelShaderRHIParamRef PixelShader,
                                        const UTexture2D* Texture,
                                        const GRenderer::DistanceFieldParams& Params) const
    {
        const FLOAT SizeX = Texture ? (FLOAT)Texture->SizeX : 1.f;
        const FLOAT SizeY = Texture ? (FLOAT)Texture->SizeY : 1.f;
        SetPixelShaderValue(PixelShader,DFWidthParameter,Params.Width / SizeX);
        if (Params.ShadowColor.Raw)
        {
            SetPixelShaderBool(PixelShader,DFShadowEnableParameter,TRUE);
            SetPixelShaderValue(PixelShader,DFShadowWidthParameter,Params.ShadowWidth / SizeX);
            SetPixelShaderValue(PixelShader,DFShadowColorParameter,FGFxColorToVector4(Params.ShadowColor));
            const FVector4 Offset(-Params.ShadowOffset.x / SizeX,-Params.ShadowOffset.y / SizeY,0.f,0.f);
            SetPixelShaderValue(PixelShader,DFShadowOffsetParameter,Offset);
        }
        else
        {
            SetPixelShaderBool(PixelShader,DFShadowEnableParameter,FALSE);
        }
        if (Params.GlowColor.Raw)
        {
            SetPixelShaderBool(PixelShader,DFGlowEnableParameter,TRUE);
            SetPixelShaderValue(PixelShader,DFGlowColorParameter,FGFxColorToVector4(Params.GlowColor));
            const FVector4 GlowSize(Params.GlowSize[0],Params.GlowSize[1],0.f,0.f);
            SetPixelShaderValue(PixelShader,DFGlowSizeParameter,GlowSize);
        }
        else
        {
            SetPixelShaderBool(PixelShader,DFGlowEnableParameter,FALSE);
        }
    }
    virtual FShader* GetNativeShader() { return this; }
    virtual FGFxPixelShaderInterface* GetShaderInterface() { return this; }

private:
    FShaderResourceParameter TextureParams[4];
    FShaderParameter ConstantColorParameter;
    FShaderParameter ColorScaleParameter;
    FShaderParameter ColorBiasParameter;
    FShaderParameter InverseGammaParameter;
    FShaderParameter DFWidthParameter;
    FShaderParameter DFShadowWidthParameter;
    FShaderParameter DFShadowOffsetParameter;
    FShaderParameter DFShadowEnableParameter;
    FShaderParameter DFShadowColorParameter;
    FShaderParameter DFGlowSizeParameter;
    FShaderParameter DFGlowEnableParameter;
    FShaderParameter DFGlowColorParameter;
};

// ---------------------------------------------------------------------------------------------
// DISHONORED(layout): the GFx filter pixel shader family. Twelve parameters (2012 Serialize
// 0x57c920 in 2013 numbering / 0x5bc920 in 2012), written interleaved: texture and texture scale
// for each of the two sources, then colour transform and shadow colour for each, then the inverse
// gamma, the colour matrix, the blur size and the shadow offset. The highlight parameter is bound
// and never serialized.
//
// The colour-transform pair doubles as the scale/bias pair: SetParametersColorScaleAndColorBias
// (2012 0x5bcd80) and SetParameterColorMatrix (0x5bc560) both write the second of the two, so
// CxformParameters[0] is the multiply / colour matrix's scale and CxformParameters[1] is the add.
// ---------------------------------------------------------------------------------------------
template<EGFxPixelShaderType FilterType>
class FGFxFilterPixelShader : public FGlobalShader, public FGFxPixelShaderInterface
{
    DECLARE_SHADER_TYPE(FGFxFilterPixelShader,Global);
public:

    static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

    static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

    FGFxFilterPixelShader() {}

    FGFxFilterPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
        FGlobalShader(Initializer)
    {
        for (INT SourceIndex = 0; SourceIndex < 2; SourceIndex++)
        {
            TextureParams[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("Texture%u"),SourceIndex),TRUE);
            TexScaleParams[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("TexScale%u"),SourceIndex),TRUE);
            CxformParameters[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("Cxform%u"),SourceIndex),TRUE);
            ShadowColorParams[SourceIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("ShadowColor%u"),SourceIndex),TRUE);
        }
        InverseGammaParameter.Bind(Initializer.ParameterMap,TEXT("InverseGamma"),TRUE);
        ColorMatrixParameter.Bind(Initializer.ParameterMap,TEXT("ColorMatrix"),TRUE);
        BlurSizeParameter.Bind(Initializer.ParameterMap,TEXT("BlurSize"),TRUE);
        ShadowOffsetParameter.Bind(Initializer.ParameterMap,TEXT("ShadowOffset"),TRUE);
        HighlightParameter.Bind(Initializer.ParameterMap,TEXT("Highlight"),TRUE);
    }

    virtual UBOOL Serialize(FArchive& Ar)
    {
        UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
        for (INT SourceIndex = 0; SourceIndex < 2; SourceIndex++)
        {
            Ar << TextureParams[SourceIndex];
            Ar << TexScaleParams[SourceIndex];
        }
        for (INT SourceIndex = 0; SourceIndex < 2; SourceIndex++)
        {
            Ar << CxformParameters[SourceIndex];
            Ar << ShadowColorParams[SourceIndex];
        }
        Ar << InverseGammaParameter;
        Ar << ColorMatrixParameter;
        Ar << BlurSizeParameter;
        Ar << ShadowOffsetParameter;
        return bShaderHasOutdatedParameters;
    }

    // FGFxPixelShaderInterface - the filter half. 2012 0x5bc740 / 0x5bcd80 / 0x5bc620 / 0x5bc560 /
    // 0x5bc5d0 / 0x5bc6e0 / 0x5bc7a0 / 0x5bc840 / 0x5bc820.
    virtual void SetParameterTextureRHI(FPixelShaderRHIParamRef PixelShader,
                                        FSamplerStateRHIParamRef SamplerState,
                                        FTextureRHIParamRef Texture, INT i) const
    {
        if (TextureParams[i].IsBound())
        {
            RHISetSamplerState(PixelShader,TextureParams[i].GetBaseIndex(),
                               TextureParams[i].GetSamplerIndex(),SamplerState,Texture,
                               0.f,-1.f,-1.f,FALSE);
        }
    }
    virtual void SetParameterConstantColor(FPixelShaderRHIParamRef PixelShader,
                                           const FLinearColor& Color) const {}
    virtual void SetParametersColorScaleAndColorBias(FPixelShaderRHIParamRef PixelShader,
                                                     const GRenderer::Cxform& Cx) const
    {
        FVector4 ColorScale, ColorBias;
        FGFxCxformToScaleAndBias(Cx,ColorScale,ColorBias);
        SetPixelShaderValue(PixelShader,CxformParameters[0],ColorScale);
        SetPixelShaderValue(PixelShader,CxformParameters[1],ColorBias);
    }
    virtual void SetParametersCxformAc(FPixelShaderRHIParamRef PixelShader,
                                       const GRenderer::Cxform& Cx) const
    {
        // 2012 0x5bc620: the alpha-composited form premultiplies the colour by the alpha multiply.
        const FLOAT A = FGFxCxformMul(Cx,3);
        const FVector4 CxformMul(FGFxCxformMul(Cx,0) * A,FGFxCxformMul(Cx,1) * A,
                                 FGFxCxformMul(Cx,2) * A,A);
        const FVector4 CxformAdd(FGFxCxformAdd(Cx,0) * A / 255.f,FGFxCxformAdd(Cx,1) * A / 255.f,
                                 FGFxCxformAdd(Cx,2) * A / 255.f,FGFxCxformAdd(Cx,3) / 255.f);
        SetPixelShaderValue(PixelShader,CxformParameters[0],CxformMul);
        SetPixelShaderValue(PixelShader,CxformParameters[1],CxformAdd);
    }
    virtual void SetParameterColorMatrix(FPixelShaderRHIParamRef PixelShader,
                                         const FLOAT* Matrix) const
    {
        // 2012 0x5bc560: 20 floats - a 4x4 matrix, then the add, which goes into the same register
        // the colour bias uses.
        SetPixelShaderValue(PixelShader,ColorMatrixParameter,*(const FMatrix*)Matrix);
        const FVector4 Add(Matrix[16],Matrix[17],Matrix[18],Matrix[19]);
        SetPixelShaderValue(PixelShader,CxformParameters[1],Add);
    }
    virtual void SetParameterTexScale(FPixelShaderRHIParamRef PixelShader, INT i, FLOAT X,
                                      FLOAT Y) const
    {
        const FVector4 Scales(X,Y,0.f,0.f);
        SetPixelShaderValue(PixelShader,TexScaleParams[i],Scales);
    }
    virtual void SetParameterFilterSize4(FPixelShaderRHIParamRef PixelShader, FLOAT X, FLOAT Y,
                                         FLOAT Z, FLOAT W) const
    {
        const FVector4 FSize(X,Y,Z,W);
        SetPixelShaderValue(PixelShader,BlurSizeParameter,FSize);
    }
    virtual void SetParameterShadowColor(FPixelShaderRHIParamRef PixelShader, INT i,
                                         GColor Color) const
    {
        SetPixelShaderValue(PixelShader,ShadowColorParams[i],FGFxColorToVector4(Color));
    }
    virtual void SetParameterShadowOffset(FPixelShaderRHIParamRef PixelShader, FLOAT X,
                                          FLOAT Y) const
    {
        const FVector4 FSize(X,Y,0.f,0.f);
        SetPixelShaderValue(PixelShader,ShadowOffsetParameter,FSize);
    }
    virtual void SetParameterInverseGamma(FPixelShaderRHIParamRef PixelShader,
                                          const FLOAT InverseGamma) const
    {
        SetPixelShaderValue(PixelShader,InverseGammaParameter,InverseGamma);
    }
    virtual void SetDistanceFieldParams(FPixelShaderRHIParamRef PixelShader,
                                        const UTexture2D* Texture,
                                        const GRenderer::DistanceFieldParams& Params) const {}
    virtual FShader* GetNativeShader() { return this; }
    virtual FGFxPixelShaderInterface* GetShaderInterface() { return this; }

private:
    FShaderResourceParameter TextureParams[2];
    FShaderParameter TexScaleParams[2];
    FShaderParameter InverseGammaParameter;
    FShaderParameter CxformParameters[2];
    FShaderParameter ColorMatrixParameter;
    FShaderParameter BlurSizeParameter;
    FShaderParameter ShadowColorParams[2];
    FShaderParameter ShadowOffsetParameter;
    FShaderParameter HighlightParameter;
};

// ---------------------------------------------------------------------------------------------
// FGFxUpdatableTexture - PDB sizeof 92, base FTextureResource. The RHI resource behind an
// Update()able or Map()pable GFx texture: it owns its own FTexture2DRHIRef rather than streaming
// mips from a package. 2012 ctor 0x5d5bb0, InitRHI 0x5d5cf0, ReleaseRHI 0x5d5e60.
// ---------------------------------------------------------------------------------------------
class FGFxUpdatableTexture : public FTextureResource
{
public:
    UINT             Width;        // @68
    UINT             Height;       // @72
    UINT             Levels;       // @76
    UINT             Flags;        // @80
    EPixelFormat     TexFormat;    // @84
    FTexture2DRHIRef Texture2DRHI; // @88

    FGFxUpdatableTexture(UINT InWidth, UINT InHeight, UINT InLevels, EPixelFormat InFormat,
                         UINT InFlags)
        : Width(InWidth), Height(InHeight), Levels(InLevels), Flags(InFlags), TexFormat(InFormat)
    {
        bGreyScaleFormat = (InFormat == PF_G8);
    }
    virtual ~FGFxUpdatableTexture() {}

    virtual void InitRHI();
    virtual void ReleaseRHI();
    virtual UINT GetSizeX() const { return Width; }
    virtual UINT GetSizeY() const { return Height; }
    virtual FString GetFriendlyName() const { return TEXT("FGFxUpdatableTexture"); }
};

// The two UTexture2D subclasses whose only job is to make CreateResource produce an
// FGFxUpdatableTexture with the right creation flags: 0x10 (TexCreate_Dynamic) for the updatable
// one, 0x50 for the mappable one, which also reports itself fully resident so the streamer leaves
// it alone. PDB sizeof 368 for both, i.e. UTexture2D with no members added.
// 2012 UGFxUpdatableTexture::CreateResource 0x5d5f50, UGFxMappableTexture::CreateResource 0x5d5cb0,
// UGFxMappableTexture::UpdateStreamingStatus 0x5d5b90.
class UGFxUpdatableTexture : public UTexture2D
{
    DECLARE_CLASS_INTRINSIC(UGFxUpdatableTexture,UTexture2D,CLASS_Transient,GFxUI)
public:
    virtual FTextureResource* CreateResource();
};

class UGFxMappableTexture : public UTexture2D
{
    DECLARE_CLASS_INTRINSIC(UGFxMappableTexture,UTexture2D,CLASS_Transient,GFxUI)
public:
    virtual FTextureResource* CreateResource();
    virtual UINT UpdateStreamingStatus(UBOOL bWaitForMipFading);
};

// ---------------------------------------------------------------------------------------------
// The element stores and the free helpers. Everything here is FGFxRendererImpl's namespace.
// ---------------------------------------------------------------------------------------------
namespace FGFxRendererImpl
{

// The usage bits GFx passes to InitDynamicTexture, and what they select (2012 0x5d8590).
enum
{
    GFxCreateTex_Normal    = 1,
    GFxCreateTex_Updatable = 2,
    GFxCreateTex_Mappable  = 3
};

// PDB sizeof 32. RefCount is shared between the two threads: the main thread hands a store over by
// pointer and drops its own reference, and whichever side sees the count reach zero frees it.
struct FGFxRenderElementStoreBase : public GRendererNode
{
    GAtomicInt<unsigned long> RefCount;          // @8
    GRenderer::CachedData*    CachedData;        // @12
    unsigned long             ElementSize;       // @16
    unsigned long             NumElements;       // @20
    void*                     Elements;          // @24
    bool                      AllocatedElements; // @28

    FGFxRenderElementStoreBase()
        : CachedData(NULL), ElementSize(0), NumElements(0), Elements(NULL),
          AllocatedElements(false)
    {
        pPrev = NULL;
        pNext = NULL;
        RefCount.Value = 1;
    }

    void Release_MainThread(GLock* ElementsLock);   // 2012 0x5c4510
    void Release_RenderThread(GLock* ElementsLock); // 2012 0x5c4630
};

// PDB sizeof 36 for all three: the base plus the format the store was filled with.
template<class FormatType>
struct FGFxRenderElementStore : public FGFxRenderElementStoreBase
{
    FormatType Format; // @32

    FGFxRenderElementStore() : Format((FormatType)0) {}

    void InitElements(FGFxRenderer* Renderer, GRenderer::CachedData* InCachedData,
                      FormatType InFormat, UINT InNumElements, void* InElements);   // 2012 0x5d47f0
    void InitElementsCopy(FormatType InFormat, UINT InNumElements,
                          const void* InElements);                                  // 2012 0x5b8db0
};

struct FGFxVertexStore : public FGFxRenderElementStore<GRenderer::VertexFormat> {};
struct FGFxIndexStore : public FGFxRenderElementStore<GRenderer::IndexFormat> {};
struct FGFxBitmapDescStore : public FGFxRenderElementStore<GRenderer::BitmapDesc*> {};

// The element size a vertex or index format needs. 2012, inlined into InitElements (0x5d47f0,
// 0x5d4880): XY16i 4, XY32f and XY16iC32 8, XY16iCF32 12; Index_16 2, Index_32 4.
UINT GetElementSize(GRenderer::VertexFormat Format);
UINT GetElementSize(GRenderer::IndexFormat Format);
UINT GetElementSize(GRenderer::BitmapDesc* Format);

// The GImageBase queries the runtime answers inside libgfx (GImageBase has only data members in the
// PDB, so these are the format arithmetic rather than a port of a body): bytes per pixel, whether the
// data is a DXT block format, the size of one mip level, and the pointer to one level inside pData.
UINT GetImageBytesPerPixel(GImageBase::ImageFormat Format);
UBOOL IsImageDataCompressed(GImageBase::ImageFormat Format);
UINT GetImageMipLevelSize(GImageBase::ImageFormat Format, UINT Width, UINT Height);
const BYTE* GetImageMipLevelData(const GImageBase& Image, UINT Level, UINT& OutWidth,
                                 UINT& OutHeight, UINT& OutPitch);

void ConvertFromUI(const GRenderer::FillTexture& In, FGFxRenderer::FFillTextureInfo& Out); // 2012 0x5b7710
void ConvertFromUI(GColor InUIColor, FLinearColor& OutColor);                              // 2012 0x5badc0
void ApplyUIColor_RenderThread(FGFxRenderer* Renderer, GColor Color,
                               GRenderer::BlendType BlendMode,
                               FGFxPixelShaderInterface& PixelShader);                     // 2012 0x5bae50
void ApplyUIBlendMode_RenderThread(UINT bAlphaComposite, GRenderer::BlendType NewMode,
                                   UINT bSourceAc);                                        // 2012 0x5cc020
FVertexDeclarationRHIRef GetUIVertexDecl_RenderThread(EGFxVertexDeclarationType DeclType,
                                                      DWORD* OutStrides);                  // 2012 0x5babe0
void GetUIBoundShaderState_RenderThread(FGFxBoundShaderState& OutBoundShaderState,
                                        TMap<DWORD,FBoundShaderStateRHIRef>& Cache,
                                        const FGFxEnumeratedBoundShaderState& Enumerated);  // 2012 0x5daef0
void DrawUIBackgroundColor_RenderThread(FGFxRenderer* Renderer, GColor BackgroundColor,
                                        const TArray<FGFxVertex_XY16i>& BackgroundQuad,
                                        const GMatrix2D& ViewportMatrix,
                                        const GMatrix2D& CurrentMatrix,
                                        TMap<DWORD,FBoundShaderStateRHIRef>& Cache);         // 2012 0x5db0d0
void LoadTexture(UINT DestPitch, INT DestBpp, BYTE* Dest, const BYTE* Src, UINT Width,
                 UINT Height, UINT SrcPitch);                                                // 2012 0x5b8060
BYTE* SoftwareResample(UINT DestWidth, GImageBase::ImageFormat Format, UINT SrcWidth,
                       UINT SrcHeight, UINT SrcPitch, const BYTE* Src, UINT DestHeight);     // 2012 0x5b7f40

// SetVertexData / SetIndexData both go through this: build a store on the calling thread, then hand
// it to the render thread, which releases the one it had. 2012 0x5d8a10 / 0x5d8d60.
template<class StoreType, class FormatType>
void SetUIRenderElementStore(FGFxRenderer* Renderer, GRenderer::CachedDataType BuffType,
                             const void* Elements, INT NumElements, FormatType Format,
                             GRenderer::CacheProvider* const Cache, StoreType*& OwnedStore);

} // namespace FGFxRendererImpl


// The shader interface for a kind, from the global shader map. 2012 0x5d77f0 / 0x5d7b00; the filter
// half of the pixel-shader enumeration is answered by gfxuishaders.cpp's
// GetUIPixelShaderInterface2_RenderThread (2012 0x5d7360).
FGFxPixelShaderInterface* GetUIPixelShaderInterface_RenderThread(EGFxPixelShaderType Type);
FGFxPixelShaderInterface* GetUIPixelShaderInterface2_RenderThread(EGFxPixelShaderType Type);
FGFxVertexShaderInterface* GetUIVertexShaderInterface_RenderThread(EGFxVertexShaderType Type);
