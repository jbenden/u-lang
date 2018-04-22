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
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Optional.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <u-lang/Basic/IdentifierTable.hpp>
#include <u-lang/u.hpp>

using namespace u;

IdentifierTable::IdentifierTable()
{
  AddIdentifiers();
}

llvm::Optional<IdentifierInfo>
IdentifierTable::get(llvm::StringRef Search)
{
  auto Result = Table.find(Search);
  if (Result != end())
  {
    return Result->second;
  }

  return llvm::None;
}

void IdentifierTable::AddIdentifiers()
{
#define KEYWORD(X)                                                                                                     \
  Table.insert(std::make_pair(#X, IdentifierInfo{#X, tok::kw_ ## X}));
#define TYPE(X)                                                                                                        \
  Table.insert(std::make_pair(#X, IdentifierInfo{#X, tok::ty_ ## X}));
#define TYPE_MODIFIER(X)                                                                                               \
  Table.insert(std::make_pair(#X, IdentifierInfo{#X, tok::ty_mod_ ## X}));

#include <u-lang/Basic/TokenKinds.def>
}