// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CLIPBOARD_DATA_TRANSFER_DATA_H_
#define EVITA_DOM_CLIPBOARD_DATA_TRANSFER_DATA_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// DataTransferData
//
class DataTransferData {
 public:
  enum class Kind {
    Blob,
    String,
  };

  virtual ~DataTransferData();

  virtual const void* bytes() const = 0;
  virtual Kind kind() const = 0;
  virtual size_t num_bytes() const = 0;

  static const char* KindToString(Kind kind);

 protected:
  DataTransferData();

 private:
  DISALLOW_COPY_AND_ASSIGN(DataTransferData);
};

//////////////////////////////////////////////////////////////////////
//
// DataTransferBlobData
//
class DataTransferBlobData final : public DataTransferData {
 public:
  DataTransferBlobData(const void* bytes, size_t num_bytes);
  ~DataTransferBlobData() final;

  const void* bytes() const final;
  Kind kind() const final;
  size_t num_bytes() const final;

 private:
  const std::vector<uint8_t> data_;

  DISALLOW_COPY_AND_ASSIGN(DataTransferBlobData);
};

//////////////////////////////////////////////////////////////////////
//
// DataTransferStringData
//
class DataTransferStringData final : public DataTransferData {
 public:
  explicit DataTransferStringData(const base::string16& string);
  ~DataTransferStringData() final;

  const void* bytes() const final;
  Kind kind() const final;
  size_t num_bytes() const final;

 private:
  const base::string16 data_;

  DISALLOW_COPY_AND_ASSIGN(DataTransferStringData);
};

}  // namespace dom

#endif  // EVITA_DOM_CLIPBOARD_DATA_TRANSFER_DATA_H_
