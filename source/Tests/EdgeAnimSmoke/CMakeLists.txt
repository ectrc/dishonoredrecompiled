# EdgeAnimSmoke: decodes cooked Edge animations (blobs extracted by resources/tools/pdb/extract_edgeanim.py) with the
# port in Engine/Src/EdgeAnimEvaluate.cpp and, when given the retail Dishonored.exe, compares every frame against the
# retail evaluator itself (__edgeAnimEvaluate, 2013 rva 0x5dada0) loaded and relocated in-process.
# Core.lib + the CoreSmoke stubs provide the UE containers used by EdgeAnim.h.
add_executable(EdgeAnimSmoke
  EdgeAnimSmoke.cpp
  ${CMAKE_SOURCE_DIR}/source/Tests/CoreSmoke/stubs.cpp
  ${CMAKE_SOURCE_DIR}/source/Tests/CoreSmoke/engine_stubs.cpp
  ${CMAKE_SOURCE_DIR}/source/Development/Src/Engine/Src/EdgeAnimEvaluate.cpp)
target_link_libraries(EdgeAnimSmoke PRIVATE Core dbghelp winmm)
dishonored_apply_defines(EdgeAnimSmoke)
target_compile_options(EdgeAnimSmoke PRIVATE ${DISHONORED_MSVC_WARNINGS})
# the retail image is mapped at a free address; keep ours away from its preferred 0x400000
target_link_options(EdgeAnimSmoke PRIVATE /BASE:0x10000000 /DYNAMICBASE)
set_target_properties(EdgeAnimSmoke PROPERTIES FOLDER "Tests")
