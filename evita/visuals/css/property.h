// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_H_
#define EVITA_VISUALS_CSS_PROPERTY_H_

#include "base/macros.h"

namespace visuals {
namespace css {

enum class PropertyId;
class PropertyVisitor;

//////////////////////////////////////////////////////////////////////
//
// Property
//
class Property {
 public:
  virtual ~Property();

  virtual PropertyId id() const = 0;
  const char* name() const;

  virtual void Accept(PropertyVisitor* visitor) = 0;

 protected:
  Property();

 private:
  DISALLOW_COPY_AND_ASSIGN(Property);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_H_
