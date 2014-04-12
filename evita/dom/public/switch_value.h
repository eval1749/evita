// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_switch_value_h)
#define INCLUDE_evita_dom_public_switch_value_h

#include "base/strings/string16.h"

namespace domapi {

class SwitchValue {
  public: enum class Type {
    Bool,
    Int,
    String,
    Void,
  };

  private: int int_value_;
  private: base::string16 str_value_;
  private: Type type_;

  public: explicit SwitchValue(bool value);
  public: explicit SwitchValue(int value);
  public: explicit SwitchValue(const base::string16& value);
  public: SwitchValue(const SwitchValue& other);
  public: SwitchValue(SwitchValue&& other);
  public: SwitchValue();
  public: ~SwitchValue();

  public: SwitchValue& operator=(const SwitchValue& other);
  public: SwitchValue& operator=(SwitchValue&& other);

  public: bool bool_value() const;
  public: int int_value() const;
  public: bool is_bool() const { return type_ == Type::Bool; }
  public: bool is_int() const { return type_ == Type::Int; }
  public: bool is_string() const { return type_ == Type::String; }
  public: const base::string16& string_value() const;
  public: Type type() const { return type_; }
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_switch_value_h)
