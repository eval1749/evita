// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_delegate_h)
#define INCLUDE_evita_dom_public_io_delegate_h

#include "base/callback_forward.h"
#include "evita/dom/public/io_callback.h"

namespace domapi {

struct IoHandle;

class IoDelegate {
  public: typedef base::Callback<
      void(uint32_t num_transfered, int error_code)> FileIoCallback;

  public: typedef base::Callback<
      void(const domapi::QueryFileStatusCallbackData&)> QueryFileStatusCallback;

  public: typedef base::Callback<void(IoHandle* handle, int error_code)>
      OpenFileCallback;

  protected: IoDelegate();
  public: virtual ~IoDelegate();

  public: virtual void CloseFile(IoHandle* io_handle) = 0;
  public: virtual void OpenFile(const base::string16& filename,
                                const base::string16& mode,
                                const OpenFileCallback& callback) = 0;
  public: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusCallback& callback) = 0;
  public: virtual void ReadFile(IoHandle* io_handle, uint8_t* buffer,
                                size_t num_read,
                                const FileIoCallback& callback) = 0;
  public: virtual void WriteFile(IoHandle* io_handle, uint8_t* buffer,
                                 size_t num_write,
                                 const FileIoCallback& callback) = 0;

  DISALLOW_COPY_AND_ASSIGN(IoDelegate);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_delegate_h)
