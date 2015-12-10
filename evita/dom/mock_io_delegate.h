// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_MOCK_IO_DELEGATE_H_
#define EVITA_DOM_MOCK_IO_DELEGATE_H_

#include <deque>
#include <ostream>
#include <vector>

#include "base/strings/string_piece.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_delegate.h"
#include "gmock/gmock.h"

namespace dom {

class MockIoDelegate final : public domapi::IoDelegate {
 public:
  MockIoDelegate();
  ~MockIoDelegate();

  const std::vector<uint8_t>& bytes() const { return bytes_; }
  void set_bytes(const std::vector<uint8_t> new_bytes);
  int num_close_called() const { return num_close_called_; }
  int num_remove_called() const { return num_remove_called_; }
  void set_check_spelling_result(bool result) {
    check_spelling_result_ = result;
  }
  void set_spelling_suggestions(
      const std::vector<base::string16>& spelling_suggestions) {
    spelling_suggestions_ = spelling_suggestions;
  }

  void SetCallResult(const base::StringPiece& name, int error_code);
  void SetCallResult(const base::StringPiece& name,
                     int erorr_code,
                     int num_transferred);
  void SetMakeTempFileName(const base::string16 file_name, int error_code);
  void SetFileStatus(const domapi::FileStatus& data, int error_code);
  void SetOpenDirectoryResult(domapi::IoContextId context_id, int error_code);
  void SetOpenFileResult(domapi::IoContextId context_id, int error_code);
  void SetReadDirectoryResult(const std::vector<domapi::FileStatus>& entries);

  // domapi::IoDelegate
  void CheckSpelling(const base::string16& word_to_check,
                     const CheckSpellingResolver& deferred) final;
  void CloseDirectory(domapi::IoContextId, const domapi::IoIntPromise& promise);
  void CloseFile(domapi::IoContextId, const domapi::IoIntPromise& deferred);
  void GetSpellingSuggestions(
      const base::string16& wrong_word,
      const GetSpellingSuggestionsResolver& deferred) final;
  void MakeTempFileName(const base::string16& dir_name,
                        const base::string16& prefix,
                        const domapi::MakeTempFileNameResolver& resolver) final;
  void MoveFile(const base::string16& src_path,
                const base::string16& dst_path,
                const domapi::MoveFileOptions& options,
                const domapi::IoBoolPromise& resolver) final;
  void OpenDirectory(const base::string16& dir_name,
                     const domapi::OpenDirectoryPromise&) final;
  void OpenFile(const base::string16& file_name,
                const base::string16& mode,
                const domapi::OpenFileDeferred&) final;
  void OpenProcess(const base::string16& command_line,
                   const domapi::OpenProcessDeferred&) final;
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

 private:
  struct CallResult final {
    int error_code;
    base::StringPiece name;
    int num_transferred;
  };

  CallResult PopCallResult(const base::StringPiece& name);
  void QueryFileStatus(const base::string16& file_name,
                       const domapi::QueryFileStatusDeferred& deferred) final;

  std::vector<uint8_t> bytes_;
  domapi::IoContextId context_id_;
  std::deque<CallResult> call_results_;
  std::vector<domapi::FileStatus> directory_entries_;
  domapi::FileStatus file_status_;
  int num_close_called_;
  int num_remove_called_;
  bool check_spelling_result_;
  std::vector<base::string16> spelling_suggestions_;
  base::string16 temp_file_name_;

  DISALLOW_COPY_AND_ASSIGN(MockIoDelegate);
};

}  // namespace dom

#endif  // EVITA_DOM_MOCK_IO_DELEGATE_H_
