// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_SELECTOR_H_
#define EVITA_VISUALS_CSS_SELECTOR_H_

#include <iosfwd>
#include <vector>

#include "evita/base/strings/atomic_string.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// Selector
//
class Selector final {
 public:
  class Builder;
  class Parser;

  Selector(const Selector& other);
  Selector(Selector&& other);
  Selector();
  ~Selector();

  Selector& operator=(const Selector& other);
  Selector& operator=(Selector&& other);

  bool operator==(const Selector& other) const;
  bool operator!=(const Selector& other) const;
  bool operator<(const Selector& other) const;

  base::AtomicString tag_name() const { return tag_name_; }
  base::AtomicString id() const { return id_; }
  const std::vector<base::AtomicString> classes() const { return classes_; }

 private:
  std::vector<base::AtomicString> classes_;
  base::AtomicString id_;
  base::AtomicString tag_name_;
};

std::ostream& operator<<(std::ostream& ostream, const Selector& selector);

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_SELECTOR_H_
