# FaceFX 1.7.3.1 (OC3 Entertainment). Retail compiles the SDK from source into Dishonored.exe
# (v:\dishonored\unrealengine3qatest\development\external\facefx\fxsdk, ~1,317 functions + the fxsdk_unreal unit,
# resources/docs/middleware.md section 2.5); there is no DLL and no SDK we can obtain. So, as for Wwise, the SDK is
# reconstructed from the 2012 PDB build: source/Development/Src/External/FaceFX/include holds the public headers
# (OC3Ent::Face, the PDB's class layouts) and Src/ the ported units, one file per retail compiland family.
# Program and conventions: resources/docs/facefx/facefx_program.md (agents FX1 core, FX2 face graph, FX3 runtime).
#
# DISHONORED_WITH_FACEFX_SDK builds the library (target facefx / Dishonored::facefx) and CoreSmoke's FaceFX checks.
# The engine switch is WITH_FACEFX (cmake/DishonoredDefines.cmake): 1 when this library exists, and then
# dishonored_apply_defines links it into every module (agent FX3).

set(DISHONORED_FACEFX_DIR "${CMAKE_SOURCE_DIR}/source/Development/Src/External/FaceFX")

if(EXISTS "${DISHONORED_FACEFX_DIR}/include/FxSDK.h")
  set(dishonored_facefx_default ON)
else()
  set(dishonored_facefx_default OFF)
endif()
option(DISHONORED_WITH_FACEFX_SDK "Compile the reconstructed FaceFX 1.7.3.1 SDK library (engine integration stays WITH_FACEFX=0)" ${dishonored_facefx_default})

if(DISHONORED_WITH_FACEFX_SDK)
  # FX1's core (Src/*.cpp), FX2's face graph (Src/FaceGraph) and FX3's runtime (Src/Runtime).
  file(GLOB facefx_sources CONFIGURE_DEPENDS "${DISHONORED_FACEFX_DIR}/Src/*.cpp" "${DISHONORED_FACEFX_DIR}/Src/FaceGraph/*.cpp" "${DISHONORED_FACEFX_DIR}/Src/Runtime/*.cpp")
  file(GLOB facefx_headers CONFIGURE_DEPENDS "${DISHONORED_FACEFX_DIR}/include/*.h" "${DISHONORED_FACEFX_DIR}/Src/*.h" "${DISHONORED_FACEFX_DIR}/Src/FaceGraph/*.h")
  add_library(facefx STATIC ${facefx_sources} ${facefx_headers})
  target_include_directories(facefx PUBLIC "${DISHONORED_FACEFX_DIR}/include")
  target_include_directories(facefx PRIVATE "${DISHONORED_FACEFX_DIR}/Src")
  target_compile_definitions(facefx PUBLIC DISHONORED_WITH_FACEFX_SDK=1)
  # Retail builds the SDK inside the UE3 build, so with the engine's /Zp4 (FxActorInstance::FxAnimPlaybackInfo
  # keeps its double at offset 12 in the PDB). The sizes are static_asserted in the units.
  target_compile_options(facefx PRIVATE /Zp4 /wd4100 /wd4127 /wd4244 /wd4245 /wd4267 /wd4389 /wd4456 /wd4457 /wd4458)
  set_target_properties(facefx PROPERTIES FOLDER "External")
  add_library(Dishonored::facefx ALIAS facefx)
  list(LENGTH facefx_sources n)
  message(STATUS "FaceFX: reconstructed 1.7.3.1 SDK (${n} compile units), engine integration WITH_FACEFX=1")
endif()
