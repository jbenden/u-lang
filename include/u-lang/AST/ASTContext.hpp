/**
 * The U Programming Language
 *
 * Copyright 2018 Joseph Benden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \author Joseph W. Benden
 * \copyright (C) 2018 Joseph Benden
 * \license apache2
 */

#ifndef U_LANG_ASTCONTEXT_HPP
#define U_LANG_ASTCONTEXT_HPP

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <llvm/Support/Allocator.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <u-lang/Basic/SourceManager.hpp>
#include <u-lang/u.hpp>

namespace u
{

class UAPI ASTContext
{
  /// \brief The allocator used to create AST objects.
  ///
  /// AST objects are never destructed; rather, all memory associated with the AST
  /// objects will be released when the ASTContext itself is destroyed.
  mutable llvm::BumpPtrAllocator BumpAlloc;

public:
  ASTContext() = default;

  // ASTContext(SourceManager& SM);
  ASTContext(ASTContext const&) = delete;

  ASTContext& operator=(ASTContext const&) = delete;

  llvm::BumpPtrAllocator& getAllocator() const { return BumpAlloc; }

  void* Allocate(size_t Size, unsigned Align = 8) const { return BumpAlloc.Allocate(Size, Align); }

  template<typename T>
  T* Allocate(size_t Num = 1) const
  {
    return static_cast<T*>(Allocate(Num * sizeof(T), alignof(T)));
  }

  void Deallocate(void* Ptr) const {}

  /// \brief Return the total amount of physical memory allocated for representing
  /// AST nodes and type information.
  size_t getASTAllocatedMemory() const { return BumpAlloc.getTotalMemory(); }
};

} /* namespace u */

/// \brief Placement new for using the ASTContext's allocator.
inline void*
operator new(size_t Bytes, const u::ASTContext& C, size_t Alignment)
{
  return C.Allocate(Bytes, static_cast<unsigned>(Alignment));
}

/// \brief Placement delete companion to the new above.
inline void
operator delete(void* Ptr, const u::ASTContext& C, size_t)
{
  C.Deallocate(Ptr);
}

/// \brief Placement new[] form uses the ASTContext's allocator for
/// obtaining memory.
inline void*
operator new[](size_t Bytes, const u::ASTContext& C, size_t Alignment = 8)
{
  return C.Allocate(Bytes, static_cast<unsigned>(Alignment));
}

/// \brief Placement delete[] companion to the new[] above.
inline void
operator delete[](void* Ptr, const u::ASTContext& C, size_t)
{
  C.Deallocate(Ptr);
}

#endif // U_LANG_ASTCONTEXT_HPP
