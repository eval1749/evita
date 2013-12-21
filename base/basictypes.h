// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_basictypes_h)
#define INCLUDE_base_basictypes_h

#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef __int64 int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned __int64 uint64;

typedef unsigned int uint;

#define DISALLOW_ASSIGN(mp_type) \
  public: void operator=(const mp_type&) = delete

#define DISALLOW_COPY(mp_type) \
  public: mp_type(const mp_type&) = delete;

#define DISALLOW_COPY_AND_ASSIGN(mp_type) \
  public: mp_type(const mp_type&) = delete; \
  public: void operator=(const mp_type&) = delete

// Number of elements in array.
template<typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

#define arraysize(array) (sizeof(ArraySizeHelper(array)))
#define ARRAYSIZE_UNSAGE(a) (sizeof(a) / sizeof(*(a)))

// Used to explicitly mark the return value of a function as unused. If you
// are really sure you don't want to do anything with the return value of a
// function that has been marked WARN_UNUSED_RESULT, wrap it with this.
// Example:
//   scoped_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     ignore_result(my_var.release());
template<typename T>
inline void ignore_result(const T&) {
}

#endif // !defined(INCLUDE_base_basictypes_h)
