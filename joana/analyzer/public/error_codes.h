// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_
#define JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_

namespace joana {

// V(pass, reason)
#define FOR_EACH_ANALYZER_ERROR(V) V(ENVIRONMENT, MULTIPLE_BINDINGS)

const auto kAnalyzerErrorCodeBase = 70000;

}  // namespace joana

#endif  // JOANA_ANALYZER_PUBLIC_ERROR_CODES_H_
