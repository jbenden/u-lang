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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <llvm/Support/MemoryBuffer.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <fstream>
#include <sstream>
#include <string>

#include <u-lang/Basic/SourceLocation.hpp>
#include <u-lang/u.hpp>

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
    return SourceLocation(fileName_, filePath_, SourceRange(position_, position_));
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
    return SourceLocation("top-level.u", ".", SourceRange(position_, position_));
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

class UAPI MemoryBufferInputStream : public std::istream
{
public:
  MemoryBufferInputStream(const uint8_t* Data, size_t Length)
    : std::istream(&buffer_)
    , buffer_{Data, Length}
  {
    rdbuf(&buffer_);
  }

private:
  class MemoryBuffer : public std::basic_streambuf<char>
  {
  public:
    MemoryBuffer(const uint8_t* Data, size_t Length)
    {
      setg(reinterpret_cast<char*>(const_cast<uint8_t*>(Data)),
           reinterpret_cast<char*>(const_cast<uint8_t*>(Data)),
           reinterpret_cast<char*>(const_cast<uint8_t*>(Data)) + Length);
    }
  };

  MemoryBuffer buffer_;
};

class UAPI MemoryBufferSource : public Source
{
public:
  MemoryBufferSource() = delete;

  explicit MemoryBufferSource(std::unique_ptr<llvm::MemoryBuffer> Buf)
    : Source()
    , source_{std::move(Buf)}
    , hasBOM_{false}
    , stream_{reinterpret_cast<const uint8_t*>(source_->getBufferStart()), source_->getBufferSize()}
    , it_{stream_.rdbuf()}
    , first_{true}
    , position_{1, 0}
    , gotNewLine_{false}
  {
  }

  MemoryBufferSource(MemoryBufferSource const&) = delete;

  MemoryBufferSource(MemoryBufferSource&&) = delete;

  MemoryBufferSource& operator=(MemoryBufferSource const&) = delete;

  MemoryBufferSource& operator=(MemoryBufferSource&&) = delete;

  explicit operator bool() const override { return it_ != end_; }

  bool hasBOM() const override { return hasBOM_; }

  uint32_t Get() override;

  SourceLocation getLocation() const override
  {
    return SourceLocation("top-level.u", ".", SourceRange(position_, position_));
  }

protected:
  std::unique_ptr<llvm::MemoryBuffer> source_;
  bool hasBOM_;
  MemoryBufferInputStream stream_;
  std::istreambuf_iterator<char> it_;
  std::istreambuf_iterator<char> end_;
  bool first_;
  SourcePosition position_;
  bool gotNewLine_;
};

} /* namespace u */

#endif // U_LANG_SOURCE_HPP
