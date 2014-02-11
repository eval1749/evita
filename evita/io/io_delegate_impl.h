// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_delegate_impl_h)
#define INCLUDE_evita_io_io_delegate_impl_h

#include "evita/dom/public/io_delegate.h"

namespace io {

class IoDelegateImpl : public domapi::IoDelegate {
  public: IoDelegateImpl();
  public: virtual ~IoDelegateImpl();

  private: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusCallback& callback) override;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_delegate_impl_h)
