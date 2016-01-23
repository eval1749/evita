// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "evita/base/memory/zone.h"

#include "evita/base/memory/zone_allocated.h"

namespace evita {

namespace {
const size_t kAllocateUnit = 8;
const size_t kMinSegmentSize = 8 * 1024;

size_t RoundUp(size_t num, size_t unit) {
  return ((num + unit - 1) / unit) * unit;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Zone::Segment
//
class Zone::Segment final {
 public:
  Segment(size_t size, Segment* next);
  ~Segment();

  Segment* next() const { return next_; }

  void* Allocate(size_t size);

 private:
  Segment* const next_;
  size_t const max_offset_;
  size_t offset_;
  char* memory_;

  DISALLOW_COPY_AND_ASSIGN(Segment);
};

Zone::Segment::Segment(size_t size, Segment* next)
    : next_(next),
      max_offset_(RoundUp(size, kMinSegmentSize)),
      offset_(0u),
      memory_(new char[max_offset_]) {}

Zone::Segment::~Segment() {
  delete memory_;
}

void* Zone::Segment::Allocate(size_t size) {
  const auto allocate_size = RoundUp(size, kAllocateUnit);
  const auto next_offset = offset_ + allocate_size;
  if (next_offset > max_offset_)
    return nullptr;
  const auto result = &memory_[offset_];
  offset_ = next_offset;
  return result;
}

//////////////////////////////////////////////////////////////////////
//
// Zone
//
Zone::Zone(Zone&& other) : name_(other.name_), segment_(other.segment_) {
  other.segment_ = nullptr;
}

Zone::Zone(const char* name) : name_(name), segment_(new Segment(0, nullptr)) {}

Zone::~Zone() {
  auto segment = segment_;
  while (segment) {
    const auto next_segment = segment->next();
    delete segment;
    segment = next_segment;
  }
}

Zone& Zone::operator=(Zone&& other) {
  segment_ = other.segment_;
  other.segment_ = nullptr;
  return *this;
}

void* Zone::Allocate(size_t size) {
  for (;;) {
    if (const auto pointer = segment_->Allocate(size))
      return pointer;
    segment_ = new Segment(size, segment_);
  }
}

}  // namespace evita
