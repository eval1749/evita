// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_mock_io_delegate_h)
#define INCLUDE_evita_dom_mock_io_delegate_h

#include <deque>
#include <ostream>
#include <vector>

#include "base/strings/string_piece.h"
#include "evita/dom/public/deferred.h"
#include "evita/dom/public/io_delegate.h"
#pragma warning(push)
#pragma warning(disable: 4365 4628)
#include "gmock/gmock.h"
#pragma warning(pop)

namespace dom {

class MockIoDelegate : public domapi::IoDelegate {
  private: struct CallResult {
    int error_code;
    base::StringPiece name;
    int num_transferred;
  };

  private: std::vector<uint8_t> bytes_;
  private: domapi::IoContextId context_id_;
  private: std::deque<CallResult> call_results_;
  private: domapi::FileStatus file_status_;
  private: int num_close_called_;
  private: base::string16 temp_file_name_;

  public: MockIoDelegate();
  public: virtual ~MockIoDelegate();

  public: const std::vector<uint8_t>& bytes() const { return bytes_; }
  public: void set_bytes(const std::vector<uint8_t> new_bytes);
  public: int num_close_called() const { return num_close_called_; }

  private: CallResult PopCallResult(const base::StringPiece& name);
  public: void SetCallResult(const base::StringPiece& name, int error_code);
  public: void SetCallResult(const base::StringPiece& name, int erorr_code,
                             int num_transferred);
  public: void SetMakeTempFileName(const base::string16 file_name,
                                   int error_code);
  public: void SetFileStatus(const domapi::FileStatus& data, int error_code);
  public: void SetOpenFileResult(domapi::IoContextId context_id,
                                 int error_code);

  // domapi::IoDelegate
  public: virtual void CloseFile(domapi::IoContextId,
                                 const domapi::FileIoDeferred& deferred);
  public: virtual void MakeTempFileName(
      const base::string16& dir_name, const base::string16& prefix,
      const domapi::MakeTempFileNameResolver& resolver) override;
  public: virtual void MoveFile(const base::string16& src_path,
                                const base::string16& dst_path,
                                const domapi::MoveFileOptions& options,
                                const domapi::IoResolver& resolver) override;
  public: virtual void OpenFile(
      const base::string16& file_name, const base::string16& mode,
      const domapi::OpenFileDeferred&) override;
  public: virtual void OpenProcess(const base::string16& command_line,
      const domapi::OpenProcessDeferred&) override;
  private: virtual void QueryFileStatus(const base::string16& filename,
      const domapi::QueryFileStatusDeferred& deferred) override;
  public: virtual void ReadFile(domapi::IoContextId context_id, void* buffer,
                        size_t num_read,
                        const domapi::FileIoDeferred& deferred) override;
  public: virtual void RemoveFile(const base::string16& file_name,
                                  const domapi::IoResolver& resolver) override;
  public: virtual void WriteFile(domapi::IoContextId context_id, void* buffer,
                         size_t num_write,
                         const domapi::FileIoDeferred& deferred) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_mock_io_delegate_h)
