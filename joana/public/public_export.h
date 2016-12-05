// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_PUBLIC_EXPORT_H_
#define JOANA_PUBLIC_PUBLIC_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(JOANA_PUBLIC_IMPLEMENTATION)
#define JOANA_PUBLIC_EXPORT __declspec(dllexport)
#else
#define JOANA_PUBLIC_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(JOANA_PUBLIC_IMPLEMENTATION)
#define JOANA_PUBLIC_EXPORT __attribute__((visibility("default")))
#else
#define JOANA_PUBLIC_EXPORT
#endif  // defined(BASE_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define JOANA_PUBLIC_EXPORT
#endif

#endif  // JOANA_PUBLIC_PUBLIC_EXPORT_H_
