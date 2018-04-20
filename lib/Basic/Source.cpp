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

#include <u-lang/u.hpp>
#include <u-lang/Basic/Source.hpp>

using namespace u;

FileSource::FileSource(std::string const &fileName)
    : Source(), fileName_{fileName}, first_{true}, hasBOM_{false}, stream_{fileName, std::ios::in}, it_{stream_.rdbuf()}
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
