// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_base_export_h)
#define INCLUDE_base_export_h

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __declspec(dllexport)
#define BASE_EXPORT_PRIVATE __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#define BASE_EXPORT_PRIVATE __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __attribute__((visibility("default")))
#define BASE_EXPORT_PRIVATE __attribute__((visibility("default")))
#else
#define BASE_EXPORT
#define BASE_EXPORT_PRIVATE
#endif  // defined(BASE_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define BASE_EXPORT
#define BASE_EXPORT_PRIVATE
#endif

#endif //!defined(INCLUDE_base_export_h)
