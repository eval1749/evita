// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_io_delegate_h)
#define INCLUDE_evita_dom_public_io_delegate_h

#include <vector>

#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_callback.h"
#include "evita/dom/public/io_context_id.h"

#undef MoveFile

namespace domapi {

class IoDelegate {
  public: typedef domapi::Deferred<bool> CheckSpellingResolver;

  public: typedef domapi::Deferred<std::vector<base::string16>>
      GetSpellingSuggestionsResolver;

  protected: IoDelegate();
  public: virtual ~IoDelegate();

  // Check spelling of |word_to_check|.
  public: virtual void CheckSpelling(const base::string16& word_to_check,
        const CheckSpellingResolver& callback) = 0;

  public: virtual void CloseFile(IoContextId context_id,
                                 const FileIoDeferred& deferred) = 0;

  // Returns list of suggested words for |wrong_word|.
  public: virtual void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& callback) = 0;

  // Make temporary file and returns its name.
  public: virtual void MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const MakeTempFileNameResolver& resolver) = 0;

  // Move file
  public: virtual void MoveFile(
      const base::string16& src_path, const base::string16& dst_path,
      const MoveFileOptions& options,
      const IoResolver& resolver) = 0;

  public: virtual void OpenFile(const base::string16& filename,
                                const base::string16& mode,
                                const OpenFileDeferred& deferred) = 0;
  public: virtual void OpenProcess(const base::string16& command_line,
                                   const OpenProcessDeferred& deferred) = 0;
  public: virtual void QueryFileStatus(const base::string16& filename,
      const QueryFileStatusDeferred& deferred) = 0;
  public: virtual void ReadFile(IoContextId context_id, void* buffer,
                                size_t num_read,
                                const FileIoDeferred& deferred) = 0;
  // Remove file
  public: virtual void RemoveFile(
      const base::string16& file_name,
      const IoResolver& resolver) = 0;

  public: virtual void WriteFile(IoContextId context_id, void* buffer,
                                 size_t num_write,
                                 const FileIoDeferred& deferred) = 0;

  DISALLOW_COPY_AND_ASSIGN(IoDelegate);
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_io_delegate_h)
