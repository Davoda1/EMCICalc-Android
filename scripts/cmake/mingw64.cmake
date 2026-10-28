# Linux-hosted Windows build using a single native MSYS2 compiler/runtime set.
# Requires Wine; the prefix must include gcc.exe/g++.exe plus matching headers,
# CRT, import libraries, Qt and libqalculate. Never use host GCC with MSYS2 DLLs.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(EMCICALC_MINGW_PREFIX "${EMCICALC_MINGW_PREFIX}" CACHE PATH "Complete native MSYS2 MINGW64 prefix")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES EMCICALC_MINGW_PREFIX)
if(NOT EXISTS "${EMCICALC_MINGW_PREFIX}/bin/g++.exe")
  message(FATAL_ERROR "Pass -DEMCICALC_MINGW_PREFIX=/absolute/path/to/mingw64 containing the matching native MSYS2 GCC compiler")
endif()
get_filename_component(_emci_runner "${CMAKE_CURRENT_LIST_DIR}/../wine_mingw.py" ABSOLUTE)
set(CMAKE_C_COMPILER "${_emci_runner}" "--prefix=${EMCICALC_MINGW_PREFIX}" "--tool=gcc")
set(CMAKE_CXX_COMPILER "${_emci_runner}" "--prefix=${EMCICALC_MINGW_PREFIX}" "--tool=g++")
# Host binutils only read/write COFF objects; they introduce no C++ ABI symbols.
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_AR x86_64-w64-mingw32-ar)
set(CMAKE_RANLIB x86_64-w64-mingw32-ranlib)
set(CMAKE_FIND_ROOT_PATH "${EMCICALC_MINGW_PREFIX}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
