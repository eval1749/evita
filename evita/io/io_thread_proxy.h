// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_IO_IO_THREAD_PROXY_H_
#define EVITA_IO_IO_THREAD_PROXY_H_

#include "base/macros.h"
#include "evita/dom/public/io_delegate.h"

namespace base {
class Thread;
}

namespace io {

class IoThreadProxy : public domapi::IoDelegate {
 public:
  IoThreadProxy(domapi::IoDelegate* delegate, base::Thread* thread);
  ~IoThreadProxy() final;

 private:
  // domapi::IoDelegate
  void CheckSpelling(const base::string16& word_to_check,
                     const CheckSpellingResolver& deferred) final;
  void CloseDirectory(domapi::IoContextId context_id,
                      const domapi::IoIntPromise& deferred) final;
  void CloseFile(domapi::IoContextId context_id,
                 const domapi::IoIntPromise& deferred) final;
  void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& deferred) final;
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
                const domapi::OpenFilePromise& deferred) final;
  void OpenProcess(const base::string16& command_line,
                   const domapi::OpenProcessDeferred& deferred) final;
  void QueryFileStatus(const base::string16& file_name,
                       const domapi::QueryFileStatusDeferred& deferred) final;
  void ReadDirectory(domapi::IoContextId context_id,
                     size_t num_read,
                     const domapi::ReadDirectoryPromise& promise) final;
  void ReadFile(domapi::IoContextId context_id,
                void* buffer,
                size_t num_read,
                const domapi::IoIntPromise& deferred) final;
  void RemoveFile(const base::string16& file_name,
                  const domapi::IoBoolPromise& resolver) final;
  void WriteFile(domapi::IoContextId context_id,
                 void* buffer,
                 size_t num_write,
                 const domapi::IoIntPromise& deferred) final;

  domapi::IoDelegate* const delegate_;
  base::Thread* const thread_;

  DISALLOW_COPY_AND_ASSIGN(IoThreadProxy);
};

}  // namespace io

#endif  // EVITA_IO_IO_THREAD_PROXY_H_
