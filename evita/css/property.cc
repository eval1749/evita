// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <vector>

#include "evita/css/property.h"

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "evita/css/properties.h"
#include "evita/css/values/value.h"

namespace css {

namespace {
//////////////////////////////////////////////////////////////////////
//
// PropertyNames
//
class PropertyNames final {
 public:
  PropertyNames();
  ~PropertyNames() = default;

  const base::string16& Get(PropertyId id) const {
    return names_[static_cast<size_t>(id)];
  }

  const char* GetAscii(PropertyId id) const {
    const auto index = static_cast<size_t>(id);
    if (index >= ascii_names_.size())
      return "???";
    return ascii_names_[index];
  }

  static PropertyNames* GetInstance() {
    return base::Singleton<PropertyNames>::get();
  }

 private:
  std::array<const char*, kMaxPropertyId + 1> ascii_names_;
  std::vector<base::string16> names_;

  DISALLOW_COPY_AND_ASSIGN(PropertyNames);
};

PropertyNames::PropertyNames() : names_(kMaxPropertyId + 1) {
#define V(Name, name, type, text)                          \
  names_[static_cast<size_t>(PropertyId::Name)] = L##text; \
  ascii_names_[static_cast<size_t>(PropertyId::Name)] = text;
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Property
//
Property::Property(PropertyId id, const Value& value)
    : id_(id), value_(value) {}

Property::Property(PropertyId id, Value&& value)
    : id_(id), value_(std::move(value)) {}

Property::Property(const Property& other) : Property(other.id_, other.value_) {}

Property::Property(Property&& other)
    : Property(other.id_, std::move(other.value_)) {}

Property::Property() : Property(PropertyId::Invalid, Value()) {}
Property::~Property() {}

Property Property::operator=(const Property& other) {
  id_ = other.id_;
  value_ = other.value_;
  return *this;
}

Property Property::operator=(Property&& other) {
  id_ = other.id_;
  value_ = std::move(other.value_);
  return *this;
}

bool Property::operator==(const Property& other) const {
  return id_ == other.id_ && value_ == other.value_;
}

bool Property::operator!=(const Property& other) const {
  return !operator==(other);
}

const char* Property::AsciiNameOf(PropertyId id) {
  return PropertyNames::GetInstance()->GetAscii(id);
}

const base::string16& Property::NameOf(PropertyId id) {
  return PropertyNames::GetInstance()->Get(id);
}

}  // namespace css
