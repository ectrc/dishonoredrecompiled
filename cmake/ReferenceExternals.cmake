# SDKs that cannot be fetched (DirectX 9 June-2010-era headers, PhysX 2.8 "Novodex", ...) are
# taken from the reference engine tree next to this repo. DISHONORED_REFERENCE_DIR points at the
# CodeRedModding/UnrealEngine3 clone; nothing from it is copied into the repo.

set(DISHONORED_REFERENCE_DIR "${CMAKE_SOURCE_DIR}/../UnrealEngine3" CACHE PATH "UE3 build 10897 reference tree (CodeRedModding/UnrealEngine3)")
set(_ref_external "${DISHONORED_REFERENCE_DIR}/Development/External")

function(dishonored_reference_sdk name header)
  cmake_parse_arguments(arg "" "" "INCLUDE_DIRS;LIB_DIRS;LIBS;EXCLUDE_HEADERS;REWRITE_INCLUDES" ${ARGN})
  set(candidates "")
  foreach(dir IN LISTS arg_INCLUDE_DIRS)
    list(APPEND candidates "${_ref_external}/${dir}")
  endforeach()
  find_path(${name}_INCLUDE_DIR NAMES ${header} PATHS ${candidates} NO_DEFAULT_PATH)
  if(NOT ${name}_INCLUDE_DIR)
    message(FATAL_ERROR "${name}: ${header} not found under ${_ref_external} (${arg_INCLUDE_DIRS}); set DISHONORED_REFERENCE_DIR")
  endif()
  set(include_dir "${${name}_INCLUDE_DIR}")
  if(arg_EXCLUDE_HEADERS OR arg_REWRITE_INCLUDES)
    # Old SDKs ship copies of Windows headers (rpcsal.h, ...) that shadow the Windows Kit's, or
    # include their siblings by relative paths that only work inside the reference tree. Mirror
    # the include dir into the build tree without the excluded files and with includes rewritten
    # (REWRITE_INCLUDES takes "old=new" pairs applied as plain string replacement).
    set(include_dir "${CMAKE_BINARY_DIR}/reference-sdk/${name}/include")
    file(MAKE_DIRECTORY "${include_dir}")
    file(GLOB sdk_headers RELATIVE "${${name}_INCLUDE_DIR}" "${${name}_INCLUDE_DIR}/*")
    foreach(h IN LISTS sdk_headers)
      if(NOT h IN_LIST arg_EXCLUDE_HEADERS AND NOT IS_DIRECTORY "${${name}_INCLUDE_DIR}/${h}")
        if(arg_REWRITE_INCLUDES AND h MATCHES "\\.(h|hpp|inl)$")
          file(READ "${${name}_INCLUDE_DIR}/${h}" content)
          foreach(pair IN LISTS arg_REWRITE_INCLUDES)
            string(REPLACE "=" ";" pair_list "${pair}")
            list(GET pair_list 0 old)
            list(GET pair_list 1 new)
            string(REPLACE "${old}" "${new}" content "${content}")
          endforeach()
          file(WRITE "${include_dir}/${h}" "${content}")
        else()
          configure_file("${${name}_INCLUDE_DIR}/${h}" "${include_dir}/${h}" COPYONLY)
        endif()
      endif()
    endforeach()
  endif()
  add_library(Dishonored::${name} INTERFACE IMPORTED)
  target_include_directories(Dishonored::${name} INTERFACE "${include_dir}")
  foreach(dir IN LISTS arg_LIB_DIRS)
    if(IS_DIRECTORY "${_ref_external}/${dir}")
      target_link_directories(Dishonored::${name} INTERFACE "${_ref_external}/${dir}")
    endif()
  endforeach()
  if(arg_LIBS)
    target_link_libraries(Dishonored::${name} INTERFACE ${arg_LIBS})
  endif()
  message(STATUS "reference SDK ${name}: ${${name}_INCLUDE_DIR}")
endfunction()

# DirectX 9 (Core/Inc/UnMathSSE.h includes d3dx9.h; D3D9Drv needs the whole SDK)
dishonored_reference_sdk(DirectX9 d3dx9.h
  INCLUDE_DIRS DirectX9/Include DirectX9/include
  LIB_DIRS DirectX9/Lib DirectX9/Lib/x86
  LIBS d3d9 d3dx9 dxguid dinput8 xinput
  EXCLUDE_HEADERS rpcsal.h)

# libpng: the reference Development/External/libPNG is 1.2.5 (header and lib) but Engine's
# UnPNG.cpp targets libpng 1.5.13 (png_set_add_alpha, its own png_check_sig shim), so the
# reference copy cannot link Engine. Dishonored::libPNG now comes from cmake/Dependencies.cmake.

# nvtt (Engine/Src/UnTexCompress.cpp: DXT compression in the game build under
# !UE3_LEAN_AND_MEAN && !DEDICATED_SERVER)
dishonored_reference_sdk(nvtt nvtt/nvtt.h
  INCLUDE_DIRS nvtt/include
  LIB_DIRS nvtt/lib
  LIBS nvtt)

# nvTriStrip: not wired. The reference Development/External/nvTriStrip (and the Engine/Src copy of
# its header) is the stock 16-bit-index library, but Engine/Src/RawIndexBuffer.cpp calls Epic's
# 32-bit fork (GenerateStrips(const unsigned int*, ...)); WITH_NVTRISTRIP=0 in DishonoredDefines.
