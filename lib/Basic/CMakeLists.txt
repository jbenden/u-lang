#----------------------------------------------------------------------
# Copyright (C) 2018 Joseph Benden <joe@benden.us>
#----------------------------------------------------------------------

add_library(ulangBasic STATIC Diagnostic.cpp DiagnosticIDs.cpp TokenKinds.cpp Source.cpp PunctuatorTable.cpp IdentifierTable.cpp SourceManager.cpp)
add_dependencies(ulangBasic stdtypes_h)
target_link_libraries(ulangBasic ${LLVM_LIBRARIES})

# vim: set ts=2 sw=2 expandtab :