# CoreSmoke: console program that links Core.lib and exercises FName, containers, app* helpers
# and FPackageFileSummary deserialization (Phase 2 P2.12). stubs.cpp provides the globals and
# hooks that Core expects from Launch/Engine (see resources/docs/agents/agentD.md).
add_executable(CoreSmoke CoreSmoke.cpp stubs.cpp engine_stubs.cpp)
# dbghelp/winmm: Core's stack walker (UnVcWin32.cpp Sym*/StackWalk64) and appPlatformInit
# (timeBeginPeriod) need them; UE3BuildWin32.cs adds dbghelp.lib to every Win32 link.
target_link_libraries(CoreSmoke PRIVATE Core dbghelp winmm)
dishonored_apply_defines(CoreSmoke)
target_compile_options(CoreSmoke PRIVATE ${DISHONORED_MSVC_WARNINGS})
set_target_properties(CoreSmoke PROPERTIES FOLDER "Tests")
# Agent IH1 (wave 26): the ported Wwise bank side (WwiseSmoke.cpp) against the retail banks, when the
# audio surface is in the build (dishonored_apply_defines already links Dishonored::wwise).
if(TARGET wwise)
  # Agent IH2 (wave 26): the ported Vorbis decoder on a retail voice line (WwiseVorbisSmoke.cpp) reaches into the
  # decoder's private headers, as CAkSrcBankVorbis does inside the wwise library.
  target_sources(CoreSmoke PRIVATE WwiseSmoke.cpp WwiseVorbisSmoke.cpp WwiseMusicSmoke.cpp WwiseFxSmoke.cpp WwiseFx2Smoke.cpp)
  target_include_directories(CoreSmoke PRIVATE "${DISHONORED_WWISE_DIR}/Src" "${DISHONORED_WWISE_DIR}/Src/AkVorbisDecoder" "${DISHONORED_WWISE_DIR}/Src/AkAudioLib/Common"
                                               "${DISHONORED_WWISE_DIR}/Src/AkAudioLib/SoftwarePipeline" "${DISHONORED_WWISE_DIR}/Src/AkAudioLib/Win32"
                                               "${DISHONORED_WWISE_DIR}/Src/AkMusicEngine/Common" "${DISHONORED_WWISE_DIR}/Src/Plugins/Common" "${DISHONORED_WWISE_DIR}/Src/Plugins/Effects/Common")
endif()

# Agents FX1/FX2/FX3 (wave 26): the reconstructed FaceFX 1.7.3.1 SDK against the retail archives - the core
# (FaceFXSmoke.cpp), the face graph (FaceFXGraphSmoke.cpp) and the runtime (FaceFXRuntimeSmoke.cpp: a head actor, an anim
# set, an instance playing one line).
if(TARGET facefx)
  target_sources(CoreSmoke PRIVATE FaceFXSmoke.cpp FaceFXGraphSmoke.cpp FaceFXRuntimeSmoke.cpp)
  target_link_libraries(CoreSmoke PRIVATE Dishonored::facefx)
  target_compile_definitions(CoreSmoke PRIVATE DISHONORED_WITH_FACEFX_GRAPH=1)
endif()
