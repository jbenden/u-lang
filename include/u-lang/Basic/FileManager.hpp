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

#ifndef U_LANG_FILEMANAGER_HPP
#define U_LANG_FILEMANAGER_HPP

#include <string>
#include <vector>

#include <u-lang/Basic/VirtualFileSystem.hpp>
#include <u-lang/u.hpp>

namespace u
{

class UAPI FileManager
{
  std::unique_ptr<vfs::ConcatenatedOverlayFileSystem> VFS;
  std::vector<std::string> SystemModulePaths;
  std::vector<std::string> UserModulePaths;

public:
  FileManager()
  {
    AddDefaultSystemModulePath();
    AddDefaultUserModulePath();
    Initialize();
  }

  void AddUserModulePath(std::string Path)
  {
    UserModulePaths.emplace_back(std::move(Path));
  }

  void AddSystemModulePath(std::string Path)
  {
    SystemModulePaths.emplace_back(std::move(Path));
  }

  void SetSystemModulePath(std::string Path)
  {
    SystemModulePaths.clear();

    SystemModulePaths.emplace_back(std::move(Path));

    Initialize();
  }

  /// \brief Get the status of the entry at \p Path, if one exists.
  llvm::ErrorOr<vfs::Status> status(const llvm::Twine& Path)
  {
    return VFS->status(Path);
  }

  /// \brief Get a \p File object for the file at \p Path, if one exists.
  llvm::ErrorOr<std::unique_ptr<vfs::File>> openFileForRead(const llvm::Twine& Path)
  {
    return VFS->openFileForRead(Path);
  }

  /// This is a convenience method that opens a file, gets its content and then
  /// closes the file.
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> getBufferForFile(const llvm::Twine& Name,
                                                                      uint64_t FileSize,
                                                                      bool RequiresNullTerminator = true,
                                                                      bool IsVolatile = false)
  {
    return VFS->getBufferForFile(Name, FileSize, RequiresNullTerminator, IsVolatile);
  }

  /// \brief Get a directory_iterator for \p Dir.
  /// \note The 'end' iterator is directory_iterator().
  vfs::directory_iterator dir_begin(const llvm::Twine& Dir, std::error_code& EC)
  {
    return VFS->dir_begin(Dir, EC);
  }

  /// Set the working directory. This will affect all following operations on
  /// this file system and may propagate down for nested file systems.
  std::error_code setCurrentWorkingDirectory(const llvm::Twine& Path)
  {
    return VFS->setCurrentWorkingDirectory(Path);
  }

  /// Get the working directory of this file system.
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const
  {
    return VFS->getCurrentWorkingDirectory();
  }

  /// Check whether a file exists. Provided for convenience.
  bool exists(const llvm::Twine& Path)
  {
    return VFS->exists(Path);
  }

private:
  void AddDefaultSystemModulePath()
  {
    SystemModulePaths.emplace_back(ULANG_SYS_MODULE_PATH);
  }

  void AddDefaultUserModulePath()
  {
    llvm::SmallVector<char, 0> UserModules;
    if (llvm::sys::fs::real_path("~/.u-lang/modules", UserModules, true))
    {
      std::string path;
      std::copy(UserModules.begin(), UserModules.end(), std::back_inserter(path));

      UserModulePaths.emplace_back(path);
    }
  }

  void Initialize()
  {
    IntrusiveRefCntPtr<vfs::InMemoryFileSystem> inMemoryFileSystem = new vfs::InMemoryFileSystem();

    VFS = std::make_unique<vfs::ConcatenatedOverlayFileSystem>(inMemoryFileSystem);

    for (auto& Path : SystemModulePaths)
    {
      IntrusiveRefCntPtr<vfs::FileSystem> realFileSystem = new vfs::RealFileSystem(Path);

      VFS->pushOverlay(realFileSystem);
    }

    for (auto& Path : UserModulePaths)
    {
      IntrusiveRefCntPtr<vfs::FileSystem> realFileSystem = new vfs::RealFileSystem(Path);

      VFS->pushOverlay(realFileSystem);
    }
  }
};

} /* namespace u */

#endif //U_LANG_FILEMANAGER_HPP
