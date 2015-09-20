// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_SCRIPT_HOST_STATE_H_
#define EVITA_DOM_PUBLIC_SCRIPT_HOST_STATE_H_

namespace domapi {

enum class ScriptHostState {
  Error,
  Running,
  Stopped,
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_SCRIPT_HOST_STATE_H_
