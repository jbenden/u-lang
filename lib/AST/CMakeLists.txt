#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------

add_library(ulangAST STATIC ASTNode.cpp)
add_dependencies(ulangAST stdtypes_h)
target_link_libraries(ulangAST ${LLVM_LIBRARIES} ulangBasic)

# vim: set ts=2 sw=2 expandtab :