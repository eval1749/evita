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
                     const CheckSpellingResolver& promise) final;
  void CloseContext(const domapi::IoContextId& context_id,
                    const domapi::IoIntPromise& promise) final;
  void GetWinResourceNames(const domapi::WinResourceId& resource_id,
                           const base::string16& type,
                           const GetWinResourceNamessPromise& promise) final;
  void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& promise) final;
  void LoadWinResource(const domapi::WinResourceId& resource_id,
                       const base::string16& type,
                       const base::string16& name,
                       uint8_t* buffer,
                       size_t buffer_size,
                       const domapi::IoIntPromise& promise) final;
  void MakeTempFileName(const base::string16& dir_name,
                        const base::string16& prefix,
                        const domapi::MakeTempFileNamePromise& resolver) final;
  void MoveFile(const base::string16& src_path,
                const base::string16& dst_path,
                const domapi::MoveFileOptions& options,
                const domapi::IoBoolPromise& resolver) final;
  void OpenDirectory(const base::string16& dir_name,
                     const domapi::OpenDirectoryPromise& promise) final;
  void OpenFile(const base::string16& file_name,
                const base::string16& mode,
                const domapi::OpenFilePromise& promise) final;
  void OpenProcess(const base::string16& command_line,
                   const domapi::OpenProcessPromise& promise) final;
  void OpenWinResource(const base::string16& file_name,
                       const domapi::OpenWinResourcePromise& promise) final;
  void QueryFileStatus(const base::string16& file_name,
                       const domapi::QueryFileStatusPromise& promise) final;
  void ReadDirectory(domapi::IoContextId context_id,
                     size_t num_read,
                     const domapi::ReadDirectoryPromise& promise) final;
  void ReadFile(domapi::IoContextId context_id,
                void* buffer,
                size_t num_read,
                const domapi::IoIntPromise& promise) final;
  void RemoveFile(const base::string16& file_name,
                  const domapi::IoBoolPromise& resolver) final;
  void WriteFile(domapi::IoContextId context_id,
                 void* buffer,
                 size_t num_write,
                 const domapi::IoIntPromise& promise) final;

  std::unordered_map<domapi::IoContextId, IoContext*> context_map_;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif  // EVITA_IO_IO_DELEGATE_IMPL_H_
