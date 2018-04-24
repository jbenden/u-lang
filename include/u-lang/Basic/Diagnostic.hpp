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

#ifndef U_LANG_DIAGNOSTIC_HPP
#define U_LANG_DIAGNOSTIC_HPP

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wmacro-redefined"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <glog/logging.h>

#include <u-lang/Basic/DiagnosticIDs.hpp>
#include <u-lang/Basic/SourceLocation.hpp>
#include <u-lang/u.hpp>

#include <cassert>

namespace u
{

class Diagnostic;

class DiagnosticBuilder;

class DiagnosticConsumer;

class UAPI DiagnosticEngine
{
  std::shared_ptr<DiagnosticConsumer> DC;
  std::unique_ptr<diag::DiagnosticMapping> Mappings;

public:
  enum ArgumentKind
  {
    ak_std_string, ///< std::string
    ak_c_string,   ///< const char *
    ak_sint,       ///< int
    ak_uint,       ///< unsigned
    ak_tokenkind,  ///< enum TokenKind : unsigned
  };

  /// \brief Represents on argument value, which is a union discriminated
  /// by ArgumentKind, with a value.
  typedef std::pair<ArgumentKind, intptr_t> ArgumentValue;

public:
  explicit DiagnosticEngine(std::shared_ptr<DiagnosticConsumer> consumer = nullptr);

  inline DiagnosticBuilder Report(SourceLocation Loc, diag::DiagnosticID DiagID);

  inline DiagnosticBuilder Report(diag::DiagnosticID DiagID);

  void Reset();

  std::shared_ptr<DiagnosticConsumer> getClient() { return DC; }

private:
  friend class Diagnostic;

  friend class DiagnosticBuilder;

  SourceLocation CurDiagLoc;

  diag::DiagnosticID CurDiagID;

  enum
  {
    /// \brief The maximum number of arguments we can hold.
    ///
    /// We currently only support up to 10 arguments (%0-%9).  A single
    /// diagnostic with more than that almost certainly has to be simplified
    /// anyway.
      MaxArguments = 10,
  };

  signed char NumDiagArgs;

  /// \brief Specifies whether an argument is in DiagArgumentsStr or
  /// in DiagArguments.
  unsigned char DiagArgumentsKind[MaxArguments];

  /// \brief Holds the values of each string argument for the current
  /// diagnostic.
  ///
  /// This is only used when the corresponding ArgumentKind is ak_std_string.
  std::string DiagArgumentsStr[MaxArguments];

  /// \brief The values for the various substitution positions.
  ///
  /// This is used when the argument is not an std::string. The specific
  /// value is mangled into an intptr_t and the interpretation depends on
  /// exactly what sort of argument kind it is.
  intptr_t DiagArgumentsVal[MaxArguments];

  // bool ProcessDiag();

protected:
  bool EmitCurrentDiagnostic(bool Force = false);
};

//===----------------------------------------------------------------------===//
// DiagnosticBuilder
//===----------------------------------------------------------------------===//

/// \brief A little helper class used to produce diagnostics.
///
/// This is constructed by the DiagnosticEngine::Report method, and
/// allows insertion of extra information (arguments and source ranges) into
/// the currently "in flight" diagnostic.  When the temporary for the builder
/// is destroyed, the diagnostic is issued.
///
/// Note that many of these will be created as temporary objects (many call
/// sites), so we want them to be small and we never want their address taken.
/// This ensures that compilers with somewhat reasonable optimizers will promote
/// the common fields to registers, eliminating increments of the NumArgs field,
/// for example.
class DiagnosticBuilder
{
  mutable DiagnosticEngine* DiagObj = nullptr;
  mutable signed char NumArgs = 0;

  /// \brief Status variable indicating if this diagnostic is still active.
  ///
  // NOTE: This field is redundant with DiagObj (IsActive iff (DiagObj == 0)),
  // but LLVM is not currently smart enough to eliminate the null check that
  // Emit() would end up with if we used that as our status variable.
  mutable bool IsActive = false;

  /// \brief Flag indicating that this diagnostic is being emitted via a
  /// call to ForceEmit.
  mutable bool IsForceEmit = false;

  friend class DiagnosticEngine;

  DiagnosticBuilder() = default;

  explicit DiagnosticBuilder(DiagnosticEngine* diagObj)
    : DiagObj(diagObj)
    , IsActive(true)
  {
    assert(diagObj && "DiagnosticBuilder requires a valid DiagnosticEngine!");
  }

protected:
  void FlushCounts() { DiagObj->NumDiagArgs = NumArgs; }

  /// \brief Clear out the current diagnostic.
  void Clear() const
  {
    DiagObj = nullptr;
    IsActive = false;
    IsForceEmit = false;
  }

  /// \brief Determine whether this diagnostic is still active.
  bool isActive() const { return IsActive; }

  /// \brief Force the diagnostic builder to emit the diagnostic now.
  ///
  /// Once this function has been called, the DiagnosticBuilder object
  /// should not be used again before it is destroyed.
  ///
  /// \returns true if a diagnostic was emitted, false if the
  /// diagnostic was suppressed.
  bool Emit()
  {
    // If this diagnostic is inactive, then its soul was stolen.
    if (!isActive())
      return false; // LCOV_EXCL_LINE

    // When emitting diagnostics, we set the final argument count into
    // the DiagnosticEngine object.
    FlushCounts();

    // Process the diagnostic.
    bool Result = DiagObj->EmitCurrentDiagnostic(IsForceEmit);

    // This diagnostic is dead.
    Clear();

    return Result;
  }

public:
  DiagnosticBuilder(const DiagnosticBuilder& D)
  {
    DiagObj = D.DiagObj;
    IsActive = D.IsActive;
    IsForceEmit = D.IsForceEmit;
    D.Clear();
    NumArgs = D.NumArgs;
  }

  DiagnosticBuilder& operator=(const DiagnosticBuilder&) = delete;

  /// \brief Emits the diagnostic.
  ~DiagnosticBuilder() { Emit(); }

  /// \brief Retrieve an empty diagnostic builder.
  static DiagnosticBuilder getEmpty() { return DiagnosticBuilder(); }

  /// \brief Forces the diagnostic to be emitted.
  const DiagnosticBuilder& setForceEmit() const
  {
    IsForceEmit = true;
    return *this;
  }

  /// \brief Conversion of DiagnosticBuilder to bool always returns \c true.
  ///
  /// This allows is to be used in boolean error contexts (where \c true is
  /// used to indicate that an error has occurred), like:
  /// \code
  /// return Diag(...);
  /// \endcode
  explicit operator bool() const { return true; }

  void AddString(llvm::StringRef S) const
  {
    assert(isActive() && "Clients must not add to cleared diagnostic!"); // LCOV_EXCL_LINE
    assert(NumArgs < DiagnosticEngine::MaxArguments &&                   // LCOV_EXCL_LINE
      "Too many arguments to diagnostic!");                         // LCOV_EXCL_LINE
    DiagObj->DiagArgumentsKind[NumArgs] = DiagnosticEngine::ak_std_string;
    DiagObj->DiagArgumentsStr[NumArgs++] = S;
  }

  void AddTaggedVal(intptr_t V, DiagnosticEngine::ArgumentKind Kind) const
  {
    assert(isActive() && "Clients must not add to cleared diagnostic!"); // LCOV_EXCL_LINE
    assert(NumArgs < DiagnosticEngine::MaxArguments &&                   // LCOV_EXCL_LINE
      "Too many arguments to diagnostic!");                         // LCOV_EXCL_LINE
    DiagObj->DiagArgumentsKind[NumArgs] = Kind;
    DiagObj->DiagArgumentsVal[NumArgs++] = V;
  }
};

inline const DiagnosticBuilder&
operator<<(const DiagnosticBuilder& DB, llvm::StringRef S)
{
  DB.AddString(S);
  return DB;
}

inline const DiagnosticBuilder&
operator<<(const DiagnosticBuilder& DB, const char* Str)
{
  DB.AddTaggedVal(reinterpret_cast<intptr_t>(Str), DiagnosticEngine::ak_c_string);
  return DB;
}

inline const DiagnosticBuilder&
operator<<(const DiagnosticBuilder& DB, int I)
{
  DB.AddTaggedVal(I, DiagnosticEngine::ak_sint);
  return DB;
}

// We use enable_if here to prevent that this overload is selected for
// pointers or other arguments that are implicitly convertible to bool.
template<typename T>
inline typename std::enable_if<std::is_same<T, bool>::value, const DiagnosticBuilder&>::type
operator<<(const DiagnosticBuilder& DB, T I)
{
  DB.AddTaggedVal(I, DiagnosticEngine::ak_sint);
  return DB;
}

inline const DiagnosticBuilder&
operator<<(const DiagnosticBuilder& DB, unsigned I)
{
  DB.AddTaggedVal(I, DiagnosticEngine::ak_uint);
  return DB;
}

inline const DiagnosticBuilder&
operator<<(const DiagnosticBuilder& DB, tok::TokenKind I)
{
  DB.AddTaggedVal(static_cast<unsigned>(I), DiagnosticEngine::ak_tokenkind);
  return DB;
}

inline DiagnosticBuilder
DiagnosticEngine::Report(SourceLocation Loc, diag::DiagnosticID DiagID)
{
  assert(CurDiagID == diag::NUM_DIAGNOSTICS && "Multiple diagnostics in flight at once!");
  CurDiagLoc = Loc; // NOLINT
  CurDiagID = DiagID;
  return DiagnosticBuilder(this);
}

inline DiagnosticBuilder
DiagnosticEngine::Report(diag::DiagnosticID DiagID)
{
  return Report(SourceLocation(), DiagID);
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

/// A little helper class (which is basically a smart pointer that forwards
/// info from DiagnosticEngine) that allows clients to enquire about the
/// currently in-flight diagnostic.
class Diagnostic
{
  const DiagnosticEngine* DiagObj;

public:
  explicit Diagnostic(const DiagnosticEngine* DO)
    : DiagObj(DO)
  {
  }

  const DiagnosticEngine* getDiags() const { return DiagObj; }

  diag::DiagnosticID getID() const { return DiagObj->CurDiagID; }

  const SourceLocation& getLocation() const { return DiagObj->CurDiagLoc; }

#if 0
  bool hasSourceManager() const { return DiagObj->hasSourceManager(); }
  SourceManager &getSourceManager() const { return DiagObj->getSourceManager();}
#endif

  unsigned getNumArgs() const { return static_cast<unsigned>(DiagObj->NumDiagArgs); }

  /// \brief Return the kind of the specified index.
  ///
  /// Based on the kind of argument, the accessors below can be used to get
  /// the value.
  ///
  /// \pre Idx < getNumArgs()
  DiagnosticEngine::ArgumentKind getArgKind(unsigned Idx) const
  {
    assert(Idx < getNumArgs() && "Argument index out of range!");
    return (DiagnosticEngine::ArgumentKind) DiagObj->DiagArgumentsKind[Idx];
  }

  /// \brief Return the provided argument string specified by \p Idx.
  /// \pre getArgKind(Idx) == DiagnosticEngine::ak_std_string
  const std::string& getArgStdStr(unsigned Idx) const
  {
    assert(getArgKind(Idx) == DiagnosticEngine::ak_std_string && "invalid argument accessor!");
    return DiagObj->DiagArgumentsStr[Idx];
  }

  /// \brief Return the specified C string argument.
  /// \pre getArgKind(Idx) == DiagnosticEngine::ak_c_string
  const char* getArgCStr(unsigned Idx) const
  {
    assert(getArgKind(Idx) == DiagnosticEngine::ak_c_string && "invalid argument accessor!");
    return reinterpret_cast<const char*>(DiagObj->DiagArgumentsVal[Idx]);
  }

  /// \brief Return the specified signed integer argument.
  /// \pre getArgKind(Idx) == DiagnosticEngine::ak_sint
  int getArgSInt(unsigned Idx) const
  {
    assert(getArgKind(Idx) == DiagnosticEngine::ak_sint && "invalid argument accessor!");
    return (int) DiagObj->DiagArgumentsVal[Idx];
  }

  /// \brief Return the specified unsigned integer argument.
  /// \pre getArgKind(Idx) == DiagnosticEngine::ak_uint
  unsigned getArgUInt(unsigned Idx) const
  {
    assert(getArgKind(Idx) == DiagnosticEngine::ak_uint && "invalid argument accessor!");
    return (unsigned) DiagObj->DiagArgumentsVal[Idx];
  }

  /// \brief Return the specified non-string argument in an opaque form.
  /// \pre getArgKind(Idx) != DiagnosticEngine::ak_std_string
  intptr_t getRawArg(unsigned Idx) const
  {
    assert(getArgKind(Idx) != DiagnosticEngine::ak_std_string && "invalid argument accessor!");
    return DiagObj->DiagArgumentsVal[Idx];
  }

  /// \brief Format this diagnostic into a string, substituting the
  /// formal arguments into the %0 slots.
  ///
  /// The result is appended onto the \p OutStr array.
  void FormatDiagnostic(llvm::SmallVectorImpl<char>& OutStr) const;

  /// \brief Format the given format-string into the output buffer using the
  /// arguments stored in this diagnostic.
  void FormatDiagnostic(const char* DiagStr, const char* DiagEnd, llvm::SmallVectorImpl<char>& OutStr) const;
};

/// \brief Abstract interface, implemented by clients of the front-end, which
/// formats and prints fully processed diagnostics.
class DiagnosticConsumer
{
protected:
  unsigned NumWarnings = 0; ///< Number of warnings reported
  unsigned NumErrors = 0;   ///< Number of errors reported

public:
  DiagnosticConsumer() = default;

  virtual ~DiagnosticConsumer() = default;

  unsigned getNumErrors() const { return NumErrors; }

  unsigned getNumWarnings() const { return NumWarnings; }

  virtual void clear() { NumWarnings = NumErrors = 0; }

  /// \brief Indicates whether the diagnostics handled by this
  /// DiagnosticConsumer should be included in the number of diagnostics
  /// reported by DiagnosticEngine.
  ///
  /// The default implementation returns true.
  virtual bool IncludeInDiagnosticCounts() const;

  /// \brief Handle this diagnostic, reporting it to the user or
  /// capturing it to a log as needed.
  ///
  /// The default implementation just keeps track of the total number of
  /// warnings and errors.
  virtual void HandleDiagnostic(diag::Severity DiagLevel, const Diagnostic& Info);
};

/// \brief A diagnostic client that ignores all diagnostics.
class IgnoringDiagConsumer : public DiagnosticConsumer
{
  void HandleDiagnostic(diag::Severity DiagLevel, const Diagnostic& Info) override
  {
    // Just ignore it.
  }
};

} /* namespace u */

#endif // U_LANG_DIAGNOSTIC_HPP
