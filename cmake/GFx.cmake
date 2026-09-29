# Scaleform GFx 3.3.89 (Autodesk/Scaleform). Retail statically links the whole runtime: there is no
# GFx DLL anywhere, not one of the 33 DLLs in Binaries\Win32 contains a single GFx symbol, and
# libgfx + libgfx_ime are 5,635 functions and 1.13 MiB inside Dishonored.exe's own .text - 9.7 % of
# it (resources/docs/gfx_decision.md 1). So route 1 of the PhysX/Steamworks method does not apply:
# there is nothing to import and no import library to build.
#
# What this file builds instead is our own reconstruction of the API, read out of the 2012 Shipping
# PDB with resources/tools/pdb/dia_types.py the same way cmake/PhysX.cmake's headers were read out
# of PhysXCore.pdb. 5,235 of the 5,635 libgfx functions (92.9 %) are byte-identical between the 2012
# QA build, which has a PDB, and the retail 2013 build, so the 2012 layouts ARE the retail layouts.
# resources/docs/agents/agentBB.md.
#
# With DISHONORED_WITH_GFX3=ON:
#   * source/Development/Src/External/GFx3 builds as the static library `gfx3`, whose single
#     assertion translation unit (GFx3Layout.cpp) carries 171 sizeof and 380 offsetof checks against
#     the PDB - if it compiles, the reconstruction agrees with the PDB byte for byte;
#   * dishonored_apply_defines() sets DISHONORED_WITH_GFX3=1 on every target and links
#     Dishonored::gfx3, which also carries the header directory. Every module gets it for the same
#     reason Bink and PhysX do: GFxUI and DishonoredGame both name these types;
#   * the four GFxUI seam units (gfxuirenderer.cpp, gfxuifile.cpp, gfxuiimageinfo.cpp,
#     gfxuiallocator.cpp) come out of GFxUI's exclude list, so GRenderer's 54 slots, GTexture's 12,
#     GRenderTarget's 7, GFxFileOpener's 4 and GFile's 19 are all implemented in the build.
#
# Package BC (resources/docs/agents/agentBC.md) has since put the ActionScript 2 machine and the player
# on these headers: the value model and object interface, the display list, the sprite timeline, the
# tag and character model and the bytecode interpreter, driven by the GFx3Run harness. Nothing in the
# game instantiates any of it, which is why turning this on still cannot change how the game runs.
#
# GFx3Dump is the acceptance harness and is EXCLUDE_FROM_ALL - build it by name:
#   cmake --build <dir> --target GFx3Dump
#   <dir>/Binaries/Win32/GFx3Dump.exe --slots
#   <dir>/Binaries/Win32/GFx3Dump.exe --parse --verbose <payload>.gfx
# Payloads come out of the cooked *_SF.upk packages with build/agentBB/extract_gfx.py.

set(DISHONORED_GFX3_DIR "${CMAKE_SOURCE_DIR}/source/Development/Src/External/GFx3")
if(EXISTS "${DISHONORED_GFX3_DIR}/GFx3.h")
  set(_dishonored_gfx3_default ON)
else()
  set(_dishonored_gfx3_default OFF)
endif()
option(DISHONORED_WITH_GFX3 "Compile the reconstructed Scaleform GFx 3.3 API and the GFxUI renderer/file/image seam" ${_dishonored_gfx3_default})

if(DISHONORED_WITH_GFX3)
  add_library(gfx3 STATIC
    "${DISHONORED_GFX3_DIR}/GFx3Layout.cpp"
    "${DISHONORED_GFX3_DIR}/GFx3Support.cpp"
    "${DISHONORED_GFX3_DIR}/GFx3RuntimeStubs.cpp"
    "${DISHONORED_GFX3_DIR}/GFxInput.cpp"
    "${DISHONORED_GFX3_DIR}/GFxGfxFile.cpp"
    # The ActionScript 2 machine and the player, package BC (resources/docs/agents/agentBC.md).
    # These units include no engine header either, so they do not change what the game compiles
    # against; nothing in the engine instantiates GFxMovieRoot yet, so linking them in cannot change
    # how the game runs.
    "${DISHONORED_GFX3_DIR}/GFxAS2Value.cpp"
    "${DISHONORED_GFX3_DIR}/GFxAS2Object.cpp"
    "${DISHONORED_GFX3_DIR}/GFxAS2Runtime.cpp"
    "${DISHONORED_GFX3_DIR}/GFxAS2Interp.cpp"
    "${DISHONORED_GFX3_DIR}/GFxAS2Lib.cpp"
    "${DISHONORED_GFX3_DIR}/GFxPlayerData.cpp"
    "${DISHONORED_GFX3_DIR}/GFxPlayerSprite.cpp"
    "${DISHONORED_GFX3_DIR}/GFxPlayerRoot.cpp"
    # The mouse's half of the player, package DQ (resources/docs/agents/agentDQ.md): which display
    # object is under the pointer, and the seven button events that follow from it changing.
    "${DISHONORED_GFX3_DIR}/GFxHitTest.cpp"
    # The text engine and the glyph rasteriser, package CB (resources/docs/agents/agentCB.md).
    # Agent BB proved the game fonts are DefineFont3 glyph outlines with no font-texture tag
    # anywhere in the cook, so a rasteriser has to exist for any character of text to appear.
    # These eight units include no engine header either and nothing in the game instantiates a
    # text field yet, so linking them in cannot change how the game runs.
    "${DISHONORED_GFX3_DIR}/GFxShape.cpp"
    "${DISHONORED_GFX3_DIR}/GFxRasterizer.cpp"
    "${DISHONORED_GFX3_DIR}/GFxFont.cpp"
    "${DISHONORED_GFX3_DIR}/GFxGlyphCache.cpp"
    "${DISHONORED_GFX3_DIR}/GFxTextFormat.cpp"
    "${DISHONORED_GFX3_DIR}/GFxStyledText.cpp"
    "${DISHONORED_GFX3_DIR}/GFxTextDocView.cpp"
    "${DISHONORED_GFX3_DIR}/GFxTextField.cpp"
    # The tag loaders and the character definitions, package CD (resources/docs/agents/agentCD.md).
    # These are what turn a dictionary entry from a placeholder into a real definition: the two
    # retail loader tables, the shape/edit-text/static-text/button/image/font definitions, and the
    # import binding. They include no engine header either.
    "${DISHONORED_GFX3_DIR}/GFxCharacterDefs.cpp"
    "${DISHONORED_GFX3_DIR}/GFxTagLoaders.cpp"
    # The loader and the display half, package DC (resources/docs/agents/agentDC.md). GFxLoaderImpl.cpp
    # is GFxLoader's non-virtual API - CreateMovie and GetMovieInfo, the two functions the engine opens
    # a movie with, which agent BC named as the one thing between the runtime and the engine
    # (agentBC.md 6.6) - plus the loader-side state bag and the synchronous bind. GFxDisplay.cpp is the
    # display-list traversal, the shape tessellation and the glyph submission: the loop agent BC left
    # empty, agent CB stopped one call short of and agent CC had nothing to be handed.
    "${DISHONORED_GFX3_DIR}/GFxLoaderImpl.cpp"
    "${DISHONORED_GFX3_DIR}/GFxDisplay.cpp")
  target_include_directories(gfx3 PUBLIC "${DISHONORED_GFX3_DIR}")
  target_compile_definitions(gfx3 PRIVATE _CRT_SECURE_NO_WARNINGS)
  # UE3's 4-byte packing, the same option every module gets. The GFx headers push pack(8) of their
  # own, because GFx itself was not built with /Zp4: GFxValue::DisplayInfo puts `bool Visible` at @48
  # and the next `double Z` at @56, i.e. 8-byte alignment, and the struct is 232 bytes.
  target_compile_options(gfx3 PRIVATE /Zp4)
  set_target_properties(gfx3 PROPERTIES FOLDER "External")

  add_library(Dishonored::gfx3 INTERFACE IMPORTED)
  target_link_libraries(Dishonored::gfx3 INTERFACE gfx3)

  # The acceptance harness: parses a cooked movie payload through our container parser and calls
  # every slot of the seam through a base-class pointer. It links the seam units directly rather
  # than the GFxUI module, which is the point - the seam needs no engine.
  set(_gfx3_seam_dir "${CMAKE_SOURCE_DIR}/source/Development/Src/GFxUI")
  add_executable(GFx3Dump EXCLUDE_FROM_ALL
    "${DISHONORED_GFX3_DIR}/Tools/GFx3Dump.cpp"
    "${_gfx3_seam_dir}/Src/gfxuirenderer.cpp"
    "${_gfx3_seam_dir}/Src/gfxuifile.cpp"
    "${_gfx3_seam_dir}/Src/gfxuiimageinfo.cpp"
    "${_gfx3_seam_dir}/Src/gfxuiallocator.cpp")
  target_include_directories(GFx3Dump PRIVATE "${_gfx3_seam_dir}/Inc")
  target_compile_definitions(GFx3Dump PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(GFx3Dump PRIVATE /Zp4)
  target_link_libraries(GFx3Dump PRIVATE gfx3)
  set_target_properties(GFx3Dump PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Win32"
    FOLDER "External")

  # Package BC's acceptance harness: it drives the AS2 machine from a cooked asset with no engine, no
  # renderer and no game.
  #   cmake --build <dir> --target GFx3Run
  #   <dir>/Binaries/Win32/GFx3Run.exe --run <payload>.gfx --frames 5 --verbose
  #   <dir>/Binaries/Win32/GFx3Run.exe --opcodes --classes
  add_executable(GFx3Run EXCLUDE_FROM_ALL "${DISHONORED_GFX3_DIR}/Tools/GFx3Run.cpp")
  # A crash in the machine prints its stack as module-relative addresses (agent DG); the map is what
  # turns them back into names, with build/agentDG/map.py.
  target_link_options(GFx3Run PRIVATE /MAP:$<TARGET_FILE_DIR:GFx3Run>/GFx3Run.map)
  target_compile_definitions(GFx3Run PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(GFx3Run PRIVATE /Zp4)
  target_link_libraries(GFx3Run PRIVATE gfx3)
  set_target_properties(GFx3Run PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Win32"
    FOLDER "External")

  # Package CB's acceptance harness: the fonts, the rasteriser and the layout engine, driven from
  # a cooked payload with no engine, no renderer and no game.
  #   cmake --build <dir> --target GFx3Text
  #   <dir>/Binaries/Win32/GFx3Text.exe --fonts DisFonts_SF.gfxfontlib.gfx
  #   <dir>/Binaries/Win32/GFx3Text.exe --raster <fontlib.gfx> --string Dishonored --size 32 --dump <dir>
  #   <dir>/Binaries/Win32/GFx3Text.exe --run <asset.gfx> --fontlib <fontlib.gfx> --verbose
  #   <dir>/Binaries/Win32/GFx3Text.exe --layout <fontlib.gfx> --string "a b c" --box 260 140
  #   <dir>/Binaries/Win32/GFx3Text.exe --table
  add_executable(GFx3Text EXCLUDE_FROM_ALL "${DISHONORED_GFX3_DIR}/Tools/GFx3Text.cpp")
  target_compile_definitions(GFx3Text PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(GFx3Text PRIVATE /Zp4)
  target_link_libraries(GFx3Text PRIVATE gfx3)
  set_target_properties(GFx3Text PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Binaries/Win32"
    FOLDER "External")

  message(STATUS "GFx: DISHONORED_WITH_GFX3=1, reconstructed GFx 3.3.89 API + the GFxUI seam")
endif()
