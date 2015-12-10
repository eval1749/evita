// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/io/directory_io_context.h"

#include "base/bind.h"
#include "base/files/file_enumerator.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/io/io_context_utils.h"

namespace io {

namespace {
std::unique_ptr<base::FileEnumerator> CreateFileEnumerator(
    const base::string16& dir_name) {
  const auto recursive = false;
  return std::make_unique<base::FileEnumerator>(
      base::FilePath(dir_name), recursive,
      base::FileEnumerator::FILES | base::FileEnumerator::DIRECTORIES);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DirectoryIoContext
//
DirectoryIoContext::DirectoryIoContext(const base::string16& dir_name)
    : dir_name_(dir_name), file_enumerator_(CreateFileEnumerator(dir_name)) {}

DirectoryIoContext::~DirectoryIoContext() {}

void DirectoryIoContext::Close(const domapi::IoIntPromise& promise) {
  Resolve(promise.resolve, 0);
}

void DirectoryIoContext::Read(size_t num_read,
                              const domapi::ReadDirectoryPromise& promise) {
  std::vector<domapi::FileStatus> entries;
  while (entries.size() < num_read) {
    const auto& file_path = file_enumerator_->Next();
    if (file_path.empty())
      break;
    const auto& file_info = file_enumerator_->GetInfo();
    domapi::FileStatus entry;
    entry.file_size = entry.is_directory = file_info.IsDirectory();
    entry.is_symlink = false;
    entry.last_write_time = file_info.GetLastModifiedTime();
    entry.name = file_info.GetName().value();
    entry.readonly =
        !!(file_info.find_data().dwFileAttributes & FILE_ATTRIBUTE_READONLY);
    entries.push_back(entry);
  }
  RunCallback(base::Bind(promise.resolve, entries));
}

}  // namespace io
