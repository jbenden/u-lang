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

#ifndef U_LANG_TOKEN_HPP
#define U_LANG_TOKEN_HPP

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
#include <u-lang/Basic/TokenKinds.hpp>
#include <u-lang/u.hpp>

#include <cassert>

namespace u
{

class UAPI Token
{
  /// Kind - The actual flavor of token this is.
  tok::TokenKind Kind;
  SourceLocation Loc;

public:
  Token(tok::TokenKind K, SourceLocation L)
    : Kind{K}
    , Loc{std::move(L)} {}

  tok::TokenKind getKind() const { return Kind; }

  void setKind(tok::TokenKind K) { Kind = K; }

  /// is/isNot - Predicates to check if this token is a specific kind, as in
  /// "if (Tok.is(tok::l_brace)) {...}".
  bool is(tok::TokenKind K) const { return Kind == K; }

  bool isNot(tok::TokenKind K) const { return Kind != K; }

  bool isOneOf(tok::TokenKind K1, tok::TokenKind K2) const
  {
    return is(K1) || is(K2);
  }

  template<typename... Ts>
  bool isOneOf(tok::TokenKind K1, tok::TokenKind K2, Ts... Ks) const
  {
    return is(K1) || isOneOf(K2, Ks...);
  }

  /// \brief Return true is this is a non-keyword identifier.
  bool isAnyIdentifier() const
  {
    return is(tok::identifier);
  }

  /// \brief Return true if this is a "literal", like a numeric constant,
  /// string, etc.
  bool isLiteral() const
  {
    return is(tok::numeric_constant);
  }

  SourceLocation getLocation() const
  {
    return Loc;
  }

  void setLocation(SourceLocation L) { Loc = std::move(L); }

  const char* getName() const { return tok::getTokenName(Kind); }
};

} /* namespace u */

#endif //U_LANG_TOKEN_HPP
