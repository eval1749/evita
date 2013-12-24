// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_COMMON_EXPORT_H_
#define COMMON_COMMON_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(COMMON_IMPLEMENTATION)
#define COMMON_EXPORT __declspec(dllexport)
#define COMMON_EXPORT_PRIVATE __declspec(dllexport)
#else
#define COMMON_EXPORT __declspec(dllimport)
#define COMMON_EXPORT_PRIVATE __declspec(dllimport)
#endif  // defined(COMMON_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(COMMON_IMPLEMENTATION)
#define COMMON_EXPORT __attribute__((visibility("default")))
#define COMMON_EXPORT_PRIVATE __attribute__((visibility("default")))
#else
#define COMMON_EXPORT
#define COMMON_EXPORT_PRIVATE
#endif  // defined(COMMON_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define COMMON_EXPORT
#define COMMON_EXPORT_PRIVATE
#endif

#endif  // COMMON_COMMON_EXPORT_H_
