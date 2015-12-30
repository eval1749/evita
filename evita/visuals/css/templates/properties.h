// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTIES_H_
#define EVITA_VISUALS_CSS_PROPERTIES_H_

#include <iosfwd>

#include "evita/visuals/css/properties_forward.h"
#include "evita/visuals/css/property.h"
#include "evita/visuals/css/values.h"

namespace visuals {
namespace css {

enum class PropertyId {
{% for property in properties %}
  {{property.Name}},
{% endfor %}
};

{% for property in properties %}
//////////////////////////////////////////////////////////////////////
//
// {{property.Name}} represents CSS property {{property.text}}
//
class {{property.Name}}Property final : public Property {
 public:
  ~{{property.Name}}Property() final;

  {{property.type.Return}} value() const { return value_; }

 private:
  friend class PropertyVisitor;

  explicit {{property.Name}}Property({{property.type.Parameter}} value);

  // Property
  PropertyId id() const final;

  void Accept(PropertyVisitor* visitor) final;

  {{property.type.Parameter}} value_;

  DISALLOW_COPY_AND_ASSIGN({{property.Name}}Property);
};

std::ostream& operator<<(std::ostream& ostream, const {{property.Name}}Property& {{property.name}});

{% endfor %}

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTIES_H_
