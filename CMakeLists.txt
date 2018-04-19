#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)

if(POLICY CMP0022)
  cmake_policy(SET CMP0022 NEW) # automatic when 2.8.12 is required
endif()

project(u-lang)

enable_language(CXX C)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

#----------------------------------------------------------------------
# Options
#----------------------------------------------------------------------
option(ULANG_BUILD_TESTS "Enable generation of build files for tests." ON)
option(ULANG_USE_GCOV "Enable a GCov build." ON)

# Generate a CompilationDatabase (compile_commands.json file) for our build,
# for use by clang_complete, YouCompleteMe, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

#----------------------------------------------------------------------
# Project Dependencies
#----------------------------------------------------------------------

# https://github.com/ldc-developers/ldc/blob/master/cmake/Modules/FindLLVM.cmake
set(LLVM_ENABLE_RTTI ON)
set(LLVM_ENABLE_EH ON)
find_package(LLVM 3.9.0 REQUIRED COMPONENTS core)
include_directories(${LLVM_INCLUDE_DIRS})

string(REPLACE "-fno-exceptions" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-pedantic" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-fno-rtti" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-O2" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-DNDEBUG" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-g1" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-W " "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-Wno-maybe-uninitialized" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})
string(REPLACE "-Wl,-fuse-ld=gold" "" LLVM_CXXFLAGS ${LLVM_CXXFLAGS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

add_subdirectory(third-party/gflags)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(third-party/glog)

if(ULANG_BUILD_TESTS)
  add_subdirectory(third-party/gmock)
endif()

#----------------------------------------------------------------------
# Defines
#----------------------------------------------------------------------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

#----------------------------------------------------------------------
# Compiler Flags
#----------------------------------------------------------------------
set(SKIP_OPT_WHOLE_PROGRAM ON)
include(CommonCppFlags)
include(EnableStdTypes)
include(Versioning)

read_version("include/u-lang/u.hpp" UC)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND
  (CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "MinSizeRel"
    OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo") AND MINGW)
  message(FATAL_ERROR "FIXME: 32-bit mingw-w64 Clang crashes when compiling with any optimizations.")
endif()

if (ULANG_USE_GCOV)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()

#----------------------------------------------------------------------
# Include Directories
#----------------------------------------------------------------------
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/third-party/glog/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third-party/utfcpp/source")

#----------------------------------------------------------------------
# Libraries
#----------------------------------------------------------------------
add_custom_target(stdtypes_h DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/stdtypes.hpp)
enable_std_types(${CMAKE_CURRENT_BINARY_DIR}/stdtypes.hpp "cmake/modules" CXX)

find_package(Threads REQUIRED)

#----------------------------------------------------------------------
# Sub-projects
#----------------------------------------------------------------------
if(ULANG_BUILD_TESTS)
  add_subdirectory(tests)
endif()

#----------------------------------------------------------------------
# Packaging
#----------------------------------------------------------------------
set(PACKAGE_NAME U-lang)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/jbenden/u-lang/issues")
set(PACKAGE_VERSION "${UC_MAJOR_VERSION}.${UC_MINOR_VERSION}.${UC_MICRO_VERSION}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "U-lang")
set(CPACK_PACKAGE_VENDOR "Joseph Benden <joe@benden.us>")
set(CPACK_PACKAGE_VERSION_MAJOR ${UC_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${UC_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${UC_MICRO_VERSION})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma \r\n SetCompressorDictSize 32")
if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "U-lang")
  set(CPACK_NSIS_MODIFY_PATH "ON")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  if(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  endif()
endif()
include(CPack)

# vim: set ts=2 sw=2 expandtab :
