// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <ostream>

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property_visitor.h"

#include "base/logging.h"

namespace visuals {
namespace css {

{% for property in properties %}
//////////////////////////////////////////////////////////////////////
//
// CSS Property {{property.Name}}
//
{{property.Name}}Property::{{property.Name}}Property({{property.type.Parameter}} value) : value_(value) {}
{{property.Name}}Property::~{{property.Name}}Property() {}

PropertyId {{property.Name}}Property::id() const {
  return PropertyId::{{property.Name}};
}

std::ostream& operator<<(std::ostream& ostream,
                         const {{property.Name}}Property& {{property.name}}) {
  return ostream << "{{property.name}}:" << {{property.name}}.value();
}

{% endfor %}

}  // namespace css
}  // namespace visuals
