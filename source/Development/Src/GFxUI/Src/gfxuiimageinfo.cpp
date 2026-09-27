// GFxUI/src/gfxuiimageinfo.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (3):
//   0x5b9a40  public: virtual bool __thiscall FGFxImageInfo::Recreate(class GRenderer *)
//   0x5b9a80  public: virtual unsigned int __thiscall FGFxImageInfo::GetExternalBytes(void)const
//   0x5cf330  public: virtual class GTexture * __thiscall FGFxImageInfo::GetTexture(class GRenderer *)

// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the image seam. Declarations and 2013 rvas in gfxuiimageinfo.h.
// What these three classes are for, measured rather than assumed: the cook strips every bitmap out
// of the movie and leaves a tag-1009 GFx_DefineExternalImage2 naming a .tga, with the pixels in a
// UE3 Texture2D in the same package - 1009 count == Texture2D export count in every single-movie
// package (agentBB.md). LoadImageW gets that name; CreateImage gets the GFxImageCreateInfo the
// runtime built from the tag. Resolving the name to a package object needs UObject, so those two
// bodies are DISHONORED(bringup) and belong to package BE; everything that is pure bookkeeping is
// real code.
#include "gfxuiimageinfo.h"

FGFxImageInfo::FGFxImageInfo(const GString& InFileName, unsigned int InWidth,
                             unsigned int InHeight)
    : FileName(InFileName)
{
    // 2013 0x585c00. GImageInfo carries the target size; the texture arrives later, from the
    // engine texture the export name resolves to.
    TargetWidth = InWidth;
    TargetHeight = InHeight;
    ReleaseImage = false;
    GFXUI_SEAM_TRACE("FGFxImageInfo::FGFxImageInfo");
}

FGFxImageInfo::~FGFxImageInfo() {}

GTexture* FGFxImageInfo::GetTexture(GRenderer* Renderer)
{
    // 2013 0x58e540.
    (void)Renderer;
    GFXUI_SEAM_TRACE("FGFxImageInfo::GetTexture");
    return pTexture.GetPtr();
}

bool FGFxImageInfo::Recreate(GRenderer* Renderer)
{
    // 2013 0x575070: retail drops the texture so the next GetTexture rebuilds it.
    (void)Renderer;
    pTexture.Clear();
    GFXUI_SEAM_TRACE("FGFxImageInfo::Recreate");
    return true;
}

unsigned int FGFxImageInfo::GetExternalBytes() const
{
    // 2013 0x5750b0: the bytes the image costs outside GFx's own heaps, i.e. the engine texture.
    // Reported as 0 until the texture substitution is live.
    return 0;
}

FGFxImageLoader::~FGFxImageLoader() {}

GImageInfoBase* FGFxImageLoader::LoadImageW(const char* Url)
{
    // 2013 0x586020.
    (void)Url;
    GFXUI_SEAM_TRACE("FGFxImageLoader::LoadImageW");
    return 0;
}

FGFxImageCreator::~FGFxImageCreator() {}

GImageInfoBase* FGFxImageCreator::CreateImage(const GFxImageCreateInfo& Info)
{
    // 2013 0x585d70. Info.pExportName is the tag-1009 ExportName; Info.Type says whether the
    // runtime already has pixels (GFxImageCreateInfo::Input_Image) or only a file reference.
    (void)Info;
    GFXUI_SEAM_TRACE("FGFxImageCreator::CreateImage");
    return 0;
}
