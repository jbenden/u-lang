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
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
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

static Token
ConvertFloat(std::string& num, const SourceLocation& w)
{
  llvm::APFloat v{-1.0};
  v.convertFromString(num, llvm::APFloat::rmTowardZero);

  return Token(tok::real_constant, w, v);
}

static Token
ConvertInt(std::string& num, const SourceLocation& w, int base)
{
  // (Over-)estimate the required number of bits.
  unsigned NumBits = (((unsigned) num.size() * 64) / 19) + 2u;
  llvm::APInt Tmp(NumBits, num, (uint8_t) base);

#if 0
  std::cerr << "Converted '" << num << "' base " << base << " to: " << Tmp.getSExtValue() << std::endl;
  std::cerr << Tmp.getActiveBits() << " active bits" << std::endl;
  std::cerr << "Sign bit " << Tmp.isSignBitSet() << std::endl;
  std::cerr << "Sign extended is " << Tmp.trunc(Tmp.getActiveBits() + 1).sextOrSelf(32).getSExtValue() << std::endl;
#endif

  return Token(tok::integer_constant, w, Tmp);
}

Token
Lexer::NumberToken()
{
  uint32_t ch = CurChar();
  SourceLocation w = getLocation();
  std::string num;
  int base = 10;

  if (ch == '0' && PeekChar() == 'x')
  {
    base = 16;
    NextChar();
    ch = NextChar();
  }
  else if (ch == '0' && PeekChar() == 'b')
  {
    base = 2;
    NextChar();
    ch = NextChar();
  }

  num = static_cast<char>(ch);

  enum State
  {
    Intpart,
    Fraction,
    Exponent,
    Done,
  } state = Intpart;

  bool isFloat = false;
  while (state != Done)
  {
    switch (state)
    {
    case Intpart:ch = NextChar();
      while ((base == 10 && isdigit(ch)) || (base == 16 && isxdigit(ch)) || (base == 2 && (ch == '0' || ch == '1')) ||
        (ch == '_'))
      {
        if (ch != '_')
        {
          num += (char) ch;
        }
        ch = NextChar();
      }
      break;

    case Fraction:assert(ch == '.' && "fraction should start with '.'");
      if (PeekChar() == '.' || PeekChar() == ')')
      {
        break;
      }
      isFloat = true;
      num += (char) ch;
      ch = NextChar();
      while (isdigit(ch) || ch == '_')
      {
        if (ch != '_')
        {
          num += (char) ch;
        }
        ch = NextChar();
      }
      break;

    case Exponent:isFloat = true;
      assert((ch == 'e' || ch == 'E') && "exponent should start with 'e' or 'E'");
      num += (char) ch;
      ch = NextChar();
      if (ch == '+' || ch == '-')
      {
        num += (char) ch;
        ch = NextChar();
      }
      while (isdigit(ch))
      {
        num += (char) ch;
        ch = NextChar();
      }
      break;

    default:throw std::runtime_error("should not have gotten to this point!"); // LCOV_EXCL_LINE
      break;
    }

    if (ch == '.' && state != Fraction && base != 16)
    {
      state = Fraction;
    }
    else if (state != Exponent && (ch == 'E' || ch == 'e'))
    {
      state = Exponent;
    }
    else
    {
      state = Done;
    }
  }

  // adjust the range of this entire Token.
  w.getRange().getEnd().setColumn(source_.getLocation().getRange().getEnd().getColumn());

  // dispatch to float or integer conversion.
  if (isFloat)
  {
    return ConvertFloat(num, w);
  }

  return ConvertInt(num, w, base);
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

  // Handle integer and real values.
  if (std::isdigit(ch))
  {
    return NumberToken();
  }

  // Handle newlines.
  if (ch == '\n')
  {
    sourceRange_.getBegin().incrementLineNumber();
    sourceRange_.getBegin().setColumn(0);

    sourceRange_.getEnd().incrementLineNumber();
    sourceRange_.getEnd().setColumn(0);

    NextChar();

    return Token(tok::eol, w);
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