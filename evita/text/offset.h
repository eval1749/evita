// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <functional>
#include <iosfwd>

#ifndef EVITA_TEXT_OFFSET_H_
#define EVITA_TEXT_OFFSET_H_

namespace text {

//////////////////////////////////////////////////////////////////////
//
// OffsetDelta
//
class OffsetDelta final {
 public:
  explicit OffsetDelta(size_t delta);
  explicit OffsetDelta(int delta);
  OffsetDelta(const OffsetDelta& other);
  OffsetDelta();
  ~OffsetDelta();

  OffsetDelta& operator=(const OffsetDelta& other);

  operator int() const { return value_; }
  int value() const { return value_; }

  bool operator==(const OffsetDelta& other) const;
  bool operator!=(const OffsetDelta& other) const;

  bool operator<(const OffsetDelta& other) const;
  bool operator<=(const OffsetDelta& other) const;
  bool operator>(const OffsetDelta& other) const;
  bool operator>=(const OffsetDelta& other) const;

  OffsetDelta operator+(const OffsetDelta& other) const;
  OffsetDelta operator-(const OffsetDelta& other) const;

 private:
  int value_;
};

//////////////////////////////////////////////////////////////////////
//
// Offset
//
class Offset final {
 public:
  explicit Offset(int value);
  Offset(const Offset& other);
  Offset();
  ~Offset();

  Offset& operator=(const Offset& other);

  int value() const { return value_; }

  bool operator==(const Offset& other) const;
  bool operator!=(const Offset& other) const;

  bool operator<(const Offset& other) const;
  bool operator<=(const Offset& other) const;
  bool operator>(const Offset& other) const;
  bool operator>=(const Offset& other) const;

  Offset operator+(const OffsetDelta& delta) const;
  Offset operator+(const Offset& delta) const = delete;
  Offset operator+(int delta) const = delete;

  Offset operator-(const OffsetDelta& delta) const;
  OffsetDelta operator-(const Offset& other) const;
  Offset operator-(int delta) const = delete;

  Offset& operator+=(const OffsetDelta& delta);
  Offset& operator-=(const OffsetDelta& delta);

  Offset& operator++();
  Offset& operator--();

  bool IsValid() const { return value_ >= 0; }

  static Offset Invalid();
  static Offset Max();

 private:
  int value_;
};

}  // namespace text

namespace std {

template <>
struct hash<text::Offset> {
  size_t operator()(const text::Offset& offset) const;
};

ostream& operator<<(ostream& ostream, const text::OffsetDelta& delta);
ostream& operator<<(ostream& ostream, const text::Offset& offset);
}  // namespace std

#endif  // EVITA_TEXT_OFFSET_H_
