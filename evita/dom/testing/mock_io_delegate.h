// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TESTING_MOCK_IO_DELEGATE_H_
#define EVITA_DOM_TESTING_MOCK_IO_DELEGATE_H_

#include <stdint.h>

#include <deque>
#include <string>
#include <vector>

#include "base/strings/string_piece.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/public/promise.h"
#include "testing/gmock/include/gmock/gmock.h"

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
  void SetCallResult(base::StringPiece name, int error_code);
  void SetCallResult(base::StringPiece name,
                     int erorr_code,
                     int num_transferred);
  void SetComputeFullPathName(base::StringPiece16 result, int error_code);
  void SetMakeTempFileName(base::StringPiece16 file_name, int error_code);
  void SetFileStatus(const domapi::FileStatus& data, int error_code);
  void SetOpenDirectoryResult(domapi::IoContextId context_id, int error_code);
  void SetOpenFileResult(domapi::IoContextId context_id, int error_code);
  void SetOpenResult(base::StringPiece name, int error_code);
  void SetReadDirectoryResult(const std::vector<domapi::FileStatus>& entries);
  void SetResourceResult(base::StringPiece operation,
                         int error_code,
                         base::StringPiece16 type,
                         base::StringPiece16 name,
                         const std::vector<uint8_t>& data);
  void SetStrings(base::StringPiece name,
                  int error_code,
                  const std::vector<base::string16>& strings);

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
                     domapi::OpenDirectoryPromise) final;
  void OpenFile(const base::string16& file_name,
                const base::string16& mode,
                domapi::OpenFilePromise) final;
  void OpenProcess(const base::string16& command_line,
                   domapi::OpenProcessPromise) final;
  void OpenWinResource(const base::string16& file_name,
                       domapi::OpenWinResourcePromise promise) final;
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

 private:
  struct CallResult final {
    int error_code;
    std::string name;
    int num_transferred;
  };

  CallResult PopCallResult(base::StringPiece name);
  void QueryFileStatus(const base::string16& file_name,
                       domapi::QueryFileStatusPromise promise) final;

  std::vector<uint8_t> bytes_;
  domapi::IoContextId context_id_;
  std::deque<CallResult> call_results_;
  std::vector<domapi::FileStatus> directory_entries_;
  domapi::FileStatus file_status_;
  int num_close_called_;
  int num_remove_called_;
  bool check_spelling_result_;
  std::vector<base::string16> strings_;
  std::vector<uint8_t> resource_data_;

  DISALLOW_COPY_AND_ASSIGN(MockIoDelegate);
};

}  // namespace dom

#endif  // EVITA_DOM_TESTING_MOCK_IO_DELEGATE_H_
