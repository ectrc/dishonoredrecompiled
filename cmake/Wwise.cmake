# Wwise 2012.1 (Audiokinetic). Retail links the sound engine STATICALLY into Dishonored.exe - there is no
# DLL to import and no runtime to bind against (resources/docs/middleware.md section 2.4: ~3,300 functions
# of aksoundengine / akmusicengine / akstreammgr / ak*fx / akvorbisdecoder, PDB source root
# d:\branches\wwise_v2012.1\, bank format v65). So unlike Bink, PhysX and Steam there is nothing shipped
# that we can call: either the licensed SDK is installed, or we supply the API ourselves.
#
# This file offers both, behind one include path and one target, so the AkAudio port never changes:
#
#   DISHONORED_WWISE_SDK not set (the default)
#     source/Development/Src/External/Wwise2012/include is OUR reconstruction of the 2012.1 public
#     headers (written from the 2012 symbolized build's PDB: types, enum values, class layouts and the
#     exact demangled signatures; every declaration carries its evidence), and
#     source/Development/Src/External/Wwise2012/Src is the silent backend behind them: banks are opened
#     and their BKHD chunk read through the game's own low-level IO hook, events return real playing ids
#     and fire AK_EndOfEvent, RTPCs / switches / states / triggers / positions are recorded and queryable,
#     and nothing is decoded, mixed or output. See resources/docs/agents/agentAN.md.
#
#   DISHONORED_WWISE_SDK=<path to the installed Wwise 2012.1 SDK>
#     <path>/include is used instead and <path>/Win32_vc100/Release/lib supplies AkSoundEngine.lib,
#     AkMusicEngine.lib, AkStreamMgr.lib, AkMemoryMgr.lib, AkVorbisDecoder.lib, the effect plug-ins and
#     AkSink. The silent backend is then not compiled, DISHONORED_WWISE_SILENT=0 is defined, and
#     AK::AllocHook / AK::FreeHook come from AkAudio's own akaudiodevice.cpp as the SDK expects.
#
# DISHONORED_WITH_WWISE defaults ON because the bindings are in the tree; it turns the whole audio surface
# back off (DISHONORED_WITH_WWISE=0) for a build that wants the previous all-stub state.

set(DISHONORED_WWISE_DIR "${CMAKE_SOURCE_DIR}/source/Development/Src/External/Wwise2012")
set(DISHONORED_WWISE_SDK "" CACHE PATH "Installed Wwise 2012.1 SDK folder (with include/ and Win32_vc*/); empty = use our own headers plus the silent backend")

if(EXISTS "${DISHONORED_WWISE_DIR}/include/AK/SoundEngine/Common/AkSoundEngine.h")
  set(dishonored_wwise_default ON)
else()
  set(dishonored_wwise_default OFF)
endif()
option(DISHONORED_WITH_WWISE "Compile the Wwise 2012.1 audio surface (our headers + silent backend, or the installed SDK)" ${dishonored_wwise_default})

if(DISHONORED_WITH_WWISE AND NOT DISHONORED_ENABLE_AKAUDIO)
  message(STATUS "Wwise: DISHONORED_ENABLE_AKAUDIO is OFF, so the audio surface is off too (UAkAudioDevice, the low-level IO hook and the AK allocation hooks are that module's)")
  set(DISHONORED_WITH_WWISE OFF)
endif()

if(DISHONORED_WITH_WWISE)
  if(DISHONORED_WWISE_SDK)
    if(NOT EXISTS "${DISHONORED_WWISE_SDK}/include/AK/SoundEngine/Common/AkSoundEngine.h")
      message(FATAL_ERROR "DISHONORED_WWISE_SDK=${DISHONORED_WWISE_SDK} has no include/AK/SoundEngine/Common/AkSoundEngine.h")
    endif()
    add_library(wwise INTERFACE)
    target_include_directories(wwise INTERFACE "${DISHONORED_WWISE_SDK}/include")
    target_compile_definitions(wwise INTERFACE DISHONORED_WITH_WWISE=1 DISHONORED_WWISE_SILENT=0)
    foreach(cfg_dir IN ITEMS Win32_vc100/Release Win32_vc90/Release)
      if(EXISTS "${DISHONORED_WWISE_SDK}/${cfg_dir}/lib/AkSoundEngine.lib")
        set(wwise_lib_dir "${DISHONORED_WWISE_SDK}/${cfg_dir}/lib")
        break()
      endif()
    endforeach()
    if(NOT wwise_lib_dir)
      message(FATAL_ERROR "DISHONORED_WWISE_SDK: no Win32_vc100/Release/lib or Win32_vc90/Release/lib with AkSoundEngine.lib")
    endif()
    foreach(ak_lib IN ITEMS AkSoundEngine AkMusicEngine AkStreamMgr AkMemoryMgr AkVorbisDecoder
                            AkRoomVerbFX AkMatrixReverbFX AkParametricEQFX AkDelayFX AkTimeStretchFX
                            AkPitchShifterFX AkTremoloFX AkHarmonizerFX AkStereoDelayFX AkSilenceSource)
      if(EXISTS "${wwise_lib_dir}/${ak_lib}.lib")
        target_link_libraries(wwise INTERFACE "${wwise_lib_dir}/${ak_lib}.lib")
      endif()
    endforeach()
    # AkSink (DirectSound / XAudio2) pulls these in; retail imports dsound and XAudio2 the same way.
    target_link_libraries(wwise INTERFACE dsound dxguid ole32)
    message(STATUS "Wwise: installed 2012.1 SDK at ${DISHONORED_WWISE_SDK} (libs from ${wwise_lib_dir})")
  else()
    file(GLOB wwise_silent_sources "${DISHONORED_WWISE_DIR}/Src/*.cpp")
    file(GLOB_RECURSE wwise_headers "${DISHONORED_WWISE_DIR}/include/*.h" "${DISHONORED_WWISE_DIR}/Src/*.h")
    add_library(wwise STATIC ${wwise_silent_sources} ${wwise_headers})
    target_include_directories(wwise PUBLIC "${DISHONORED_WWISE_DIR}/include")
    target_include_directories(wwise PRIVATE "${DISHONORED_WWISE_DIR}/Src")
    target_compile_definitions(wwise PUBLIC DISHONORED_WITH_WWISE=1 DISHONORED_WWISE_SILENT=1)
    # No /Zp4 here: the AK structs carry their own #pragma pack(8) (the packing Audiokinetic builds with)
    # and the sizes are static_asserted against the 2012 PDB inside the headers.
    target_compile_options(wwise PRIVATE /wd4100 /wd4244 /wd4245 /wd4267 /wd4389)
    set_target_properties(wwise PROPERTIES FOLDER "External")
    list(LENGTH wwise_silent_sources n)
    message(STATUS "Wwise: our 2012.1 headers + silent backend (${n} compile units), no SDK needed")
  endif()
  add_library(Dishonored::wwise ALIAS wwise)
endif()
