// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_marker_h)
#define INCLUDE_evita_text_marker_h

#include <functional>
#include <ostream>

#include "base/basictypes.h"
#include "common/strings/atomic_string.h"

namespace text {

class MarkerSet;

class Marker {
  friend class MarkerSet;

  private: Posn end_;
  private: Posn start_;
  private: common::AtomicString type_;

  public: Marker(Posn start, Posn end, const common::AtomicString& type);
  public: Marker(const Marker& other);
  private: explicit Marker(Posn start);
  public: Marker();
  public: ~Marker();

  public: Marker& operator=(const Marker& other);

  public: bool operator==(const Marker& other) const;
  public: bool operator!=(const Marker& other) const;

  public: Posn end() const { return end_; }
  public: Posn start() const { return start_; }
  public: const common::AtomicString& type() const { return type_; }

  public: bool Contains(Posn offset) const;
};

}  // namespace text

namespace std {
template<>
struct less<text::Marker*> {
  bool operator() (const text::Marker* x, const text::Marker* y) const {
    return x->end() < y->end();
  }
};

ostream& operator<<(ostream& ostream, const text::Marker& marker);
ostream& operator<<(ostream& ostream, const text::Marker* marker);
}  // namespace std

#endif // !defined(INCLUDE_evita_text_marker_h)
