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

#include <u-lang/Basic/SourceManager.hpp>
#include <u-lang/u.hpp>

#include <string>

#include <utf8.h>

using namespace u;

std::string
FileInfo::getLine(unsigned Num)
{
  assert(Num - 1 < Lines.size() && "Requested index is out of bounds!");

  auto Line = Lines[Num - 1];

  std::string Result{};
  utf8::utf32to8(Line.begin(), Line.end(), std::back_inserter(Result));

  return Result;
}

void
FileInfo::AddCharacter(uint64_t LineNum, uint32_t Char)
{
  while (Lines.size() < LineNum)
  {
    // The line number does not exist, so we need to create it first.
    Lines.emplace_back(std::vector<uint32_t>());
  }

  Lines[LineNum - 1].push_back(Char);
}

FileInfo&
SourceManager::getOrInsertFileInfo(llvm::sys::fs::UniqueID id, std::string file, std::string path)
{
  auto Entry = FileTable.find(std::make_tuple(id, file, path));

  if (Entry != FileTable.end())
  {
    return Entry->second;
  }
  else
  {
    auto Result = FileTable.insert(std::make_pair(std::make_tuple(id, file, path), FileInfo{id, file, path}));

    return Result.first->second;
  }
}

std::shared_ptr<Source>
SourceManager::getFile(std::string Path)
{
  auto File = FM->openFileForRead(Path);
  if (!File)
  {
    return nullptr;
  }

  auto FileStatus = (*File)->status();
  auto fileSize = (*FileStatus).getSize();
  auto FileContent = (*File)->getBuffer(Path, fileSize);

  return std::make_shared<MemoryBufferSource>(FileStatus->getUniqueID(),
                                              FileStatus->getActualName().empty() ? FileStatus->getName()
                                                                                  : FileStatus->getActualName(),
                                              std::move(*FileContent));
}