# dishonored_module(<Name>): one static library per UE3 module under source/Development/Src/<Name>.
#
# * compiles Src/**/*.cpp minus the <Name>_EXCLUDE list from <Name>/Sources.cmake (written by
#   resources/tools/import_reference.py)
# * exposes Inc/, Inc/Licensee, Inc/Epic (when present) and depends on every module declared
#   before it, because UE3 modules include each other's headers freely
# * uses <Name>Private.h as the precompiled header when it exists
# * applies DISHONORED_DEFINES and the shared MSVC options
#
# Silenced warnings live in DISHONORED_MSVC_WARNINGS with a reason each; add to it only from
# resources/docs/porting_notes.md.

set(DISHONORED_MSVC_WARNINGS
  /wd4100  # unreferenced formal parameter: pervasive in UE3 virtual interfaces
  /wd4127  # conditional expression is constant: while(1) / template branches
  /wd4201  # nameless struct/union: FVector / FColor unions
  /wd4244  # int/float narrowing: UE3 relies on implicit conversions everywhere
  /wd4245  # signed/unsigned mismatch in initialization
  /wd4267  # size_t narrowing
  /wd4305  # double -> float truncation of literals (UE3 writes 0.5 for FLOAT everywhere)
  /wd4324  # structure padded due to alignment specifier (MS_ALIGN)
  /wd4471  # forward declaration of an unscoped enum without underlying type (EPixelFormat in UnAsyncWork.h)
  /wd4595  # inline non-member operator new/delete in UnFile.h; revisit when the exe links (milestone 1)
  /wd4389  # signed/unsigned == comparison
  /wd4456  # declaration hides previous local declaration
  /wd4457  # declaration hides function parameter
  /wd4458  # declaration hides class member
  /wd4459  # declaration hides global declaration
  /wd4505  # unreferenced function with internal linkage removed
  /wd4611  # setjmp/C++ object interaction
  /wd4701  # potentially uninitialized local
  /wd4702  # unreachable code
  /wd4996  # deprecated CRT functions
)

set(DISHONORED_MODULE_LIST "" CACHE INTERNAL "modules declared so far, in order")
option(DISHONORED_USE_PCH "Use <Module>Private.h as a CMake precompiled header (see note in dishonored_module)" OFF)

function(dishonored_module name)
  set(module_dir "${CMAKE_SOURCE_DIR}/source/Development/Src/${name}")
  if(NOT EXISTS "${module_dir}/Src")
    message(FATAL_ERROR "dishonored_module(${name}): ${module_dir}/Src does not exist")
  endif()

  set(${name}_EXCLUDE "")
  set(${name}_NOT_IN_PDB "")
  if(EXISTS "${module_dir}/Sources.cmake")
    include("${module_dir}/Sources.cmake")
  endif()

  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${module_dir}/Src/*.cpp")
  foreach(excluded IN LISTS ${name}_EXCLUDE)
    list(REMOVE_ITEM sources "${module_dir}/${excluded}")
  endforeach()
  file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${module_dir}/Inc/*.h" "${module_dir}/Src/*.h")

  add_library(${name} STATIC ${sources} ${headers})
  target_include_directories(${name} PUBLIC "${module_dir}/Inc")
  foreach(extra IN ITEMS Licensee Epic)
    if(EXISTS "${module_dir}/Inc/${extra}")
      target_include_directories(${name} PUBLIC "${module_dir}/Inc/${extra}")
    endif()
  endforeach()
  target_include_directories(${name} PRIVATE "${module_dir}/Src")
  # UE3 uses a flat include model (UnrealBuildTool adds every module's Inc to every compile):
  # Core's UnVcWin32.h includes WinDrv's PreWindowsApi.h, Engine headers include GameFramework's, ...
  file(GLOB module_inc_dirs LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/source/Development/Src/*/Inc")
  foreach(inc IN LISTS module_inc_dirs)
    if(IS_DIRECTORY "${inc}")
      target_include_directories(${name} PUBLIC "${inc}")
    endif()
  endforeach()

  foreach(dep IN LISTS DISHONORED_MODULE_LIST)
    target_link_libraries(${name} PUBLIC ${dep})
  endforeach()

  # CMake's target_precompile_headers force-includes the header (/FI) on top of the TU's own
  # #include "<Name>Private.h"; UE3's private headers have no include guards (they relied on the
  # classic /Yu model), so this double-includes UnLinker.h & co. Off until a guard-safe scheme exists.
  if(DISHONORED_USE_PCH)
    if(EXISTS "${module_dir}/Inc/${name}Private.h")
      target_precompile_headers(${name} PRIVATE "${module_dir}/Inc/${name}Private.h")
    elseif(EXISTS "${module_dir}/Src/${name}Private.h")
      target_precompile_headers(${name} PRIVATE "${module_dir}/Src/${name}Private.h")
    endif()
  endif()

  dishonored_apply_defines(${name})
  target_compile_options(${name} PRIVATE ${DISHONORED_MSVC_WARNINGS})
  set_target_properties(${name} PROPERTIES FOLDER "Engine")

  set(DISHONORED_MODULE_LIST "${DISHONORED_MODULE_LIST};${name}" CACHE INTERNAL "modules declared so far, in order")
  list(LENGTH sources n)
  message(STATUS "module ${name}: ${n} compile units")
endfunction()
