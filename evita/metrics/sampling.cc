// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/metrics/sampling.h"

#include <algorithm>

namespace metrics {

Sampling::Sampling(size_t max_samples)
    : max_samples_(max_samples), samples_(max_samples) {
  maximum_ = minimum_ = samples_.front();
}

Sampling::~Sampling() {}

void Sampling::AddSample(base::TimeDelta sample) {
  AddSample(static_cast<float>(sample.InMillisecondsF()));
}

void Sampling::AddSample(float sample) {
  auto const discard_sample = samples_.front();
  samples_.pop_front();
  samples_.push_back(sample);
  maximum_ = std::max(maximum_, sample);
  minimum_ = std::min(minimum_, sample);
  if (discard_sample != maximum_ && discard_sample != minimum_)
    return;
  maximum_ = minimum_ = samples_.front();
  for (auto const runner : samples_) {
    maximum_ = std::max(maximum_, runner);
    minimum_ = std::min(minimum_, runner);
  }
}

base::TimeTicks Sampling::NowTimeTicks() {
  return base::TimeTicks::Now();
}

}  // namespace metrics
