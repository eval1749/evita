// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_clipboard_data_transfer_data_h)
#define INCLUDE_evita_dom_clipboard_data_transfer_data_h

#include "base/basictypes.h"

#include <vector>

#include "base/strings/string16.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// DataTransferData
//
class DataTransferData {
  public: enum class Kind {
    Blob,
    String,
  };

  protected: DataTransferData();
  public: virtual ~DataTransferData();

  public: virtual const void* bytes() const = 0;
  public: virtual Kind kind() const = 0;
  public: virtual size_t num_bytes() const = 0;

  public: static const char* KindToString(Kind kind);

  DISALLOW_COPY_AND_ASSIGN(DataTransferData);
};

//////////////////////////////////////////////////////////////////////
//
// DataTransferBlobData
//
class DataTransferBlobData : public DataTransferData {
  private: const std::vector<uint8_t> data_;

  public: DataTransferBlobData(const void* bytes, size_t num_bytes);
  public: virtual ~DataTransferBlobData();

  public: virtual const void* bytes() const override;
  public: virtual Kind kind() const override;
  public: virtual size_t num_bytes() const override;;

  DISALLOW_COPY_AND_ASSIGN(DataTransferBlobData);
};

//////////////////////////////////////////////////////////////////////
//
// DataTransferStringData
//
class DataTransferStringData : public DataTransferData {
  private: const base::string16 data_;

  public: DataTransferStringData(const base::string16& string);
  public: virtual ~DataTransferStringData();

  public: virtual const void* bytes() const override;
  public: virtual Kind kind() const override;
  public: virtual size_t num_bytes() const override;;

  DISALLOW_COPY_AND_ASSIGN(DataTransferStringData);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_clipboard_data_transfer_data_h)
