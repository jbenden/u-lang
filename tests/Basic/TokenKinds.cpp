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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <u-lang/Basic/TokenKinds.hpp>
#include <u-lang/u.hpp>

using namespace u;

TEST(TokenKinds, EnumValueExists) // NOLINT
{
  EXPECT_GE(1, tok::eof);
}

TEST(TokenKinds, TokenNameExists) // NOLINT
{
  EXPECT_STREQ("eof", tok::getTokenName(tok::eof));
}

TEST(TokenKinds, PunctuationSpellingExists) // NOLINT
{
  EXPECT_STREQ("&", tok::getPunctuatorSpelling(tok::amp));
}

TEST(TokenKinds, IncorrectTokenReturnsNullPunctuationSpelling) // NOLINT
{
  EXPECT_TRUE(tok::getPunctuatorSpelling(tok::eof) == nullptr);
}

TEST(TokenKinds, KeywordSpellingExists) // NOLINT
{
  EXPECT_STREQ("fn", tok::getKeywordSpelling(tok::kw_fn));
}

TEST(TokenKinds, IncorrectTokenReturnsNullKeywordSpelling) // NOLINT
{
  EXPECT_TRUE(tok::getKeywordSpelling(tok::eof) == nullptr);
}