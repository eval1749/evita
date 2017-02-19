// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_
#define JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_

#include "base/strings/string_piece.h"
#include "joana/analyzer/public/analyzer_export.h"

namespace joana {

// V(pass, reason)
#define FOR_EACH_ANALYZER_ERROR_CODE(V)       \
  V(CLASS_TREE, CYCLE)                        \
  V(CLASS_TREE, UNDEFINED_CLASS)              \
  V(JSDOC, EMPTY_RECORD_TYPE)                 \
  V(JSDOC, EXPECT_GENERIC_CLASS)              \
  V(JSDOC, EXPECT_NAME)                       \
  V(JSDOC, EXPECT_NULLABLE_TYPE)              \
  V(JSDOC, EXPECT_PARAMETER)                  \
  V(JSDOC, EXPECT_PROPERTY)                   \
  V(JSDOC, EXPECT_REST)                       \
  V(JSDOC, EXPECT_TYPE)                       \
  V(JSDOC, INVALID_ARGUMENTS)                 \
  V(JSDOC, MULTIPLE_NAME)                     \
  V(JSDOC, MULTIPLE_PARAMETER)                \
  V(JSDOC, MULTIPLE_PROPERTY)                 \
  V(JSDOC, MULTIPLE_TAG)                      \
  V(JSDOC, UNDEFINED_TYPE)                    \
  V(JSDOC, UNEXPECT_INITIALIZER)              \
  V(JSDOC, UNEXPECT_OPTIONAL)                 \
  V(JSDOC, UNEXPECT_PARAMETER)                \
  V(JSDOC, UNEXPECT_REST)                     \
  V(JSDOC, UNEXPECT_TAG)                      \
  V(JSDOC, UNEXPECT_VISIBILITY)               \
  V(ENVIRONMENT, EXPECT_METHOD)               \
  V(ENVIRONMENT, EXPECT_NAME)                 \
  V(ENVIRONMENT, EXPECT_OBJECT_MEMBER)        \
  V(ENVIRONMENT, EXPECT_PROPERTY)             \
  V(ENVIRONMENT, EXPECT_VARIABLE_DECLARATION) \
  V(ENVIRONMENT, INVALID_CONSTRUCTOR)         \
  V(ENVIRONMENT, MULTIPLE_OCCURRENCES)        \
  V(ENVIRONMENT, UNDEFINED_TYPE)              \
  V(ENVIRONMENT, UNDEFINED_VARIABLE)          \
  V(ENVIRONMENT, UNEXPECT_INITIALIZER)        \
  V(ENVIRONMENT, UNEXPECT_ANNOTATION)         \
  V(TYPE_CHECKER, UNINITIALIZED_VARIABLE)     \
  V(TYPE_RESOLVER, EXPECT_ARRAY_CLASS)        \
  V(TYPE_RESOLVER, EXPECT_CLASS)              \
  V(TYPE_RESOLVER, EXPECT_CLASS_TYPE)         \
  V(TYPE_RESOLVER, EXPECT_INTERFACE)          \
  V(TYPE_RESOLVER, EXPECT_OBJECT_CLASS)       \
  V(TYPE_RESOLVER, EXPECT_RECORD_TYPE)        \
  V(TYPE_RESOLVER, EXPECT_TUPLE_TYPE)         \
  V(TYPE_RESOLVER, MISMATCHED_TYPES)          \
  V(TYPE_RESOLVER, MULTIPLE_OCCURRENCES)      \
  V(TYPE_RESOLVER, UNEXPECT_BINDING)          \
  V(TYPE_RESOLVER, UNEXPECT_EXTENDS)          \
  V(TYPE_RESOLVER, UNEXPECT_IMPLEMENTS)

const auto kAnalyzerErrorCodeBase = 70000;

namespace analyzer {

JOANA_ANALYZER_EXPORT base::StringPiece ErrorStringOf(int error_code);

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_
