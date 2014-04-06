// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_delegate_h)
#define INCLUDE_evita_dom_public_io_delegate_h

#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_context_id.h"

namespace domapi {

class IoDelegate {
  protected: IoDelegate();
  public: virtual ~IoDelegate();

  public: virtual void CloseFile(IoContextId context_id,
                                 const FileIoDeferred& deferred) = 0;

  // Make temporary file and returns its name.
  public: virtual void MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const domapi::MakeTempFileNameResolver& resolver) = 0;

  public: virtual void OpenFile(const base::string16& filename,
                                const base::string16& mode,
                                const OpenFileDeferred& deferred) = 0;
  public: virtual void OpenProcess(const base::string16& command_line,
                                   const OpenProcessDeferred& deferred) = 0;
  public: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusDeferred& deferred) = 0;
  public: virtual void ReadFile(IoContextId context_id, void* buffer,
                                size_t num_read,
                                const FileIoDeferred& deferred) = 0;
  public: virtual void WriteFile(IoContextId context_id, void* buffer,
                                 size_t num_write,
                                 const FileIoDeferred& deferred) = 0;

  DISALLOW_COPY_AND_ASSIGN(IoDelegate);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_delegate_h)
