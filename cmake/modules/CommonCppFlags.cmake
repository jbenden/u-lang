#-------------------------------------------------------------------------------------------
# Compiler Flags
#-------------------------------------------------------------------------------------------
if(UNIX AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -debug inline-debug-info -parallel-source-info=2 -std=c++11 -shared-intel -fasm-blocks")
endif()

# NOT LESS == GREATER_OR_EQUAL; CMake doesn't support this out of the box.
if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  if(NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.1"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.1" AND (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.3")))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
  endif()
  # It seems Apple started changing version numbers after 3.1, going straight to 4.0 after 3.1.
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.3" AND (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.1")))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.1")
    message(FATAL_ERROR "Building with a Apple clang version less than 3.1 is not supported.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # non-Apple clangs uses different versioning.
  if(NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.5.0"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.5.0" AND (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.2")))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.2" AND (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.0")))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.0")
    message(FATAL_ERROR "Building with a clang version less than 3.0 is not supported.")
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if(NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.2.0"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.2.0" AND (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1")))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.1" AND (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.3")))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.3")
    message(FATAL_ERROR "Building with a gcc version less than 4.7.3 is not supported.")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG} -shared-libgcc")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG} -shared-libgcc")

endif()

#-------------------------------------------------------------------------------------------
# Common Compiler Flags
#-------------------------------------------------------------------------------------------
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-return-type -D_DEBUG -DDEBUG")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wextra -Werror -D_DEBUG -DDEBUG")

  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -Wextra -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -Wextra -DNDEBUG")

  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -Wextra")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -Wextra")
endif()

include(AppendCXXFlagIfSupported)
append_cxx_flag_if_supported(-Wmissing-prototypes COMMON_WARNINGS)
append_cxx_flag_if_supported(-Wtautological-compare COMMON_WARNINGS)
append_cxx_flag_if_supported(-Wshorten-64-to-32 COMMON_WARNINGS)
append_cxx_flag_if_supported(-Wno-literal-suffix COMMON_WARNINGS)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_WARNINGS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${COMMON_WARNINGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMMON_WARNINGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_WARNINGS}")

#-------------------------------------------------------------------------------------------
# Common optimizations (GCC and compatible)
#-------------------------------------------------------------------------------------------
if(NOT SKIP_OPT_WHOLE_PROGRAM)
  append_cxx_flag_if_supported(--combine COMMON_OPTS)
  append_cxx_flag_if_supported(-fwhole-program COMMON_OPTS)
endif()
append_cxx_flag_if_supported(-ffast-math COMMON_OPTS)
append_cxx_flag_if_supported(-ffunction-sections COMMON_OPTS)
append_cxx_flag_if_supported(-fno-omit-frame-pointer COMMON_OPTS) # allow VTune to analyze frame pointers

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # VTune: Permit user-mode sampling and tracing collection
  set(COMMON_OPTS "${COMMON_OPTS} -u malloc -u free -u realloc -u getenv -u setenv -u __errno_location")

  # VTune: Permit POSIX threads analysis
  set(COMMON_OPTS "${COMMON_OPTS} -u pthread_key_create -u pthread_key_delete -u pthread_setspecific -u pthread_getspecific -u pthread_spin_init -u pthread_spin_destroy -u pthread_spin_lock -u pthread_spin_trylock -u pthread_spin_unlock -u pthread_mutex_init -u pthread_mutex_destroy -u pthread_mutex_trylock -u pthread_mutex_lock -u pthread_mutex_unlock -u pthread_cond_init -u pthread_cond_destroy -u pthread_cond_signal -u pthread_cond_wait -u _pthread_cleanup_push -u _pthread_cleanup_pop -u pthread_setcancelstate -u pthread_self -u pthread_yield")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(COMMON_OPTS "${COMMON_OPTS} -Wno-unused-command-line-argument -Wno-uninitialized")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wno-unused-command-line-argument")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_OPTS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_OPTS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMMON_OPTS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${COMMON_OPTS}")

#-------------------------------------------------------------------------------------------
# Doxygen
#-------------------------------------------------------------------------------------------
find_package(Doxygen)
if(DOXYGEN_FOUND AND NOT SKIP_DOXYGEN)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doxygen-${PROJECT_NAME}
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# vim: set ts=2 sw=2 expandtab :