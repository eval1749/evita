// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mock_io_delegate_h)
#define INCLUDE_evita_dom_mock_io_delegate_h

#include "evita/dom/public/io_delegate.h"
#pragma warning(push)
#pragma warning(disable: 4365 4628)
#include "gmock/gmock.h"
#pragma warning(pop)

namespace dom {

class MockIoDelegate : public domapi::IoDelegate {
  private: domapi::QueryFileStatusCallbackData data_;

  public: MockIoDelegate();
  public: virtual ~MockIoDelegate();

  public: void SetQueryFileStatusCallbackData(
      const domapi::QueryFileStatusCallbackData& data);

  // domapi::IoDelegate
  private: void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusCallback& callback) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_io_delegate_h)
