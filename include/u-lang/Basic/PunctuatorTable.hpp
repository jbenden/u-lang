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

#ifndef U_LANG_PUNCTUATORTABLE_HPP
#define U_LANG_PUNCTUATORTABLE_HPP

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

#include <map>

#include <u-lang/Basic/TokenKinds.hpp>
#include <u-lang/u.hpp>

namespace u
{

class UAPI PunctuatorInfo
{
  std::string Spelling;
  tok::TokenKind Kind;
  int Precedence;

public:
  PunctuatorInfo(std::string S, tok::TokenKind K, int P)
    : Spelling{std::move(S)}
    , Kind{K}
    , Precedence{P} {}

  std::string getSpelling() const { return Spelling; }

  tok::TokenKind getKind() const { return Kind; }

  int getPrecedence() const { return Precedence; }
};

class UAPI PunctuatorTable
{
  typedef std::map<std::string, PunctuatorInfo> TableT;
  typedef TableT::iterator iterator;
  typedef TableT::const_iterator const_iterator;
  TableT Table;

public:
  PunctuatorTable();

  llvm::Optional<PunctuatorInfo> get(llvm::StringRef Search);

  iterator begin() { return Table.begin(); }

  iterator end() { return Table.end(); }

  const_iterator begin() const { return Table.begin(); }

  const_iterator end() const { return Table.end(); }

protected:
  void AddPunctuators();
};

} /* namespace u */

#endif //U_LANG_PUNCTUATORTABLE_HPP
