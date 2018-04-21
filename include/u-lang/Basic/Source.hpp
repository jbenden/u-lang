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

#ifndef U_LANG_SOURCE_HPP
#define U_LANG_SOURCE_HPP

#include <fstream>
#include <sstream>
#include <string>

#include <u-lang/u.hpp>
#include <u-lang/Basic/SourceLocation.hpp>

namespace u
{

class UAPI Source
{
public:
  Source() = default;

  Source(Source const&) = delete;

  Source(Source&&) = delete;

  Source& operator=(Source const&) = delete;

  Source& operator=(Source&&) = delete;

  virtual explicit operator bool() const = 0;

  virtual uint32_t Get() = 0;

  virtual SourceLocation getLocation() const = 0;

  virtual bool hasBOM() const = 0;
};

class UAPI FileSource : public Source
{
public:
  FileSource() = delete;

  explicit FileSource(std::string const& fileName);

  FileSource(FileSource const&) = delete;

  FileSource(FileSource&&) = delete;

  FileSource& operator=(FileSource const&) = delete;

  FileSource& operator=(FileSource&&) = delete;

  explicit operator bool() const override { return it_ != end_; }

  bool hasBOM() const override { return hasBOM_; }

  uint32_t Get() override;

  SourceLocation getLocation() const override
  {
    return SourceLocation(fileName_,
                          filePath_,
                          SourceRange(position_, position_));
  }

protected:
  std::string fileName_;
  std::string filePath_;
  bool first_;
  bool hasBOM_;
  std::fstream stream_;
  std::istreambuf_iterator<char> it_;
  std::istreambuf_iterator<char> end_;
  SourcePosition position_;
  bool gotNewLine_;
};

class UAPI StringSource : public Source
{
public:
  StringSource() = delete;

  explicit StringSource(std::string const& source)
    : Source()
    , source_{source}
    , hasBOM_{false}
    , stream_{source}
    , it_{stream_.rdbuf()}
    , first_{true}
    , position_{1, 0}
    , gotNewLine_{false}
  {
  }

  StringSource(StringSource const&) = delete;

  StringSource(StringSource&&) = delete;

  StringSource& operator=(StringSource const&) = delete;

  StringSource& operator=(StringSource&&) = delete;

  explicit operator bool() const override { return it_ != end_; }

  bool hasBOM() const override { return hasBOM_; }

  uint32_t Get() override;

  SourceLocation getLocation() const override
  {
    return SourceLocation("top-level.u",
                          ".",
                          SourceRange(position_, position_));
  }

protected:
  std::string source_;
  bool hasBOM_;
  std::stringstream stream_;
  std::istreambuf_iterator<char> it_;
  std::istreambuf_iterator<char> end_;
  bool first_;
  SourcePosition position_;
  bool gotNewLine_;
};

} /* namespace u */

#endif // U_LANG_SOURCE_HPP
