// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_script_host_state_h)
#define INCLUDE_evita_dom_public_script_host_state_h

namespace domapi {

enum class ScriptHostState {
  Error,
  Running,
  Stopped,
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_script_host_state_h)
