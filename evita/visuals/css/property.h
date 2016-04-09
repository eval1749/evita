// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_H_
#define EVITA_VISUALS_CSS_PROPERTY_H_

#include <iosfwd>
#include <string>

#include "base/strings/string16.h"
#include "evita/visuals/css/values/value.h"

namespace visuals {
namespace css {

enum class PropertyId : uint32_t;

//////////////////////////////////////////////////////////////////////
//
// Property
//
class Property final {
 public:
  // See "evita/visuals/css/property_editor.cc" for implementation.
  class Editor;

  Property(PropertyId id, const Value& other);
  Property(PropertyId id, Value&& other);
  Property(const Property& other);
  Property(Property&& other);
  Property();
  ~Property();

  Property operator=(const Property& other);
  Property operator=(Property&& other);

  bool operator==(const Property& other) const;
  bool operator!=(const Property& other) const;

  PropertyId id() const { return id_; }
  const base::string16& name() const { return NameOf(id_); }
  const char* ascii_name() const { return AsciiNameOf(id_); }
  const Value& value() const { return value_; }

  static const base::string16& NameOf(PropertyId id);
  static const char* AsciiNameOf(PropertyId id);

 private:
  PropertyId id_;
  Value value_;
};

static_assert(sizeof(Property) == sizeof(void*) * 2,
              "sizeof(Property) == sizeof(void*) * 2");

std::ostream& operator<<(std::ostream& ostream, const Property& property);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_H_
