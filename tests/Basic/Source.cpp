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

#include <u-lang/Basic/Source.hpp>
#include <u-lang/u.hpp>

using namespace u;

TEST(FileSource, CanPassAbsolutePathSanityCheck) // NOLINT
{
  FileSource source{ULANG_TEST_FIXTURE_PATH "/Basic/FileSource-CanPassSanityCheck.u"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(102, source.Get());
  EXPECT_FALSE(source.hasBOM());
  EXPECT_STREQ("FileSource-CanPassSanityCheck.u", source.getLocation().getFileName().c_str());
  EXPECT_STREQ(ULANG_TEST_FIXTURE_PATH
                 "/Basic", source.getLocation().getFilePath().c_str());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, source.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getEnd().getColumn());
}

TEST(FileSource, CanPassRelativePathSanityCheck) // NOLINT
{
  FileSource source{"Basic/FileSource-CanPassSanityCheck.u"};

  EXPECT_TRUE(!source);
}

TEST(FileSource, WillSkipBOM) // NOLINT
{
  FileSource source{ULANG_TEST_FIXTURE_PATH "/Basic/FileSource-CanPassSanityCheck-BOM.u"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(102, source.Get());
  EXPECT_TRUE(source.hasBOM());
}

TEST(FileSource, WillSkipFormFeed) // NOLINT
{
  FileSource source{ULANG_TEST_FIXTURE_PATH "/Basic/FileSource-WillSkipFormFeed.u"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(104, source.Get());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getColumn());

  // skip ahead
  source.Get();
  source.Get();
  source.Get();
  source.Get();

  // must now be at the newline
  EXPECT_EQ(10, source.Get());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(6, source.getLocation().getRange().getBegin().getColumn());

  // test the next line, first column
  EXPECT_EQ(119, source.Get());
  EXPECT_EQ(2, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getColumn());
}

TEST(StringSource, CanPassSanityCheck) // NOLINT
{
  StringSource source{"hello world"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(104, source.Get());
  EXPECT_FALSE(source.hasBOM());
  EXPECT_STREQ("top-level.u", source.getLocation().getFileName().c_str());
  EXPECT_STREQ(".", source.getLocation().getFilePath().c_str());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getColumn());
  EXPECT_EQ(1, source.getLocation().getRange().getEnd().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getEnd().getColumn());
}

TEST(StringSource, WillSkipBOM) // NOLINT
{
  StringSource source{"\xef\xbb\xbfhello world"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(104, source.Get());
  EXPECT_TRUE(source.hasBOM());
}

TEST(StringSource, WillSkipFormFeed) // NOLINT
{
  StringSource source{"\xef\xbb\xbfhello\r\nworld"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(104, source.Get());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getColumn());

  // skip ahead
  source.Get();
  source.Get();
  source.Get();
  source.Get();

  // must now be at the newline
  EXPECT_EQ(10, source.Get());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(6, source.getLocation().getRange().getBegin().getColumn());

  // test the next line, first column
  EXPECT_EQ(119, source.Get());
  EXPECT_EQ(2, source.getLocation().getRange().getBegin().getLineNumber());
  EXPECT_EQ(1, source.getLocation().getRange().getBegin().getColumn());
}