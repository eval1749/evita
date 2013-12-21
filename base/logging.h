// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_logging_h)
#define INCLUDE_base_logging_h

#include "base_export.h"
#include "base/strings/string16.h"
#include <sstream>

namespace logging {

#if _DEBUG
#define DCHECK_OP(name, op, a, b) \
  ::logging::Check##name( \
      (a), (b), #a, #b, #op, __FILE__, __LINE__, __FUNCTION__)

#define DCHECK(expr) \
  ::logging::CheckTrue(static_cast<bool>(expr), #expr, \
      __FILE__, __LINE__, __FUNCTION__)

#else
#define DCHECK_OP(name, op, a, b) __noop(a, b)
#define DCHECK(expr) __noop(expr)
#endif

#define DCHECK_EQ(a, b) DCHECK_OP(Eq, ==, a, b)
#define DCHECK_GE(a, b) DCHECK_OP(Ge, >=, a, b)
#define DCHECK_GT(a, b) DCHECK_OP(Gt, >, a, b)
#define DCHECK_LE(a, b) DCHECK_OP(Le, <=, a, b)
#define DCHECK_LT(a, b) DCHECK_OP(Lt, <, a, b)
#define DCHECK_NE(a, b) DCHECK_OP(Ne, !=, a, b)

void BASE_EXPORT __declspec(noreturn)
    CheckFailed(
        const base::string16& a,
        const base::string16& b,
        const char* a_str,
        const char* b_str,
        const char* op,
        const char* filename, int linenum, const char* fname);

void BASE_EXPORT CheckTrue(
    bool expr,
    const char* message,
    const char* filename,
    int linenum,
    const char* fname);

#define DEFINE_CHECK_OP(name, op) \
  template<typename A, typename B> \
  inline void Check##name( \
      const A& a, const B& b, \
      const char* const a_str, \
      const char* const b_str, \
      const char* const op_str, \
      const char* const filename, \
      int const linenum, \
      const char* const fname) { \
    if (!(a op b)) \
      CheckFailed(ToString16(a), ToString16(b), a_str, b_str, op_str, \
                  filename, linenum, fname); \
  }

template<typename T>
base::string16 ToString16(const T& value) {
  std::basic_ostringstream<char16> stream;
  stream << value;
  return stream.str();
}

DEFINE_CHECK_OP(Eq, ==)
DEFINE_CHECK_OP(Ge, >=)
DEFINE_CHECK_OP(Gt, >)
DEFINE_CHECK_OP(Le, <=)
DEFINE_CHECK_OP(Lt, <)
DEFINE_CHECK_OP(Ne, !=)

} // namespace logging

#endif //!defined(INCLUDE_base_logging_h)
