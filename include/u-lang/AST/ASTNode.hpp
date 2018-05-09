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

#ifndef U_LANG_EXPR_HPP
#define U_LANG_EXPR_HPP

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <llvm/ADT/iterator.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/Allocator.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <u-lang/AST/ASTContext.hpp>
#include <u-lang/u.hpp>

#include <cassert>

namespace u
{

class UAPI ASTNode
{
public:
  enum ExprKind
  {
    NoExprKind = 0,
#define ABSTRACT_STMT(type)
#define STMT(TYPE, BASE) TYPE##Kind
#include <u-lang/AST/ASTNodes.def>
  };

private:
  ExprKind Kind;

  std::vector<ASTNode*> IncomingNodes;

protected:
  void* operator new(size_t bytes) noexcept { llvm_unreachable("ASTNode cannot be allocated with regular 'new'."); } // LCOV_EXCL_LINE

  void operator delete(void* data) noexcept { llvm_unreachable("ASTNode cannot be released with regular 'delete'."); } // LCOV_EXCL_LINE

public:
  void* operator new(size_t bytes, const u::ASTContext& C, unsigned alignment = 8);

  void* operator new(size_t bytes, const u::ASTContext* C, unsigned alignment = 8)
  {
    return operator new(bytes, *C, alignment);
  }

  void* operator new(size_t bytes, void* mem) noexcept { return mem; }

  void operator delete(void*, const ASTContext&, unsigned) noexcept {}

  void operator delete(void*, const ASTContext*, unsigned) noexcept {}

  void operator delete(void*, size_t) noexcept {}

  void operator delete(void*, void*) noexcept {}

protected:
  /// \brief Iterator for iterating over ASTNode* arrays.
  struct ExprIterator : llvm::iterator_adaptor_base<ExprIterator, ASTNode**, std::random_access_iterator_tag, ASTNode*>
  {
    ExprIterator()
      : iterator_adaptor_base(nullptr)
    {
    }

    ExprIterator(ASTNode** I)
      : iterator_adaptor_base(I)
    {
    }

    reference operator*() const { return *reinterpret_cast<ASTNode**>(I); }
  };

  //  struct ConstExprIterator : llvm::iterator_adaptor_base<ConstExprIterator, const ASTNode *const*,
  //  std::random_access_iterator_tag, const ASTNode*const>
  //  {
  //    ConstExprIterator() : iterator_adaptor_base(nullptr) {}
  //    ConstExprIterator(const ASTNode *const *I) : iterator_adaptor_base(I) {}
  //
  //    reference operator*() const
  //    {
  //      return *reinterpret_cast<const ASTNode *const*>(I);
  //    }
  //  };

public:
  explicit ASTNode(ExprKind K)
    : Kind{K}
  {
  }

  virtual ~ASTNode() = default; // LCOV_EXCL_LINE

  /// \brief The type of an incoming ASTNode iterator; an iterator over all incoming adjacent vertices.
  typedef std::vector<ASTNode*>::iterator incoming_iterator;

  /// \brief The type of an incoming ASTNode iterator's range; holding the first and last iterator positions.
  typedef llvm::iterator_range<incoming_iterator> incoming_range;

  /// \brief Retrieve an iterator range; holding the start and end of all incoming adjacent ASTNode vertices.
  virtual incoming_range incoming()
  {
    return incoming_range{IncomingNodes.begin(), IncomingNodes.end()};
  }

  /// \brief Retrieve an iterator to the first incoming adjacent ASTNode vertex.
  incoming_iterator incoming_begin() { return incoming().begin(); }

  /// \brief Retrieve an iterator at the end of all incoming adjacent ASTNode vertices.
  incoming_iterator incoming_end() { return incoming().end(); }

  /// \brief The type of an outgoing ASTNode iterator; an iterator over all outgoing adjacent vertices.
  typedef ExprIterator outgoing_iterator;

  /// \brief The type of an outgoing ASTNode iterator's range; holding the first and last iterator positions.
  typedef llvm::iterator_range<outgoing_iterator> outgoing_range;

  /// \brief Retrieve an iterator range; holding the start and end of all outgoing adjacent ASTNode vertices.
  outgoing_range outgoing();

  /// \brief Retrieve an iterator to the first outgoing adjacent ASTNode vertex.
  outgoing_iterator outgoing_begin() { return outgoing().begin(); }

  /// \brief Retrieve an iterator at the end of all outgoing adjacent ASTNode vertices.
  outgoing_iterator outgoing_end() { return outgoing().end(); }

  /// \brief Retrieves the ASTNode's implementation kind/type.
  ExprKind getKind() const { return Kind; }
};

/// \brief Represents an entire unit of compilation, holding all inner AST graph vertices.
class UAPI UnitAST : public ASTNode // LCOV_EXCL_LINE
{
  /// \brief A unit contains a variable number of children/adjacent ASTNode vertices.
  std::vector<ASTNode*> OutgoingNodes;

public:
  /// \brief Constructor.
  UnitAST()
    : ASTNode(UnitASTKind)
  {
  }

  outgoing_range outgoing()
  {
    return outgoing_range{outgoing_iterator(&OutgoingNodes[0]),
                          outgoing_iterator(&OutgoingNodes[OutgoingNodes.size()])};
  }

  static bool classof(const ASTNode* T) { return T->getKind() == UnitASTKind; }
};

} /* namespace u */

#endif // U_LANG_EXPR_HPP
