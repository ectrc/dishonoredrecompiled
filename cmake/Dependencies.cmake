# Third-party dependencies are pulled with FetchContent into <repo>/external/<name>-src and built
# as part of this project. No package manager. Add one dishonored_fetch() block per library as
# Phase 4 decides on it; keep pinned tags or commits, never branches.
include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external" CACHE PATH "" FORCE)
set(FETCHCONTENT_QUIET OFF)

function(dishonored_fetch name url tag)
  FetchContent_Declare(${name}
    GIT_REPOSITORY ${url}
    GIT_TAG ${tag}
    GIT_SHALLOW TRUE
    SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/${name}-src"
    BINARY_DIR "${FETCHCONTENT_BASE_DIR}/${name}-build"
  )
  FetchContent_MakeAvailable(${name})
endfunction()

# zlib: Core (UnMisc.cpp) compresses/decompresses package chunks with it; the cooked packages use
# COMPRESS_ZLIB (compression_flags 0x2 in resources/docs/symbols/package_summary.md).
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
dishonored_fetch(zlib https://github.com/madler/zlib.git v1.3.1)
add_library(Dishonored::zlib INTERFACE IMPORTED)
target_link_libraries(Dishonored::zlib INTERFACE zlibstatic)
target_include_directories(Dishonored::zlib INTERFACE "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}")

# Examples for Phase 4 (uncomment when the module that needs them exists):
# dishonored_fetch(libpng https://github.com/pnggroup/libpng.git    v1.6.43)
# dishonored_fetch(ogg    https://github.com/xiph/ogg.git           v1.3.5)
# dishonored_fetch(vorbis https://github.com/xiph/vorbis.git        v1.3.7)
# dishonored_fetch(lzo    https://github.com/nemequ/lzo.git         2.10)
