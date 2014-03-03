// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/metrics/time_scope.h"

namespace metrics {

Histogram::Histogram(const base::StringPiece& name) : name_(name) {
}

Histogram::~Histogram() {
}

void Histogram::AddSample(int sample) {
  auto const it = map_.find(sample);
  if (it != map_.end())
    ++it->second;
  else
    map_[sample] = 1;
}

HistogramSet::HistogramSet() {
}

HistogramSet::~HistogramSet() {
}

Histogram* HistogramSet::GetOrCreate(const base::StringPiece& name) {
  auto const it = map_.find(name);
  if (it != map_.end())
    return it->second;
  auto const histogram = new Histogram(name);
  map_[name] = histogram;
  return histogram;
}

TimeScope::TimeScope(const base::StringPiece& name)
    : name_(name), start_at_(base::Time::Now()) {
}

TimeScope::~TimeScope() {
  auto const end_at = base::Time::Now();
  auto const histogram = HistogramSet::instance()->GetOrCreate(name_);
  histogram->AddSample(static_cast<int>(
    (end_at - start_at_).InMicroseconds()));
}

}  // namespace metrics
