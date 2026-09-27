# GFxUI module sources (agent T, Phase 3 wave 2). Only the generated registrant units compile:
# GFxUIRegistrants.cpp / GFxUINativeStubs.cpp (resources/tools/symbols/gen_classes_header.py --sdk)
# register the 17 native classes of the retail GFxUI.upk with the retail layout. The Epic GFx-4
# glue (Scaleform*.cpp, Render/*) needs the Scaleform SDK (WITH_GFx=0) and the Arkane GFx-3 PDB
# skeletons (gfxui*.cpp) are comment-only; both stay out until Phase 4 (resources/docs/middleware.md).
# Agent BE (PHASE8.md package BE) ported Src/gfxuimovie.cpp (UGFxObject + UGFxMoviePlayer, 113 natives) and
# added Src/gfxuinatives.cpp, Src/gfxuiexternalinterface.cpp and Src/gfxuigfx3absent.cpp, so those four
# compile; the rest of the GFx-3 skeletons are agent BB's and stay excluded until that package lands.
# Agent BD (package BD) put the 50 GFx shader type declarations of the cooked global shader cache into
# Src/gfxuishaders.cpp (types only, no Scaleform runtime), so that unit compiles too.
set(GFxUI_EXCLUDE
  Src/GFxUI.cpp
  Src/Render/RHI_ConsoleMeshCache.cpp
  Src/Render/RHI_HAL.cpp
  Src/Render/RHI_HALSetup.cpp
  Src/Render/RHI_MeshCache.cpp
  Src/Render/RHI_Shader.cpp
  Src/Render/RHI_Texture.cpp
  Src/Render/RHI_XeBufferMemory.cpp
  Src/Render/RHI_XeRenderTarget.cpp
  Src/ScaleformDataStore.cpp
  Src/ScaleformEngine.cpp
  Src/ScaleformFile.cpp
  Src/ScaleformFont.cpp
  Src/ScaleformFullscreenMovie.cpp
  Src/ScaleformInteraction.cpp
  Src/ScaleformLocalization.cpp
  Src/ScaleformMovie.cpp
  Src/ScaleformSound.cpp
  Src/ScaleformStats.cpp
  Src/gfxuidatastore.cpp
  Src/gfxuiengine.cpp
  Src/gfxuifile.cpp
  Src/gfxuifont.cpp
  Src/gfxuiimageinfo.cpp
  Src/gfxuiinteraction.cpp
  Src/gfxuilocalization.cpp
  Src/gfxuirenderer.cpp
)
set(GFxUI_NOT_IN_PDB
)

# Agent BB (package BB, PHASE8.md): with DISHONORED_WITH_GFX3 the four seam units come back into the
# build. They implement the interfaces libgfx calls into and that nothing in this tree implemented -
# GRenderer 54 slots, GTexture 12, GRenderTarget 7, GFxFileOpener 4, GFile 19, GFxImageLoader and
# GFxImageCreator 1 each, GSysAllocPaged 5 - against the reconstructed headers in
# source/Development/Src/External/GFx3. They include no engine header, so they cannot drag the module
# into anything new, and nothing instantiates them yet. resources/docs/agents/agentBB.md.
if(DISHONORED_WITH_GFX3)
  list(REMOVE_ITEM GFxUI_EXCLUDE
    Src/gfxuirenderer.cpp
    Src/gfxuifile.cpp
    Src/gfxuiimageinfo.cpp)
  # Agent DC (PHASE9.md package DC): gfxuiengine.cpp is FGFxEngine - the object that owns the loader
  # and the renderer, opens a movie out of a cooked USwfMovie, advances the open movies once a frame
  # and draws them once a frame. gfxuiinteraction.cpp is UGFxInteraction, which is where retail's
  # per-frame advance and input come from (measured, build/agentDC/xr.py). Both need the runtime, and
  # DISHONORED_GFXUI_GFX3_RUNTIME follows DISHONORED_WITH_GFX3 in Inc/gfxui_gfx3.h.
  list(REMOVE_ITEM GFxUI_EXCLUDE
    Src/gfxuiengine.cpp
    Src/gfxuiinteraction.cpp)
else()
  list(APPEND GFxUI_EXCLUDE Src/gfxuiallocator.cpp)
endif()
