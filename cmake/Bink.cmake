# Bink video (RAD Game Tools). Retail ships binkw32.dll 1.9p (1.9.16.0) and imports 20 stdcall entry
# points from it by their decorated names (_BinkOpen@8 ...; resources/docs/symbols/imports_2013.csv);
# there is no SDK in the tree. With DISHONORED_WITH_BINK=ON:
#   * source/Development/Src/Engine/Bink/Src/binkw32_stub.cpp is linked into a throw-away
#     build/<dir>/bink/stub/binkw32.dll whose import library (build/<dir>/bink/binkw32.lib) is
#     exactly what RAD's dllexport build produces: the exe then imports binkw32.dll!_BinkOpen@8
#     like retail. lib.exe /def cannot produce decorated-name imports on x86 (it either strips or
#     doubles the underscore; only import-by-ordinal works), see resources/docs/agents/agentU.md.
#   * the reconstructed bink.h next to it is what Engine/Bink/Src/BinkHeaders.h includes;
#   * dishonored_apply_defines() sets USE_BINK_CODEC=1 on every target and links Dishonored::bink.
# The real DLL stays in the retail tree (DISHONORED_RETAIL_DIR); the staging step puts the exe
# next to it. Default ON (agent HK): the loading movies, the briefing page and the intro play as in retail;
# -nomovie still skips them at run time. The import library needs no DLL to build, so a tree without the
# retail binkw32.dll configures with a warning; the exe then needs the DLL next to it to start.
# See resources/docs/middleware.md, section "Bink".

option(DISHONORED_WITH_BINK "Compile the Bink movie glue (USE_BINK_CODEC=1) and link the binkw32.dll import library" ON)
# A build directory configured while the default was OFF has DISHONORED_WITH_BINK=OFF in its cache, which option() keeps;
# the first configure after the flip turns it on once. An explicit -DDISHONORED_WITH_BINK=OFF on any later configure sticks.
if(NOT DEFINED CACHE{DISHONORED_WITH_BINK_DEFAULT_ON})
  set(DISHONORED_WITH_BINK ON CACHE BOOL "Compile the Bink movie glue (USE_BINK_CODEC=1) and link the binkw32.dll import library" FORCE)
  set(DISHONORED_WITH_BINK_DEFAULT_ON 1 CACHE INTERNAL "the DISHONORED_WITH_BINK default flip has been applied to this cache")
endif()
set(DISHONORED_RETAIL_DIR "D:/RecompileDishonored/Dishonored_Latest2026" CACHE PATH "Retail 2013 install (Binaries/Win32/binkw32.dll and the other shipped DLLs)")

if(DISHONORED_WITH_BINK)
  set(bink_dll "${DISHONORED_RETAIL_DIR}/Binaries/Win32/binkw32.dll")
  if(NOT EXISTS "${bink_dll}")
    message(WARNING "Bink: ${bink_dll} not found. The build goes on (the import library comes from binkw32_stub), but the exe "
      "imports binkw32.dll and will not start without the retail DLL beside it: set DISHONORED_RETAIL_DIR to the retail install, "
      "or configure with -DDISHONORED_WITH_BINK=OFF to build without movies.")
  endif()
  add_library(binkw32_stub SHARED "${CMAKE_SOURCE_DIR}/source/Development/Src/Engine/Bink/Src/binkw32_stub.cpp")
  set_target_properties(binkw32_stub PROPERTIES
    OUTPUT_NAME binkw32
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bink/stub"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bink"
    FOLDER "External")
  add_library(Dishonored::bink INTERFACE IMPORTED)
  target_link_libraries(Dishonored::bink INTERFACE binkw32_stub)
  message(STATUS "Bink: USE_BINK_CODEC=1, import library from binkw32_stub (retail DLL ${bink_dll})")
endif()
