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

#include <u-lang/Lex/Lexer.hpp>
#include <u-lang/u.hpp>

using namespace u;

TEST(Lexer, SanityCheck) // NOLINT
{
  StringSource source{" fn"};
  Lexer lexer{source};

  Token subject = lexer.Lex();
  EXPECT_EQ(tok::unknown, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(2, subject.getLocation().getRange().getBegin().getColumn());

  Token nextSubject = lexer.Lex();
  EXPECT_EQ(tok::unknown, nextSubject.getKind());
  EXPECT_EQ(1, nextSubject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(3, nextSubject.getLocation().getRange().getBegin().getColumn());

  Token finalSubject = lexer.Lex();
  EXPECT_EQ(tok::eof, finalSubject.getKind());
  EXPECT_EQ(1, finalSubject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(4, finalSubject.getLocation().getRange().getBegin().getColumn());
}

TEST(Lexer, NewLineIncrementsLineNumberAndResetsColumn) // NOLINT
{
  StringSource source{"let a = 1.42\nlet b = 3.1415"};
  Lexer lexer{source};

  Token subject1 = lexer.Lex();
  EXPECT_EQ(tok::unknown, subject1.getKind());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getColumn());

  for (unsigned i = 0; i < 8; ++i)
    lexer.Lex();

  Token subject2 = lexer.Lex();
  EXPECT_EQ(tok::eol, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(13, subject2.getLocation().getRange().getBegin().getColumn());

  Token subject3 = lexer.Lex();
  EXPECT_EQ(tok::unknown, subject3.getKind());
  EXPECT_EQ(2, subject3.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject3.getLocation().getRange().getBegin().getColumn());
}