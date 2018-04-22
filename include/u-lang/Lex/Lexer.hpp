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

#ifndef U_LANG_LEXER_HPP
#define U_LANG_LEXER_HPP

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <llvm/ADT/StringRef.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/SourceLocation.hpp>
#include <u-lang/Lex/Token.hpp>
#include <u-lang/Basic/Source.hpp>
#include <u-lang/u.hpp>

#include <cassert>

namespace u
{

class UAPI Lexer
{
  Source& source_;
  std::string fileName_;
  std::string filePath_;
  SourceRange sourceRange_;
  uint32_t curChar_;
  uint32_t nextChar_;
  uint32_t curValid_;

public:
  explicit Lexer(Source& source);

  Lexer(Lexer const&) = delete;

  Lexer(Lexer&&) = delete;

  Lexer& operator=(Lexer const&) = delete;

  Lexer& operator=(Lexer&&) = delete;

  Token Lex();

  SourceLocation getLocation() const { return SourceLocation(fileName_, filePath_, sourceRange_); }

protected:
  Token NumberToken();

private:
  uint32_t NextChar();

  uint32_t CurChar();

  uint32_t PeekChar();

  uint32_t GetChar();
};

} /* namespace u */

#endif //U_LANG_LEXER_HPP
