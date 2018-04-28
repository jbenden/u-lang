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

#ifndef U_LANG_SOURCEMANAGER_HPP
#define U_LANG_SOURCEMANAGER_HPP

#include <map>
#include <string>
#include <vector>

#include <u-lang/Basic/FileManager.hpp>
#include <u-lang/u.hpp>

#include <utf8.h>

namespace u
{

class UAPI FileInfo
{
  typedef std::vector<uint32_t> LineT;
  typedef std::vector<LineT> LinesT;

  std::string FileName;
  std::string FilePath;
  LinesT Lines;

public:
  FileInfo(std::string const& FN, std::string const& FP) // NOLINT
    : FileName{FN}
    , FilePath{FP} {}

  std::string getFileName() const { return FileName; }

  std::string getFilePath() const { return FilePath; }

  std::string getLine(unsigned Num);

private:
  friend class Lexer;

  void AddCharacter(uint64_t LineNum, uint32_t Char);
};

class UAPI SourceManager
{
  typedef std::pair<std::string, std::string> FilePathT;
  typedef std::map<FilePathT, FileInfo> FilePathTableT;

  typedef FilePathTableT::iterator iterator;
  typedef FilePathTableT::const_iterator const_iterator;

  /// \brief Dictionary containing a mapping between a pair of file and path to all source code lines for
  /// the specified pair.
  FilePathTableT FileTable;

  std::unique_ptr<FileManager> FM;

public:
  SourceManager()
  {
    FM = std::make_unique<FileManager>();
  }

  /// \brief Retrieve or create the FileInfo for the specified filename and path.
  FileInfo& getOrInsertFileInfo(std::string file, std::string path);

  /// \brief Returns an iterator pointing at the beginning of the FileTable data.
  iterator begin() { return FileTable.begin(); }

  /// \brief Returns an iterator pointing past the end of the FileTable data.
  iterator end() { return FileTable.end(); }

  /// \brief Returns a constant iterator pointing at the beginning of the FileTable data.
  const_iterator begin() const { return FileTable.cbegin(); }

  /// \brief Returns a constant iterator pointing past the end of the FileTable data.
  const_iterator end() const { return FileTable.cend(); }
};

} /* namespace u */

#endif //U_LANG_SOURCEMANAGER_HPP
