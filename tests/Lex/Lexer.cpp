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
#include <u-lang/Lex/Lexer.hpp>
#include <u-lang/u.hpp>

using namespace u;

TEST(Lexer, SanityCheck) // NOLINT
{
  StringSource source{" fn"};
  Lexer lexer(source);

  Token subject = lexer.Lex();
  EXPECT_EQ(tok::kw_fn, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(2, subject.getLocation().getRange().getBegin().getColumn());

  Token finalSubject = lexer.Lex();
  EXPECT_EQ(tok::eof, finalSubject.getKind());
  EXPECT_EQ(1, finalSubject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(4, finalSubject.getLocation().getRange().getBegin().getColumn());
}

TEST(Lexer, NewLineIncrementsLineNumberAndResetsColumn) // NOLINT
{
  StringSource source{"let a = 1.42\nlet b = 3.1415"};
  Lexer lexer(source);

  Token subject1 = lexer.Lex();
  EXPECT_EQ(tok::kw_let, subject1.getKind());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject1.getLocation().getRange().getEnd().getColumn());

  for (unsigned i = 0; i < 3; ++i)
    lexer.Lex();

  Token subject2 = lexer.Lex();
  EXPECT_EQ(tok::eol, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(13, subject2.getLocation().getRange().getBegin().getColumn());

  Token subject3 = lexer.Lex();
  EXPECT_EQ(tok::kw_let, subject3.getKind());
  EXPECT_EQ(2, subject3.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject3.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(2, subject3.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject3.getLocation().getRange().getEnd().getColumn());
}

TEST(Lexer, HandlesSimpleIntegerConstant) // NOLINT
{
  StringSource source{"42"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject.getKind());
}

TEST(Lexer, HandlesSimpleIntegerConstantWithThousandsSeparator) // NOLINT
{
  StringSource source{"42_000"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject.getKind());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, HandlesSimpleFloatingPointConstantInScientificNotation) // NOLINT
{
  StringSource source{"3.14e+00"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::real_constant, subject.getKind());
}

TEST(Lexer, HandlesSimpleBase16IntegerConstant) // NOLINT
{
  StringSource source{"0x20"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject.getKind());
}

TEST(Lexer, HandlesSimpleBase2IntegerConstant) // NOLINT
{
  StringSource source{"0b1111"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject.getKind());
}

TEST(Lexer, HandlesArrayRangeIntegerConstant) // NOLINT
{
  StringSource source{"1..2"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject.getKind());
}

TEST(Lexer, HandlesSimpleFloatingPointConstant) // NOLINT
{
  StringSource source{"3.1415"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::real_constant, subject.getKind());
}

TEST(Lexer, HandlesSimpleFloatingPointConstantWithThousandsSeparator) // NOLINT
{
  StringSource source{"31_415.62"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::real_constant, subject.getKind());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, IntegerDoesNotIncludeLeadingMinus) // NOLINT
{
  StringSource source{"-42"};
  Lexer lexer(source);

  Token subject1 = lexer.Lex();

  EXPECT_EQ(tok::minus, subject1.getKind());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getColumn());

  Token subject2 = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(2, subject2.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject2.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject2.getLocation().getRange().getEnd().getColumn());
}

TEST(Lexer, FloatingPointDoesNotIncludeLeadingMinus) // NOLINT
{
  StringSource source{"-3.1415"};
  Lexer lexer(source);

  Token subject1 = lexer.Lex();

  EXPECT_EQ(tok::minus, subject1.getKind());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getColumn());

  Token subject2 = lexer.Lex();

  EXPECT_EQ(tok::real_constant, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(2, subject2.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject2.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(7, subject2.getLocation().getRange().getEnd().getColumn());
}

TEST(Lexer, HandlesIntegerMinusInteger) // NOLINT
{
  StringSource source{"42-144"};
  Lexer lexer(source);

  Token subject1 = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject1.getKind());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(2, subject1.getLocation().getRange().getEnd().getColumn());

  Token subject2 = lexer.Lex();

  EXPECT_EQ(tok::minus, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(3, subject2.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject2.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject2.getLocation().getRange().getEnd().getColumn());

  Token subject3 = lexer.Lex();

  EXPECT_EQ(tok::integer_constant, subject3.getKind());
  EXPECT_EQ(1, subject3.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(4, subject3.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject3.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(6, subject3.getLocation().getRange().getEnd().getColumn());
}

TEST(Lexer, HandlesFloatingPointMinusFloatingPoint) // NOLINT
{
  StringSource source{"1.42-3.1415"};
  Lexer lexer(source);

  Token subject1 = lexer.Lex();

  EXPECT_EQ(tok::real_constant, subject1.getKind());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject1.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject1.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(4, subject1.getLocation().getRange().getEnd().getColumn());

  Token subject2 = lexer.Lex();

  EXPECT_EQ(tok::minus, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(5, subject2.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject2.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(5, subject2.getLocation().getRange().getEnd().getColumn());

  Token subject3 = lexer.Lex();

  EXPECT_EQ(tok::real_constant, subject3.getKind());
  EXPECT_EQ(1, subject3.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(6, subject3.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject3.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(11, subject3.getLocation().getRange().getEnd().getColumn());
}

TEST(Lexer, HandlesTwoCharacterPunctuator) // NOLINT
{
  StringSource source{"&&"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::ampamp, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(2, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, HandlesSimpleFnDecl) // NOLINT
{
  StringSource source{"fn joe"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::kw_fn, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(2, subject.getLocation().getRange().getEnd().getColumn());

  Token subject2 = lexer.Lex();

  EXPECT_EQ(tok::identifier, subject2.getKind());
  EXPECT_EQ(1, subject2.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(4, subject2.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject2.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(6, subject2.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesARune) // NOLINT
{
  StringSource source{"'a'"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::rune_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesStringLiteral) // NOLINT
{
  StringSource source{"'fn'"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::string_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(4, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesDoubleQuoteRune) // NOLINT
{
  StringSource source{"'\"'"}; // NOLINT
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::rune_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesSingleQuoteRune) // NOLINT
{
  StringSource source{"\"'\""}; // NOLINT
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::rune_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesStringLiteralEndingInQuote) // NOLINT
{
  StringSource source{"''''a''''"}; // NOLINT
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::string_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(9, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesEscapesInStringLiteral) // NOLINT
{
  StringSource source{"'\\\\a\\tb\\rc\\0\\n'"}; // NOLINT
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::string_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(2, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesHexStringLiteral) // NOLINT
{
  StringSource source{"'\\x20'"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::rune_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesUnicodeStringLiteral) // NOLINT
{
  StringSource source{"'\\U0020'"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::rune_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(3, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesAnyTypeLiteral) // NOLINT
{
  StringSource source{"'a\n"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::identifier, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(2, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eol, lexer.Lex().getKind());
}

TEST(Lexer, ParsesDoubleQuoteLargeStringLiteral) // NOLINT
{
  StringSource source{"\"\"\"abcdefghijklmnopqrstuvwxyz\"\"\""}; // NOLINT
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::string_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(32, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesSingleQuoteLargerStringLiteral) // NOLINT
{
  StringSource source{"'abcdefghijklmnopqrstuvwxyz'"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::string_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(28, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesMultilineStringLiteral) // NOLINT
{
  StringSource source{"'''Hello\n"
                      "World'''"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::string_constant, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(2, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(8, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesComment) // NOLINT
{
  StringSource source{"// comment is here\n"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::line_comment, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(18, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST(Lexer, ParsesCommentWithLeadingSlash) // NOLINT
{
  StringSource source{"/// comment is here\n"};
  Lexer lexer(source);

  Token subject = lexer.Lex();

  EXPECT_EQ(tok::line_comment, subject.getKind());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, subject.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, subject.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(19, subject.getLocation().getRange().getEnd().getColumn());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

class LexerTest : public ::testing::Test
{
protected:
  LexerTest& SetFixture(const char* str)
  {
    source = std::make_shared<StringSource>(str);
    sourceManager = std::make_shared<SourceManager>();
    diagClient = std::make_shared<DiagnosticConsumer>();
    diagEngine = std::make_shared<DiagnosticEngine>(sourceManager, diagClient);
    lexer = std::make_shared<Lexer>(sourceManager, diagEngine, *source);

    return *this;
  }

  std::shared_ptr<StringSource> source;
  std::shared_ptr<SourceManager> sourceManager;
  std::shared_ptr<DiagnosticConsumer> diagClient;
  std::shared_ptr<DiagnosticEngine> diagEngine;
  std::shared_ptr<Lexer> lexer;
};

TEST_F(LexerTest, ExpectUnterminatedString) // NOLINT
{
  SetFixture("'");

  Token subject = lexer->Lex();

  EXPECT_EQ(1, lexer->getDiags()->getClient()->getNumErrors());
  EXPECT_EQ(1, diagClient->getNumErrors());

  EXPECT_EQ(tok::eof, lexer->Lex().getKind());
}

TEST_F(LexerTest, ExpectBadHexDigit) // NOLINT
{
  SetFixture("'\\xg0'");

  Token subject = lexer->Lex();

  EXPECT_EQ(1, lexer->getDiags()->getClient()->getNumWarnings());
  EXPECT_EQ(1, diagClient->getNumWarnings());

  EXPECT_EQ(tok::eof, lexer->Lex().getKind());
}

TEST(Lexer, WarningsAreIgnored) // NOLINT
{
  StringSource source{"'\\xg0'"};

  std::shared_ptr<SourceManager> sourceManager = std::make_shared<SourceManager>();
  std::shared_ptr<DiagnosticConsumer> diagClient = std::make_shared<IgnoringDiagConsumer>();
  std::shared_ptr<DiagnosticEngine> diagEngine = std::make_shared<DiagnosticEngine>(sourceManager, diagClient);
  Lexer lexer(diagEngine, source);

  Token subject = lexer.Lex();

  EXPECT_NE(1u, lexer.getDiags()->getClient()->getNumWarnings());
  EXPECT_NE(1u, diagClient->getNumWarnings());

  EXPECT_EQ(tok::eof, lexer.Lex().getKind());
}

TEST_F(LexerTest, SourceManagerContainsOurSource) // NOLINT
{
  SetFixture("'\\xg0'");

  (void) lexer->Lex();

  EXPECT_EQ(tok::eof, lexer->Lex().getKind());

  auto& FI = sourceManager->getOrInsertFile("top-level.u", ".");
  EXPECT_GE(FI.getLine(1).size(), 0u);

  EXPECT_STREQ("'\\xg0'", FI.getLine(1).c_str());
}
