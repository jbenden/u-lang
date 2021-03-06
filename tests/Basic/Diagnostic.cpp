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

TEST(DiagnosticEngine, ExpectUnterminatedString) // NOLINT
{
  StringSource source{"'"};

  std::shared_ptr<SourceManager> sourceManager = std::make_shared<SourceManager>();
  std::shared_ptr<DiagnosticConsumer> diagClient = std::make_shared<DiagnosticConsumer>();
  std::shared_ptr<DiagnosticEngine> diagEngine = std::make_shared<DiagnosticEngine>(sourceManager, diagClient);

  diagEngine->Report(diag::unterminated_string);

  EXPECT_EQ(1, diagClient->getNumErrors());
}

class RecordingDiagConsumer : public DiagnosticConsumer
{
public:
  typedef std::pair<diag::Severity, std::string> DiagnosticT;
  typedef std::vector<DiagnosticT> DiagnosticsT;

  DiagnosticsT D;

private:
  void HandleDiagnostic(diag::Severity DiagLevel,
                        const Diagnostic& Info) override
  {
    clear();

    llvm::SmallVector<char, 32> msg;
    Info.FormatDiagnostic(msg);

    std::string s;
    std::copy(msg.begin(), msg.end(), std::back_inserter(s));

    D.push_back(std::make_pair(DiagLevel, s));
  }
};

class DiagnosticEngineTest : public ::testing::Test
{
protected:
  DiagnosticEngineTest& SetFixture(const char* str)
  {
    source = std::make_shared<StringSource>(str);
    sourceManager = std::make_shared<SourceManager>();
    diagClient = std::make_shared<RecordingDiagConsumer>();
    diagEngine = std::make_shared<DiagnosticEngine>(sourceManager, diagClient);

    return *this;
  }

  std::shared_ptr<StringSource> source;
  std::shared_ptr<SourceManager> sourceManager;
  std::shared_ptr<RecordingDiagConsumer> diagClient;
  std::shared_ptr<DiagnosticEngine> diagEngine;
};

TEST_F(DiagnosticEngineTest, ExpectFormattedUnterminatedString) // NOLINT
{
  SetFixture("'");

  diagEngine->Report(diag::unterminated_string);

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Fatal, diagClient->D[0].first);
  EXPECT_STREQ("An unterminated string was encountered while lexing the source code shown above.",
               diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedBadHexSequenceAsChar) // NOLINT
{
  SetFixture("'\\xp0'");

  diagEngine->Report(diag::bad_hex_digit) << "p";

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Warning, diagClient->D[0].first);
  EXPECT_STREQ("An invalid hexadecimal digit 'p' was encountered while lexing the source code shown above.",
               diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedBadHexSequenceAsNullChar) // NOLINT
{
  SetFixture("'\\xp0'");

  diagEngine->Report(diag::bad_hex_digit) << (const char*) nullptr;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Warning, diagClient->D[0].first);
  EXPECT_STREQ("An invalid hexadecimal digit '(null)' was encountered while lexing the source code shown above.",
               diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedBadHexSequenceAsStdString) // NOLINT
{
  SetFixture("'\\xp0'");

  diagEngine->Report(diag::bad_hex_digit) << std::string("p");

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Warning, diagClient->D[0].first);
  EXPECT_STREQ("An invalid hexadecimal digit 'p' was encountered while lexing the source code shown above.",
               diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedSInt) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0001) << (int) 0;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("I have 0 sense.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedSIntOne) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0002) << (int) 1;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("I have 1 sense.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedSIntTwo) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0002) << (int) 2;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("I have 2 senses.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedSIntOrdinal) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0003) << (int) 3;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("The 3rd sense.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedUInt) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0004) << (unsigned) 0;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("I have 0 sense.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedUIntOne) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0005) << (unsigned) 1;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("I have 1 sense.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedUIntTwo) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0005) << (unsigned) 2;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("I have 2 senses.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedUIntOrdinal) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0006) << (unsigned) 3;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("The 3rd sense.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedPunctuatorKeyword) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0007) << tok::minus;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("Token '-'.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedKeyword) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0007) << tok::kw_fn;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("Token fn.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedTokenName) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0007) << tok::identifier;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("Token <identifier>.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedPercentSign) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0008);

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("%.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedVerbatim) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0009) << std::string("Hello world.");

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("Hello world.", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedSIntSelect) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0010) << (int) 1;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("b", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, ExpectFormattedUIntSelect) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0010) << (unsigned) 1;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("b", diagClient->D[0].second.c_str());
}

TEST_F(DiagnosticEngineTest, HandlesFormattingSelectWithEscape) // NOLINT
{
  SetFixture("");

  diagEngine->Report(diag::unit_test_0011) << (unsigned) 1;

  EXPECT_EQ(1, diagClient->D.size());

  EXPECT_EQ(diag::Severity::Ignore, diagClient->D[0].first);
  EXPECT_STREQ("%", diagClient->D[0].second.c_str());
}
