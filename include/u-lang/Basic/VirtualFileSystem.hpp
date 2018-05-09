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
 * This file heavily borrows code from Clang. Thanks!
 *
 * \author Joseph W. Benden
 * \copyright (C) 2018 Joseph Benden
 * \license apache2
 */

#ifndef U_LANG_VIRTUALFILESYSTEM_HPP
#define U_LANG_VIRTUALFILESYSTEM_HPP

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SourceMgr.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/DiagnosticIDs.hpp>
#include <u-lang/Basic/SourceLocation.hpp>
#include <u-lang/u.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <memory>
#include <stack>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using llvm::IntrusiveRefCntPtr;
using llvm::SmallVector;
using llvm::SmallVectorBase;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::Twine;

#include <chrono>

namespace llvm
{

#if LLVM_VERSION_MAJOR < 4

namespace sys {

/// A time point on the system clock. This is provided for two reasons:
/// - to insulate us agains subtle differences in behavoir to differences in
///   system clock precision (which is implementation-defined and differs between
///   platforms).
/// - to shorten the type name
/// The default precision is nanoseconds. If need a specific precision specify
/// it explicitly. If unsure, use the default. If you need a time point on a
/// clock other than the system_clock, use std::chrono directly.
template <typename D = std::chrono::nanoseconds>
using TimePoint = std::chrono::time_point<std::chrono::system_clock, D>;

 /// Convert a TimePoint to std::time_t
LLVM_ATTRIBUTE_ALWAYS_INLINE inline TimePoint<> toTimePoint(llvm::sys::TimeValue const& TV) {
  using namespace std::chrono;
  return TimePoint<nanoseconds>(nanoseconds((TV.seconds() * 1000000000UL) + TV.nanoseconds()));
}

/// Convert a TimePoint to std::time_t
LLVM_ATTRIBUTE_ALWAYS_INLINE inline std::time_t toTimeT(TimePoint<> TP) {
 using namespace std::chrono;
 return system_clock::to_time_t(
     time_point_cast<system_clock::time_point::duration>(TP));
}

/// Convert a std::time_t to a TimePoint
LLVM_ATTRIBUTE_ALWAYS_INLINE inline TimePoint<std::chrono::seconds>
toTimePoint(std::time_t T) {
 using namespace std::chrono;
 return time_point_cast<seconds>(system_clock::from_time_t(T));
}

} // namespace sys
#endif

#if LLVM_VERSION_MAJOR < 5
namespace sys
{

namespace fs
{

std::error_code set_current_path(const Twine &path);

} // namespace fs

} // namespace sys
#endif

class MemoryBuffer;

} // end namespace llvm

namespace u
{

namespace vfs
{

/// \brief The result of a \p status operation.
class Status
{
  std::string Name;
  llvm::sys::fs::UniqueID UID;
  llvm::sys::TimePoint<> MTime;
  uint32_t User;
  uint32_t Group;
  uint64_t Size;
  llvm::sys::fs::file_type Type;
  llvm::sys::fs::perms Perms;
  std::string MountPoint;
  std::string VirtualName;

public:
  Status() // NOLINT
    : Type(llvm::sys::fs::file_type::status_error)
  {
  }

  Status(StringRef Name,
         llvm::sys::fs::UniqueID UID,
         llvm::sys::TimePoint<> MTime,
         uint32_t User,
         uint32_t Group,
         uint64_t Size,
         llvm::sys::fs::file_type Type,
         llvm::sys::fs::perms Perms,
         StringRef MP);

  static Status copyWithNewName(const llvm::sys::fs::file_status& In, StringRef NewName, StringRef NewMountPoint = "");

  /// \brief Returns the name that should be used for this file or directory.
  StringRef getName() const { return VirtualName; }

  /// \brief Returns the full, physical, on-disk name for this file or directory, if present.
  StringRef getActualName() const { return Name; }

  /// @name Status interface from llvm::sys::fs
  /// @{
  llvm::sys::fs::file_type getType() const { return Type; }

  llvm::sys::fs::perms getPermissions() const { return Perms; }

  llvm::sys::TimePoint<> getLastModificationTime() const { return MTime; }

  llvm::sys::fs::UniqueID getUniqueID() const { return UID; }

  uint32_t getUser() const { return User; }

  uint32_t getGroup() const { return Group; }

  uint64_t getSize() const { return Size; }

  /// @}
  /// @name Status queries
  /// These are static queries in llvm::sys::fs.
  /// @{
  bool equivalent(const Status& Other) const;

  bool isDirectory() const;

  bool isRegularFile() const;

  bool isOther() const;

  bool isSymlink() const;

  bool isStatusKnown() const;

  bool exists() const;
  /// @}
};

/// \brief Represents an open file.
class File
{
public:
  /// \brief Destroy the file after closing it (if open).
  /// Sub-classes should generally call close() inside their destructors.  We
  /// cannot do that from the base class, since close is virtual.
  virtual ~File();

  /// \brief Get the status of the file.
  virtual llvm::ErrorOr<Status> status() = 0;

  /// \brief Get the name of the file
  virtual llvm::ErrorOr<std::string> getName()
  {
    if (auto Status = status())
      return Status->getName().str();
    else
      return Status.getError(); // LCOV_EXCL_LINE
  }

  /// \brief Get the contents of the file as a \p MemoryBuffer.
  virtual llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> getBuffer(const Twine& Name, // NOLINT
                                                                       uint64_t FileSize = ~1ULL,
                                                                       bool RequiresNullTerminator = true,
                                                                       bool IsVolatile = false) = 0;

  /// \brief Closes the file.
  virtual std::error_code close() = 0;
};

namespace detail
{

/// \brief An interface for virtual file systems to provide an iterator over the
/// (non-recursive) contents of a directory.
struct DirIterImpl
{
  virtual ~DirIterImpl();

  /// \brief Sets \c CurrentEntry to the next entry in the directory on success,
  /// or returns a system-defined \c error_code.
  virtual std::error_code increment() = 0;

  Status CurrentEntry;
};

} // end namespace detail

/// \brief An input iterator over the entries in a virtual path, similar to
/// llvm::sys::fs::directory_iterator.
class directory_iterator
{
  std::shared_ptr<detail::DirIterImpl> Impl; // Input iterator semantics on copy

public:
  explicit directory_iterator(std::shared_ptr<detail::DirIterImpl> I)
    : Impl(std::move(I))
  {
    assert(Impl && "requires non-null implementation");
    if (!Impl->CurrentEntry.isStatusKnown())
      Impl.reset(); // Normalize the end iterator to Impl == nullptr.
  }

  /// \brief Construct an 'end' iterator.
  directory_iterator() = default;

  /// \brief Equivalent to operator++, with an error code.
  directory_iterator& increment(std::error_code& EC)
  {
    assert(Impl && "attempting to increment past end");
    EC = Impl->increment();
    if (!Impl->CurrentEntry.isStatusKnown())
      Impl.reset(); // Normalize the end iterator to Impl == nullptr.
    return *this;
  }

  const Status& operator*() const { return Impl->CurrentEntry; }

  const Status* operator->() const { return &Impl->CurrentEntry; }

  bool operator==(const directory_iterator& RHS) const
  {
    if (Impl && RHS.Impl)
      return Impl->CurrentEntry.equivalent(RHS.Impl->CurrentEntry); // LCOV_EXCL_LINE
    return !Impl && !RHS.Impl;
  }

  bool operator!=(const directory_iterator& RHS) const { return !(*this == RHS); }
};

class FileSystem;

/// \brief An input iterator over the recursive contents of a virtual path,
/// similar to llvm::sys::fs::recursive_directory_iterator.
class recursive_directory_iterator // NOLINT
{
  typedef std::stack<directory_iterator, std::vector<directory_iterator>> IterState;

  FileSystem* FS;
  std::shared_ptr<IterState> State; // Input iterator semantics on copy.

public:
  recursive_directory_iterator(FileSystem& FS, const Twine& Path, std::error_code& EC);

  /// \brief Construct an 'end' iterator.
  recursive_directory_iterator() = default;

  /// \brief Equivalent to operator++, with an error code.
  recursive_directory_iterator& increment(std::error_code& EC);

  const Status& operator*() const { return *State->top(); }

  const Status* operator->() const { return &*State->top(); }

  bool operator==(const recursive_directory_iterator& Other) const
  {
    return State == Other.State; // identity
  }

  bool operator!=(const recursive_directory_iterator& RHS) const { return !(*this == RHS); }

  /// \brief Gets the current level. Starting path is at level 0.
  unsigned long level() const
  {
    assert(!State->empty() && "Cannot get level without any iteration state");
    return State->size() - 1;
  }
};

/// \brief The virtual file system interface.
class FileSystem : public llvm::ThreadSafeRefCountedBase<FileSystem>
{
public:
  virtual ~FileSystem();

  /// \brief Get the status of the entry at \p Path, if one exists.
  virtual llvm::ErrorOr<Status> status(const Twine& Path) = 0;

  /// \brief Get a \p File object for the file at \p Path, if one exists.
  virtual llvm::ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine& Path) = 0;

  /// This is a convenience method that opens a file, gets its content and then
  /// closes the file.
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> getBufferForFile(const Twine& Name,
                                                                      uint64_t FileSize,
                                                                      bool RequiresNullTerminator = true,
                                                                      bool IsVolatile = false);

  /// \brief Get a directory_iterator for \p Dir.
  /// \note The 'end' iterator is directory_iterator().
  virtual directory_iterator dir_begin(const Twine& Dir, std::error_code& EC) = 0;

  /// Set the working directory. This will affect all following operations on
  /// this file system and may propagate down for nested file systems.
  virtual std::error_code setCurrentWorkingDirectory(const Twine& Path) = 0;

  /// Get the working directory of this file system.
  virtual llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const = 0;

  /// Check whether a file exists. Provided for convenience.
  bool exists(const Twine& Path);

  /// Make \a Path an absolute path.
  ///
  /// Makes \a Path absolute using the current directory if it is not already.
  /// An empty \a Path will result in the current directory.
  ///
  /// /absolute/path   => /absolute/path
  /// relative/../path => <current-directory>/relative/../path
  ///
  /// \param Path A path that is modified to be an absolute path.
  /// \returns success if \a path has been made absolute, otherwise a
  ///          platform-specific error_code.
  std::error_code makeAbsolute(SmallVectorImpl<char>& Path) const;
};

class RealFileSystem : public FileSystem
{
  std::string MountPoint;

public:
  explicit RealFileSystem(Twine const& RootedAt)
    : MountPoint{RootedAt.str()}
  {
  } // NOLINT

  llvm::ErrorOr<Status> status(const Twine& Path) override;

  llvm::ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine& Path) override;

  directory_iterator dir_begin(const Twine& Dir, std::error_code& EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override;

  std::error_code setCurrentWorkingDirectory(const Twine& Path) override;
};

/// \brief A file system that allows overlaying one \p AbstractFileSystem on top
/// of another.
///
/// Consists of a stack of >=1 \p FileSystem objects, which are treated as being
/// one merged file system. When there is a directory that exists in more than
/// one file system, the \p OverlayFileSystem contains a directory containing
/// the union of their contents.  The attributes (permissions, etc.) of the
/// top-most (most recently added) directory are used.  When there is a file
/// that exists in more than one file system, the file in the top-most file
/// system overrides the other(s).
class OverlayFileSystem : public FileSystem
{
  typedef SmallVector<IntrusiveRefCntPtr<FileSystem>, 1> FileSystemList;
  /// \brief The stack of file systems, implemented as a list in order of
  /// their addition.
  FileSystemList FSList;

public:
  explicit OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> Base);

  /// \brief Pushes a file system on top of the stack.
  void pushOverlay(IntrusiveRefCntPtr<FileSystem> FS);

  llvm::ErrorOr<Status> status(const Twine& Path) override;

  llvm::ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine& Path) override;

  directory_iterator dir_begin(const Twine& Dir, std::error_code& EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override;

  std::error_code setCurrentWorkingDirectory(const Twine& Path) override;

  typedef FileSystemList::reverse_iterator iterator;

  /// \brief Get an iterator pointing to the most recently added file system.
  iterator overlays_begin() { return FSList.rbegin(); }

  /// \brief Get an iterator pointing one-past the least recently added file
  /// system.
  iterator overlays_end() { return FSList.rend(); }
};

/// \brief A file system that allows overlaying one \p AbstractFileSystem on top
/// of another.
///
/// Consists of a stack of >=1 \p FileSystem objects, which are treated as being
/// one merged file system. When there is a directory that exists in more than
/// one file system, the \p OverlayFileSystem contains a directory containing
/// the union of their contents.  The attributes (permissions, etc.) of the
/// top-most (most recently added) directory are used.  When there is a file
/// that exists in more than one file system, the content is a concatenation
/// of all files, from the bottom-to-top-most file; but with each
/// file having a newline appended to prevent parsing problems.
class ConcatenatedOverlayFileSystem : public FileSystem
{
  typedef SmallVector<IntrusiveRefCntPtr<FileSystem>, 1> FileSystemList;
  /// \brief The stack of file systems, implemented as a list in order of
  /// their addition.
  FileSystemList FSList;

  std::vector<void*> Cleanup;

public:
  explicit ConcatenatedOverlayFileSystem(IntrusiveRefCntPtr<FileSystem> Base);

  ~ConcatenatedOverlayFileSystem();

  /// \brief Pushes a file system on top of the stack.
  void pushOverlay(IntrusiveRefCntPtr<FileSystem> FS);

  llvm::ErrorOr<Status> status(const Twine& Path) override;

  llvm::ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine& Path) override;

  directory_iterator dir_begin(const Twine& Dir, std::error_code& EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override;

  std::error_code setCurrentWorkingDirectory(const Twine& Path) override;

  typedef FileSystemList::reverse_iterator iterator;

  /// \brief Get an iterator pointing to the most recently added file system.
  iterator overlays_begin() { return FSList.rbegin(); }

  /// \brief Get an iterator pointing one-past the least recently added file
  /// system.
  iterator overlays_end() { return FSList.rend(); }

  typedef FileSystemList::iterator reverse_iterator;

  /// \brief Get a reverse iterator pointing to the most recently added file system.
  reverse_iterator overlays_rbegin() { return FSList.begin(); }

  /// \brief Get a reverse iterator pointing one-past the least recently added file
  /// system.
  reverse_iterator overlays_rend() { return FSList.end(); }
};

namespace detail
{

class InMemoryDirectory;

} // end namespace detail

/// An in-memory file system.
class InMemoryFileSystem : public FileSystem
{
  std::unique_ptr<detail::InMemoryDirectory> Root;
  std::string WorkingDirectory;
  bool UseNormalizedPaths = true;

public:
  explicit InMemoryFileSystem(bool UseNormalizedPaths = true);

  ~InMemoryFileSystem() override;

  /// Add a buffer to the VFS with a path. The VFS owns the buffer.
  /// \return true if the file was successfully added, false if the file already
  /// exists in the file system with different contents.
  bool addFile(const Twine& Path, time_t ModificationTime, std::unique_ptr<llvm::MemoryBuffer> Buffer);

  /// Add a buffer to the VFS with a path. The VFS does not own the buffer.
  /// \return true if the file was successfully added, false if the file already
  /// exists in the file system with different contents.
  bool addFileNoOwn(const Twine& Path, time_t ModificationTime, llvm::MemoryBuffer* Buffer);

  /// Return true if this file system normalizes . and .. in paths.
  bool useNormalizedPaths() const { return UseNormalizedPaths; }

  llvm::ErrorOr<Status> status(const Twine& Path) override;

  llvm::ErrorOr<std::unique_ptr<File>> openFileForRead(const Twine& Path) override;

  directory_iterator dir_begin(const Twine& Dir, std::error_code& EC) override;

  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override { return WorkingDirectory; } // LCOV_EXCL_LINE

  std::error_code setCurrentWorkingDirectory(const Twine& Path) override; // LCOV_EXCL_LINE
};

/// \brief Get a globally unique ID for a virtual file or directory.
llvm::sys::fs::UniqueID
getNextVirtualUniqueID();

} /* namespace vfs */

} /* namespace u */

#endif // U_LANG_VIRTUALFILESYSTEM_HPP
