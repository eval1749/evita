// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_delegate_impl_h)
#define INCLUDE_evita_io_io_delegate_impl_h

#include "evita/dom/public/io_delegate.h"

#include <memory>
#include <unordered_map>

namespace io {

class IoContext;

class IoDelegateImpl : public domapi::IoDelegate {
  private: std::unordered_map<domapi::IoContextId, IoContext*> context_map_;

  public: IoDelegateImpl();
  public: virtual ~IoDelegateImpl();

  // domapi::IoDelegate
  private: virtual void CloseFile(domapi::IoContextId context_id,
        const domapi::CloseFileCallback& callback) override;
  private: virtual void NewProcess(const base::string16& command_line,
        const domapi::NewProcessCallback& callback) override;
  private: virtual void OpenFile(const base::string16& filename,
      const base::string16& mode,
      const domapi::OpenFileCallback& callback) override;
  private: virtual void QueryFileStatus(const base::string16& filename,
      const domapi::QueryFileStatusCallback& callback) override;
  private: virtual void ReadFile(domapi::IoContextId context_id, void* buffer,
      size_t num_read, const domapi::FileIoCallback& callback) override;
  private: virtual void WriteFile(domapi::IoContextId context_id, void* buffer,
      size_t num_write, const domapi::FileIoCallback& callback) override;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_delegate_impl_h)
