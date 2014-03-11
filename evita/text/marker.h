// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_marker_h)
#define INCLUDE_evita_text_marker_h

#include <functional>
#include "base/basictypes.h"

namespace text {

class MarkerSet;

class Marker {
  friend class MarkerSet;

  public: static int None = 0;
  private: Posn end_;
  private: Posn start_;
  private: int type_;

  public: Marker(Type type, Posn start, Posn end);
  private: Marker(Posn start);
  private: Marker();
  public: ~Marker();

  public: Posn start() const { return start_; }
  public: int type() const { return type_; }

  DISALLOW_COPY_AND_ASSIGN(Marker);
};

}  // namespace text

namespace std {
template<>
struct less<text::Marker*> {
  bool operator() (const text::Marker* x, const text::Marker* y) const {
    return x->start() < y->start();
  }
};
}  // namespace

#endif // !defined(INCLUDE_evita_text_marker_h)
