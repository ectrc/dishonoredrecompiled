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
// DISHONORED(port): FGFxRendererImpl is the render-thread half of the seam. In retail it is a
// namespace-like class holding the element stores the renderer hands to the RHI plus two
// conversion helpers; 31 PDB functions are attributed to this header and 221 to gfxuirenderer.cpp
// (the list above), almost all of them the FGFxPixelShader<N> / FGFxVertexShader<N> instantiations
// that belong with package BD's shader work, not with the seam.
//
// What package BB reproduces here is the element-store contract, because that is the part
// GRenderer's SetVertexData / SetIndexData / DrawBitmaps slots need in order to be callable at all:
//   FGFxRendererImpl::FGFxRenderElementStoreBase   PDB sizeof 32, base GRendererNode, members
//     @8 GAtomicInt<unsigned long> RefCount, @12 GRenderer::CachedData* CachedData,
//     @16 ULONG ElementSize, @20 ULONG NumElements, @24 void* Elements, @28 bool AllocatedElements
//   FGFxRendererImpl::FGFxVertexStore / FGFxIndexStore / FGFxBitmapDescStore   sizeof 36 each
// 2012 rva of the one conversion helper that is pure logic: FGFxRendererImpl::ConvertFromUI
// 0x5b7710 -> 2013 0x572c30 (byte-identical, ratio 1.000).
#include "gfxuirenderer.h"

class FGFxRendererImpl
{
public:
    // The store GRenderer's vertex/index/bitmap-descriptor slots write into. Layout is the PDB's.
    class FGFxRenderElementStoreBase : public GRendererNode
    {
    public:
        GAtomicInt<unsigned long> RefCount;          // @8
        GRenderer::CachedData*    CachedData;        // @12
        unsigned long             ElementSize;       // @16
        unsigned long             NumElements;       // @20
        void*                     Elements;          // @24
        bool                      AllocatedElements; // @28

        FGFxRenderElementStoreBase()
            : CachedData(0), ElementSize(0), NumElements(0), Elements(0),
              AllocatedElements(false) {}

        void AddRef() { ++RefCount; }
        void Release() { if (--RefCount == 0) delete this; }
        void SetElements(void* InElements, unsigned long InCount, unsigned long InSize)
        {
            Elements = InElements;
            NumElements = InCount;
            ElementSize = InSize;
            AllocatedElements = false;
        }
    };

    // The three instantiations retail uses, one per element kind (PDB sizeof 36 each: the base plus
    // the format the store was filled with).
    class FGFxVertexStore : public FGFxRenderElementStoreBase
    {
    public:
        GRenderer::VertexFormat Format;
        FGFxVertexStore() : Format(GRenderer::Vertex_None) {}
    };
    class FGFxIndexStore : public FGFxRenderElementStoreBase
    {
    public:
        GRenderer::IndexFormat Format;
        FGFxIndexStore() : Format(GRenderer::Index_None) {}
    };
    class FGFxBitmapDescStore : public FGFxRenderElementStoreBase
    {
    public:
        GRenderer::BitmapDesc* Descs;
        FGFxBitmapDescStore() : Descs(0) {}
    };

    // PDB sizeof 40: the render-thread copy of a GRenderer::FillTexture.
    class FFillTextureInfo
    {
    public:
        FTexture*                   Texture;       // @0
        GMatrix2D                   TextureMatrix; // @4
        GRenderer::BitmapWrapMode   WrapMode;      // @28
        GRenderer::BitmapSampleMode SampleMode;    // @32
        unsigned int                bUseMips;      // @36

        FFillTextureInfo()
            : Texture(0), WrapMode(GRenderer::Wrap_Repeat), SampleMode(GRenderer::Sample_Linear),
              bUseMips(0) {}
    };

    // 2013 0x572c30: flatten a GRenderer::FillTexture into the render-thread copy. Pure logic, so
    // this is a real port rather than a bringup stub - the only body in the seam that is.
    static void ConvertFromUI(const GRenderer::FillTexture& In, FFillTextureInfo& Out);
};
