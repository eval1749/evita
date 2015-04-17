// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_io_io_delegate_impl_h)
#define INCLUDE_evita_io_io_delegate_impl_h

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "evita/dom/public/io_delegate.h"

namespace io {

class IoContext;

class IoDelegateImpl final : public domapi::IoDelegate {
  private: std::unordered_map<domapi::IoContextId, IoContext*> context_map_;

  public: IoDelegateImpl();
  public: ~IoDelegateImpl() final;

  // domapi::IoDelegate
  private: void CheckSpelling(const base::string16& word_to_check,
        const CheckSpellingResolver& deferred) final;
  private: void CloseFile(domapi::IoContextId context_id,
        const domapi::FileIoDeferred& deferred) final;
  private: void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& deferred) final;
  private: void MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const domapi::MakeTempFileNameResolver& resolver) final;
  private: void MoveFile(
      const base::string16& src_path, const base::string16& dst_path,
      const domapi::MoveFileOptions& options,
      const domapi::IoResolver& resolver) final;
  private: void OpenFile(const base::string16& file_name,
      const base::string16& mode,
      const domapi::OpenFileDeferred& deferred) final;
  private: void OpenProcess(const base::string16& command_line,
        const domapi::OpenProcessDeferred& deferred) final;
  private: void QueryFileStatus(const base::string16& file_name,
      const domapi::QueryFileStatusDeferred& deferred) final;
  private: void ReadFile(domapi::IoContextId context_id, void* buffer,
      size_t num_read, const domapi::FileIoDeferred& deferred) final;
  private: void RemoveFile(
      const base::string16& file_name,
      const domapi::IoResolver& resolver) final;
  private: void WriteFile(domapi::IoContextId context_id, void* buffer,
      size_t num_write, const domapi::FileIoDeferred& deferred) final;

  DISALLOW_COPY_AND_ASSIGN(IoDelegateImpl);
};

}  // namespace io

#endif //!defined(INCLUDE_evita_io_io_delegate_impl_h)
