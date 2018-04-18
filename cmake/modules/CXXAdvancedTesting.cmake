#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------
if (__ulang_cxxadvanced_testing_isloaded)
    return ()
endif ()
set (__ulang_cxxadvanced_testing_isloaded YES)

add_custom_target (check DEPENDS tests COMMAND $<TARGET_FILE:tests>)

find_program (VALGRIND_EXE NAMES valgrind
              DOC "Valgrind memory leak detector tool")

if (VALGRIND_EXE)
    message (STATUS "Will memory leak check tests.")
    unset (VALGRIND_SUPPS)
    file (GLOB_RECURSE VALGRIND_SUPPS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.supp")
    set (VALGRIND_SUPP "")
    foreach (supp ${VALGRIND_SUPPS})
        set (VALGRIND_SUPP ${VALGRIND_SUPP} --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/${supp})
    endforeach ()
    add_custom_target (valgrind DEPENDS tests
        COMMAND ${VALGRIND_EXE} --leak-check=full --show-leak-kinds=all --error-exitcode=192 --child-silent-after-fork=yes --track-origins=yes --gen-suppressions=all --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../cmake/valgrind.supp ${VALGRIND_SUPP} $<TARGET_FILE:tests>
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} VERBATIM)
else ()
    message (STATUS "Will not perform memory leak checks. (Missing valgrind.)")
endif ()

# vim: set ts=2 sw=2 expandtab :