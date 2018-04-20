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
  EXPECT_EQ(0, source.Get());
  EXPECT_FALSE(source.hasBOM());
}

TEST(FileSource, CanPassRelativePathSanityCheck) // NOLINT
{
  FileSource source{"Basic/FileSource-CanPassSanityCheck.u"};

  EXPECT_TRUE(!source);
}

TEST(StringSource, CanPassSanityCheck) // NOLINT
{
  StringSource source{"hello world"};

  EXPECT_TRUE(!!source);
  EXPECT_EQ(0, source.Get());
  EXPECT_FALSE(source.hasBOM());
}