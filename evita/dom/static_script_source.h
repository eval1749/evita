// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_STATIC_SCRIPT_SOURCE_H_
#define EVITA_DOM_STATIC_SCRIPT_SOURCE_H_

#include <vector>

namespace dom {
namespace internal {

struct StaticScriptSource {
  const char* file_name;
  const char* script_text;
};

const std::vector<StaticScriptSource>& GetJsLibSources();

}  // namespace internal
}  // namespace dom

#endif  // EVITA_DOM_STATIC_SCRIPT_SOURCE_H_
