// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_SELECTOR_H_
#define EVITA_CSS_SELECTOR_H_

#include <iosfwd>
#include <set>

#include "evita/base/strings/atomic_string.h"

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

  // Returns true if |this| selector is more specific than |other|.
  bool operator<(const Selector& other) const;

  bool has_classes() const { return !classes_.empty(); }
  bool has_id() const { return !id_.empty(); }
  base::AtomicString id() const { return id_; }
  bool is_universal() const { return tag_name_.empty(); }
  base::AtomicString tag_name() const { return tag_name_; }
  const std::set<base::AtomicString>& classes() const { return classes_; }

  bool IsMoreSpecific(const Selector& other) const;

  // Returns true if set of elements selected by |this| selector is subset of
  // elements selected by |other|.
  bool IsSubsetOf(const Selector& other) const;

  base::string16 ToString() const;

 private:
  std::set<base::AtomicString> classes_;
  base::AtomicString id_;
  base::AtomicString tag_name_;
};

std::ostream& operator<<(std::ostream& ostream, const Selector& selector);

}  // namespace css

#endif  // EVITA_CSS_SELECTOR_H_
