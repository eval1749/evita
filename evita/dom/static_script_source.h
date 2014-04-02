// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_static_script_source_h)
#define INCLUDE_evita_dom_static_script_source_h

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

#endif //!defined(INCLUDE_evita_dom_static_script_source_h)
