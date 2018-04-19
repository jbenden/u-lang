#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------

enable_testing()

if(ULANG_USE_GCOV)
  include(CodeCoverage)
endif()

add_definitions(-DULANG_TEST_FIXTURE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/fixtures\")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../third-party/gmock/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../third-party/gmock/gtest/include")

add_executable(tests tests.cpp Basic/Source.cpp)
add_dependencies(tests stdtypes_h)
target_link_libraries(tests ulangBasic
                      gmock
                      ${LLVM_LIBRARIES})
set_target_properties(tests PROPERTIES
                      COMPILE_FLAGS "${CMAKE_CXX_FLAGS_COVERAGE}"
                      LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")

include(CXXAdvancedTesting)

if(ULANG_USE_GCOV)
  setup_target_for_coverage(coverage
                            $<TARGET_FILE:tests>
                            ${CMAKE_CURRENT_BINARY_DIR}/coverage)
endif()

# vim: set ts=2 sw=2 expandtab :