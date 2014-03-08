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
  private: int error_code_;
  private: domapi::IoContextId context_id_;
  private: int num_transferred_;

  public: MockIoDelegate();
  public: virtual ~MockIoDelegate();

  public: void SetFileIoCallbackData(int num_transferred, int error_code);
  public: void SetOpenFileCallbackData(domapi::IoContextId context_id,
                                       int error_code);
  public: void SetQueryFileStatusCallbackData(
      const domapi::QueryFileStatusCallbackData& data);

  // domapi::IoDelegate
  MOCK_METHOD1(CloseFile, void(domapi::IoContextId));
  MOCK_METHOD2(NewProcess, void(const base::string16&,
                                const domapi::NewProcessCallback&));
  public: void OpenFile(const base::string16&, const base::string16&,
                        const domapi::OpenFileCallback&);
  private: void QueryFileStatus(const base::string16& filename,
      const domapi::QueryFileStatusCallback& callback) override;
  public: void ReadFile(domapi::IoContextId context_id, void* buffer,
                        size_t num_read,
                        const domapi::FileIoCallback& callback);
  public: void WriteFile(domapi::IoContextId context_id, void* buffer,
                         size_t num_write,
                         const domapi::FileIoCallback& callback);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_io_delegate_h)
