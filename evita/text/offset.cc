// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/offset.h"

#include "base/logging.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// OffsetDelta
//
OffsetDelta::OffsetDelta(int value) : value_(value) {}
OffsetDelta::OffsetDelta(size_t value) : value_(static_cast<int>(value)) {}
OffsetDelta::OffsetDelta(const OffsetDelta& other) : value_(other.value_) {}
OffsetDelta::OffsetDelta() : OffsetDelta(0) {}
OffsetDelta::~OffsetDelta() {}

OffsetDelta& OffsetDelta::operator=(const OffsetDelta& other) {
  value_ = other.value_;
  return *this;
}

bool OffsetDelta::operator==(const OffsetDelta& other) const {
  return value_ == other.value_;
}

bool OffsetDelta::operator!=(const OffsetDelta& other) const {
  return !operator==(other);
}

bool OffsetDelta::operator<(const OffsetDelta& other) const {
  return value_ < other.value_;
}

bool OffsetDelta::operator<=(const OffsetDelta& other) const {
  return value_ <= other.value_;
}

bool OffsetDelta::operator>(const OffsetDelta& other) const {
  return value_ > other.value_;
}

bool OffsetDelta::operator>=(const OffsetDelta& other) const {
  return value_ >= other.value_;
}

OffsetDelta OffsetDelta::operator+(const OffsetDelta& other) const {
  return OffsetDelta(value_ + other.value_);
}

OffsetDelta OffsetDelta::operator-(const OffsetDelta& other) const {
  return OffsetDelta(value_ - other.value_);
}

//////////////////////////////////////////////////////////////////////
//
// Offset
//
Offset::Offset(int value) : value_(value) {
  DCHECK_GE(value, 0);
}
Offset::Offset(const Offset& other) : value_(other.value_) {}
Offset::Offset() : Offset(0) {}
Offset::~Offset() {}

Offset& Offset::operator=(const Offset& other) {
  value_ = other.value_;
  return *this;
}

bool Offset::operator==(const Offset& other) const {
  return value_ == other.value_;
}

bool Offset::operator!=(const Offset& other) const {
  return !operator==(other);
}

bool Offset::operator<(const Offset& other) const {
  DCHECK(IsValid());
  DCHECK(other.IsValid());
  return value_ < other.value_;
}

bool Offset::operator<=(const Offset& other) const {
  DCHECK(IsValid());
  DCHECK(other.IsValid());
  return value_ <= other.value_;
}

bool Offset::operator>(const Offset& other) const {
  DCHECK(IsValid());
  DCHECK(other.IsValid());
  return value_ > other.value_;
}

bool Offset::operator>=(const Offset& other) const {
  DCHECK(IsValid());
  DCHECK(other.IsValid());
  return value_ >= other.value_;
}

Offset Offset::operator+(const OffsetDelta& delta) const {
  DCHECK(IsValid());
  return Offset(value_ + delta.value());
}

OffsetDelta Offset::operator-(const Offset& other) const {
  DCHECK(IsValid());
  DCHECK(other.IsValid());
  return OffsetDelta(value_ - other.value_);
}

Offset Offset::operator-(const OffsetDelta& delta) const {
  DCHECK(IsValid());
  return Offset(value_ - delta.value());
}

Offset& Offset::operator+=(const OffsetDelta& delta) {
  DCHECK(IsValid());
  value_ += delta.value();
  DCHECK_GE(value_, 0);
  return *this;
}

Offset& Offset::operator-=(const OffsetDelta& delta) {
  DCHECK(IsValid());
  value_ -= delta.value();
  DCHECK(IsValid());
  return *this;
}

Offset& Offset::operator++() {
  DCHECK(IsValid());
  ++value_;
  return *this;
}

Offset& Offset::operator--() {
  DCHECK(IsValid());
  --value_;
  DCHECK(IsValid());
  return *this;
}

// static
Offset Offset::Invalid() {
  auto invalid = Offset();
  invalid.value_ = -1;
  return invalid;
}

Offset Offset::Max() {
  return Offset((1 << 28) - 1);
}

}  // namespace text

namespace std {
size_t hash<text::Offset>::operator()(const text::Offset& offset) const {
  return std::hash<int>()(offset.value());
}

ostream& operator<<(ostream& ostream, const text::OffsetDelta& delta) {
  return ostream << delta.value();
}

ostream& operator<<(ostream& ostream, const text::Offset& offset) {
  return ostream << offset.value();
}

}  // namespace std
