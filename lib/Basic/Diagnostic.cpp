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
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/Locale.h>
#include <llvm/Support/raw_ostream.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/Diagnostic.hpp>
#include <u-lang/Basic/DiagnosticIDs.hpp>
#include <u-lang/u.hpp>

#include <utf8.h>

using namespace u;

DiagnosticEngine::DiagnosticEngine(std::shared_ptr<SourceManager> M,                // NOLINT
                                   std::shared_ptr<u::DiagnosticConsumer> consumer) // NOLINT
  : SM{M}
  , DC{consumer}                                                                    // NOLINT
  , Mappings{std::make_unique<diag::DiagnosticMapping>()}
{
  Reset();
}

void
DiagnosticEngine::Reset()
{
  CurDiagID = diag::NUM_DIAGNOSTICS;
}

bool
DiagnosticEngine::EmitCurrentDiagnostic(bool Force)
{
  (void) Force;

  Diagnostic Info(this);

  // Lookup the severity and other detail.
  diag::Severity Severity = Mappings->get(Info.getID())->Level;

  if (DC)
  {
    DC->HandleDiagnostic(Severity, Info);
  }

  CurDiagID = diag::NUM_DIAGNOSTICS;

  // Clear out the current diagnostic object.
  // Clear();

  return true;
}

void
DiagnosticConsumer::HandleDiagnostic(u::diag::Severity DiagLevel, u::Diagnostic const& Info)
{
  if (!IncludeInDiagnosticCounts())
    return; // LCOV_EXCL_LINE

  if (DiagLevel == diag::Severity::Warning)
    ++NumWarnings;
  else if (DiagLevel == diag::Severity::Error || DiagLevel == diag::Severity::Fatal)
    ++NumErrors;
}

/// ModifierIs - Return true if the specified modifier matches specified string.
template<std::size_t StrLen>
static bool
ModifierIs(const char* Modifier, unsigned long ModifierLen, const char (& Str)[StrLen])
{
  return StrLen - 1 == ModifierLen && !memcmp(Modifier, Str, StrLen - 1);
}

/// ScanForward - Scans forward, looking for the given character, skipping
/// nested clauses and escaped characters.
static const char*
ScanFormat(const char* I, const char* E, char Target)
{
  unsigned Depth = 0;

  for (; I != E; ++I)
  {
    if (Depth == 0 && *I == Target)
      return I;
    if (Depth != 0 && *I == '}')
      Depth--; // LCOV_EXCL_LINE

    if (*I == '%')
    {
      I++;
      if (I == E)
        break; // LCOV_EXCL_LINE

      // Escaped characters get implicitly skipped here.

      // Format specifier.
      if (!isdigit(*I) && !ispunct(*I))
      {
        for (I++; I != E && !isdigit(*I) && *I != '{'; I++) // LCOV_EXCL_LINE
          ; // LCOV_EXCL_LINE
        if (I == E) // LCOV_EXCL_LINE
          break;       // LCOV_EXCL_LINE
        if (*I == '{') // LCOV_EXCL_LINE
          Depth++;     // LCOV_EXCL_LINE
      }
    }
  }
  return E; // LCOV_EXCL_LINE
}

/// HandleSelectModifier - Handle the integer 'select' modifier.  This is used
/// like this:  %select{foo|bar|baz}2.  This means that the integer argument
/// "%2" has a value from 0-2.  If the value is 0, the diagnostic prints 'foo'.
/// If the value is 1, it prints 'bar'.  If it has the value 2, it prints 'baz'.
/// This is very useful for certain classes of variant diagnostics.
static void
HandleSelectModifier(const Diagnostic& DInfo,
                     unsigned ValNo,
                     const char* Argument,
                     unsigned long ArgumentLen,
                     llvm::SmallVectorImpl<char>& OutStr)
{
  const char* ArgumentEnd = Argument + ArgumentLen;

  // Skip over 'ValNo' |'s.
  while (ValNo)
  {
    const char* NextVal = ScanFormat(Argument, ArgumentEnd, '|');
    assert(NextVal != ArgumentEnd && "Value for integer select modifier was" // LCOV_EXCL_LINE
                                     " larger than the number of options in the diagnostic string!");
    Argument = NextVal + 1; // Skip this string.
    --ValNo;
  }

  // Get the end of the value.  This is either the } or the |.
  const char* EndPtr = ScanFormat(Argument, ArgumentEnd, '|');

  // Recursively format the result of the select clause into the output string.
  DInfo.FormatDiagnostic(Argument, EndPtr, OutStr);
}

/// HandleIntegerSModifier - Handle the integer 's' modifier.  This adds the
/// letter 's' to the string if the value is not 1.  This is used in cases like
/// this:  "you idiot, you have %4 parameter%s4!".
static void
HandleIntegerSModifier(unsigned ValNo, llvm::SmallVectorImpl<char>& OutStr)
{
  if (ValNo != 1)
    OutStr.push_back('s');
}

/// HandleOrdinalModifier - Handle the integer 'ord' modifier.  This
/// prints the ordinal form of the given integer, with 1 corresponding
/// to the first ordinal.  Currently this is hard-coded to use the
/// English form.
static void
HandleOrdinalModifier(unsigned ValNo, llvm::SmallVectorImpl<char>& OutStr)
{
  assert(ValNo != 0 && "ValNo must be strictly positive!");

  llvm::raw_svector_ostream Out(OutStr);

  // We could use text forms for the first N ordinals, but the numeric
  // forms are actually nicer in diagnostics because they stand out.
  Out << ValNo << llvm::getOrdinalSuffix(ValNo);
}

/// FormatDiagnostic - Format this diagnostic into a string, substituting the
/// formal arguments into the %0 slots.  The result is appended onto the Str
/// array.
void
Diagnostic::FormatDiagnostic(llvm::SmallVectorImpl<char>& OutStr) const
{
  std::string Diag = getDiags()->Mappings->get(getID())->Message;

  FormatDiagnostic(Diag.c_str(), Diag.c_str() + Diag.size(), OutStr);
}

void
Diagnostic::FormatDiagnostic(const char* DiagStr, const char* DiagEnd, llvm::SmallVectorImpl<char>& OutStr) const
{

  // When the diagnostic string is only "%0", the entire string is being given
  // by an outside source.  Remove unprintable characters from this string
  // and skip all the other string processing.
  if (DiagEnd - DiagStr == 2 && llvm::StringRef(DiagStr, DiagEnd - DiagStr).equals("%0") &&
    getArgKind(0) == DiagnosticEngine::ak_std_string)
  {
    const std::string& S = getArgStdStr(0);
    for (char c : S)
    {
      if (isprint(c) || c == '\t')
      {
        OutStr.push_back(c);
      }
    }
    return;
  }
  /// FormattedArgs - Keep track of all of the arguments formatted by
  /// ConvertArgToString and pass them into subsequent calls to
  /// ConvertArgToString, allowing the implementation to avoid redundancies in
  /// obvious cases.
  llvm::SmallVector<DiagnosticEngine::ArgumentValue, 8> FormattedArgs;

  /// QualTypeVals - Pass a vector of arrays so that QualType names can be
  /// compared to see if more information is needed to be printed.
  llvm::SmallVector<intptr_t, 2> QualTypeVals;
  llvm::SmallVector<char, 64> Tree;

  while (DiagStr != DiagEnd)
  {
    if (DiagStr[0] != '%')
    {
      // Append non-%0 substrings to Str if we have one.
      const char* StrEnd = std::find(DiagStr, DiagEnd, '%');
      OutStr.append(DiagStr, StrEnd);
      DiagStr = StrEnd;
      continue;
    }
    else if (ispunct(DiagStr[1]))
    {
      OutStr.push_back(DiagStr[1]); // %% -> %.
      DiagStr += 2;
      continue;
    }

    // Skip the %.
    ++DiagStr;

    // This must be a placeholder for a diagnostic argument.  The format for a
    // placeholder is one of "%0", "%modifier0", or "%modifier{arguments}0".
    // The digit is a number from 0-9 indicating which argument this comes from.
    // The modifier is a string of digits from the set [-a-z]+, arguments is a
    // brace enclosed string.
    const char* Modifier = nullptr, * Argument = nullptr;
    unsigned long ModifierLen = 0, ArgumentLen = 0;
    (void) ArgumentLen;

    // Check to see if we have a modifier.  If so eat it.
    if (!isdigit(DiagStr[0]))
    {
      Modifier = DiagStr;
      while (DiagStr[0] == '-' || (DiagStr[0] >= 'a' && DiagStr[0] <= 'z'))
        ++DiagStr;
      ModifierLen = DiagStr - Modifier;

      // If we have an argument, get it next.
      if (DiagStr[0] == '{')
      {
        ++DiagStr; // Skip {.
        Argument = DiagStr;

        DiagStr = ScanFormat(DiagStr, DiagEnd, '}');
        assert(DiagStr != DiagEnd && "Mismatched {}'s in diagnostic string!");
        ArgumentLen = DiagStr - Argument;
        ++DiagStr; // Skip }.
      }
    }

    assert(isdigit(*DiagStr) && "Invalid format for argument in diagnostic");
    unsigned ArgNo = *DiagStr++ - '0';

    DiagnosticEngine::ArgumentKind Kind = getArgKind(ArgNo);
    int SVal;
    unsigned UVal;

    switch (Kind)
    {
      // ---- STRINGS ----
    case DiagnosticEngine::ak_std_string:
    {
      const std::string& S = getArgStdStr(ArgNo);
      assert(ModifierLen == 0 && "No modifiers for strings yet");
      OutStr.append(S.begin(), S.end());
      break;
    }

    case DiagnosticEngine::ak_c_string:
    {
      const char* S = getArgCStr(ArgNo);
      assert(ModifierLen == 0 && "No modifiers for strings yet");

      // Don't crash if get passed a null pointer by accident.
      if (!S)
        S = "(null)";
      OutStr.append(S, S + strlen(S));
      break;
    }

      // ---- INTEGERS ----
    case DiagnosticEngine::ak_sint:
    {
      SVal = getArgSInt(ArgNo);

      if (ModifierIs(Modifier, ModifierLen, "select"))
      {
        HandleSelectModifier(*this, (unsigned) SVal, Argument, ArgumentLen, OutStr);
      }
      else if (ModifierIs(Modifier, ModifierLen, "s"))
      {
        HandleIntegerSModifier(SVal, OutStr);
      }
      else if (ModifierIs(Modifier, ModifierLen, "ordinal"))
      {
        HandleOrdinalModifier((unsigned) SVal, OutStr);
      }
      else
      {
        assert(ModifierLen == 0 && "Unknown integer modifier");
        llvm::raw_svector_ostream(OutStr) << SVal;
      }
    }
      break;

    case DiagnosticEngine::ak_uint:
    {
      UVal = getArgUInt(ArgNo);

      if (ModifierIs(Modifier, ModifierLen, "select"))
      {
        HandleSelectModifier(*this, UVal, Argument, ArgumentLen, OutStr);
      }
      else if (ModifierIs(Modifier, ModifierLen, "s"))
      {
        HandleIntegerSModifier(UVal, OutStr);
      }
      else if (ModifierIs(Modifier, ModifierLen, "ordinal"))
      {
        HandleOrdinalModifier(UVal, OutStr);
      }
      else
      {
        assert(ModifierLen == 0 && "Unknown integer modifier");
        llvm::raw_svector_ostream(OutStr) << UVal;
      }
    }
      break;

      // ---- TOKEN SPELLINGS ----
    case DiagnosticEngine::ak_tokenkind:
    {
      tok::TokenKind Kind = static_cast<tok::TokenKind>(getRawArg(ArgNo));
      assert(ModifierLen == 0 && "No modifiers for token kinds yet");

      llvm::raw_svector_ostream Out(OutStr);
      if (const char* S = tok::getPunctuatorSpelling(Kind))
        // Quoted token spelling for punctuators.
        Out << '\'' << S << '\'';
      else if (const char* S = tok::getKeywordSpelling(Kind))
        // Unquoted token spelling for keywords.
        Out << S;
      else if (const char* S = tok::getTokenName(Kind))
        // Debug name, shouldn't appear in user-facing diagnostics.
        Out << '<' << S << '>';
      else
        Out << "(null)"; // LCOV_EXCL_LINE
    }
      break;
    }

    // Remember this argument info for subsequent formatting operations.  Turn
    // std::strings into a null terminated string to make it be the same case as
    // all the other ones.
    if (Kind != DiagnosticEngine::ak_std_string)
      FormattedArgs.push_back(std::make_pair(Kind, getRawArg(ArgNo)));
    else
      FormattedArgs.push_back(std::make_pair(DiagnosticEngine::ak_c_string, (intptr_t) getArgStdStr(ArgNo).c_str()));
  }

  // Append the type tree to the end of the diagnostics.
  OutStr.append(Tree.begin(), Tree.end());
}

/// IncludeInDiagnosticCounts - This method (whose default implementation
///  returns true) indicates whether the diagnostics handled by this
///  DiagnosticConsumer should be included in the number of diagnostics
///  reported by DiagnosticEngine.
bool
DiagnosticConsumer::IncludeInDiagnosticCounts() const
{
  return true;
}
