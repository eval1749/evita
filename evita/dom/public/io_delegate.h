// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_IO_DELEGATE_H_
#define EVITA_DOM_PUBLIC_IO_DELEGATE_H_

#include <stdint.h>

#include <vector>

#include "base/strings/string16.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_context_id.h"
#include "evita/dom/public/promise.h"

#undef MoveFile

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// IoDelegate
//
class IoDelegate {
 public:
  using CheckSpellingResolver = domapi::Promise<bool>;

  using GetSpellingSuggestionsResolver =
      domapi::Promise<std::vector<base::string16>>;

  virtual ~IoDelegate();

  // Check spelling of |word_to_check|.
  virtual void CheckSpelling(const base::string16& word_to_check,
                             const CheckSpellingResolver& callback) = 0;

  virtual void CloseContext(const IoContextId& context_id,
                            const IoIntPromise& promise) = 0;

  // Returns list of suggested words for |wrong_word|.
  virtual void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& callback) = 0;

  virtual void LoadWinResource(const WinResourceId& resource_id,
                               const base::string16& type,
                               const base::string16& name,
                               uint8_t* buffer,
                               size_t buffer_size,
                               const IoIntPromise& promise) = 0;

  // Make temporary file and returns its name.
  virtual void MakeTempFileName(const base::string16& dir_name,
                                const base::string16& prefix,
                                const MakeTempFileNamePromise& resolver) = 0;

  // Move file
  virtual void MoveFile(const base::string16& src_path,
                        const base::string16& dst_path,
                        const MoveFileOptions& options,
                        const IoBoolPromise& resolver) = 0;

  // Open directory for reading entries
  virtual void OpenDirectory(const base::string16& dir_name,
                             const OpenDirectoryPromise& promise) = 0;

  virtual void OpenFile(const base::string16& file_name,
                        const base::string16& mode,
                        const OpenFilePromise& promise) = 0;
  virtual void OpenProcess(const base::string16& command_line,
                           const OpenProcessPromise& promise) = 0;
  virtual void QueryFileStatus(const base::string16& file_name,
                               const QueryFileStatusPromise& promise) = 0;

  virtual void OpenWinResource(const base::string16& file_name,
                               const OpenWinResourcePromise& promise) = 0;

  virtual void ReadDirectory(IoContextId context_id,
                             size_t num_read,
                             const ReadDirectoryPromise& promise) = 0;

  virtual void ReadFile(IoContextId context_id,
                        void* buffer,
                        size_t num_read,
                        const IoIntPromise& promise) = 0;
  // Remove file
  virtual void RemoveFile(const base::string16& file_name,
                          const IoBoolPromise& resolver) = 0;

  virtual void WriteFile(IoContextId context_id,
                         void* buffer,
                         size_t num_write,
                         const IoIntPromise& promise) = 0;

 protected:
  IoDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(IoDelegate);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_IO_DELEGATE_H_
