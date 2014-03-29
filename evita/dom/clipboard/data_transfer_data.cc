// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/clipboard/data_transfer_data.h"

#include <ostream>

#include "base/logging.h"

namespace std {
ostream& operator<<(ostream& ostream, dom::DataTransferData::Kind kind) {
  return ostream << dom::DataTransferData::KindToString(kind);
}
}  // namespace std

namespace dom {

namespace {
const char* const kind_strings[] = { "blob", "string" };
}

//////////////////////////////////////////////////////////////////////
//
// DataTransferData
//
DataTransferData::DataTransferData() {
}

DataTransferData::~DataTransferData() {
}

const char* DataTransferData::KindToString(Kind kind) {
  auto const index = static_cast<size_t>(kind);
  return index < arraysize(kind_strings) ? kind_strings[index] : "invalid";
}

//////////////////////////////////////////////////////////////////////
//
// DataTransferBlobgData
//
DataTransferBlobData::DataTransferBlobData(const void* bytes, size_t num_bytes)
    : data_(std::vector<uint8_t>(num_bytes)) {
  ::memcpy(const_cast<uint8_t*>(&data_[0]), bytes, num_bytes);
}

DataTransferBlobData::~DataTransferBlobData() {
}

const void* DataTransferBlobData::bytes() const {
  return data_.data();
}

DataTransferData::Kind DataTransferBlobData::kind() const {
  return Kind::Blob;
}

size_t DataTransferBlobData::num_bytes() const {
  return data_.size();
}

//////////////////////////////////////////////////////////////////////
//
// DataTransferStringData
//
DataTransferStringData::DataTransferStringData(const base::string16& string)
    : data_(string) {
}

DataTransferStringData::~DataTransferStringData() {
}

const void* DataTransferStringData::bytes() const {
  return data_.data();
}

DataTransferData::Kind DataTransferStringData::kind() const {
  return Kind::String;
}

size_t DataTransferStringData::num_bytes() const {
  return data_.size() * sizeof(data_[0]);
}


}  // namespace dom
