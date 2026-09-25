# Third-party dependencies are pulled with FetchContent into <repo>/external/<name>-src and built
# as part of this project. No package manager. Add one dishonored_fetch() block per library as
# Phase 4 decides on it; keep pinned tags or commits, never branches.
include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external" CACHE PATH "" FORCE)
set(FETCHCONTENT_QUIET OFF)

# Extra arguments go to FetchContent_Declare (OVERRIDE_FIND_PACKAGE, ...).
function(dishonored_fetch name url tag)
  FetchContent_Declare(${name}
    GIT_REPOSITORY ${url}
    GIT_TAG ${tag}
    GIT_SHALLOW TRUE
    SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/${name}-src"
    BINARY_DIR "${FETCHCONTENT_BASE_DIR}/${name}-build"
    ${ARGN}
  )
  FetchContent_MakeAvailable(${name})
  # FetchContent sets these in this function's scope only; the callers use them for include dirs
  set(${name}_SOURCE_DIR "${${name}_SOURCE_DIR}" PARENT_SCOPE)
  set(${name}_BINARY_DIR "${${name}_BINARY_DIR}" PARENT_SCOPE)
endfunction()

# zlib: Core (UnMisc.cpp) supports COMPRESS_ZLIB (=1) chunks and libpng needs it. Note the cooked
# packages themselves use COMPRESS_LZO (CompressionFlags=2), see WITH_LZO in DishonoredDefines.cmake.
# OVERRIDE_FIND_PACKAGE redirects libpng's find_package(ZLIB) to this build; the -extra file
# provides the ZLIB::ZLIB target that redirect expects.
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
dishonored_fetch(zlib https://github.com/madler/zlib.git v1.3.1 OVERRIDE_FIND_PACKAGE)
add_library(Dishonored::zlib INTERFACE IMPORTED)
target_link_libraries(Dishonored::zlib INTERFACE zlibstatic)
target_include_directories(Dishonored::zlib INTERFACE "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")
file(WRITE "${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/zlib-extra.cmake" [=[
if(NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB INTERFACE IMPORTED GLOBAL)
  target_link_libraries(ZLIB::ZLIB INTERFACE zlibstatic)
endif()
set(ZLIB_FOUND TRUE)
set(ZLIB_LIBRARIES ZLIB::ZLIB)
get_target_property(ZLIB_INCLUDE_DIRS zlibstatic INTERFACE_INCLUDE_DIRECTORIES)
]=])

# libpng: Engine/Inc/UnPNG.h (reached from Core through Engine.h) and Engine/Src/UnPNG.cpp, which
# was written against libpng 1.5.13 (png_set_add_alpha); the reference tree's copy is 1.2.5.
set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_TOOLS OFF CACHE BOOL "" FORCE)
set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE)
set(PNG_FRAMEWORK OFF CACHE BOOL "" FORCE)
dishonored_fetch(libpng https://github.com/pnggroup/libpng.git v1.6.43)
add_library(Dishonored::libPNG INTERFACE IMPORTED)
target_link_libraries(Dishonored::libPNG INTERFACE png_static)
target_include_directories(Dishonored::libPNG INTERFACE "${libpng_SOURCE_DIR}" "${libpng_BINARY_DIR}")

# NVAPI: Engine/Inc/ue3stereo.h (NVIDIA 3D Vision, included by SceneRenderTargets.cpp and
# D3D9Drv.h under !CONSOLE) needs nvapi.h; Shipping links nvapi.lib (10 NvAPI_ stubs in the PDB)
# but the reference tree has no Development/External/nvapi. NVIDIA's public SDK (MIT) is header +
# import lib only, no CMakeLists; pinned to the R535 main commit.
dishonored_fetch(nvapi https://github.com/NVIDIA/nvapi.git 70d337db9186e968eab622f7e786de7e437faf3d)
add_library(Dishonored::nvapi INTERFACE IMPORTED)
target_include_directories(Dishonored::nvapi INTERFACE "${nvapi_SOURCE_DIR}")
target_link_libraries(Dishonored::nvapi INTERFACE "${nvapi_SOURCE_DIR}/x86/nvapi.lib")

# lzokay: LZO1X-1 compressor/decompressor (MIT, C++14, stream-compatible with LZO1X). Every cooked
# package is PKG_StoreCompressed with CompressionFlags=2 = COMPRESS_LZO; the retail exe links LZO
# Professional (lzopro_lzo1x_decompress_safe), whose SDK is not available. Core/Src/UnMisc.cpp
# implements appCompressMemoryLZO/appUncompressMemoryLZO on it (WITH_LZO=1 in DishonoredDefines.cmake).
# Pinned to master as of 2026-09 (the project has no tags). Its CMakeLists also declares the test
# executable and the C wrapper, which nothing here uses.
dishonored_fetch(lzokay https://github.com/jackoalan/lzokay.git db2df1fcbebc2ed06c10f727f72567d40f06a2be)
set_target_properties(lzokaytest lzokay-c PROPERTIES EXCLUDE_FROM_ALL TRUE FOLDER "External")
set_target_properties(lzokay PROPERTIES FOLDER "External")
add_library(Dishonored::lzokay INTERFACE IMPORTED)
target_link_libraries(Dishonored::lzokay INTERFACE lzokay)
target_include_directories(Dishonored::lzokay INTERFACE "${lzokay_SOURCE_DIR}")

# Examples for Phase 4 (uncomment when the module that needs them exists):
# dishonored_fetch(ogg    https://github.com/xiph/ogg.git           v1.3.5)
# dishonored_fetch(vorbis https://github.com/xiph/vorbis.git        v1.3.7)
