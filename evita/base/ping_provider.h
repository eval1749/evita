// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_BASE_PING_PROVIDER_H_
#define EVITA_BASE_PING_PROVIDER_H_

#include <atomic>

#include "base/macros.h"
#include "evita/base/evita_base_export.h"

namespace base {

//////////////////////////////////////////////////////////////////////
//
// PingProvider
//
class EVITA_BASE_EXPORT PingProvider {
 public:
  virtual ~PingProvider();

  virtual void Ping(std::atomic<bool>* ping_to_false) = 0;

 protected:
  PingProvider();

  static void PingInternal(std::atomic<bool>* ping_to_false);

 private:
  DISALLOW_COPY_AND_ASSIGN(PingProvider);
};

}  // namespace base

#endif  // EVITA_BASE_PING_PROVIDER_H_
