// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PUBLIC_PARSER_EXPORT_H_
#define JOANA_PARSER_PUBLIC_PARSER_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(JOANA_PARSER_IMPLEMENTATION)
#define JOANA_PARSER_EXPORT __declspec(dllexport)
#else
#define JOANA_PARSER_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(JOANA_PARSER_IMPLEMENTATION)
#define JOANA_PARSER_EXPORT __attribute__((visibility("default")))
#else
#define JOANA_PARSER_EXPORT
#endif  // defined(BASE_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define JOANA_PARSER_EXPORT
#endif

#endif  // JOANA_PARSER_PUBLIC_PARSER_EXPORT_H_
