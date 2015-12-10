// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_IO_CALLBACK_H_
#define EVITA_DOM_PUBLIC_IO_CALLBACK_H_

#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "base/time/time.h"

namespace domapi {

template <typename ResolveType, typename RejectType>
struct Promise;
class DirectoryId;
class FileId;
class IoContextId;
struct IoError;
class ProcessId;

struct FileStatus final {
  int file_size;
  bool is_directory;
  bool is_symlink;
  base::Time last_write_time;
  base::string16 name;
  bool readonly;
};

struct MoveFileOptions final {
  bool no_overwrite;
};

using IoBoolPromise = Promise<bool, IoError>;
using IoIntPromise = Promise<int, IoError>;
using MakeTempFileNamePromise = Promise<base::string16, IoError>;
using OpenDirectoryPromise = Promise<DirectoryId, IoError>;
using OpenFilePromise = Promise<FileId, IoError>;
using OpenProcessPromise = Promise<ProcessId, IoError>;
using QueryFileStatusPromise = Promise<FileStatus, IoError>;
using ReadDirectoryPromise = Promise<const std::vector<FileStatus>&, IoError>;

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_IO_CALLBACK_H_
