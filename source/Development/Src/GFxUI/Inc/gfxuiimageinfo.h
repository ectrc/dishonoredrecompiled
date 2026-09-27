#pragma once
// GFxUI/inc/gfxuiimageinfo.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (1):
//   0x5c9a70  public: __thiscall FGFxImageInfo::FGFxImageInfo(class GString const &, unsigned int, unsigned int)

// ---------------------------------------------------------------------------------------------
// DISHONORED(port): the image seam - the half of the cook that makes the stripped bitmaps work.
// The cooked assets are gfxexport GFX with every bitmap replaced by a tag-1009
// GFx_DefineExternalImage2 naming a .tga, and the pixels live in a UE3 Texture2D in the same
// package (agentBB.md, measured over all 22 retail movies). GFxImageLoader::LoadImageW and
// GFxImageCreator::CreateImage are where that name is turned back into a texture, so these two
// one-slot interfaces are load-bearing, not an optional extra.
//   FGFxImageInfo::FGFxImageInfo     2013 0x585c00  (2012 0x5c9a70)
//   FGFxImageInfo::GetTexture        2013 0x58e540  (2012 0x5cf330)
//   FGFxImageInfo::Recreate          2013 0x575070  (2012 0x5b9a40)
//   FGFxImageInfo::GetExternalBytes  2013 0x5750b0  (2012 0x5b9a80)
//   FGFxImageLoader::LoadImageW      2013 0x586020  (2012 0x5c9e90)
//   FGFxImageCreator::CreateImage    2013 0x585d70  (2012 0x5c9be0)
#include "gfxuirenderer.h"

// FGFxImageInfo: PDB sizeof 40, base GImageInfo (36) plus GString FileName @36, and one new slot.
class FGFxImageInfo : public GImageInfo
{
public:
    GString FileName;   // @36

    FGFxImageInfo(const GString& InFileName, unsigned int InWidth,
                  unsigned int InHeight);                                  // 2013 0x585c00

    virtual ~FGFxImageInfo();                                              // vt[0]
    virtual GTexture* GetTexture(GRenderer* Renderer);                     // vt[3] 2013 0x58e540
    virtual bool Recreate(GRenderer* Renderer);                            // ChangeHandler slot 2
    virtual unsigned int GetExternalBytes() const;                         // vt[7] 2013 0x5750b0
};

class FGFxImageLoader : public GFxImageLoader
{
public:
    virtual ~FGFxImageLoader();                                            // vt[0]
    virtual GImageInfoBase* LoadImageW(const char* Url);                   // vt[1] 2013 0x586020
};

class FGFxImageCreator : public GFxImageCreator
{
public:
    virtual ~FGFxImageCreator();                                           // vt[0]
    virtual GImageInfoBase* CreateImage(const GFxImageCreateInfo& Info);   // vt[1] 2013 0x585d70
};
