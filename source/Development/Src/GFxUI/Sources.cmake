# GFxUI module sources (agent T, Phase 3 wave 2). Only the generated registrant units compile:
# GFxUIRegistrants.cpp / GFxUINativeStubs.cpp (resources/tools/symbols/gen_classes_header.py --sdk)
# register the 17 native classes of the retail GFxUI.upk with the retail layout. The Epic GFx-4
# glue (Scaleform*.cpp, Render/*) needs the Scaleform SDK (WITH_GFx=0) and the Arkane GFx-3 PDB
# skeletons (gfxui*.cpp) are comment-only; both stay out until Phase 4 (resources/docs/middleware.md).
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
  Src/gfxuimovie.cpp
  Src/gfxuirenderer.cpp
  Src/gfxuishaders.cpp
)
set(GFxUI_NOT_IN_PDB
)
