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

struct FileStatus {
  int file_size;
  bool is_directory;
  bool is_symlink;
  base::Time last_write_time;
  bool readonly;
};

struct MoveFileOptions {
  bool no_overwrite;
};

typedef Deferred<bool, IoError> IoResolver;
typedef Deferred<int, IoError> FileIoDeferred;
typedef Deferred<base::string16, IoError> MakeTempFileNameResolver;
typedef Deferred<FileId, IoError> OpenFileDeferred;
typedef Deferred<ProcessId, IoError> OpenProcessDeferred;
typedef Deferred<FileStatus, IoError> QueryFileStatusDeferred;

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_callback_h)
