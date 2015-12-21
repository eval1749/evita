// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "evita/text/marker.h"

#include "base/logging.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Marker
//
Marker::Marker(Offset start, Offset end, const common::AtomicString& type)
    : end_(end), start_(start), type_(type) {
  DCHECK(!type.empty());
  DCHECK_LT(start, end);
}

Marker::Marker(const Marker& other)
    : end_(other.end_), start_(other.start_), type_(other.type_) {}

Marker::Marker(Offset offset)
    : end_(offset), start_(offset), type_(common::AtomicString::Empty()) {}

Marker::Marker() : Marker(Offset()) {}

Marker::~Marker() {}

Marker& Marker::operator=(const Marker& other) {
  end_ = other.end_;
  start_ = other.start_;
  type_ = other.type_;
  return *this;
}

bool Marker::operator==(const Marker& other) const {
  return type_ == other.type_ && start_ == other.start_ && end_ == other.end_;
}

bool Marker::operator!=(const Marker& other) const {
  return !operator==(other);
}

bool Marker::Contains(Offset offset) const {
  return offset >= start_ && offset < end_;
}

//////////////////////////////////////////////////////////////////////
//
// Marker::Editor
//
Marker::Editor::Editor(Marker* marker) : marker_(marker) {}
Marker::Editor::~Editor() {}

void Marker::Editor::SetEnd(Offset new_end) {
  DCHECK_NE(marker_->end_, new_end);
  DCHECK_LT(marker_->start_, new_end);
  marker_->end_ = new_end;
}

void Marker::Editor::SetRange(Offset new_start, Offset new_end) {
  DCHECK_LT(new_start, new_end);
  marker_->start_ = new_start;
  marker_->end_ = new_end;
}

void Marker::Editor::SetStart(Offset new_start) {
  DCHECK_NE(marker_->start_, new_start);
  DCHECK_LT(new_start, marker_->end_);
  marker_->start_ = new_start;
}

void Marker::Editor::SetType(const common::AtomicString& type) {
  DCHECK(!type.empty());
  marker_->type_ = type;
}

std::ostream& operator<<(std::ostream& ostream, const Marker& marker) {
  if (marker.start() == marker.end())
    return ostream << "text::Marker()";
  return ostream << "text::Marker(" << marker.type() << ", [" << marker.start()
                 << ", " << marker.end() << "])";
}

std::ostream& operator<<(std::ostream& ostream, const Marker* marker) {
  if (!marker)
    return ostream << "NullMarker";
  return ostream << *marker;
}

}  // namespace text
