//////////////////////////////////////////////////////////////////////////////
//
// Common Debug Utility
// z_debug.h
//
// Copyright (C) 2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/z_debug.h#2 $
//
#if !defined(INCLUDE_common_debugger_h)
#define INCLUDE_common_debugger_h

#if _DEBUG
    #define ASSERT(mp_expr) \
    { \
        bool const mp_result = static_cast<bool>(mp_expr); \
        Debugger::Assert( \
            __FILE__, __LINE__, __FUNCTION__, #mp_expr, mp_result); \
        __assume(mp_result); \
    } // ASSERT

    #define DEBUG_PRINTF(mp_fmt, ...) \
        CPRINTF(mp_fmt, __VA_ARGS__)
#else
    #define ASSERT(mp_expr) __assume(static_cast<bool>(mp_expr))
    #define DEBUG_PRINTF(mp_fmt, ...) __noop(__VA_ARGS__)
#endif

#define CAN_NOT_HAPPEN() \
    Debugger::CanNotHappen(__FILE__, __LINE__, __FUNCTION__)

#define CASSERT(mp_expr) \
    typedef char CASSERT_name(__COUNTER__)[(mp_expr) ? 1 : -1]

#define CASSERT_name(x) CASSERT_conc(macro_CASSERT, x)
#define CASSERT_conc(x,y) x ## y

#define CPRINTF(mp_fmt, ...) \
    { \
        Debugger::Printf("%u: %hs(%d): ", \
            ::GetTickCount(), __FUNCTION__, __LINE__); \
        Debugger::Printf(mp_fmt, __VA_ARGS__); \
    } // CPRIHNTF

namespace Debugger
{
    void Assert(const char*, int, const char*, const char*, bool);
    void __declspec(noreturn) CanNotHappen(const char*, int, const char*);
    void __declspec(noreturn) Fail(const char*, ...);
    void Printf(const char*, ...);
} // Debugger

#define WIN32_VERIFY(expr) { \
  if (!(expr)) { \
    auto const error = ::GetLastError(); \
    Debugger::Fail("error=%d " #expr "\n", error); \
  } \
}

#endif //!defined(INCLUDE_common_debugger_h)
