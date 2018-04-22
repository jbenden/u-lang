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

#include <u-lang/Basic/PunctuatorTable.hpp>
#include <u-lang/u.hpp>

using namespace u;

TEST(PunctuatorTable, CanFindSingleCharEntry) // NOLINT
{
  PunctuatorTable table;
  auto subject = table.get("-");

  EXPECT_TRUE(!!subject);
  EXPECT_EQ(tok::minus, subject->getKind());
}

TEST(PunctuatorTable, DoesNotThrowOnNonexistantEntry) // NOLINT
{
  PunctuatorTable table;

  auto subject = table.get("nonexistant");

  EXPECT_FALSE(!!subject);
}