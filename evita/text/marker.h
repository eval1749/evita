// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_MARKER_H_
#define EVITA_TEXT_MARKER_H_

#include <functional>
#include <ostream>

#include "base/basictypes.h"
#include "common/strings/atomic_string.h"
#include "evita/precomp.h"

namespace text {

class MarkerSet;

class Marker final {
 public:
  Marker(Posn start, Posn end, const common::AtomicString& type);
  Marker(const Marker& other);
  Marker();
  ~Marker();

  Marker& operator=(const Marker& other);

  bool operator==(const Marker& other) const;
  bool operator!=(const Marker& other) const;

  Posn end() const { return end_; }
  Posn start() const { return start_; }
  const common::AtomicString& type() const { return type_; }

  bool Contains(Posn offset) const;

 private:
  friend class MarkerSet;

  explicit Marker(Posn start);

  Posn end_;
  Posn start_;
  common::AtomicString type_;
};

}  // namespace text

namespace std {
template <>
struct less<text::Marker*> {
  bool operator()(const text::Marker* x, const text::Marker* y) const {
    return x->end() < y->end();
  }
};

ostream& operator<<(ostream& ostream, const text::Marker& marker);
ostream& operator<<(ostream& ostream, const text::Marker* marker);
}  // namespace std

#endif  // EVITA_TEXT_MARKER_H_
