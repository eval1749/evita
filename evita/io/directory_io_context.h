// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_DIRECTORY_IO_CONTEXT_H_
#define EVITA_IO_DIRECTORY_IO_CONTEXT_H_

#include <string>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/io/io_context.h"

namespace base {
class FileEnumerator;
}

namespace io {

//////////////////////////////////////////////////////////////////////
//
// DirectoryIoContext
//
class DirectoryIoContext final : public IoContext {
 public:
  explicit DirectoryIoContext(const base::string16& dir_name);
  ~DirectoryIoContext() final;

  void Close(const domapi::FileIoDeferred& promise);
  void Read(size_t num_read, const domapi::ReadDirectoryPromise& promise);

 private:
  const base::string16& dir_name_;
  const std::unique_ptr<base::FileEnumerator> file_enumerator_;

  DISALLOW_COPY_AND_ASSIGN(DirectoryIoContext);
};

}  // namespace io

#endif  // EVITA_IO_DIRECTORY_IO_CONTEXT_H_
