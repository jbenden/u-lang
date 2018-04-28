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

#ifndef U_LANG_SOURCELOCATION_HPP
#define U_LANG_SOURCELOCATION_HPP

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <llvm/Support/FileSystem.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <string>

#include <u-lang/u.hpp>

namespace u
{

class UAPI SourcePosition
{
public:
  SourcePosition(uint64_t line, uint64_t column)
    : line_{line}
    , column_{column} {}

  uint64_t getLineNumber() const { return line_; }

  SourcePosition& setLineNumber(uint64_t line)
  {
    line_ = line;
    return *this;
  }

  uint64_t getColumn() const { return column_; }

  SourcePosition& setColumn(uint64_t column)
  {
    column_ = column;
    return *this;
  }

  SourcePosition& incrementColumn()
  {
    ++column_;
    return *this;
  }

  SourcePosition& incrementLineNumber()
  {
    ++line_;
    return *this;
  }

protected:
  uint64_t line_;
  uint64_t column_;
};

class UAPI SourceRange
{
public:
  SourceRange(SourcePosition const& begin, SourcePosition const& end)
    : begin_{begin}
    , end_{end} {}

  SourcePosition const& getBegin() const { return begin_; }

  SourcePosition const& getEnd() const { return end_; }

  SourcePosition& getBegin() { return begin_; }

  SourcePosition& getEnd() { return end_; }

protected:
  SourcePosition begin_;
  SourcePosition end_;
};

class UAPI SourceLocation
{
public:
  SourceLocation()
    : id_{}
    , fileName_{""}
    , filePath_{""}
    , range_(SourceRange(SourcePosition(0, 0), SourcePosition(0, 0))) {}

  SourceLocation(std::string const& fileName, std::string const& filePath, SourceRange const& range) // NOLINT
    : id_{}
    , fileName_{fileName}
    , filePath_{filePath}
    , range_{range} {}

  SourceLocation(llvm::sys::fs::UniqueID id,
                 std::string const& fileName, // NOLINT
                 std::string const& filePath, // NOLINT
                 SourceRange const& range)
    : id_{id}
    , fileName_{fileName}
    , filePath_{filePath}
    , range_{range}
  {
  }

  llvm::sys::fs::UniqueID getFileID() const { return id_; }

  std::string const& getFileName() const { return fileName_; }

  std::string const& getFilePath() const { return filePath_; }

  SourceRange const& getRange() const { return range_; }

  SourceRange& getRange() { return range_; }

protected:
  llvm::sys::fs::UniqueID id_;
  std::string fileName_;
  std::string filePath_;
  SourceRange range_;
};

}; /* namespace u */

#endif // U_LANG_SOURCELOCATION_HPP
