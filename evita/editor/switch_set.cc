// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/editor/switch_set.h"

#include <unordered_set>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace editor {

SwitchSet::SwitchSet() {
}

SwitchSet::~SwitchSet() {
}

std::vector<base::string16> SwitchSet::names() const {
  std::vector<base::string16> names(map_.size());
  names.resize(0);
  for (const auto& key_value : map_) {
    names.push_back(key_value.first);
  }
  return std::move(names);
}

domapi::SwitchValue SwitchSet::Get(const base::string16& name) const {
  auto const it = map_.find(name);
  if (it == map_.end())
    return domapi::SwitchValue();
  switch (it->second.type) {
    case domapi::SwitchValue::Type::Bool:
      return domapi::SwitchValue(*reinterpret_cast<bool*>(it->second.pointer));
    case domapi::SwitchValue::Type::Int:
      return domapi::SwitchValue(*reinterpret_cast<int*>(it->second.pointer));
    case domapi::SwitchValue::Type::String:
      return domapi::SwitchValue(*reinterpret_cast<base::string16*>(
          it->second.pointer));
    case domapi::SwitchValue::Type::Void:
      return domapi::SwitchValue();
  }
  return domapi::SwitchValue();
}

void SwitchSet::Register(const base::StringPiece&  ascii_name,
                         bool* pointer) {
  auto const name = base::ASCIIToUTF16(ascii_name);
  DCHECK(map_.find(name) == map_.end());
  ValueLocation location;
  location.type = domapi::SwitchValue::Type::Bool;
  location.pointer = pointer;
  map_[name] = location;
}

void SwitchSet::Register(const base::StringPiece&  ascii_name,
                         int* pointer) {
  auto const name = base::ASCIIToUTF16(ascii_name);
  DCHECK(map_.find(name) == map_.end());
  ValueLocation location;
  location.type = domapi::SwitchValue::Type::Int;
  location.pointer = pointer;
  map_[name] = location;
}

void SwitchSet::Set(const base::string16& name,
                    const domapi::SwitchValue& new_value) {
  auto const it = map_.find(name);
  if (it == map_.end())
    return;
  if (it->second.type != new_value.type())
    return;
  switch (it->second.type) {
    case domapi::SwitchValue::Type::Bool:
      *reinterpret_cast<bool*>(it->second.pointer) = new_value.bool_value();
      break;
    case domapi::SwitchValue::Type::Int:
      *reinterpret_cast<int*>(it->second.pointer) = new_value.int_value();
      break;
    case domapi::SwitchValue::Type::String:
      *reinterpret_cast<base::string16*>(it->second.pointer) =
          new_value.string_value();
      break;
    case domapi::SwitchValue::Type::Void:
      break;
  }
}

}  // namespace editor
