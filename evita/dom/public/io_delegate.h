// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_delegate_h)
#define INCLUDE_evita_dom_public_io_delegate_h

#include "base/callback_forward.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_context_id.h"

namespace domapi {

class IoContextId;

class IoDelegate {
  public: typedef base::Callback<
      void(int num_transfered, int error_code)> FileIoCallback;

  public: typedef base::Callback<void(IoContextId context_id, int error_code)>
      OpenFileCallback;

  public: typedef base::Callback<
      void(const domapi::QueryFileStatusCallbackData&)> QueryFileStatusCallback;

  protected: IoDelegate();
  public: virtual ~IoDelegate();

  public: virtual void CloseFile(IoContextId context_id) = 0;
  public: virtual void OpenFile(const base::string16& filename,
                                const base::string16& mode,
                                const OpenFileCallback& callback) = 0;
  public: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusCallback& callback) = 0;
  public: virtual void ReadFile(IoContextId context_id, void* buffer,
                                size_t num_read,
                                const FileIoCallback& callback) = 0;
  public: virtual void WriteFile(IoContextId context_id, void* buffer,
                                 size_t num_write,
                                 const FileIoCallback& callback) = 0;

  DISALLOW_COPY_AND_ASSIGN(IoDelegate);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_delegate_h)
