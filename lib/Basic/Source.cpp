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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/Source.hpp>
#include <u-lang/u.hpp>
#include <utf8.h>

using namespace u;

FileSource::FileSource(std::string const& fileName)
  : Source()
  , fileName_{fileName}
  , first_{true}
  , hasBOM_{false}
  , stream_{fileName, std::ios::in}
  , it_{stream_.rdbuf()}
  , position_{1, 0}
  , gotNewLine_{false}
{
  VLOG(1) << "Reading from " << fileName_;

  llvm::SmallVector<char, 0> theFileName;
  llvm::Twine f{fileName};

  f.toVector(theFileName);

  // Is the fileName path absolute?
  if (!llvm::sys::path::is_absolute(theFileName))
  {
    llvm::sys::fs::make_absolute(theFileName);
  }

  std::string fn;
  std::copy(theFileName.begin(), theFileName.end(), std::back_inserter(fn));
  fileName_ = llvm::sys::path::filename(fn).str();

  llvm::sys::path::remove_filename(theFileName);
  std::copy(theFileName.begin(), theFileName.end(), std::back_inserter(filePath_));
}

uint32_t
FileSource::Get()
{
  if (first_)
  {
    if (utf8::starts_with_bom(it_, end_))
    {
      ++it_;
      hasBOM_ = true;
    }
    else
    {
      stream_.unget();
    }

    first_ = false;
  }

  // process previous new-line
  if (gotNewLine_)
  {
    position_.setColumn(0);
    position_.incrementLineNumber();

    gotNewLine_ = false;
  }

  uint32_t ch = utf8::next(it_, end_);

  // increment column
  position_.incrementColumn();

  while (ch == '\r') // LCOV_EXCL_BR_LINE
  {
    ch = utf8::next(it_, end_); // LCOV_EXCL_LINE
  }

  // if NL, then reset column and increment line number.
  if (ch == '\n')
  {
    gotNewLine_ = true;
  }

  return ch;
}

uint32_t
StringSource::Get()
{
  if (first_)
  {
    if (utf8::starts_with_bom(it_, end_))
    {
      ++it_;
      hasBOM_ = true;
    }
    else
    {
      stream_.unget();
    }

    first_ = false;
  }

  // process previous new-line
  if (gotNewLine_)
  {
    position_.setColumn(0);
    position_.incrementLineNumber();

    gotNewLine_ = false;
  }

  uint32_t ch = utf8::next(it_, end_);

  // increment column
  position_.incrementColumn();

  while (ch == '\r')
  {
    ch = utf8::next(it_, end_);
  }

  // if NL, then reset column and increment line number.
  if (ch == '\n')
  {
    gotNewLine_ = true;
  }

  return ch;
}

MemoryBufferSource::MemoryBufferSource(llvm::sys::fs::UniqueID ID,
                                       llvm::StringRef Path,
                                       std::unique_ptr<llvm::MemoryBuffer> Buf)
  : Source()
  , id_{ID}
  , source_{std::move(Buf)}
  , hasBOM_{false}
  , stream_{reinterpret_cast<const uint8_t*>(source_->getBufferStart()), source_->getBufferSize()}
  , it_{stream_.rdbuf()}
  , first_{true}
  , position_{1, 0}
  , gotNewLine_{false}
{
  VLOG(1) << "Reading from " << Path.str();

  llvm::SmallVector<char, 0> theFileName;
  llvm::Twine f{Path};

  f.toVector(theFileName);

  std::string fn;
  std::copy(theFileName.begin(), theFileName.end(), std::back_inserter(fn));
  fileName_ = llvm::sys::path::filename(fn).str();

  llvm::sys::path::remove_filename(theFileName);
  std::copy(theFileName.begin(), theFileName.end(), std::back_inserter(filePath_));
}

uint32_t
MemoryBufferSource::Get()
{
  if (first_)
  {
    if (utf8::starts_with_bom(it_, end_))
    {
      ++it_;
      hasBOM_ = true;
    }
    else
    {
      stream_.unget();
    }

    first_ = false;
  }

  // process previous new-line
  if (gotNewLine_)
  {
    position_.setColumn(0);
    position_.incrementLineNumber();

    gotNewLine_ = false;
  }

  uint32_t ch = utf8::next(it_, end_);

  // increment column
  position_.incrementColumn();

  while (ch == '\r') // LCOV_EXCL_BR_LINE
  {
    ch = utf8::next(it_, end_); // LCOV_EXCL_LINE
  }

  // if NL, then reset column and increment line number.
  if (ch == '\n')
  {
    gotNewLine_ = true;
  }

  return ch;
}