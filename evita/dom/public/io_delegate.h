// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_delegate_h)
#define INCLUDE_evita_dom_public_io_delegate_h

#include "base/callback_forward.h"
#include "evita/dom/public/io_callback.h"

namespace domapi {

class IoDelegate {
  public: typedef base::Callback<
      void(const domapi::QueryFileStatusCallbackData&)> QueryFileStatusCallback;

  protected: IoDelegate();
  public: virtual ~IoDelegate();

  public: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusCallback& callback) = 0;

  DISALLOW_COPY_AND_ASSIGN(IoDelegate);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_delegate_h)
