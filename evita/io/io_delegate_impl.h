// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_IO_DELEGATE_IMPL_H_
#define EVITA_IO_IO_DELEGATE_IMPL_H_

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "evita/dom/public/io_delegate.h"

namespace io {

class IoContext;

class IoDelegateImpl final : public domapi::IoDelegate {
 public:
  IoDelegateImpl();
  ~IoDelegateImpl() final;

 private:
  IoContext* IoContextOf(domapi::IoContextId context_id) const;

  // domapi::IoDelegate
  void CheckSpelling(const base::string16& word_to_check,
                     CheckSpellingResolver promise) final;
  void CloseContext(const domapi::IoContextId& context_id,
                    domapi::IoIntPromise promise) final;
  void ComputeFullPathName(const base::string16& path_name,
                           ComputeFullPathNamePromise promise) final;
  void GetWinResourceNames(const domapi::WinResourceId& resource_id,
                           const base::string16& type,
                           GetWinResourceNamessPromise promise) final;
  void GetSpellingSuggestions(const base::string16& wrong_word,
                              GetSpellingSuggestionsResolver promise) final;
  void LoadWinResource(const domapi::WinResourceId& resource_id,
                       const base::string16& type,
                       const base::string16& name,
                       uint8_t* buffer,
                       size_t buffer_size,
                       domapi::IoIntPromise promise) final;
  void MakeTempFileName(const base::string16& dir_name,
                        const base::string16& prefix,
                        domapi::MakeTempFileNamePromise resolver) final;
  void MoveFile(const base::string16& src_path,
                const base::string16& dst_path,
                const domapi::MoveFileOptions& options,
                domapi::IoBoolPromise resolver) final;
  void OpenDirectory(const base::string16& dir_name,
                     domapi::OpenDirectoryPromise promise) final;
  void OpenFile(const base::string16& file_name,
                const base::string16& mode,
                domapi::OpenFilePromise promise) final;
  void OpenProcess(const base::string16& command_line,
                   domapi::OpenProcessPromise promise) final;
  void OpenWinResource(const base::string16& file_name,
                       domapi::OpenWinResourcePromise promise) final;
  void QueryFileStatus(const base::string16& file_name,
                       domapi::QueryFileStatusPromise promise) final;
  void ReadDirectory(domapi::IoContextId context_id,
                     size_t num_read,
                     domapi::ReadDirectoryPromise promise) final;
  void ReadFile(domapi::IoContextId context_id,
                void* buffer,
                size_t num_read,
                domapi::IoIntPromise promise) final;
  void RemoveFile(const base::string16& file_name,
                  domapi::IoBoolPromise resolver) final;
  void WriteFile(domapi::IoContextId context_id,
                 void* buffer,
                 size_t num_write,
                 domapi::IoIntPromise promise) final;

  std::unordered_map<domapi::IoContextId, IoContext*> context_map_;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif  // EVITA_IO_IO_DELEGATE_IMPL_H_
