// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_SWITCH_VALUE_H_
#define EVITA_DOM_PUBLIC_SWITCH_VALUE_H_

#include "base/strings/string16.h"

namespace domapi {

class SwitchValue final {
 public:
  enum class Type {
    Bool,
    Int,
    String,
    Void,
  };

  explicit SwitchValue(bool value);
  explicit SwitchValue(int value);
  explicit SwitchValue(const base::string16& value);
  SwitchValue(const SwitchValue& other);
  SwitchValue(SwitchValue&& other);
  SwitchValue();
  ~SwitchValue();

  SwitchValue& operator=(const SwitchValue& other);
  SwitchValue& operator=(SwitchValue&& other);

  bool bool_value() const;
  int int_value() const;
  bool is_bool() const { return type_ == Type::Bool; }
  bool is_int() const { return type_ == Type::Int; }
  bool is_string() const { return type_ == Type::String; }
  const base::string16& string_value() const;
  Type type() const { return type_; }

 private:
  int int_value_;
  base::string16 str_value_;
  Type type_;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_SWITCH_VALUE_H_
