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

#include <llvm/Support/Errc.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <u-lang/Basic/FileManager.hpp>
#include <u-lang/Basic/SourceManager.hpp>
#include <u-lang/u.hpp>

using namespace u;
using namespace u::vfs;

class SourceManagerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    sourceManager = std::make_shared<SourceManager>();

    std::vector<std::string> FixturePaths;

    FixturePaths.emplace_back(ULANG_TEST_FIXTURE_PATH "/VFS");
    FixturePaths.emplace_back(ULANG_TEST_FIXTURE_PATH "/VFS-overlay");

    sourceManager->getFileManager().SetSystemModulePaths(FixturePaths);
  }

  std::shared_ptr<SourceManager> sourceManager;
};

TEST_F(SourceManagerTest, FileStatusBitsAreCorrect) // NOLINT
{
  auto Source = sourceManager->getFile("/b/1/test.txt");
  EXPECT_TRUE(!!Source);
  EXPECT_TRUE(!!Source.operator*());
  EXPECT_NE(Source->getLocation().getFileID(), llvm::sys::fs::UniqueID{});
  EXPECT_EQ(Source->Get(), 104);
  EXPECT_FALSE(Source->hasBOM());
}

TEST_F(SourceManagerTest, BOMIsSkipped) // NOLINT
{
  auto Source = sourceManager->getFile("/b/3/bom.u");
  EXPECT_TRUE(!!Source);
  EXPECT_TRUE(!!Source.operator*());
  EXPECT_NE(Source->getLocation().getFileID(), llvm::sys::fs::UniqueID{});
  EXPECT_EQ(Source->Get(), 102);
  EXPECT_TRUE(Source->hasBOM());
}

TEST_F(SourceManagerTest, ReadsUntilEOF) // NOLINT
{
  auto Source = sourceManager->getFile("/b/1/test.txt");
  EXPECT_TRUE(!!Source);
  EXPECT_TRUE(!!Source.operator*());

  unsigned count{0};
  while (!!Source.operator*())
  {
    (void) Source->Get();

    ++count;
  }

  EXPECT_EQ(count, 25u);
  EXPECT_FALSE(Source->hasBOM());

#ifdef LLVM_ON_UNIX
  EXPECT_EQ(Source->detectedLineEndings(), eol::UnixLineEndings);
#else
  EXPECT_EQ(Source->detectedLineEndings(), eol::WindowsLineEndings);
#endif
}
