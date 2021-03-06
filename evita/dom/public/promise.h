// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_PROMISE_H_
#define EVITA_DOM_PUBLIC_PROMISE_H_

#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/strings/string16.h"

namespace domapi {

template <typename ResolveType, typename RejectType = int>
struct Promise final {
  base::OnceCallback<void(RejectType)> reject;
  base::OnceCallback<void(ResolveType)> resolve;
  int sequence_num = 0;

  Promise(Promise&& other) = default;
  Promise() = default;

  Promise& operator=(Promise&& other) = default;

  DISALLOW_COPY(Promise);
};

using IntegerPromise = domapi::Promise<int>;
using StringPromise = domapi::Promise<base::string16>;

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_PROMISE_H_
