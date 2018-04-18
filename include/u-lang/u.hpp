/**
 * The U Programming Language
 * (C) 2018 Joseph Benden <joe@benden.us>
 *
 * Licensed under the terms of the LGPL v2 license. For more information please
 * see the LICENSE file in the root of the project's directory or located in
 * the doc directory.
 *
 * \author Joseph W. Benden
 * \copyright (C) 2018 Joseph Benden
 * \license lgpl
 */

#ifndef ULANG_U_H
#define ULANG_U_H

#define UC_MAJOR_VERSION 0
#define UC_MINOR_VERSION 0
#define UC_MICRO_VERSION 1
#define UC_VERSION_SEP   .

#define PPCAT_NX(A, B)        A ## B
#define PPCAT_NX3(A, B, C)    A ## B ## C
#define PPCAT_NX4(A,B,C,D)    A ## B ## C ## D
#define PPCAT_NX5(A,B,C,D,E)  A ## B ## C ## D ## E
#define PPCAT(A, B)           PPCAT_NX(A,B)
#define PPCAT3(A, B, C)       PPCAT_NX3(A,B,C)
#define PPCAT4(A, B, C, D)    PPCAT_NX4(A,B,C,D)
#define PPCAT5(A, B, C, D, E) PPCAT_NX5(A,B,C,D,E)

#define STRINGIZE_NX(A)       #A
#define STRINGIZE(A)          STRINGIZE_NX(A)

#define UC_VERSION STRINGIZE(PPCAT5(UC_MAJOR_VERSION, \
                                    UC_VERSION_SEP,   \
                                    UC_MINOR_VERSION, \
                                    UC_VERSION_SEP,   \
                                    UC_MICRO_VERSION))

#if defined(_WIN32) && defined(_DLL)
#if !defined(U_DLL) && !defined(U_STATIC)
#define U_DLL
#endif
#endif

//
// The following block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the foundation_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// foundation_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if (defined(_WIN32) || defined(_WIN32_WCE)) && defined(U_DLL)
#if defined(UEXPORTS)
#define UAPI __declspec(dllexport)
#else
#define UAPI __declspec(dllimport)
#endif
#endif

#if !defined(UAPI)
#if !defined(U_NO_GCC_API_ATTRIBUTE) && defined (__GNUC__) && (__GNUC__ >= 4)
#define UAPI __attribute__ ((visibility ("default")))
#else
#define UAPI
#endif
#endif

/* Here we provide G_GNUC_EXTENSION as an alias for __extension__,
 * where this is valid. This allows for warningless compilation of
 * "long long" types even in the presence of '-ansi -pedantic'.
 */
#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#define U_GNUC_EXTENSION __extension__
#else
#define U_GNUC_EXTENSION
#endif

/* Provide macros to feature the GCC function attribute.
 */
#if    __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define U_GNUC_PURE __attribute__((__pure__))
#define U_GNUC_MALLOC __attribute__((__malloc__))
#else
#define U_GNUC_PURE
#define U_GNUC_MALLOC
#endif

#if     __GNUC__ >= 4
#define U_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define U_GNUC_NULL_TERMINATED
#endif

/* Guard C code in headers, while including them from C++ */
#ifdef  __cplusplus
#define U_BEGIN_DECLS  extern "C" {
#define U_END_DECLS    }
#else
#define U_BEGIN_DECLS
#define U_END_DECLS
#endif

#undef  UMAX
#define UMAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef  UMIN
#define UMIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef  UABS
#define UABS(a)     (((a) < 0) ? -(a) : (a))

#undef  UCLAMP
#define UCLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define U_N_ELEMENTS(arr)               (sizeof (arr) / sizeof ((arr)[0]))


#if (defined(__GNUC__)  && __GNUC__ >= 4) || defined (_MSC_VER)
#define U_STRUCT_OFFSET(struct_type, member)    \
  ((glong) offsetof (struct_type, member))
#else
#define U_STRUCT_OFFSET(struct_type, member)        \
  ((glong) ((guint8*) &((struct_type*) 0)->member))
#endif

#define U_STRUCT_MEMBER_P(struct_p, struct_offset)              \
  ((gpointer) ((guint8*) (struct_p) + (glong) (struct_offset)))
#define U_STRUCT_MEMBER(member_type, struct_p, struct_offset)       \
  (*(member_type*) U_STRUCT_MEMBER_P ((struct_p), (struct_offset)))

/*
 * The U_LIKELY and U_UNLIKELY macros let the programmer give hints to
 * the compiler about the expected result of an expression. Some compilers
 * can use this information for optimization.
 *
 * The _U_BOOLEAN_EXPR macro is intended to trigger a gcc warning when
 * putting assignments in g_return_if_fail ().
 */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _U_BOOLEAN_EXPR(expr)                   \
  U_GNUC_EXTENSION ({                           \
                     int _g_boolean_var_;       \
                     if (expr)                  \
                       _g_boolean_var_ = 1;     \
                     else                       \
                       _g_boolean_var_ = 0;     \
                     _g_boolean_var_;           \
    })
#define U_LIKELY(expr) (__builtin_expect (_U_BOOLEAN_EXPR((expr)), 1))
#define U_UNLIKELY(expr) (__builtin_expect (_U_BOOLEAN_EXPR((expr)), 0))
#else
#define U_LIKELY(expr) (expr)
#define U_UNLIKELY(expr) (expr)
#endif

#ifdef __cplusplus
#define UINLINE inline
#else
#define UINLINE
#endif

#if     __GNUC__ >= 4
#define UINLINE_ALWAYS UINLINE __attribute__((always_inline))
#else
#define UINLINE_ALWAYS UINLINE
#endif

#ifdef __cplusplus

#include <iostream>
#include <system_error>
#include <assert.h>

#undef HAVE_INTTYPES_H
#undef HAVE_STDINT_H
#undef HAVE_UINT64_T
#include <stdtypes.hpp>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

using size_t = ::std::size_t;

#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Value - Value) + (Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

inline uint32_t
SafeTruncateUInt64(uint64_t Value)
{
  assert(Value <= 0xFFFFFFFF);
  uint32_t Result = (uint32_t)Value;
  return(Result);
}

inline uint16_t
SafeTruncateToU16(uint32_t Value)
{
  assert(Value <= 0xFFFF);
  uint16_t Result = (uint16_t)Value;
  return(Result);
}

inline uint8_t
SafeTruncateToU16(uint16_t Value)
{
  assert(Value <= 0xFF);
  uint8_t Result = (uint8_t)Value;
  return(Result);
}

#endif

