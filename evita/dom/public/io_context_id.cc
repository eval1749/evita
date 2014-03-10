// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/io_context_id.h"

#include "base/logging.h"

namespace domapi {

IoContextId::IoContextId(const IoContextId& other)
    : IoContextId(other.value_) {
}

IoContextId::IoContextId(int value) : value_(value) {
}

IoContextId::IoContextId() : IoContextId(0) {
}

IoContextId::~IoContextId() {
}

IoContextId& IoContextId::operator=(const IoContextId& other) {
  DCHECK(!value_);
  value_ = other.value_;
  return *this;
}

bool IoContextId::operator==(const IoContextId& other) const {
  return value_ == other.value_;
}

bool IoContextId::operator!=(const IoContextId& other) const {
  return !operator==(other);
}

IoContextId IoContextId::New() {
  static int static_value;
  ++static_value;
  return IoContextId(static_value);
}

//////////////////////////////////////////////////////////////////////
//
// FileId
//
FileId::FileId(IoContextId context_id)
    : IoContextId(context_id) {
}

FileId::~FileId() {
}

//////////////////////////////////////////////////////////////////////
//
// ProcessId
//
ProcessId::ProcessId(IoContextId context_id)
    : IoContextId(context_id) {
}

ProcessId::~ProcessId() {
}

}  // namespace domapi

namespace std {
size_t hash<domapi::IoContextId>::operator()(
    const domapi::IoContextId& context_id) const {
  return std::hash<int>()(context_id.value());
}
}  // namespace std
