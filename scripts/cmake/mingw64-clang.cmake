# Native Linux Clang/LLD cross-build using one MSYS2 MINGW64 dependency set.
# Unlike mingw64.cmake, this adapter does not execute any Windows programs.
# The C++ headers, CRT, import libraries and runtime remain the prefix's GCC
# libstdc++/libgcc set. Do not substitute libc++, UCRT64 or host GCC headers.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(EMCICALC_MINGW_PREFIX "${EMCICALC_MINGW_PREFIX}" CACHE PATH "Complete MSYS2 MINGW64 dependency prefix")
set(EMCICALC_GCC_VERSION "16.2.0" CACHE STRING "GCC runtime/header version inside the prefix")
set(EMCICALC_CLANG_CXX "${EMCICALC_CLANG_CXX}" CACHE FILEPATH "Native Linux clang++ executable, version 21 or newer")
set(EMCICALC_LLD "${EMCICALC_LLD}" CACHE FILEPATH "Native Linux ld.lld executable")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
  EMCICALC_MINGW_PREFIX EMCICALC_GCC_VERSION EMCICALC_CLANG_CXX EMCICALC_LLD)

if(NOT EMCICALC_CLANG_CXX)
  find_program(_emci_clang NAMES clang++-21 clang++ REQUIRED)
  set(EMCICALC_CLANG_CXX "${_emci_clang}" CACHE FILEPATH "Native Linux clang++ executable" FORCE)
endif()
if(NOT EMCICALC_LLD)
  find_program(_emci_lld NAMES ld.lld-21 ld.lld REQUIRED)
  set(EMCICALC_LLD "${_emci_lld}" CACHE FILEPATH "Native Linux ld.lld executable" FORCE)
endif()
execute_process(COMMAND "${EMCICALC_CLANG_CXX}" --version
  OUTPUT_VARIABLE _emci_clang_version RESULT_VARIABLE _emci_clang_status)
if(NOT _emci_clang_status EQUAL 0 OR NOT _emci_clang_version MATCHES "clang version ([0-9]+)\\.")
  message(FATAL_ERROR "EMCICALC_CLANG_CXX must be an executable native Clang compiler")
endif()
if(CMAKE_MATCH_1 LESS 21)
  message(FATAL_ERROR "Use Clang 21 or newer: Clang 18 cannot parse the GCC 16 libstdc++ headers used by this release")
endif()

set(_emci_headers "${EMCICALC_MINGW_PREFIX}/include/c++/${EMCICALC_GCC_VERSION}")
set(_emci_gcc "${EMCICALC_MINGW_PREFIX}/lib/gcc/x86_64-w64-mingw32/${EMCICALC_GCC_VERSION}")
foreach(_emci_required
    "${_emci_headers}/x86_64-w64-mingw32/bits/c++config.h"
    "${_emci_gcc}/crtbegin.o" "${_emci_gcc}/crtend.o" "${_emci_gcc}/libgcc.a"
    "${EMCICALC_MINGW_PREFIX}/lib/crt2.o"
    "${EMCICALC_MINGW_PREFIX}/lib/libstdc++.dll.a"
    "${EMCICALC_MINGW_PREFIX}/bin/libstdc++-6.dll"
    "${EMCICALC_MINGW_PREFIX}/bin/libgcc_s_seh-1.dll"
    "${EMCICALC_MINGW_PREFIX}/bin/libwinpthread-1.dll")
  if(NOT EXISTS "${_emci_required}")
    message(FATAL_ERROR "Incomplete matching Windows runtime/header prefix: ${_emci_required}")
  endif()
endforeach()

set(CMAKE_CXX_COMPILER "${EMCICALC_CLANG_CXX}")
set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-windows-gnu)
set(CMAKE_SYSROOT "${EMCICALC_MINGW_PREFIX}")
set(CMAKE_CXX_FLAGS_INIT
  "-pthread -nostdinc++ -isystem \"${_emci_headers}\" -isystem \"${_emci_headers}/x86_64-w64-mingw32\" -isystem \"${_emci_headers}/backward\" -B\"${_emci_gcc}\"")
set(_emci_link_flags "-stdlib=libstdc++ --rtlib=libgcc -fuse-ld=\"${EMCICALC_LLD}\" -L\"${_emci_gcc}\" -L\"${EMCICALC_MINGW_PREFIX}/lib\"")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_emci_link_flags}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_emci_link_flags}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_emci_link_flags}")

# Native binutils only process resources/COFF archives; they add no C++ ABI.
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
# windres otherwise tries to run a separate MinGW GCC preprocessor. Reuse the
# already validated native Clang and the same target headers for RC input.
set(CMAKE_RC_FLAGS_INIT
  "--preprocessor=\"${EMCICALC_CLANG_CXX}\" --preprocessor-arg=-E --preprocessor-arg=-xc --preprocessor-arg=-DRC_INVOKED --preprocessor-arg=--target=x86_64-w64-windows-gnu --preprocessor-arg=--sysroot=\"${EMCICALC_MINGW_PREFIX}\"")
set(CMAKE_AR x86_64-w64-mingw32-ar)
set(CMAKE_RANLIB x86_64-w64-mingw32-ranlib)
set(CMAKE_FIND_ROOT_PATH "${EMCICALC_MINGW_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
