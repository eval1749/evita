// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/marker.h"

#include "base/logging.h"

namespace text {

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

}  // namespace text

namespace std {
ostream& operator<<(ostream& ostream, const text::Marker& marker) {
  return ostream << "text::Marker(" << marker.type().get() << ", ["
                 << marker.start() << ", " << marker.end() << "])";
}

ostream& operator<<(ostream& ostream, const text::Marker* marker) {
  if (!marker)
    return ostream << "NullMarker";
  return ostream << *marker;
}

}  // namespace std
