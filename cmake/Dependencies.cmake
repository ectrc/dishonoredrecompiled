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

# Examples for Phase 4 (uncomment when the module that needs them exists):
# dishonored_fetch(zlib   https://github.com/madler/zlib.git        v1.3.1)
# dishonored_fetch(libpng https://github.com/pnggroup/libpng.git    v1.6.43)
# dishonored_fetch(ogg    https://github.com/xiph/ogg.git           v1.3.5)
# dishonored_fetch(vorbis https://github.com/xiph/vorbis.git        v1.3.7)
# dishonored_fetch(lzo    https://github.com/nemequ/lzo.git         2.10)
