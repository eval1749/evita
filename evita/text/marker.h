// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_marker_h)
#define INCLUDE_evita_text_marker_h

#include <functional>
#include <ostream>
#include "base/basictypes.h"

namespace text {

class MarkerSet;

class Marker {
  friend class MarkerSet;

  public: static const int None = 0;
  private: Posn end_;
  private: Posn start_;
  private: int type_;

  public: Marker(Posn start, Posn end, int type);
  public: Marker(const Marker& other);
  private: Marker(Posn start);
  public: Marker();
  public: ~Marker();

  public: bool operator==(const Marker& other) const;
  public: bool operator!=(const Marker& other) const;

  public: Posn end() const { return end_; }
  public: Posn start() const { return start_; }
  public: int type() const { return type_; }

  DISALLOW_ASSIGN(Marker);
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
