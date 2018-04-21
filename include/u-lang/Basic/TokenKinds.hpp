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

#ifndef U_LANG_TOKENKINDS_HPP
#define U_LANG_TOKENKINDS_HPP

#include <string>

#include <u-lang/u.hpp>

namespace u
{

namespace tok
{

/// \brief Provides a simple uniform namespace for tokens.
enum TokenKind : unsigned short
{
#define TOK(X) X,
#include <u-lang/Basic/TokenKinds.def>

  NUM_TOKENS
};

/// \brief Determines the name of a token as used within the front end.
///
/// The name of a token will be an internal name and should not be used
/// as part of diagnostic messages.
const char*
getTokenName(TokenKind Kind);

/// \brief Determines the spelling of simple punctuation tokens like
/// '!' or '&', and returns NULL for other token kinds.
const char*
getPunctuatorSpelling(TokenKind Kind);

/// \brief Determines the spelling of keyword tokens. Returns NULL for
/// other token kinds.
const char*
getKeywordSpelling(TokenKind Kind);

} /* namespace tok */

} /* namespace u */

#endif // U_LANG_TOKENKINDS_HPP
