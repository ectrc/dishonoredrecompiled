# Win32 x86 toolchain. Expects to run inside a VS "x86 Native Tools" environment (vcvars32),
# which puts the 32-bit cl.exe/link.exe on PATH. run-vcvars.cmd sets that up.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)
