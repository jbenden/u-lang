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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <llvm/Support/ErrorHandling.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/AST/ASTNode.hpp>
#include <u-lang/u.hpp>

using namespace u;

void*
ASTNode::operator new(size_t Bytes, const ASTContext& C, unsigned Alignment)
{
  return ::operator new(Bytes, C, Alignment);
}

ASTNode::outgoing_range
ASTNode::outgoing()
{
  switch (getKind())
  {
  case ASTNode::NoExprKind:llvm_unreachable("statement without class"); // LCOV_EXCL_LINE

#define ABSTRACT_STMT(type)
#define STMT(type, base)                                                                                               \
  case ASTNode::type##Kind:                                                                                            \
    return static_cast<type*>(this)->outgoing();
#include <u-lang/AST/ASTNodes.def>
  }

  llvm_unreachable("unknown statement kind!"); // LCOV_EXCL_LINE
}
