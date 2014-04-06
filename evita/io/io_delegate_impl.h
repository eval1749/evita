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
        const domapi::FileIoDeferred& deferred) override;
  private: virtual void MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const domapi::MakeTempFileNameResolver& resolver) override;
  private: virtual void MoveFile(
      const base::string16& src_path, const base::string16& dst_path,
      const domapi::MoveFileOptions& options,
      const domapi::IoResolver& resolver) override;
  private: virtual void OpenFile(const base::string16& filename,
      const base::string16& mode,
      const domapi::OpenFileDeferred& deferred) override;
  private: virtual void OpenProcess(const base::string16& command_line,
        const domapi::OpenProcessDeferred& deferred) override;
  private: virtual void QueryFileStatus(const base::string16& filename,
      const domapi::QueryFileStatusDeferred& deferred) override;
  private: virtual void ReadFile(domapi::IoContextId context_id, void* buffer,
      size_t num_read, const domapi::FileIoDeferred& deferred) override;
  private: virtual void WriteFile(domapi::IoContextId context_id, void* buffer,
      size_t num_write, const domapi::FileIoDeferred& deferred) override;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_delegate_impl_h)
