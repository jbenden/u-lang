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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/Errc.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/VirtualFileSystem.hpp>
#include <u-lang/u.hpp>

using namespace u;
using namespace u::vfs;
using namespace llvm;
using llvm::sys::fs::UniqueID;
using llvm::sys::fs::file_status;
using llvm::sys::fs::file_type;
using llvm::sys::fs::perms;

#if LLVM_VERSION_MAJOR < 5
namespace llvm
{

namespace sys
{

namespace fs
{

std::error_code set_current_path(const Twine &path);

#ifndef WIN32
std::error_code set_current_path(const Twine &path) {
  SmallString<128> path_storage;
  StringRef p = path.toNullTerminatedStringRef(path_storage);

  if (::chdir(p.begin()) == -1)
    return std::error_code{errno, std::generic_category()}; // LCOV_EXCL_LINE

  return std::error_code{};
}
#else
std::error_code set_current_path(const Twine &path) {
  // Convert to utf-16.
  SmallVector<wchar_t, 128> wide_path;
  if (std::error_code ec = widenPath(path, wide_path))
    return ec;

  if (!::SetCurrentDirectoryW(wide_path.begin()))
    return mapWindowsError(::GetLastError());

  return std::error_code();
}
#endif

} // namespace fs

} // namespace sys

} // namespace llvm
#endif

Status::Status(StringRef Name,
               UniqueID UID,
               sys::TimePoint<> MTime,
               uint32_t User,
               uint32_t Group,
               uint64_t Size,
               file_type Type,
               perms Perms,
               StringRef MountPoint)
  : Name(Name)
  , UID(UID)
  , MTime(MTime)
  , User(User)
  , Group(Group)
  , Size(Size)
  , Type(Type)
  , Perms(Perms)
  , MountPoint(MountPoint)
{
  std::string theNameOwner{Name.str()};
  StringRef theName{theNameOwner};

  // Ensure MountPoint does not end with a slash!
  if (MountPoint.size() > 1 && MountPoint.endswith("/"))
  {
    MountPoint = MountPoint.substr(1, MountPoint.size() - 1); // LCOV_EXCL_LINE
  }

  // Strip MountPoint from theName, if it is present.
  if (theName.startswith(MountPoint))
  {
    theName = theName.substr(MountPoint.size());
  }

  // Ensure the virtual name starts with a slash.
  if (!theName.startswith("/"))
  {
    VirtualName = "/";
  }

  // build the virtual name.
  VirtualName +=
    theName.substr(MountPoint.size() > 1 && MountPoint.size() + 1 < theName.size() ? MountPoint.size() + 1 : 0);
}

Status
Status::copyWithNewName(const file_status& In, StringRef NewName, StringRef NewMountPoint)
{
#if LLVM_VERSION_MAJOR < 4
  auto MT = toTimePoint(In.getLastModificationTime());
#else
  auto MT = In.getLastModificationTime();
#endif

  return Status(NewName,
                In.getUniqueID(),
                MT,
                In.getUser(),
                In.getGroup(),
                In.getSize(),
                In.type(),
                In.permissions(),
                NewMountPoint);
}

bool
Status::equivalent(const Status& Other) const
{
  return getUniqueID() == Other.getUniqueID();
}

bool
Status::isDirectory() const
{
  return Type == file_type::directory_file;
}

bool
Status::isRegularFile() const
{
  return Type == file_type::regular_file;
}

bool
Status::isOther() const
{
  return exists() && !isRegularFile() && !isDirectory() && !isSymlink();
}

bool
Status::isSymlink() const
{
  return Type == file_type::symlink_file;
}

bool
Status::isStatusKnown() const
{
  return Type != file_type::status_error;
}

bool
Status::exists() const
{
  return isStatusKnown() && Type != file_type::file_not_found;
}

File::~File() {}

FileSystem::~FileSystem() {}

ErrorOr<std::unique_ptr<MemoryBuffer>>
FileSystem::getBufferForFile(const llvm::Twine& Name, uint64_t FileSize, bool RequiresNullTerminator, bool IsVolatile)
{
  auto F = openFileForRead(Name);
  if (!F)
    return F.getError(); // LCOV_EXCL_LINE

  return (*F)->getBuffer(Name, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code
FileSystem::makeAbsolute(SmallVectorImpl<char>& Path) const
{
  if (llvm::sys::path::is_absolute(Path))
    return std::error_code{};

  auto WorkingDir = getCurrentWorkingDirectory(); // LCOV_EXCL_LINE
  if (!WorkingDir) // LCOV_EXCL_LINE
    return WorkingDir.getError(); // LCOV_EXCL_LINE

  return llvm::sys::fs::make_absolute(WorkingDir.get(), Path); // LCOV_EXCL_LINE
}

bool
FileSystem::exists(const Twine& Path)
{
  auto Status = status(Path);
  return Status && Status->exists();
}

//===-----------------------------------------------------------------------===/
// RealFileSystem implementation
//===-----------------------------------------------------------------------===/

namespace
{
std::string
PhysicalToVirtualPath(StringRef const& MountPoint, StringRef const& Path)
{
  SmallVector<char, 0> Out;

  if (!Path.startswith(MountPoint))
  {
    Twine diskPath{MountPoint};

    diskPath.toVector(Out);
  }

  llvm::sys::path::append(Out, Path);

  std::string final;
  std::copy(Out.begin(), Out.end(), std::back_inserter(final));

  return final;
}

/// \brief Wrapper around a raw file descriptor.
class RealFile : public File
{
  int FD;
  Status S;
  std::string RealName;
  std::string MountPoint;

  friend class RealFileSystem;

public:
  RealFile(int FD, StringRef NewName, StringRef NewRealPathName, StringRef NewMountPoint)
    : FD(FD)
    , S(NewName, {}, {}, {}, {}, {}, llvm::sys::fs::file_type::status_error, {}, "")
    , RealName(NewRealPathName.str())
    , MountPoint(NewMountPoint.str())
  {
    assert(FD >= 0 && "Invalid or inactive file descriptor");
  }

  ~RealFile() override;

  ErrorOr<Status> status() override;

  ErrorOr<std::string> getName() override;

  ErrorOr<std::unique_ptr<MemoryBuffer>> getBuffer(const Twine& Name,
                                                   uint64_t FileSize,
                                                   bool RequiresNullTerminator,
                                                   bool IsVolatile) override;

  std::error_code close() override;
};
} // end anonymous namespace
RealFile::~RealFile()
{
  close();
}

ErrorOr<Status>
RealFile::status()
{
  assert(FD != -1 && "cannot stat closed file");
  if (!S.isStatusKnown())
  {
    file_status RealStatus;
    if (std::error_code EC = sys::fs::status(FD, RealStatus))
      return EC; // LCOV_EXCL_LINE
    S = Status::copyWithNewName(RealStatus, S.getName(), MountPoint);
  }
  return S;
}

ErrorOr<std::string>
RealFile::getName()
{
  return S.getName().str();
}

ErrorOr<std::unique_ptr<MemoryBuffer>>
RealFile::getBuffer(const Twine& Name, uint64_t FileSize, bool RequiresNullTerminator, bool IsVolatile)
{
  assert(FD != -1 && "cannot get buffer for closed file");

  auto final = PhysicalToVirtualPath(MountPoint, Name.str());

  return MemoryBuffer::getOpenFile(FD, final, FileSize, RequiresNullTerminator, IsVolatile);
}

std::error_code
RealFile::close()
{
  std::error_code EC = sys::Process::SafelyCloseFileDescriptor(FD);
  FD = -1;
  return EC;
}

ErrorOr<Status>
RealFileSystem::status(const Twine& Path)
{
  auto final = PhysicalToVirtualPath(MountPoint, Path.str());

  sys::fs::file_status RealStatus;
  if (std::error_code EC = sys::fs::status(final, RealStatus))
    return EC;
  return Status::copyWithNewName(RealStatus, Path.str(), MountPoint);
}

ErrorOr<std::unique_ptr<File>>
RealFileSystem::openFileForRead(const Twine& Name)
{
  int FD;
  SmallString<256> RealName;

  auto final = PhysicalToVirtualPath(MountPoint, Name.str());

  if (std::error_code EC = sys::fs::openFileForRead(final, FD, &RealName))
    return EC; // LCOV_EXCL_LINE

  return std::unique_ptr<File>(new RealFile(FD, Name.str(), RealName.str(), MountPoint));
}

llvm::ErrorOr<std::string>
RealFileSystem::getCurrentWorkingDirectory() const
{
  SmallString<256> Dir;
  if (std::error_code EC = llvm::sys::fs::current_path(Dir))
    return EC; // LCOV_EXCL_LINE
  return Dir.str().str();
}

std::error_code
RealFileSystem::setCurrentWorkingDirectory(const Twine& Path)
{
  // FIXME: chdir is thread hostile; on the other hand, creating the same
  // behavior as chdir is complex: chdir resolves the path once, thus
  // guaranteeing that all subsequent relative path operations work
  // on the same path the original chdir resulted in. This makes a
  // difference for example on network filesystems, where symlinks might be
  // switched during runtime of the tool. Fixing this depends on having a
  // file system abstraction that allows openat() style interactions.

  // FIXME(jbenden): Handle the fact that we are rooted at MountPoint.
  return llvm::sys::fs::set_current_path(Path);
}

namespace
{
class RealFSDirIter : public u::vfs::detail::DirIterImpl
{
  Twine MountPoint;
  llvm::sys::fs::directory_iterator Iter;

public:
  RealFSDirIter(const Twine& MP, const Twine& Path, std::error_code& EC)
    : MountPoint{MP}
    , Iter(Path, EC)
  {
    if (!EC && Iter != llvm::sys::fs::directory_iterator())
    {
      llvm::sys::fs::file_status S;
      EC = Iter->status(S);
      CurrentEntry = Status::copyWithNewName(S, Iter->path(), MountPoint.str());
    }
  }

  std::error_code increment() override
  {
    std::error_code EC;
    Iter.increment(EC);
    if (EC)
    {
      return EC; // LCOV_EXCL_LINE
    }
    else if (Iter == llvm::sys::fs::directory_iterator())
    {
      CurrentEntry = Status();
    }
    else
    {
      llvm::sys::fs::file_status S;
      EC = Iter->status(S);
      CurrentEntry = Status::copyWithNewName(S, Iter->path(), MountPoint.str());
    }
    return EC;
  }
};
}

directory_iterator
RealFileSystem::dir_begin(const Twine& Dir, std::error_code& EC)
{
  auto final = PhysicalToVirtualPath(MountPoint, Dir.str());

  return directory_iterator(std::make_shared<RealFSDirIter>(MountPoint, final, EC));
}

//===-----------------------------------------------------------------------===/
// OverlayFileSystem implementation
//===-----------------------------------------------------------------------===/
OverlayFileSystem::OverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS)
{
  FSList.push_back(std::move(BaseFS));
}

void
OverlayFileSystem::pushOverlay(IntrusiveRefCntPtr<FileSystem> FS)
{
  FSList.push_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status>
OverlayFileSystem::status(const Twine& Path)
{
  // FIXME: handle symlinks that cross file systems
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
  {
    ErrorOr<Status> Status = (*I)->status(Path);
    if (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      return Status;
  }
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<std::unique_ptr<File>>
OverlayFileSystem::openFileForRead(const llvm::Twine& Path)
{
  // FIXME: handle symlinks that cross file systems
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
  {
    auto Result = (*I)->openFileForRead(Path);
    if (Result || Result.getError() != llvm::errc::no_such_file_or_directory)
      return Result;
  } // LCOV_EXCL_LINE
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::string>
OverlayFileSystem::getCurrentWorkingDirectory() const
{
  // All file systems are synchronized, just take the first working directory.
  return FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
OverlayFileSystem::setCurrentWorkingDirectory(const Twine& Path)
{
  for (auto& FS : FSList)
    if (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      return EC; // LCOV_EXCL_LINE
  return std::error_code{}; // LCOV_EXCL_LINE
}

u::vfs::detail::DirIterImpl::~DirIterImpl() {}

namespace
{
class OverlayFSDirIterImpl : public u::vfs::detail::DirIterImpl
{
  OverlayFileSystem& Overlays;
  std::string Path;
  OverlayFileSystem::iterator CurrentFS;
  directory_iterator CurrentDirIter;
  llvm::StringSet<> SeenNames;

  std::error_code incrementFS()
  {
    assert(CurrentFS != Overlays.overlays_end() && "incrementing past end");
    ++CurrentFS;
    for (auto E = Overlays.overlays_end(); CurrentFS != E; ++CurrentFS)
    {
      std::error_code EC;
      CurrentDirIter = (*CurrentFS)->dir_begin(Path, EC);
      if (EC && EC != errc::no_such_file_or_directory)
        return EC; // LCOV_EXCL_LINE
      if (CurrentDirIter != directory_iterator())
        break; // found
    }
    return std::error_code{};
  }

  std::error_code incrementDirIter(bool IsFirstTime)
  {
    assert((IsFirstTime || CurrentDirIter != directory_iterator()) && "incrementing past end");
    std::error_code EC;
    if (!IsFirstTime)
      CurrentDirIter.increment(EC);
    if (!EC && CurrentDirIter == directory_iterator())
      EC = incrementFS();
    return EC;
  }

  std::error_code incrementImpl(bool IsFirstTime)
  {
    while (true)
    {
      std::error_code EC = incrementDirIter(IsFirstTime);
      if (EC || CurrentDirIter == directory_iterator())
      {
        CurrentEntry = Status();
        return EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.getName());
      if (SeenNames.insert(Name).second)
        return EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

public:
  OverlayFSDirIterImpl(const Twine& Path, OverlayFileSystem& FS, std::error_code& EC)
    : Overlays(FS)
    , Path(Path.str())
    , CurrentFS(Overlays.overlays_begin())
  {
    CurrentDirIter = (*CurrentFS)->dir_begin(Path, EC);
    EC = incrementImpl(true);
  }

  std::error_code increment() override { return incrementImpl(false); }
};
} // end anonymous namespace

directory_iterator
OverlayFileSystem::dir_begin(const Twine& Dir, std::error_code& EC)
{
  return directory_iterator(std::make_shared<OverlayFSDirIterImpl>(Dir, *this, EC));
}

namespace u
{
namespace vfs
{
namespace detail
{

enum InMemoryNodeKind
{
  IME_File,
  IME_Directory
};

/// The in memory file system is a tree of Nodes. Every node can either be a
/// file or a directory.
class InMemoryNode
{
  Status Stat;
  InMemoryNodeKind Kind;

public:
  InMemoryNode(Status Stat, InMemoryNodeKind Kind)
    : Stat(std::move(Stat))
    , Kind(Kind)
  {
  }

  virtual ~InMemoryNode() {}

  const Status& getStatus() const { return Stat; }

  InMemoryNodeKind getKind() const { return Kind; }
};

namespace
{
class InMemoryFile : public InMemoryNode
{
  std::unique_ptr<llvm::MemoryBuffer> Buffer;

public:
  InMemoryFile(Status Stat, std::unique_ptr<llvm::MemoryBuffer> Buffer)
    : InMemoryNode(std::move(Stat), IME_File)
    , Buffer(std::move(Buffer))
  {
  }

  llvm::MemoryBuffer* getBuffer() { return Buffer.get(); }

  static bool classof(const InMemoryNode* N) { return N->getKind() == IME_File; }
};

/// Adapt a InMemoryFile for VFS' File interface.
class InMemoryFileAdaptor : public File
{
  InMemoryFile& Node;

public:
  explicit InMemoryFileAdaptor(InMemoryFile& Node)
    : Node(Node)
  {
  }

  llvm::ErrorOr<Status> status() override { return Node.getStatus(); }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> getBuffer(const Twine& Name,
                                                               uint64_t FileSize,
                                                               bool RequiresNullTerminator,
                                                               bool IsVolatile) override
  {
    llvm::MemoryBuffer* Buf = Node.getBuffer();
    return llvm::MemoryBuffer::getMemBuffer(Buf->getBuffer(), Buf->getBufferIdentifier(), RequiresNullTerminator);
  }

  std::error_code close() override { return std::error_code{}; } // LCOV_EXCL_LINE
};
} // end anonymous namespace

class InMemoryDirectory : public InMemoryNode
{
  std::map<std::string, std::unique_ptr<InMemoryNode>> Entries;

public:
  explicit InMemoryDirectory(Status Stat)
    : InMemoryNode(std::move(Stat), IME_Directory)
  {
  }

  InMemoryNode* getChild(StringRef Name)
  {
    auto I = Entries.find(Name);
    if (I != Entries.end())
      return I->second.get();
    return nullptr;
  }

  InMemoryNode* addChild(StringRef Name, std::unique_ptr<InMemoryNode> Child)
  {
    return Entries.insert(make_pair(Name, std::move(Child))).first->second.get();
  }

  typedef decltype(Entries)::const_iterator const_iterator;

  const_iterator begin() const { return Entries.begin(); }

  const_iterator end() const { return Entries.end(); }

  static bool classof(const InMemoryNode* N) { return N->getKind() == IME_Directory; }
};
} /* namespace detail */

InMemoryFileSystem::InMemoryFileSystem(bool UseNormalizedPaths)
  : Root(new detail::InMemoryDirectory(Status("",
                                              getNextVirtualUniqueID(),
                                              llvm::sys::TimePoint<>(),
                                              0,
                                              0,
                                              0,
                                              llvm::sys::fs::file_type::directory_file,
                                              llvm::sys::fs::perms::all_all,
                                              "")))
  , UseNormalizedPaths(UseNormalizedPaths)
{
}

InMemoryFileSystem::~InMemoryFileSystem() {}

bool
InMemoryFileSystem::addFile(const Twine& P, time_t ModificationTime, std::unique_ptr<llvm::MemoryBuffer> Buffer)
{
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  assert(!EC);
  (void) EC;

  if (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  if (Path.empty())
    return false; // LCOV_EXCL_LINE

  detail::InMemoryDirectory* Dir = Root.get();
  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  while (true)
  {
    StringRef Name = *I;
    detail::InMemoryNode* Node = Dir->getChild(Name);
    ++I;
    if (!Node)
    {
      if (I == E)
      {
        // End of the path, create a new file.
        // FIXME: expose the status details in the interface.
        Status Stat(P.str(),
                    getNextVirtualUniqueID(),
                    llvm::sys::toTimePoint(ModificationTime), // LCOV_EXCL_LINE
                    0,
                    0,
                    Buffer->getBufferSize(),
                    llvm::sys::fs::file_type::regular_file,
                    llvm::sys::fs::all_all,
                    "");
        Dir->addChild(Name, llvm::make_unique<detail::InMemoryFile>(std::move(Stat), std::move(Buffer)));
        return true;
      }

      // Create a new directory. Use the path up to here.
      // FIXME: expose the status details in the interface.
      Status Stat(StringRef(Path.str().begin(), Name.end() - Path.str().begin()),
                  getNextVirtualUniqueID(),
                  llvm::sys::toTimePoint(ModificationTime), // LCOV_EXCL_LINE
                  0,
                  0,
                  Buffer->getBufferSize(),
                  llvm::sys::fs::file_type::directory_file,
                  llvm::sys::fs::all_all,
                  "");
      Dir = cast<detail::InMemoryDirectory>(
        Dir->addChild(Name, llvm::make_unique<detail::InMemoryDirectory>(std::move(Stat))));
      continue;
    }

    if (auto* NewDir = dyn_cast<detail::InMemoryDirectory>(Node)) // LCOV_EXCL_LINE
    {
      Dir = NewDir; // LCOV_EXCL_LINE
    }
    else
    {
      assert(isa<detail::InMemoryFile>(Node) && "Must be either file or directory!"); // LCOV_EXCL_LINE

      // Trying to insert a directory in place of a file.
      if (I != E) // LCOV_EXCL_LINE
        return false; // LCOV_EXCL_LINE

      // Return false only if the new file is different from the existing one.
      return cast<detail::InMemoryFile>(Node)->getBuffer()->getBuffer() == Buffer->getBuffer();
    }
  }
}

bool
InMemoryFileSystem::addFileNoOwn(const Twine& P, time_t ModificationTime, llvm::MemoryBuffer* Buffer)
{
  return addFile(
    P, ModificationTime, llvm::MemoryBuffer::getMemBuffer(Buffer->getBuffer(), Buffer->getBufferIdentifier()));
}

static ErrorOr<detail::InMemoryNode*>
lookupInMemoryNode(const InMemoryFileSystem& FS, detail::InMemoryDirectory* Dir, const Twine& P)
{
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = FS.makeAbsolute(Path);
  assert(!EC);
  (void) EC;

  if (FS.useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  if (Path.empty())
    return Dir; // LCOV_EXCL_LINE

  auto I = llvm::sys::path::begin(Path), E = llvm::sys::path::end(Path);
  while (true)
  {
    detail::InMemoryNode* Node = Dir->getChild(*I);
    ++I;
    if (!Node)
      return errc::no_such_file_or_directory; // LCOV_EXCL_LINE

    // Return the file if it's at the end of the path.
    if (auto File = dyn_cast<detail::InMemoryFile>(Node))
    {
      if (I == E)
        return File;
      return errc::no_such_file_or_directory; // LCOV_EXCL_LINE
    }

    // Traverse directories.
    Dir = cast<detail::InMemoryDirectory>(Node);
    if (I == E)
      return Dir;
  }
}

llvm::ErrorOr<Status>
InMemoryFileSystem::status(const Twine& Path)
{
  auto Node = lookupInMemoryNode(*this, Root.get(), Path);
  if (Node)
    return (*Node)->getStatus();
  return Node.getError();
}

llvm::ErrorOr<std::unique_ptr<File>>
InMemoryFileSystem::openFileForRead(const Twine& Path)
{
  auto Node = lookupInMemoryNode(*this, Root.get(), Path);
  if (!Node)
    return Node.getError(); // LCOV_EXCL_LINE

  // When we have a file provide a heap-allocated wrapper for the memory buffer
  // to match the ownership semantics for File.
  if (auto* F = dyn_cast<detail::InMemoryFile>(*Node))
    return std::unique_ptr<File>(new detail::InMemoryFileAdaptor(*F));

  // FIXME: errc::not_a_file?
  return make_error_code(llvm::errc::invalid_argument); // LCOV_EXCL_LINE
}

namespace
{
/// Adaptor from InMemoryDir::iterator to directory_iterator.
class InMemoryDirIterator : public u::vfs::detail::DirIterImpl
{
  detail::InMemoryDirectory::const_iterator I;
  detail::InMemoryDirectory::const_iterator E;

public:
  InMemoryDirIterator() = default;

  explicit InMemoryDirIterator(detail::InMemoryDirectory& Dir)
    : I(Dir.begin())
    , E(Dir.end())
  {
    if (I != E)
      CurrentEntry = I->second->getStatus();
  }

  std::error_code increment() override
  {
    ++I;
    // When we're at the end, make CurrentEntry invalid and DirIterImpl will do
    // the rest.
    CurrentEntry = I != E ? I->second->getStatus() : Status();
    return std::error_code{};
  }
};
} // end anonymous namespace

directory_iterator
InMemoryFileSystem::dir_begin(const Twine& Dir, std::error_code& EC)
{
  auto Node = lookupInMemoryNode(*this, Root.get(), Dir);
  if (!Node)
  {
    EC = Node.getError();
    return directory_iterator(std::make_shared<InMemoryDirIterator>());
  }

  if (auto* DirNode = dyn_cast<detail::InMemoryDirectory>(*Node))
    return directory_iterator(std::make_shared<InMemoryDirIterator>(*DirNode));

  EC = make_error_code(llvm::errc::not_a_directory);
  return directory_iterator(std::make_shared<InMemoryDirIterator>());
}

std::error_code
InMemoryFileSystem::setCurrentWorkingDirectory(const Twine& P)
{
  SmallString<128> Path;
  P.toVector(Path);

  // Fix up relative paths. This just prepends the current working directory.
  std::error_code EC = makeAbsolute(Path);
  assert(!EC);
  (void) EC;

  if (useNormalizedPaths())
    llvm::sys::path::remove_dots(Path, /*remove_dot_dot=*/true);

  if (!Path.empty())
    WorkingDirectory = Path.str();
  return std::error_code{};
}

//===-----------------------------------------------------------------------===/
// ConcatenatedOverlayFileSystem implementation
//===-----------------------------------------------------------------------===/
ConcatenatedOverlayFileSystem::ConcatenatedOverlayFileSystem(IntrusiveRefCntPtr<FileSystem> BaseFS)
{
  FSList.push_back(std::move(BaseFS));
}

void
ConcatenatedOverlayFileSystem::pushOverlay(IntrusiveRefCntPtr<FileSystem> FS)
{
  FSList.push_back(FS);
  // Synchronize added file systems by duplicating the working directory from
  // the first one in the list.
  FS->setCurrentWorkingDirectory(getCurrentWorkingDirectory().get());
}

ErrorOr<Status>
ConcatenatedOverlayFileSystem::status(const Twine& Path)
{
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
  {
    ErrorOr<Status> Status = (*I)->status(Path);
    if (Status || Status.getError() != llvm::errc::no_such_file_or_directory)
      return Status;
  }
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

ErrorOr<std::unique_ptr<File>>
ConcatenatedOverlayFileSystem::openFileForRead(const llvm::Twine& Path)
{
  // Get file stat for creation of faked Status object
  auto S = status(Path);
  if (!S)
    return make_error_code((errc) S.getError().value()); // LCOV_EXCL_LINE

  // Figure out the total buffer size needed for the file concatenation.
  uint64_t bufferSize{0};
  for (iterator I = overlays_begin(), E = overlays_end(); I != E; ++I)
  {
    auto Result = (*I)->openFileForRead(Path);
    if (!Result && Result.getError() != llvm::errc::no_such_file_or_directory)
    {
      // got big error!
      return Result; // LCOV_EXCL_LINE
    }
    else if (Result)
    {
      // got an entry!
      bufferSize += (*Result)->status()->getSize() + 1;
    }
  }

  // Create a buffer to fill with the concatenation.
  std::unique_ptr<MemoryBuffer> memoryBuffer = MemoryBuffer::getNewMemBuffer(bufferSize, Path.str());

  // Perform concatenation, into our new buffer.
  uint64_t offset{0};
  for (reverse_iterator I = overlays_rbegin(), E = overlays_rend(); I != E; ++I)
  {
    auto Result = (*I)->openFileForRead(Path);
    if (!Result && Result.getError() != llvm::errc::no_such_file_or_directory)
    {
      // got big error!
      return Result; // LCOV_EXCL_LINE
    }
    else if (Result)
    {
      // got an entry!
      uint64_t fileSize = (*Result)->status()->getSize();

      auto FileContent = (*Result)->getBuffer(Path.str(), fileSize);
      memcpy(const_cast<char*>(memoryBuffer->getBufferStart()) + offset, (*FileContent)->getBufferStart(), fileSize);

      memset(const_cast<char*>(memoryBuffer->getBufferStart()) + offset + fileSize, '\n', 1);

      offset += fileSize + 1;
    }
  }

  // Create dummy InMemory wrappers for our new content.
  Status Stat(Path.str(),
              getNextVirtualUniqueID(),
              S->getLastModificationTime(), // LCOV_EXCL_LINE
              0,
              0,
              memoryBuffer->getBufferSize(),
              S->getType(),
              S->getPermissions(),
              "");

  auto* F = new detail::InMemoryFile(std::move(Stat), std::move(memoryBuffer));

  return std::unique_ptr<File>(new detail::InMemoryFileAdaptor(*F));
}

llvm::ErrorOr<std::string>
ConcatenatedOverlayFileSystem::getCurrentWorkingDirectory() const
{
  // All file systems are synchronized, just take the first working directory.
  return FSList.front()->getCurrentWorkingDirectory();
}

std::error_code
ConcatenatedOverlayFileSystem::setCurrentWorkingDirectory(const Twine& Path)
{
  for (auto& FS : FSList)
    if (std::error_code EC = FS->setCurrentWorkingDirectory(Path))
      return EC; // LCOV_EXCL_LINE
  return std::error_code{}; // LCOV_EXCL_LINE
}

namespace
{
class ConcatenatedOverlayFSDirIterImpl : public u::vfs::detail::DirIterImpl
{
  ConcatenatedOverlayFileSystem& Overlays;
  std::string Path;
  ConcatenatedOverlayFileSystem::iterator CurrentFS;
  directory_iterator CurrentDirIter;
  llvm::StringSet<> SeenNames;

  std::error_code incrementFS()
  {
    assert(CurrentFS != Overlays.overlays_end() && "incrementing past end");
    ++CurrentFS;
    for (auto E = Overlays.overlays_end(); CurrentFS != E; ++CurrentFS)
    {
      std::error_code EC;
      CurrentDirIter = (*CurrentFS)->dir_begin(Path, EC);
      if (EC && EC != errc::no_such_file_or_directory)
        return EC; // LCOV_EXCL_LINE
      if (CurrentDirIter != directory_iterator())
        break; // found
    }
    return std::error_code{};
  }

  std::error_code incrementDirIter(bool IsFirstTime)
  {
    assert((IsFirstTime || CurrentDirIter != directory_iterator()) && "incrementing past end");
    std::error_code EC;
    if (!IsFirstTime)
      CurrentDirIter.increment(EC);
    if (!EC && CurrentDirIter == directory_iterator())
      EC = incrementFS();
    return EC;
  }

  std::error_code incrementImpl(bool IsFirstTime)
  {
    while (true)
    {
      std::error_code EC = incrementDirIter(IsFirstTime);
      if (EC || CurrentDirIter == directory_iterator())
      {
        CurrentEntry = Status();
        return EC;
      }
      CurrentEntry = *CurrentDirIter;
      StringRef Name = llvm::sys::path::filename(CurrentEntry.getName());
      if (SeenNames.insert(Name).second)
        return EC; // name not seen before
    }
    llvm_unreachable("returned above");
  }

public:
  ConcatenatedOverlayFSDirIterImpl(const Twine& Path, ConcatenatedOverlayFileSystem& FS, std::error_code& EC)
    : Overlays(FS)
    , Path(Path.str())
    , CurrentFS(Overlays.overlays_begin())
  {
    CurrentDirIter = (*CurrentFS)->dir_begin(Path, EC);
    EC = incrementImpl(true);
  }

  std::error_code increment() override { return incrementImpl(false); }
};
} // end anonymous namespace

directory_iterator
ConcatenatedOverlayFileSystem::dir_begin(const Twine& Dir, std::error_code& EC)
{
  return directory_iterator(std::make_shared<ConcatenatedOverlayFSDirIterImpl>(Dir, *this, EC));
}

} /* namespace vfs */
} /* namespace u */

vfs::recursive_directory_iterator::recursive_directory_iterator(FileSystem& FS_, const Twine& Path, std::error_code& EC)
  : FS(&FS_)
{
  directory_iterator I = FS->dir_begin(Path, EC);
  if (I != directory_iterator())
  {
    State = std::make_shared<IterState>();
    State->push(I);
  }
}

vfs::recursive_directory_iterator&
recursive_directory_iterator::increment(std::error_code& EC)
{
  assert(FS && State && !State->empty() && "incrementing past end");
  assert(State->top()->isStatusKnown() && "non-canonical end iterator");
  vfs::directory_iterator End;
  if (State->top()->isDirectory())
  {
    vfs::directory_iterator I = FS->dir_begin(State->top()->getName(), EC);
    if (I != End)
    {
      State->push(I);
      return *this;
    }
  }

  while (!State->empty() && State->top().increment(EC) == End)
    State->pop();

  if (State->empty())
    State.reset(); // end iterator

  return *this;
}

UniqueID vfs::getNextVirtualUniqueID()
{
  static std::atomic<unsigned> UID;
  unsigned ID = ++UID;
  // The following assumes that uint64_t max will never collide with a real
  // dev_t value from the OS.
  return UniqueID{std::numeric_limits<uint64_t>::max(), ID};
}
