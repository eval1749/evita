// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_BASE_BASE_EXPORT_H_
#define JOANA_BASE_BASE_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(JOANA_BASE_IMPLEMENTATION)
#define JOANA_BASE_EXPORT __declspec(dllexport)
#else
#define JOANA_BASE_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(JOANA_BASE_IMPLEMENTATION)
#define JOANA_BASE_EXPORT __attribute__((visibility("default")))
#else
#define JOANA_BASE_EXPORT
#endif  // defined(BASE_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define JOANA_BASE_EXPORT
#endif

#endif  // JOANA_BASE_BASE_EXPORT_H_
