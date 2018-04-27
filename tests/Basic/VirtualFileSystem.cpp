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

#include <u-lang/Basic/VirtualFileSystem.hpp>
#include <u-lang/u.hpp>

using namespace u;
using namespace u::vfs;

TEST(VirtualFileSystem, InMemoryFileSystemSanityTest) // NOLINT
{
  InMemoryFileSystem inMemoryFileSystem;

  time_t ModTime = time(nullptr);

  auto Buf = llvm::MemoryBuffer::getNewMemBuffer(64, "my_buffer");
  strcpy(const_cast<char*>(Buf->getBufferStart()), "Hello");

  inMemoryFileSystem.addFileNoOwn("/a/b/c.txt", ModTime, Buf.get());

  EXPECT_TRUE(inMemoryFileSystem.exists("/a/b/c.txt"));
  EXPECT_FALSE(inMemoryFileSystem.exists("/a.txt"));

  auto F = inMemoryFileSystem.openFileForRead("/a/b/c.txt");

  EXPECT_STREQ((*F)->getName().get().c_str(), "/a/b/c.txt");

  auto S = (*F)->status();

  EXPECT_FALSE((*S).isDirectory());
  EXPECT_TRUE((*S).isRegularFile());
  EXPECT_EQ((*S).getType(), llvm::sys::fs::file_type::regular_file);
  EXPECT_EQ(S->getPermissions(), llvm::sys::fs::perms::all_all);
  EXPECT_EQ(S->getLastModificationTime().time_since_epoch().count(), ModTime * 1000000000ULL);
  EXPECT_GT(S->getUniqueID().getFile(), 0u);
  EXPECT_EQ(S->getUser(), 0u);
  EXPECT_EQ(S->getGroup(), 0u);
  EXPECT_EQ(S->getSize(), 64u);

  unsigned count{0};
  std::error_code EC;
  for (auto it = inMemoryFileSystem.dir_begin("/", EC); it != directory_iterator(); it.increment(EC))
  {
    EXPECT_EQ(it->getName().size(), 2);
    EXPECT_TRUE(it->getName().startswith("/"));

    ++count;
  }

  EXPECT_EQ(count, 1);

  // not_a_directory
  inMemoryFileSystem.dir_begin("/a/b/c.txt", EC);
  EXPECT_TRUE(!!EC);
  EXPECT_EQ(EC.value(), llvm::errc::not_a_directory);

  // non-existent location
  inMemoryFileSystem.dir_begin("/does/not/exist", EC);
  EXPECT_TRUE(!!EC);

  // Check the content of the file
  auto Content = inMemoryFileSystem.getBufferForFile("/a/b/c.txt", 32u);
  if (!Content)
  {
    std::cerr << Content.getError().message() << std::endl;
  }
  EXPECT_TRUE(!!Content);
  EXPECT_STREQ(Content.get()->getBufferStart(), "Hello");
}

TEST(VirtualFileSystem, RealFileSystemSanityTest) // NOLINT
{
  RealFileSystem realFileSystem(ULANG_TEST_FIXTURE_PATH "/VFS");

  unsigned count{0};
  std::error_code EC;
  for (auto it = realFileSystem.dir_begin("/", EC); it != directory_iterator(); it.increment(EC))
  {
    EXPECT_EQ(it->getName().size(), 2);
    EXPECT_TRUE(it->getName().startswith("/"));

    ++count;
  }

  EXPECT_EQ(count, 3);

  EXPECT_TRUE(realFileSystem.exists("/b/1/test.txt"));

  auto S = realFileSystem.status("/b/1");
  EXPECT_TRUE(!!S);
  EXPECT_TRUE(S->isDirectory());
  EXPECT_STREQ(S->getName().str().c_str(), "/b/1");

  // Check RealFile
  auto F = realFileSystem.openFileForRead("/b/1/test.txt");

  EXPECT_STREQ((*F)->getName().get().c_str(), "/b/1/test.txt");
  auto FS = (*F)->status();
  EXPECT_TRUE(!!FS);
  EXPECT_TRUE(FS->isRegularFile());
  EXPECT_FALSE(FS->isSymlink());
  EXPECT_FALSE(FS->isOther());

  EXPECT_FALSE(FS->equivalent(*S));

  // Unit-test root recursion
  count = 0;
  for (auto ri = recursive_directory_iterator(realFileSystem, "/", EC); ri != recursive_directory_iterator();
       ri.increment(EC))
  {
    EXPECT_TRUE(ri->getName().startswith("/"));
    EXPECT_LE(ri->getName().size(), 13u);

    ++count;
  }

  EXPECT_EQ(count, 7);

  // Unit-test subdirectory recursion
  count = 0;
  for (auto ri = recursive_directory_iterator(realFileSystem, "/b", EC); ri != recursive_directory_iterator();
       ri.increment(EC))
  {
    EXPECT_TRUE(ri->getName().startswith("/"));
    EXPECT_LE(ri->getName().size(), 13u);

    ++count;
  }

  EXPECT_EQ(count, 4);
}

TEST(VirtualFileSystem, InMemoryAndSingleRealOverlaySanityTest) // NOLINT
{
  IntrusiveRefCntPtr<InMemoryFileSystem> inMemoryFileSystem = new InMemoryFileSystem();

  time_t ModTime = time(nullptr);

  auto Buf = llvm::MemoryBuffer::getNewMemBuffer(64, "my_buffer");
  strcpy(const_cast<char*>(Buf->getBufferStart()), "Hello");

  inMemoryFileSystem->addFileNoOwn("/a/b/c.txt", ModTime, Buf.get());

  IntrusiveRefCntPtr<FileSystem> realFileSystem = new RealFileSystem(ULANG_TEST_FIXTURE_PATH "/VFS");

  OverlayFileSystem overlayFileSystem{realFileSystem};
  overlayFileSystem.pushOverlay(inMemoryFileSystem);

  overlayFileSystem.setCurrentWorkingDirectory("/");

  // Begin testing.
  EXPECT_TRUE(overlayFileSystem.exists("/a/b/c.txt"));
  EXPECT_FALSE(overlayFileSystem.exists("/a.txt"));

  EXPECT_TRUE(overlayFileSystem.exists("/b/1/test.txt"));

  auto S = overlayFileSystem.status("/b/1");
  EXPECT_TRUE(!!S);
  EXPECT_TRUE(S->isDirectory());
  EXPECT_STREQ(S->getName().str().c_str(), "/b/1");
}

TEST(VirtualFileSystem, InMemoryAndTwoRealOverlaySanityTest) // NOLINT
{
  IntrusiveRefCntPtr<InMemoryFileSystem> inMemoryFileSystem = new InMemoryFileSystem();

  time_t ModTime = time(nullptr);

  auto Buf = llvm::MemoryBuffer::getNewMemBuffer(64, "my_buffer");
  strcpy(const_cast<char*>(Buf->getBufferStart()), "Hello");

  inMemoryFileSystem->addFileNoOwn("/a/b/c.txt", ModTime, Buf.get());

  IntrusiveRefCntPtr<FileSystem> realFileSystem = new RealFileSystem(ULANG_TEST_FIXTURE_PATH "/VFS");

  IntrusiveRefCntPtr<FileSystem> realFileSystemOverlay = new RealFileSystem(ULANG_TEST_FIXTURE_PATH "/VFS-overlay");

  OverlayFileSystem overlayFileSystem{realFileSystem};
  overlayFileSystem.pushOverlay(inMemoryFileSystem);
  overlayFileSystem.pushOverlay(realFileSystemOverlay);

  // Begin testing.
  EXPECT_TRUE(overlayFileSystem.exists("/a/b/c.txt"));
  EXPECT_FALSE(overlayFileSystem.exists("/a.txt"));

  EXPECT_TRUE(overlayFileSystem.exists("/b/1/test.txt"));

  auto S = overlayFileSystem.status("/b/1");
  EXPECT_TRUE(!!S);
  EXPECT_TRUE(S->isDirectory());
  EXPECT_STREQ(S->getName().str().c_str(), "/b/1");

  EXPECT_TRUE(overlayFileSystem.exists("/c/empty.txt"));

  // Overlay FS will take the top-most entry for file content.
  auto Content = overlayFileSystem.getBufferForFile("/b/1/test.txt", 32u);
  if (!Content)
  {
    std::cerr << Content.getError().message() << std::endl;
  }
  EXPECT_TRUE(!!Content);
  EXPECT_STREQ(Content.get()->getBufferStart(), "from earth!");

  // Unit-test root recursion
  unsigned count = 0;
  std::error_code EC;
  for (auto ri = recursive_directory_iterator(overlayFileSystem, "/", EC); ri != recursive_directory_iterator();
       ri.increment(EC))
  {
    EXPECT_TRUE(ri->getName().startswith("/"));
    EXPECT_LE(ri->getName().size(), 13u);

    ++count;
  }

  EXPECT_EQ(count, 10);
}