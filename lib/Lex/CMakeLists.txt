#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------

add_library(ulangLex STATIC Lexer.cpp)
add_dependencies(ulangLex stdtypes_h)
target_link_libraries(ulangLex ulangBasic ${LLVM_LIBRARIES})

# vim: set ts=2 sw=2 expandtab :