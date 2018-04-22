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
#include <llvm/Support/ErrorHandling.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/TokenKinds.hpp>
#include <u-lang/u.hpp>

using namespace u;

static const char* const TokNames[] = {
#define TOK(X) #X,
#define KEYWORD(X) #X,
#include <u-lang/Basic/TokenKinds.def>

  nullptr};

const char*
tok::getTokenName(u::tok::TokenKind Kind)
{
  if (Kind < tok::NUM_TOKENS)
    return TokNames[Kind];

  llvm_unreachable("unknown TokenKind"); // LCOV_EXCL_LINE
  return nullptr;                        // LCOV_EXCL_LINE
}

const char*
tok::getPunctuatorSpelling(u::tok::TokenKind Kind)
{
  switch (Kind)
  {
#define PUNCTUATOR(X, Y, P)                                                                                            \
  case X:                                                                                                              \
    return Y;
#include <u-lang/Basic/TokenKinds.def>

  default:break;
  }

  return nullptr;
}

const char*
tok::getKeywordSpelling(u::tok::TokenKind Kind)
{
  switch (Kind)
  {
#define KEYWORD(X)                                                                                                     \
  case kw_##X:                                                                                                         \
    return #X;
#include <u-lang/Basic/TokenKinds.def>

  default:break;
  }

  return nullptr;
}