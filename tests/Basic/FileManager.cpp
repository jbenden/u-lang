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
#include <u-lang/u.hpp>

using namespace u;
using namespace u::vfs;

class FileManagerTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    fileManager = std::make_shared<FileManager>();

    std::vector<std::string> FixturePaths;

    FixturePaths.emplace_back(ULANG_TEST_FIXTURE_PATH "/VFS");
    FixturePaths.emplace_back(ULANG_TEST_FIXTURE_PATH "/VFS-overlay");

    fileManager->SetSystemModulePaths(FixturePaths);
  }

  std::shared_ptr<FileManager> fileManager;
};

TEST_F(FileManagerTest, BasicStatTestsFunction) // NOLINT
{
  // Begin testing.
  EXPECT_FALSE(fileManager->exists("/a.txt"));

  EXPECT_TRUE(fileManager->exists("/b/1/test.txt"));

  auto S = fileManager->status("/b/1");
  EXPECT_TRUE(!!S);
  EXPECT_TRUE(S->isDirectory());
  EXPECT_STREQ(S->getName().str().c_str(), "/b/1");

  EXPECT_TRUE(fileManager->exists("/c/empty.txt"));
}

TEST_F(FileManagerTest, ConcatenatedOverlayFSWillJoinFileContentWithNewlines) // NOLINT
{
  auto Content = fileManager->getBufferForFile("/b/1/test.txt", 32u);
  if (!Content)
  {
    std::cerr << Content.getError().message() << std::endl;
  }
  EXPECT_TRUE(!!Content);
  EXPECT_STREQ(Content.get()->getBufferStart(), "hello world!\nfrom earth!\n");
}

TEST_F(FileManagerTest, RootOfFSIsIterable) // NOLINT
{
  unsigned count = 0;
  std::error_code EC;
  for (auto ri = fileManager->dir_begin("/", EC); ri != directory_iterator();
       ri.increment(EC))
  {
    EXPECT_TRUE(ri->getName().startswith("/"));
    EXPECT_EQ(ri->getName().size(), 2u);

    ++count;
  }

  EXPECT_EQ(count, 3);
}
