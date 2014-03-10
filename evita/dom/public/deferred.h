// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_deferred_h)
#define INCLUDE_evita_dom_public_deferred_h

#include "base/callback_forward.h"

namespace domapi {

template<typename ResolveType, typename RejectType = int>
struct Deferred {
  base::Callback<void(RejectType)> reject;
  base::Callback<void(ResolveType)> resolve;
};
}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_deferred_h)
