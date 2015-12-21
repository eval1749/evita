// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_MARKER_H_
#define EVITA_TEXT_MARKER_H_

#include <iosfwd>

#include "base/basictypes.h"
#include "common/strings/atomic_string.h"
#include "evita/text/offset.h"

namespace text {

class MarkerSet;

//////////////////////////////////////////////////////////////////////
//
// Marker
//
class Marker final {
 public:
  Marker(Offset start, Offset end, const common::AtomicString& type);
  Marker(const Marker& other);
  Marker();
  ~Marker();

  Marker& operator=(const Marker& other);

  bool operator==(const Marker& other) const;
  bool operator!=(const Marker& other) const;

  Offset end() const { return end_; }
  Offset start() const { return start_; }
  const common::AtomicString& type() const { return type_; }

  bool Contains(Offset offset) const;

 private:
  class Editor;

  explicit Marker(Offset start);

  Offset end_;
  Offset start_;
  common::AtomicString type_;
};

//////////////////////////////////////////////////////////////////////
//
// Marker::Editor
//
class Marker::Editor final {
 public:
  explicit Editor(Marker* marker);
  ~Editor();

  void SetEnd(Offset new_end);
  void SetRange(Offset start, Offset end);
  void SetStart(Offset new_start);
  void SetType(const common::AtomicString& type);

 private:
  Marker* const marker_;

  DISALLOW_COPY_AND_ASSIGN(Editor);
};

std::ostream& operator<<(std::ostream& ostream, const Marker& marker);
std::ostream& operator<<(std::ostream& ostream, const Marker* marker);

}  // namespace text

#endif  // EVITA_TEXT_MARKER_H_
