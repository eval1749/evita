// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_delegate_impl_h)
#define INCLUDE_evita_io_io_delegate_impl_h

#include "evita/dom/public/io_delegate.h"

#include <memory>

namespace io {

class IoDelegateImpl : public domapi::IoDelegate {
  private: class IoHandler;

  private: std::unique_ptr<IoHandler> io_handler_;

  public: IoDelegateImpl();
  public: virtual ~IoDelegateImpl();

  // domapi::IoDelegate
  private: virtual void CloseFile(domapi::IoHandle* io_handle);
  private: virtual void OpenFile(const base::string16& filename,
                                 const base::string16& mode,
                                 const OpenFileCallback& callback) override;
  private: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusCallback& callback) override;
  private: virtual void ReadFile(domapi::IoHandle* io_handle, uint8_t* buffer,
                                 size_t num_read,
                                 const FileIoCallback& callback) override;
  private: virtual void WriteFile(domapi::IoHandle* io_handle, uint8_t* buffer,
                                  size_t num_write,
                                  const FileIoCallback& callback) override;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_delegate_impl_h)
