// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/switch_value.h"

#include <ostream>

#include "base/logging.h"

namespace std {
ostream& operator<<(ostream& ostream, domapi::SwitchValue::Type type) {
  return ostream << static_cast<int>(type);
}
}   // namespace std


namespace domapi {

SwitchValue::SwitchValue(bool value) : int_value_(value), type_(Type::Bool) {
}

SwitchValue::SwitchValue(int value) : int_value_(value), type_(Type::Int) {
}

SwitchValue::SwitchValue(const base::string16& value) 
    : str_value_(value), type_(Type::String) {
}

SwitchValue::SwitchValue(const SwitchValue& other)
    : int_value_(other.int_value_), str_value_(other.str_value_),
      type_(other.type_) {
}

SwitchValue::SwitchValue(SwitchValue&& other)
    : int_value_(other.int_value_),
      str_value_(std::move(other.str_value_)), type_(other.type_) {
  other.type_ = Type::Void;
}

SwitchValue::SwitchValue() : int_value_(0), type_(Type::Void) {
}

SwitchValue::~SwitchValue() {
}

SwitchValue& SwitchValue::operator=(const SwitchValue& other) {
  int_value_ = other.int_value_;
  str_value_ = other.str_value_;
  type_ = other.type_;
  return *this;
}

SwitchValue& SwitchValue::operator=(SwitchValue&& other) {
  int_value_ = other.int_value_;
  str_value_ = std::move(other.str_value_);
  type_ = other.type_;
  other.int_value_ = 0;
  other.type_ = Type::Void;
  return *this;
}

bool SwitchValue::bool_value() const {
  DCHECK_EQ(Type::Bool, type_);
  return int_value_ != 0;
}

int SwitchValue::int_value() const {
  DCHECK_EQ(Type::Int, type_);
  return int_value_;
}

const base::string16& SwitchValue::string_value() const {
  DCHECK_EQ(Type::String, type_);
  return str_value_;
}

}  // namespace domapi
