// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_callback_h)
#define INCLUDE_evita_dom_public_io_callback_h

#include "base/callback_forward.h"
#include "base/time/time.h"

namespace domapi {

template<typename ResolveType, typename RejectType> struct Deferred;
class FileId;
class IoContextId;
struct IoError;
class ProcessId;

struct QueryFileStatusCallbackData {
  int error_code;
  int file_size;
  bool is_directory;
  bool is_symlink;
  base::Time last_write_time;
  bool readonly;
};

typedef Deferred<int, IoError> FileIoDeferred;
typedef Deferred<ProcessId, IoError> NewProcessDeferred;
typedef Deferred<FileId, IoError> OpenFileDeferred;
typedef base::Callback<void(const domapi::QueryFileStatusCallbackData&)>
    QueryFileStatusCallback;

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_callback_h)
