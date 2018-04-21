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
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Lex/Lexer.hpp>
#include <u-lang/u.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <utf8.h>

using namespace u;

static bool
IsSpace(uint32_t ch)
{
  return ch == ' ' || ch == '\t' || ch == 0xa0 || (ch >= 0x2000 && ch <= 0x200a) || ch == 0x2029 || ch == 0x202f ||
    ch == 0x205f || ch == 0x3000;
}

Lexer::Lexer(u::Source& source)
  : source_{source}
  , fileName_{source_.getLocation().getFileName()}
  , filePath_{source_.getLocation().getFilePath()}
  , sourceRange_{source_.getLocation().getRange()}
  , curValid_{0}
{
}

uint32_t
Lexer::NextChar()
{
  // increment column
  sourceRange_.getBegin().incrementColumn();
  sourceRange_.getEnd().incrementColumn();

  if (curValid_ > 1)
  {
    --curValid_;

    return curChar_ = nextChar_;
  }

  return curChar_ = GetChar();
}

uint32_t
Lexer::PeekChar()
{
  if (curValid_ > 1)
  {
    return nextChar_;
  }

  ++curValid_;
  return nextChar_ = GetChar();
}

uint32_t
Lexer::CurChar()
{
  if (!curValid_)
  {
    NextChar();
    ++curValid_;
  }

  return curChar_;
}

uint32_t
Lexer::GetChar()
{
  if (source_)
  {
    uint32_t ch = source_.Get();

    // insert character into the SourceManager entry for this file...

    return ch;
  }

  return 0;
}

Token
Lexer::Lex()
{
  uint32_t ch = CurChar();
  SourceLocation w = getLocation();

  // Skip over any UNICODE defined space characters.
  while (IsSpace(ch))
  {
    ch = NextChar();
    w = getLocation();
  }

  Token Result = Token(tok::unknown, w);

  // Check for an end-of-file condition.
  if (!source_ && curValid_ <= 1 && ch == 0)
  {
    return Token(tok::eof, w);
  }
  else
  {
    NextChar();
  }

  return Result;
}